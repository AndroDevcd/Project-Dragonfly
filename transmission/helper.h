
#ifndef HELPER_H
#define HELPER_H

#inclue "../stdimports.h"

/**
 * DTP (Data Transfer Protocol)
 * 
 * Each packet will be filled to 
 * the max except for the first and last one
 * 
 * Packet #1            Packet #2      Last Packet
 * [ {size}, 0, 0, ...] [0, 0, 0, ...] [ {len}, 0, 0, ...]
 */
struct packet {
	uint8_t len;
	uint8_t* data;
} pdata;

bool isWholeNumber(double num);

#define GET_i32w(i) ((uint8_t)(i >> 24))
#define GET_i32x(i) ((uint8_t)((i >> 16) & 0xff))
#define GET_i32y(i) ((uint8_t)((i >> 8) & 0xff ))
#define GET_i32z(i) ((uint8_t)(i & 0xff))

#define SET_i32(w,x,y,z) (((uint8_t)z) | ((uint8_t)y << 8) | ((uint8_t)x << 16) | ((uint8_t)w << 24))

#endif //HELPER_H
