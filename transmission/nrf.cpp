#include <RF24/RF24.h>
#include "generated/native_mapping.h"
#include "helper.h"

using namespace std;

#define RX_ARRD_ID 0x33
#define TX_ARRD_ID 0x1A

#define TX_PACKET_HEADER_SIZE 5
#define TX_PACKET_FOOTER_SIZE 1
#define TX_PACKET_WIDTH 32

#define TRACKED_PACKETS 100
#define MAX_SIGNAL_STRENGTH 4

uint8_t address[] = { 0x0, 0xCE, 0x2B, 0xCE, 0x5F };

bool packetSuccess[TRACKED_PACKETS];
int8_t packetsSent = 0;
bool trackingFilled = false;

uint8_t response[TX_PACKET_WIDTH];

int TIMEOUT_US = 0;

RF24 radio(22, 0);

scope(transmission, nrf24,
	
	void setup(object $instance, var &trnsLvl, var &rate, var &mode, 
		var &delay, var &retryCount) {
		radio.begin();
		
		set_transmission_lvl($instance, trnsLvl);
		set_transmission_rate($instance, rate);
		set_retry_count($instance, delay, retryCount);
		switch_mode($instance, mode);
		pdata.data = (uint8_t*)malloc(sizeof(uint8_t) * TX_PACKET_WIDTH); 
	}

	void set_transmission_lvl(object $instance, var &level) {
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

	void set_transmission_rate(object $instance, var &level) {
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
	
	void switch_mode(object $instance, var &mode) {
		var curr_mode = get<var>($instance, "mode");
		curr_mode = mode;
		
		switch((long)mode.value()) {
			case 0: // RX
				radio.startListening();
				break;
			case 1: // TX
				radio.stopListening();
				break;
		}
	}
	
	void set_retry_count(object $instance, var &delay, var &count) {
		radio.setRetries((uint8_t)delay.value(), (uint8_t)count.value() % 15);
		TIMEOUT_US = (250 * (int)delay.value()) * (int)count.value();
	}
	
	void powerDown(object $instance) {
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
		var signalStrength = getLocalField<var>();
		
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
        unsigned long started_waiting_at = millis();
        bool timeout = false;
		while (!radio.available() && !timeout) {
			if (millis() - started_waiting_at > TIMEOUT_US) {
				timeout = true;
			}
		}
		
		if (timeout) {
			if(withTimeout) return false;
			else {
				usleep(TIMEOUT_US / 2);
				goto retry;
			}
		} else {
			radio.read(&response, sizeof(uint8_t) * response);
		}
		
		return true;
	}
	
	unsigned int readHeaderPacket() {
		unsigned int packets = 
			SET_i32(response[0x0], response[0x1],
				response[0x2], response[0x3]);
		pdata.len = response[0x4];
		int startPos = TX_PACKET_HEADER_SIZE;
		
		for(int i = 0; i < pdata.len; i++) {
			pdata.data[i] = response[startPos++];
		}
	}
	
	void readFooterPacket() {
		pdata.len = response[0x0];
		int startPos = TX_PACKET_FOOTER_SIZE;
		
		for(int i = 0; i < pdata.len; i++) {
			pdata.data[i] = response[startPos++];
		}
	}
	
	void readPacket(stringstream &data) {
		for(int i = 0; i < pdata.len; i++) {
			data << pdata.data[i];
		}
	}
	
	var read(object $instance, _int8_array &data_response) {
		stringstream data;
		waitforResponse(false);
		unsigned int packets = readHeaderPacket();
		var success = getLocalField<var>();
		
		success = true;
		readPacket(data);
		
		for(unsigned int i = 1; i < packets; i++) {
			if(!waitforResponse(true)) {
				data_response = NULL;
				success = false;
				break;
			}
			
			if((i + 1) >= packets) {
				readFooterPacket();
				readPacket(data);
			} else {
				for(int j = 0; j < TX_PACKET_WIDTH; j++) {
					data << response[j];
				}
			}
		}
		
		string str = data.str();
		data_response = str;
		return success;
	}
	
	var send(object $instance, _int8_array& data8) {
		string data = stringFrom(data8);
		var success = getLocalField<var>();
		success = true;
		
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
			
			for(int j = 0; j < pdata.len; j++) 
			{
				pdata.data[startPos++] = data[pos++];
			}
			
			bool ok = radio.write(&pdata.data, sizeof(uint8_t) * TX_PACKET_WIDTH);

			// track how many packets sent over the network to track cell signal
			packetSuccess[packetsSent++ % TRACKED_PACKETS] = ok;
			if(!trackingFilled && packetsSent >= TRACKED_PACKETS) { trackingFilled = true; }
			
            if (!ok) {
				success = false;
                printf("failed.\n");
                break;
            } else {
				printf("succeded.\n");
			}
		}
		
		return success;
	} 
)
