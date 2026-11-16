//
// THE DISTANCE SENSOR: the HC-SR04 (ultrasound)
// (the Sharp side: drone/src/hardware/pi_devices.sharp - class hc_sr04)
//
// One reading: a 10 microsecond pulse on the trigger pin; the echo pin then goes high for
// as long as the sound was under way. 58 microseconds are one centimetre (there and back).
//
// The wait for the echo is limited to 20 ms (about 3.4 m): no echo is an answer too (-1).
// The old version read the clock with micros() into a `long` and could wait forever once
// that wrapped around; the times here are differences of unsigned numbers, which stay
// right across a wrap.
//
#include "hardware.h"
#include "dragonfly_sni.h"

#define PULSE_TIMEOUT_US 20000u
#define US_PER_CM 58u

#ifndef DRAGONFLY_NO_HARDWARE

#include <wiringPi.h>

void dragonfly_distance_begin(sni_env* env, int32_t trigger, int32_t echo) {
    pinMode(trigger, OUTPUT);
    pinMode(echo, INPUT);
    digitalWrite(trigger, LOW);
}

// the distance in centimetres; -1 when no echo came back in time
int32_t dragonfly_distance_read_cm(sni_env* env, int32_t trigger, int32_t echo) {
    digitalWrite(trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger, LOW);

    unsigned int started = micros();
    while(digitalRead(echo) == LOW) {
        if(micros() - started > PULSE_TIMEOUT_US) return -1;
    }
    started = micros();
    while(digitalRead(echo) == HIGH) {
        if(micros() - started > PULSE_TIMEOUT_US) return -1;
    }
    return (int32_t) ((micros() - started) / US_PER_CM);
}

#else   // ---- no hardware under this build

void dragonfly_distance_begin(sni_env* env, int32_t trigger, int32_t echo) { }
int32_t dragonfly_distance_read_cm(sni_env* env, int32_t trigger, int32_t echo) { return -1; }

#endif
