#include "../../../../stdimports.h"
#include <wiringPi.h>

scope_begin(common_gpio)

    void setup() {
        wiringPiSetup();
        internal::return_call();
    }

    void pin_mode(var pin, var mode) {
        std::cout << " % " << pin << ": " << val << endl;
        pinMode((long)pin, (long)mode);
        internal::return_call();
    }

    void write_pin(var pin, var val) {
        std::cout << " >> " << pin << ": " << val << endl;
        digitalWrite((long)pin, (long)val ? HIGH : LOW);
        internal::return_call();
    }

    SharpObject read_pin(var pin) {
        var result = digitalRead((long)pin);

        auto returnData = create_new_primitive_wrapper("std#bool", result, std__bool::_bool2);
        internal::return_call();
        return returnData;
    }
scope_end()
