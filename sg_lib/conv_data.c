
#include <stdint.h>
#include <string.h>

//------------------------------------------------------------------------------
// конвертация символа в число  стр(hex) -> bin
// Возвращает результата операции = 0 - ОК, 
//------------------------------------------------------------------------------
int conv_char_to_byte(const uint8_t sin, uint8_t * bout)
{
    if (sin >= 'a' && sin <= 'f'){
        *bout = (uint8_t)sin - 0x57;
        return 0;
    } 
    
    if (sin >= 'A' && sin <= 'Z'){
        *bout = (uint8_t)sin - 0x37;
        return 0;
    } 

    if (sin >= '0' && sin <= '9'){
        *bout = (uint8_t)sin - 0x30;
        return 0;
    } 

    return 1;
}

//------------------------------------------------------------------------------
// конвертация 2 символов в число,  стр дестич(30) -> uint8_t(1e) 
// Возвращает результата операции = 0 - ОК, 
//------------------------------------------------------------------------------
int conv_2char_to_uint8(const uint8_t h_in, const uint8_t l_in, uint8_t *bout)
{
    uint8_t l = 0;
    uint8_t h = 0;
    int ret = 0;
    
    ret = conv_char_to_byte(h_in, &h);
    ret += conv_char_to_byte(l_in, &l);
    
    if (ret == 0){
        *bout = (h *10) + l;
        return 0;
    }
    
    return 1;
}

//------------------------------------------------------------------------------
// конвертация строки (до 8 символов) в число  стр(hex) -> bin 
// формат строки 0x[0-9a-f] 0xa0000001 и т.д.
// Возвращает результата операции = 0 - ERROR, >0 длинна строки в словах  
//
// const uint8_t * sin - входная строка, длинна строки должна быть не более 2+8 символом
// uint8_t n           - ожидаемое количество байт во входной строке для конвертации (без 0x)
// uint32_t * bout     - выходное слово 32 бита (4 байта)
//
//------------------------------------------------------------------------------
// not tested
#define CONV_STR_SIZE_MAX    (10) // 0x12345678
int conv_str_to_uint32(const uint8_t * sin, uint32_t * bout)
{
    int res = 0;
    uint32_t l,i;
    uint32_t conv = 0;
    uint8_t c;

    l = strlen( (const char*)sin );

    if (l == 0 || l <= 2 || l > CONV_STR_SIZE_MAX){
        return 0;
    }
    
    i = 0;
    c = *(sin + i);
    if (c != '0') return 0;

    i++;
    c = *(sin + i);
    if (c != 'x' && c != 'X') return 0;

    i++;
    while ( (c=*(sin + i)) != '\0'){
        res = conv_char_to_byte( c, &c);
        if (res) return 0; // error
        conv = conv << 4;
        conv = conv | c;
        i++;
    }   
    
    *bout = conv;
    return (i - 2) / 2; // количество байт в входной строке (-2 байта 0x)
}

//------------------------------------------------------------------------------
// конвертация строки (2 символа) в число  стр(hex) -> bin
// Возвращает результата операции = 0 - ОК, 
//------------------------------------------------------------------------------
//int conv_str_to_byte(const uint8_t * sin, uint8_t * bout)
//{
//    int res = 0;
//    uint8_t ch, cl;
    
//    if (strlen((char*)sin) == 0 || strlen((char*)sin) > 2 || strlen((char*)sin) == 1){
//        return 1;
//    }

//    ch = (uint8_t)sin[0];
//    cl = (uint8_t)sin[1];
    
//    res = conv_char_to_byte( ch, &ch);
//    if (res) return res; // error

//    res = conv_char_to_byte( cl, &cl);
//    if (res) return res; // error
    
//    *bout = (ch << 4) | cl;
//    return 0;
//}
