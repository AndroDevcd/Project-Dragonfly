#include <RF24/RF24.h>
#include "../../../generated/native_mapping.h"
#include "helper.h"

using namespace std;

#define RX_ARRD_ID 0x33
#define TX_ARRD_ID 0x1A

#define TX_PACKET_HEADER_SIZE 5
#define TX_PACKET_FOOTER_SIZE 1
#define TX_PACKET_WIDTH 32

#define TRACKED_PACKETS 100
#define MAX_SIGNAL_STRENGTH 4

#define DEFAULT_RF_CHANNEL 121

uint8_t address[] = { 0x0, 0xCE, 0x2B, 0xCE, 0x5F };

bool packetSuccess[TRACKED_PACKETS];
uint8_t packetsSent = 0;
bool trackingFilled = false;
packet pdata;

uint8_t response[TX_PACKET_WIDTH];

int TIMEOUT_US = 0;
int last_error = 0;

// extern wiringpi function
#ifdef __cplusplus
extern "C" {
#endif

extern unsigned int micros (void);

#ifdef __cplusplus
}
#endif


RF24 radio(22, 0);

scope_begin(common_network_driver)
	
	void setup(var &trnsLvl, var &rate, var &delay,
	        var &retryCount, var &isClient) {
		radio.begin();
		
		set_transmission_lvl(trnsLvl);
		set_transmission_rate(rate);
		set_retry_count(delay, retryCount);

		if((int)isClient.value()) {
            address[0] = 0x23;
            radio.openWritingPipe(address);
            address[0] = 0x1C;
            radio.openReadingPipe(1, address);
        } else {
            address[0] = 0x1C;
            radio.openWritingPipe(address);
            address[0] = 0x23;
            radio.openReadingPipe(1, address);
		}

        radio.stopListening();
		pdata.data = (uint8_t*)malloc(sizeof(uint8_t) * TX_PACKET_WIDTH); 
	}

	void set_transmission_lvl(var &level) {
		switch((long)level.value()) {
			case 0:
			case 1:
			case 2:
				radio.setPALevel((uint8_t)level.value());
				break;
			default:
				radio.setPALevel(0);
				break;
		}
	}

	void set_transmission_rate(var &level) {
		switch((long)level.value()) {
			case 0:
				radio.setDataRate(RF24_250KBPS);
				break;
			case 1:
				radio.setDataRate(RF24_1MBPS);
				break;
			case 2:
				radio.setDataRate(RF24_2MBPS);
				break;
			default:
				radio.setDataRate(RF24_250KBPS);
				break;
		}
	}
	
	void dump_details() {
		radio.printDetails();
	}

	void set_retry_count(var &delay, var &count) {
		radio.setRetries((uint8_t)delay.value(), (uint8_t)count.value() % 16);
		TIMEOUT_US = (250 * (int)delay.value()) * (int)count.value();
	}
	
	void power_down(object $instance) {
		radio.powerDown();
	}
	
	void addHeader(packet &p, uint32_t packet_count, uint8_t len) 
	{
		p.len = len;
		p.data[0x0] = GET_i32w(packet_count);
		p.data[0x1] = GET_i32x(packet_count);
		p.data[0x2] = GET_i32y(packet_count);
		p.data[0x3] = GET_i32z(packet_count);
		p.data[0x4] = len;
	} 
	
	void addFooter(packet &p, uint8_t len) 
	{
		p.len = len;
		p.data[0x0] = len;
	} 
	
	var get_signal_strength(object $instance) 
	{
		var signalStrength = createLocalField<var>();
		
		if(!trackingFilled) {
			signalStrength = MAX_SIGNAL_STRENGTH;
		} else {
			int8_t successfulPackets = 0;
			for(int i = 0; i < TRACKED_PACKETS; i++) 
			{
				if(packetSuccess[i]) successfulPackets++;
			}
			
			signalStrength = round((double)successfulPackets / 25);
		}
		
		return signalStrength;
	}
	
	bool waitforResponse(bool withTimeout) {
		// Wait here until we get a response, or timeout 
		retry:
        unsigned long started_waiting_at = micros();
        bool timeout = false;
		while (!radio.available() && !timeout) {
			if (micros() - started_waiting_at > TIMEOUT_US) {
				timeout = true;
			}
		}
		
		if (timeout) {
			if(withTimeout) return false;
			else {
                delayMicroseconds (TIMEOUT_US / 2);
				goto retry;
			}
		} else {
			radio.read(&response, TX_PACKET_WIDTH);
		}
		
		return true;
	}
	
	unsigned int readHeaderPacket(stringstream &data) {
		unsigned int packets = 
			SET_i32(response[0x0], response[0x1],
				response[0x2], response[0x3]);
        unsigned int len = response[0x4];
		int startPos = TX_PACKET_HEADER_SIZE;
		
		for(int i = 0; i < len; i++) {
			data << response[startPos++];
		}
		
		return packets;
	}
	
	void readFooterPacket(stringstream &data) {
		unsigned int len = response[0x0];
		int startPos = TX_PACKET_FOOTER_SIZE;
		
		for(int i = 0; i < len; i++) {
			data << response[startPos++];
		}
	}
	
	var_array read() {
		stringstream data;
        radio.startListening();
        var_array data_response(createLocalField<var_array>());
		
		if(!waitforResponse(true)) {
            last_error = 1;
			radio.stopListening();
            return data_response;
		}
		
		unsigned int packets = readHeaderPacket(data);
		var success = createLocalField<var>();
		
        last_error = 0;
		for(unsigned int i = 1; i < packets; i++) {
			if(!waitforResponse(true)) {
                last_error = 1;
				break;
			}
			
			if((i + 1) >= packets) {
				readFooterPacket(data);
			} else {
				for(int j = 0; j < TX_PACKET_WIDTH; j++) {
					data << response[j];
				}
			}
		}
		
		string str = data.str();
		data_response = str;
		
		radio.stopListening();
		return data_response;
	}
	
	void send(_int8_array& data8) {
		string data = stringFrom(data8);
        last_error = 0;

        radio.stopListening();
		unsigned int packetSize, startPos, dataConsumed, pos = 0;
		if(data.size() <= (TX_PACKET_WIDTH - TX_PACKET_HEADER_SIZE)) {
			packetSize = 1;
		} else {
			if(isWholeNumber(((double)data.size() + TX_PACKET_HEADER_SIZE + TX_PACKET_FOOTER_SIZE) / TX_PACKET_WIDTH)) {
				packetSize = (data.size() + TX_PACKET_HEADER_SIZE + TX_PACKET_FOOTER_SIZE) / TX_PACKET_WIDTH;
			} else 
				packetSize = ((data.size() + TX_PACKET_HEADER_SIZE + TX_PACKET_FOOTER_SIZE) / TX_PACKET_WIDTH) + 1;
		}
		
		for(unsigned int i = 0; i < packetSize; i++) {
			if(i == 0)
			{
				if(packetSize == 1) {
					dataConsumed = data.size();
					addHeader(pdata, packetSize, data.size());
				} else {
					dataConsumed = (TX_PACKET_WIDTH - TX_PACKET_HEADER_SIZE);
					addHeader(pdata, packetSize, (TX_PACKET_WIDTH - TX_PACKET_HEADER_SIZE));
				}
				
				startPos = TX_PACKET_HEADER_SIZE;
			} else if((i + 1) <= packetSize) {
				addFooter(pdata, data.size() - dataConsumed);
				startPos = TX_PACKET_FOOTER_SIZE;
			} else {
				startPos = 0;
				dataConsumed += TX_PACKET_WIDTH;
				pdata.len = TX_PACKET_WIDTH;
			}
			
			for(int j = startPos; j < pdata.len + startPos; j++) 
			{
				pdata.data[j] = data[pos++];
				cout << pdata.data[j];
			}
			
			cout << endl;
			bool ok = radio.write(pdata.data,TX_PACKET_WIDTH);

			// track how many packets sent over the network to track cell signal
			packetSuccess[packetsSent++ % TRACKED_PACKETS] = ok;
			if(!trackingFilled && packetsSent >= TRACKED_PACKETS) { trackingFilled = true; }
			
            if (!ok) {
                last_error = 1;
                return;
            }
		}
	}

	var get_last_error() {
        var result = createLocalField<var>();
        result = last_error;
        return result;
    }
scope_end()
