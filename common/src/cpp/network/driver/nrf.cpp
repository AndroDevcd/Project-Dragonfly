#include <RF24/RF24.h>
#include "helper.h"

using namespace std;
using namespace snb_api;

#define RX_ARRD_ID 0x33
#define TX_ARRD_ID 0x1A

#define TX_ROLE true
#define RX_ROLE false

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


RF24 radio(22, 0, 4000000);

scope_begin(common_network_driver)

    int64_t time_ms() {
        return std::chrono::duration_cast<std::chrono::microseconds>
                (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }

    void switch_role(bool tx) {
        if(tx) {
            cout << "tx mode\n";
            radio.stopListening();
            delayMicroseconds (55 * 1000);
        }
        else {
            cout << "rx mode\n";
            radio.startListening();
            delayMicroseconds (55 * 1000);
        }
    }

    void set_transmission_lvl(var level) {
        cout << "set_transmission_lvl()" << endl;
        switch((long)level) {
            case 0:
            case 1:
            case 2:
                radio.setPALevel((uint8_t)level);
                break;
            default:
                radio.setPALevel(0);
                break;
        }

        transmissionLvl = (int)level;
    }

    void set_transmission_rate(var level) {
        cout << "set_transmission_rate()" << endl;
        switch((long)level) {
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

        transmissionRate = (int)level;
    }

    void set_retry_count(var delay, var count) {
        cout << "set_retry_count()" << endl;
        radio.setRetries((uint8_t)delay, (uint8_t)count % 16);
        TIMEOUT_US = (250 * (int)delay) * (int)count;
        retryCount = (uint8_t)count % 16;
        retryDelay = (uint8_t)delay;
    }

	void setup(var trnsLvl, var rate, var delay,
	        var retryCount, var isClient) {
        cout << "setup(" << trnsLvl << ", " << rate << ", " << delay
            << ", " << retryCount << ", " << isClient << ")" << endl;

        if (!radio.begin()) {
            cout << "radio hardware not responding!" << endl;
        }
		set_transmission_lvl(trnsLvl);
		set_transmission_rate(rate);
		set_retry_count(delay, retryCount);

		if((int)isClient) {
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

        radio.setChannel(70);
        switch_role(TX_ROLE);
		pdata.data = (uint8_t*)malloc(sizeof(uint8_t) * TX_PACKET_WIDTH);
        for(int j = 0; j < TX_PACKET_WIDTH; j++)
        {
            pdata.data[j] = 1; // fill with random data
        }
	}
	
	void dump_details() {
        cout << "dump_details()" << endl;
		radio.printDetails();
	}
	
	void power_down(SharpObject instance) {
        cout << "power_down()" << endl;
		radio.powerDown();
	}

	SharpObject get_network_quality() {
        cout << "get_network_quality()" << endl;
        LocalVariable quality = create_local_variable();
        internal::new_array(128, TYPE_VAR);
        check_for_err();

        use_var(quality,
           internal::assign_object(quality.obj, internal::pop_object());
        )

        auto raw = internal::get_raw_number_data(quality.obj);
        uint8_t channel = radio.getChannel();

        for(int i = 0; i < 25; i++) {
            for(int j = 0; j < 128; j++) {
                radio.setChannel(j);

                switch_role(RX_ROLE);
                delayMicroseconds (100);
                switch_role(TX_ROLE);

                if( radio.testCarrier() )
                    raw[j]++;
            }
        }

        radio.setChannel(channel);
        return quality.obj;
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
	
	SharpObject get_signal_strength()
	{
        cout << "get_signal_strength()" << endl;
		var signalStrength;
		
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

        return create_new_primitive_wrapper("std#int", signalStrength, std__int::_int2);
	}
	
	bool waitforResponse(bool timeout) {
		// Wait here until we get a response, or timeout
        unsigned long long started_waiting_at = time_ms();
		while (!radio.available()) {
			if ((time_ms() - started_waiting_at) > TIMEOUT_US) {
				if(timeout) return false;
                else {
//                    radio.flush_rx();
                    delayMicroseconds (1000);
                    started_waiting_at = time_ms();
                }
			}
		}

        cout << "got data" << endl;
        radio.read(&response, TX_PACKET_WIDTH);
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
	
	SharpObject process_packets() {
		stringstream data;
        LocalVariable data_response = create_local_variable();
		unsigned int packets = readHeaderPacket(data);

        cout << "processing " << packets << " packets" << endl;
        last_error = 0;
		for(unsigned int i = 1; i < packets; i++) {
            cout << "packet #" << i << endl;
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
        internal::new_array(str.size(), TYPE_INT8);
        check_for_err();

        use_var(data_response,
            internal::assign_object(data_response.obj, internal::pop_object());
        )
        auto raw = internal::get_raw_number_data(data_response.obj);

        for(long i = 0; i < str.size(); i++) {
            raw[i] = str[i];
        }

        switch_role(TX_ROLE);
		return data_response.obj;
	}
	
	void jam(var wifiChannel) {
        cout << "jam()" << endl;
		int channel = (int)wifiChannel % 20;
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
                switch_role(RX_ROLE);
				if (radio.available()) {
					
				   radio.setPALevel(transmissionLvl);
	               radio.setRetries(retryDelay, retryCount);
				   return;
				} else {
                    switch_role(TX_ROLE);
				}
			}
		}
	}
	
	SharpObject read() {
        cout << "read()" << endl;
        switch_role(RX_ROLE);

		if(!waitforResponse(true)) {
            last_error = 1;
            switch_role(TX_ROLE);

            LocalVariable data_response = create_local_variable();
            internal::assign_object(data_response.obj, nullptr);
            return data_response.obj;
		}

        return process_packets();
	}
	
	SharpObject listen() {
        cout << "listen()" << endl;
        switch_role(RX_ROLE);
		
		waitforResponse(false);
		return process_packets();
	}
	
	void send(SharpObject data8) {
        cout << "send()" << endl;
		string data;
        string_from(data, data8);
        last_error = 0;

        switch_role(TX_ROLE);
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
                cout << "send failed!" << endl;
                radio.printDetails();
                last_error = 1;
                return;
            }
		}
	}

	SharpObject get_last_error() {
        cout << "get_last_error()" << endl;
        return create_new_primitive_wrapper("std#int", last_error, std__int::_int2);;
    }
scope_end()
