#include <stdint.h>
#include "printf_hal.h"
//#include <string.h>

#define DUMP_STRING_LEN (16)

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// print array TO hex
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void dump_1(const uint8_t *buf_in, const uint32_t len)
{
    uint32_t n,i;
    uint8_t t;

    printf_d("\n");
    n=0;

    for (i=0; i<len; i++){
        t = *(buf_in + i);
        printf_d(" %x",t);
        n++;
        if ( n == DUMP_STRING_LEN ){
            n=0;
            printf_d("\n");
            printf_d("addres= %lx: ",i+1);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// print array TO hex
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void dump_2(const uint8_t *buf_in, uint32_t len)
{
    uint32_t n,i,j;
    uint8_t c;
    uint8_t s[ DUMP_STRING_LEN ];

    printf_d("\n");
    n=0;

    for (i=0; i<len; i++){
        c = *(buf_in + i);
        printf_d(" %02X", c);

        if (c<0x20)
            s[n] = '.';
        else
            s[n] = c;

        n++;
        if (n == DUMP_STRING_LEN){
            printf_d(" | ");
            for (j=0;j<DUMP_STRING_LEN;j++) 
                printf_d("%c",s[j]);
                n=0;
                printf_d("\n");
                //printf_d("addres= %04lx: ",i+1);
        }
    }

    if (n > 0 && n < DUMP_STRING_LEN){
        for (j=0;j<DUMP_STRING_LEN-n;j++) 
            printf_d("   ");
        printf_d(" | ");
        for (j=0;j<n;j++) 
            printf_d("%c",s[j]);
    }

    printf_d("\n");
}
