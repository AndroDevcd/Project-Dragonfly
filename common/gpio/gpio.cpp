#include "../../stdimports.h"
#include <wiringPi.h>

scope_begin(common_gpio)

    void setup() {
        wiringPiSetup();
    }

    void pin_mode(var &pin, var &mode) {
        pinMode((long)pin.value(), (long)mode.value());
    }

    void write_pin(var &pin, var& val) {
        digitalWrite((long)pin.value(), (long)val.value() ? HIGH : LOW);
    }

    var read_pin(var &pin) {
        var result = createLocalField<var>();
        result = digitalRead((long)pin.value());
        return result;
    }
scope_end()
