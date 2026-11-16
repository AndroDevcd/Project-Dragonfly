//
// THE PINS: wiringPi
// (the Sharp side: link/src/hardware/pi_gpio.sharp - class pi_gpio)
//
// Pins are numbered the way wiringPi numbers them, as in the project's wiring.
//
#include "hardware.h"
#include "dragonfly_sni.h"

#ifndef DRAGONFLY_NO_HARDWARE

#include <wiringPi.h>

// 1 when the pins can be used
int32_t dragonfly_gpio_begin(sni_env* env) {
    static bool started = false;
    if(!started) {
        if(wiringPiSetup() == -1) return 0;
        started = true;
    }
    return 1;
}

void dragonfly_gpio_mode(sni_env* env, int32_t pin, int32_t output) {
    pinMode(pin, output ? OUTPUT : INPUT);
}

void dragonfly_gpio_write(sni_env* env, int32_t pin, int32_t high) {
    digitalWrite(pin, high ? HIGH : LOW);
}

int32_t dragonfly_gpio_read(sni_env* env, int32_t pin) {
    return digitalRead(pin) == HIGH ? 1 : 0;
}

#else   // ---- no hardware under this build

int32_t dragonfly_gpio_begin(sni_env* env) { return 0; }
void dragonfly_gpio_mode(sni_env* env, int32_t pin, int32_t output) { }
void dragonfly_gpio_write(sni_env* env, int32_t pin, int32_t high) { }
int32_t dragonfly_gpio_read(sni_env* env, int32_t pin) { return 0; }

#endif
