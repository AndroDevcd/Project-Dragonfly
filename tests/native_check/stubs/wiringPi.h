// A STAND-IN for wiringPi.h: only what the drivers use (see stubs/RF24/RF24.h).
#ifndef STUB_WIRINGPI_H
#define STUB_WIRINGPI_H
#define INPUT 0
#define OUTPUT 1
#define LOW 0
#define HIGH 1
#ifdef __cplusplus
extern "C" {
#endif
int wiringPiSetup(void);
void pinMode(int pin, int mode);
void digitalWrite(int pin, int value);
int digitalRead(int pin);
void delayMicroseconds(unsigned int howLong);
void delay(unsigned int howLong);
unsigned int micros(void);
#ifdef __cplusplus
}
#endif
#endif
