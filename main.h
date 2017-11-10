#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include "stm32f10x_bkp.h"
#include "ff.h"
#include "queue_buf.h"
#include "led_set_status.h"

#define printf_d xprintf

#define QUEUE_CONSOL_RX_SIZE	(8) // длинна входного буфера консоли

typedef enum {DEV_POWER_OFF = 0, DEV_POWER_ON} device_state_e; // состояние устройства
typedef enum {REC_OFF=0, REC_ON} record_e;      // состояние запись файла ВКЛ-ВЫКЛ
typedef enum {SD_EMPTY=0, SD_LOAD } sd_slot_e;  // состояние наличия карты в слоте
typedef enum {MOUNT_NO=0, MOUNT_OK, MOUNT_ERROR } mount_status_e;             // состояние монтирывания файловой системы
typedef enum {W_STOP=0, W_CREATEFILE, W_WRDATA, W_CLOSEFILE, W_ERROR} write_automat_e; // состояния автомата записи данных

// флаги ошибок при работе устройства
#define ER_NO          (0x00000000UL)   // нет ощибок -  исходное состояние
#define ER_SD_PRES     (0x00000001UL)   // нет карты в слоте
#define ER_INIT        (0x00000002UL)   // ошибка инициальзация карты
#define ER_MOUNT       (0x00000004UL)   // ошибка монтирывания диска
#define ER_CR_FILE     (0x00000008UL)   // ошибка создания файла
#define ER_WR_DATA     (0x00000010UL)   // ошибка записи данных в файл
#define ER_WR_DATAM    (0x00000020UL)   // записано было меньше данных чем требовалось
#define ER_FREE_SPACE  (0x00000040UL)   // нет места на карте
#define ER_RX_BUF_FULL (0x00000080UL)   // приемные буфер переполнен
#define ER_UART_SPEED  (0x00000100UL)   // неверная скорость работы uart порта

#define DATA_PATH "0:/sd_log" // путь по умолчанию для записи файлов на диск
#define MKDIR_DATA_DIR "sd_log" // путь по умолчанию для создания DIR для записи файлов на диск

// структура текущие значения всех состояний устройства
typedef struct{
    device_state_e dev_state;
    sd_slot_e sd_slot;
    mount_status_e mount_status;
    record_e rec;
    uint32_t speed;
    led_state_e led_rec;
    write_automat_e wr_data_st;
    led_state_e led_st;        // состояние на светодиодах
    uint32_t error_dev_st;     // Флаги ошибок, при работе устройства
    FRESULT fres;              // результат операции IO, номер ошибки, для детального анализа.
} device_config_st;

// структура файлы данных ------------------------------------------------------
#define BUF_W_SIZE     (512)   // размер промежуточного буфера записи данных в файл
#define FILE_N         (2)     // количество файлов на запись
#define FILE_NAME_SIZE (32)    // длинна имени файла
#define RX_BUF_SIZE    (4096)

typedef struct {
    uint8_t file_name[ FILE_N ][ FILE_NAME_SIZE ];   // строка, имя файла
    FIL fil[ FILE_N ];                     // fatfs File object structure
    queue_buf_t q_buf[ FILE_N ];           // кольцевой буфер приема данных
    uint32_t wr_data_size[ FILE_N ];       // количество данных записанных в файл
    uint8_t buf_w[ BUF_W_SIZE ];           // промежуточный буфер для записи данных в файл COPY(q_data)->buf_w[ BUF_W_SIZE ]-> FILE to disk
} tfile_st;

#define BUF_DUMP_SIZE   (512)
typedef struct {
    uint8_t file_name[ FILE_NAME_SIZE ];   // строка, имя файла
    FIL fil;                               // fatfs File object structure
    uint32_t lseek;                        // позиция чтения в файле
    uint32_t fsize;                        // размер файла
    uint8_t buf_r[ BUF_DUMP_SIZE ];        // промежуточный буфер для записи данных в файл COPY(q_data)->buf_w[ BUF_W_SIZE ]-> FILE to disk
} tfile_dump_st;


// Backup память для сохранения значений
#define ADR_CFG         BKP_DR1 // 0xA5A5 - значение конфигурации RTC состоялась
#define ADR_SPEED_L     BKP_DR2 //- мл. uint32_t скорость работы портов
#define ADR_SPEED_H     BKP_DR3 //- ст. uint32_t скорость работы портов

#define FILE_DATA_SIZE_MAX  (10*1024*1024) // максимальный размер файла для записи данных


#endif /* MAIN_H_ */
