#ifndef _CONSOLE_CFG_H_
#define _CONSOLE_CFG_H_

#define CONSOLE_IN_STR_MAX_SIZE    (64)  // maximalniy razmer vhodnoy stroki
#define CMD_MAX_SIZE               (16)  // dlina 1 komandi 
#define CONSOLE_ARGC_SIZE          (8)   // kolichestvo parametrov v komande
#define CONSOLE_HIST_STR_SIZE      (256) // dlinna stoki istorii, dla sohrneniya vvedenih strok, Dlinna kratna ^2 !!! t.k. buf - RING

#define PRINT_PROMT                "===>" // promt-> priglashenie cmd stroki
#define PRINT_CMD_NOT_FOUND        "\r\n ERROR: command not found.\r\n" // cmd ne naydena

#define CONSOLE_ALLFUNC_EN         (1)   // vkluchenie rashirennogo funchionala: history buffer + edit string
#define CONSOLE_DEBUG              (0)   // enbale sdio.h printf DEBUG output

#endif
