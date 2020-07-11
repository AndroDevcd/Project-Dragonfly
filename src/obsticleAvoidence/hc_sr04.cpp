#include "../../stdimports.h"

#define PULSE_TIMEOUT 45000000
#define CM_PER_USEC 0.034

namespace main_global {

      unsigned long timeInNano() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>
                (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
      }


    var get_distance(var &trg, var& echo) {
        digitalWrite((long)trg.value(), LOW);
        delayMicroseconds(2);
        digitalWrite((long)trg.value(), HIGH);
        delayMicroseconds(10);
        digitalWrite((long)trg.value(), LOW);
	/*
        while(digitalRead((long)echo.value()) == LOW);

        //Wait for echo end
        long startTime = micros();
        while(digitalRead((long)echo.value()) == HIGH);
        long travelTime = micros() - startTime;

        //Get distance in cm
        int dst = travelTime / 58;


        cout << "total tim " << travelTime << "us" << endl;
        var distance = createLocalField<var>();
        distance = dst;
        return distance;*/
	      return trg;
    }
}
