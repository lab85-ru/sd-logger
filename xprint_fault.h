/*
 * xprint.h
 *
 *  Created on: 26.04.2012
 *      Author: SGD
 */

#ifndef XPRINT_H_
#define XPRINT_H_

#include <stdint.h>

void bin2ascii(char in, char *out_high, char *out_low);
char ascii2bin(char in);
void xtx_char(uint8_t ch);
void xprint_str(char * s);
void xprint_hexbyte( uint8_t b);
void xprint_hexuint32( uint32_t x);

#endif /* XPRINT_H_ */
