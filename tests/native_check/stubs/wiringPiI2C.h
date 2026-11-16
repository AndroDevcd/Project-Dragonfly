// A STAND-IN for wiringPiI2C.h: only what vendor/ssd1306_i2c.cpp uses.
#ifndef STUB_WIRINGPII2C_H
#define STUB_WIRINGPII2C_H
#ifdef __cplusplus
extern "C" {
#endif
int wiringPiI2CSetup(const int devId);
int wiringPiI2CWriteReg8(int fd, int reg, int data);
int wiringPiI2CWrite(int fd, int data);
int wiringPiI2CReadReg8(int fd, int reg);
#ifdef __cplusplus
}
#endif
#endif
