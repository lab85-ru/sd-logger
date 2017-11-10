#include <stdint.h>
#include "stm32f10x.h"
#include "hardware.h"

void bin2ascii(char in, char *out_high, char *out_low)
{
	uint8_t th,tl;
	
	th = ((uint8_t)in & 0xf0) >> 4;

	if ( th >= 0 && th <= 9 )
		th = th + 0x30;
	else
		th = th + 0x37;

	*out_high = th;

	tl = (in & 0x0f);
	
	if ( tl >= 0 && tl <= 9 )
		tl = tl + 0x30;
	else
		tl = tl + 0x37;

	*out_low = tl;
}

char ascii2bin(char in)
{
	if (in>='0' && in<='9')
		return in - 0x30;
	else
		return in - 0x37;
}


void xtx_char(uint8_t ch)
{
  while (USART_GetFlagStatus(H_COM1, USART_FLAG_TC) == RESET);
  USART_SendData(H_COM1, (uint8_t) ch);
}

void xprint_str(char * s)
{
	if (s == 0) return;
	do{
		xtx_char(*s);
		s++;
	}while (*s != '\0');
}

void xprint_hexbyte( uint8_t b)
{
	char h,l;

	bin2ascii( b, &h, &l);
        xtx_char(h);
	xtx_char(l);
}

void xprint_hexuint32( uint32_t x)
{
	char h,l;

	bin2ascii( x>>24, &h, &l);
        xtx_char(h);
	xtx_char(l);

	bin2ascii( x>>16, &h, &l);
        xtx_char(h);
	xtx_char(l);

	bin2ascii( x>>8, &h, &l);
        xtx_char(h);
	xtx_char(l);

	bin2ascii( x, &h, &l);
        xtx_char(h);
	xtx_char(l);

}

