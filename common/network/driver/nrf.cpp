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

// device settings
int transmissionLvl = 0;
int transmissionRate = 0;
int retryDelay = 0;
int retryCount = 0;

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
            address[0] = RX_ARRD_ID;
            radio.openWritingPipe(address);
            address[0] = TX_ARRD_ID;
            radio.openReadingPipe(1, address);
        } else {
            address[0] = TX_ARRD_ID;
            radio.openWritingPipe(address);
            address[0] = RX_ARRD_ID;
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
		
		transmissionLvl = (int)level.value();
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
		
		transmissionRate = (int)level.value();
	}
	
	void dump_details() {
		radio.printDetails();
	}

	void set_retry_count(var &delay, var &count) {
		radio.setRetries((uint8_t)delay.value(), (uint8_t)count.value() % 16);
		TIMEOUT_US = (250 * (int)delay.value()) * (int)count.value();
		retryCount = (uint8_t)count.value() % 16;
		retryDelay = (uint8_t)delay.value();
	}
	
	void power_down(object $instance) {
		radio.powerDown();
	}

	var_array get_network_quality() {
        var_array quality = createLocalField<var_array>();
        createVarArray(quality, 128);
        uint8_t channel = radio.getChannel();

        for(int i = 0; i < 25; i++) {
            for(int j = 0; j < 128; j++) {
                radio.setChannel(j);

                radio.startListening();
                delayMicroseconds (100);
                radio.stopListening();

                if( radio.testCarrier() )
                    quality[j]++;
            }
        }

        radio.setChannel(channel);
        return quality;
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
	
	var get_signal_strength() 
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
				timeout = false;
                delayMicroseconds (1000);
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
	
	var_array process_packets() {
		
		stringstream data;
        var_array data_response(createLocalField<var_array>());
		unsigned int packets = readHeaderPacket(data);
		
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
	
	void jam(var &wifiChannel) {
		int channel = (int)wifiChannel.value() % 20;
		long past = micros();
		int oldChannel = radio.getChannel();
        const char *text = "{fmju=j\"!@#$%^&\",hygt3454fr7";
        
        radio.setPALevel(RF24_PA_MAX);
	    radio.setRetries(1, 1);
		
		for(;;) {
			for(int i = ((channel *5) + 1); i < ((channel *5) + 25); i++) {
				radio.setChannel(i);
				radio.write(text, sizeof(char) * 30);
			}
			
			if(((micros() - past) / 1000) > 10000) {
				past = micros();
				
		        radio.setChannel(oldChannel);
				radio.startListening();
				if (radio.available()) {
					
				   radio.setPALevel(transmissionLvl);
	               radio.setRetries(retryDelay, retryCount);
				   return;
				} else {
					radio.stopListening();
				}
			}
		}
	}
	
	var_array read() {
        radio.startListening();
		
		if(!waitforResponse(true)) {
            last_error = 1;
			radio.stopListening();
			
            var_array data_response(createLocalField<var_array>());
            return data_response;
		}
		
		return process_packets();
	}
	
	var_array listen() {
        radio.startListening();
		
		waitforResponse(false);
		return process_packets();
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
			} else if((i + 1) >= packetSize) {
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
			}

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
