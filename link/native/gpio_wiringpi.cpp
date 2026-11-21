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
    // An input is held LOW by the Pi's own resistor (about 50 kOhm) while nothing drives it: a
    // button wired from 3.3 V to the pin then reads 0 when let go and 1 when pressed, with no
    // resistor of its own. Without it the pin "floats" and reads whatever it picks up.
    if(!output) pullUpDnControl(pin, PUD_DOWN);
}

void dragonfly_gpio_write(sni_env* env, int32_t pin, int32_t high) {
    digitalWrite(pin, high ? HIGH : LOW);
}

int32_t dragonfly_gpio_read(sni_env* env, int32_t pin) {
    return digitalRead(pin) == HIGH ? 1 : 0;
}

// An input with the Pi's PULL-UP: it reads HIGH (1) while nothing drives it, and a switch wired
// from the pin to GND reads LOW (0) when it is closed. The sport-mode switch is wired that way
// (to ground), so it needs this rather than the pull-DOWN that gpio_mode gives an ordinary input.
void dragonfly_gpio_input_pull_up(sni_env* env, int32_t pin) {
    pinMode(pin, INPUT);
    pullUpDnControl(pin, PUD_UP);
}

#else   // ---- no hardware under this build

int32_t dragonfly_gpio_begin(sni_env* env) { return 0; }
void dragonfly_gpio_mode(sni_env* env, int32_t pin, int32_t output) { }
void dragonfly_gpio_write(sni_env* env, int32_t pin, int32_t high) { }
int32_t dragonfly_gpio_read(sni_env* env, int32_t pin) { return 0; }
void dragonfly_gpio_input_pull_up(sni_env* env, int32_t pin) { }

#endif
