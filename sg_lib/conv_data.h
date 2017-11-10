#ifndef _SGLIB_CONVERT_H_
#define _SGLIB_CONVERT_H_

#include <stdint.h>

int conv_2char_to_uint8(const uint8_t h_in, const uint8_t l_in, uint8_t *bout);
int conv_char_to_byte(const uint8_t sin, uint8_t * bout);
int conv_str_to_uint32(const uint8_t * sin, uint32_t * bout);

#endif