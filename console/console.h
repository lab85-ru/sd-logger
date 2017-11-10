#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <stdint.h>
#include "console_cfg.h"

#define CODE_ESC   (0x1b)
#define CODE_CR    (0xd)
#define CODE_LF    (0xa)

#define CODE_BS   (0x08) // backspace
#define CODE_BS2  (0x7f) // backspace linux terminal


#define ESC_CUR_UP      "\033[A"
#define ESC_CUR_DOWN    "\033[B"
#define ESC_CUR_LEFT    "\033[D"
#define ESC_CUR_RIGTH   "\033[C"

#define ESC_CLR_STR     "\033[2K"


#define ESC_STR_MAX_SIZE (6) // dlinna stroki sborki ESC POSLEDOVATELNOSTI

typedef enum {MODE_STR = 0, MODE_ESC = 1} console_mode_e; // reshim raboti consoli, vvod stroki ili vvod ESC coda

// global structure -----------------------------------------------------
// dla kashdoy otdelnoy consoli svoya structura !
typedef struct
{
    uint8_t argc;                              // kolichestvo argumentov
    uint8_t* argv[ CONSOLE_ARGC_SIZE ];        // ukazatel na argumenti
    uint8_t in_str[ CONSOLE_IN_STR_MAX_SIZE ]; // buffer dla hraneniya vhodnoy stroki
    uint16_t in_str_size;                      // kolichestvo simvolov vo shodnoy stroke
    void (*call_print)(const char *s);         // CALL prog -> print to console
    console_mode_e mode;                       // reshim raboti consoli, vvod stroki ili vvod ESC coda
    uint8_t esc_str[ ESC_STR_MAX_SIZE + 1 ];   // stroka dla sborki ESC posledovatelnosti  + 1 dla '\0'
    uint8_t esc_str_size;                      // kolichestvo simvolov v stroke
    uint8_t hist[ CONSOLE_HIST_STR_SIZE ];     // istoriya vvedenih komand
    uint32_t hist_in;                          // hvost - ring,  istoriya vvedenih komand
    uint32_t hist_p;                           // ukazatel na stroku, kotoraya budet vivoditcha v in_str
    uint32_t cur_pos;                          // cursor position, = 0 cursor v konche stroki
} console_var_type;

typedef enum {VIEW_PREV = 0, VIEW_NEXT = 1} view_hist_t;  // napravlenie perehoda v buffere istorii

typedef struct console_cmd_st
{
    uint8_t cmd[ CMD_MAX_SIZE ];
    void(*call)(const console_var_type *cn);

} console_cmd_type;

typedef struct console_esc_code_st
{
    uint8_t esc[ CMD_MAX_SIZE ];
    void(*call)(console_var_type *cn);

} console_esc_code_type;

void console (console_var_type * cn, uint8_t c);
void console_init(console_var_type * cn);

#endif
