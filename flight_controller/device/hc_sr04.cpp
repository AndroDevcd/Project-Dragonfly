#include "../../stdimports.h"
#include <wiringPi.h>

#define PULSE_TIMEOUT 20000
#define CM_TIME_CONVERSION 58

scope_begin(device)

    var get_dst_cm(var &trg, var& echo) {
        digitalWrite((long)trg.value(), HIGH);
        delayMicroseconds(10);
        digitalWrite((long)trg.value(), LOW);
        
        long startTime = micros();
        while(digitalRead((long)echo.value()) == LOW && TIME_SINCE(startTime) < PULSE_TIMEOUT);

        startTime = micros();
        while(digitalRead((long)echo.value()) == HIGH && TIME_SINCE(startTime) < PULSE_TIMEOUT);
        long travelTime = TIME_SINCE(startTime);

        //Get distance in cm
        int dst = travelTime / CM_TIME_CONVERSION;
        var distance = createLocalField<var>();
        distance = dst;
        return distance;
    }
scope_end()
