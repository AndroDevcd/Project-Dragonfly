//
// IS THERE REAL HARDWARE UNDER THIS BUILD?
//
// The C++ in this project drives chips that hang off a Raspberry Pi: the nRF24L01
// radio (the RF24 library), the pins (wiringPi), the OLED display, the distance
// sensor. Those libraries exist on the Pi only.
//
// So every driver file has two halves:
//
//      #ifndef DRAGONFLY_NO_HARDWARE       the real thing
//      #else                               the same functions, saying "no hardware here"
//
// DRAGONFLY_NO_HARDWARE is set by itself on anything that is not an ARM board, and
// can be forced with -DDRAGONFLY_NO_HARDWARE. That is what lets `sharpc -P controller`
// build on a PC: the C++ is compiled and checked, the library exists, and a program
// that asks it for a device is told there is none (and uses the simulated ones).
//
#ifndef DRAGONFLY_HARDWARE_H
#define DRAGONFLY_HARDWARE_H

#if !defined(DRAGONFLY_NO_HARDWARE) && !defined(__arm__) && !defined(__aarch64__)
#define DRAGONFLY_NO_HARDWARE
#endif

#endif //DRAGONFLY_HARDWARE_H
