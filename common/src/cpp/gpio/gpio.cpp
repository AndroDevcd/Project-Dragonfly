#include "../../../../stdimports.h"
#include <wiringPi.h>

scope_begin(common_gpio)

    void setup() {
        wiringPiSetup();
    }

    void pin_mode(var pin, var mode) {
        pinMode((long)pin, (long)mode);
    }

    void write_pin(var pin, var val) {
        digitalWrite((long)pin, (long)val ? HIGH : LOW);
    }

    SharpObject read_pin(var pin) {
        var result = digitalRead((long)pin);
        return create_new_primitive_wrapper("std#bool", result, std__bool::_bool2);;
    }
scope_end()
