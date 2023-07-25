
#ifndef HELPER_H
#define HELPER_H

#include "../../../../../stdimports.h"

/**
 * DTP (Data Transmission Protocol)
 * 
 * Each packet will be filled to 
 * the max except for the first and last one
 *
 * All packets have a max size of 32 bytes
 * 
 * Packet #1              Packet #2      Packet #N
 * [ {header}, 0, 0, ...] [0, 0, 0, ...] [ {footer}, 0, 0, ...]
 */
struct packet {
	uint8_t len;
	uint8_t* data;
};

bool isWholeNumber(double num);

#define GET_i32w(i) ((uint8_t)(i >> 24))
#define GET_i32x(i) ((uint8_t)((i >> 16) & 0xff))
#define GET_i32y(i) ((uint8_t)((i >> 8) & 0xff ))
#define GET_i32z(i) ((uint8_t)(i & 0xff))

#define SET_i32(w,x,y,z) (((uint8_t)z) | ((uint8_t)y << 8) | ((uint8_t)x << 16) | ((uint8_t)w << 24))

#endif //HELPER_H
