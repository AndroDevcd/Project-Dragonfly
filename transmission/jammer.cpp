
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <RF24/RF24.h>

using namespace std;

RF24 radio(22, 0);


int main(int argc, char** argv)
{
    // Setup and configure rf radio
    radio.begin();
    
    radio.setPALevel(RF24_PA_MAX);
    radio.setDataRate(RF24_2MBPS);
    
    // Dump the configuration of the rf unit for debugging
    radio.printDetails();
    
    radio.stopListening();
    
    int channel = 4, count=0;
	const char *garbage  = "kjrfnkjrg87@#$%^&*()}{>L<#$%G^&J*(IOP";
	
    cout << "jamming..." << endl;
    for(;;) {
		for(int i = ((channel * 5) + 1); i < ((channel * 5) + 23); i++) {
			radio.setChannel(i);
			radio.write(&garbage, strlen(garbage), 1);
		}
		
	}
}
