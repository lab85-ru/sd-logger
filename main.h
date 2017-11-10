#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include "stm32f10x_bkp.h"
#include "ff.h"
#include "queue_buf.h"
#include "led_set_status.h"

#define printf_d xprintf

#define QUEUE_CONSOL_RX_SIZE	(8) // ������ �������� ������ �������

typedef enum {DEV_POWER_OFF = 0, DEV_POWER_ON} device_state_e; // ��������� ����������
typedef enum {REC_OFF=0, REC_ON} record_e;      // ��������� ������ ����� ���-����
typedef enum {SD_EMPTY=0, SD_LOAD } sd_slot_e;  // ��������� ������� ����� � �����
typedef enum {MOUNT_NO=0, MOUNT_OK, MOUNT_ERROR } mount_status_e;             // ��������� ������������ �������� �������
typedef enum {W_STOP=0, W_CREATEFILE, W_WRDATA, W_CLOSEFILE, W_ERROR} write_automat_e; // ��������� �������� ������ ������

// ����� ������ ��� ������ ����������
#define ER_NO          (0x00000000UL)   // ��� ������ -  �������� ���������
#define ER_SD_PRES     (0x00000001UL)   // ��� ����� � �����
#define ER_INIT        (0x00000002UL)   // ������ ������������� �����
#define ER_MOUNT       (0x00000004UL)   // ������ ������������ �����
#define ER_CR_FILE     (0x00000008UL)   // ������ �������� �����
#define ER_WR_DATA     (0x00000010UL)   // ������ ������ ������ � ����
#define ER_WR_DATAM    (0x00000020UL)   // �������� ���� ������ ������ ��� �����������
#define ER_FREE_SPACE  (0x00000040UL)   // ��� ����� �� �����
#define ER_RX_BUF_FULL (0x00000080UL)   // �������� ����� ����������
#define ER_UART_SPEED  (0x00000100UL)   // �������� �������� ������ uart �����

#define DATA_PATH "0:/sd_log" // ���� �� ��������� ��� ������ ������ �� ����
#define MKDIR_DATA_DIR "sd_log" // ���� �� ��������� ��� �������� DIR ��� ������ ������ �� ����

// ��������� ������� �������� ���� ��������� ����������
typedef struct{
    device_state_e dev_state;
    sd_slot_e sd_slot;
    mount_status_e mount_status;
    record_e rec;
    uint32_t speed;
    led_state_e led_rec;
    write_automat_e wr_data_st;
    led_state_e led_st;        // ��������� �� �����������
    uint32_t error_dev_st;     // ����� ������, ��� ������ ����������
    FRESULT fres;              // ��������� �������� IO, ����� ������, ��� ���������� �������.
} device_config_st;

// ��������� ����� ������ ------------------------------------------------------
#define BUF_W_SIZE     (512)   // ������ �������������� ������ ������ ������ � ����
#define FILE_N         (2)     // ���������� ������ �� ������
#define FILE_NAME_SIZE (32)    // ������ ����� �����
#define RX_BUF_SIZE    (4096)

typedef struct {
    uint8_t file_name[ FILE_N ][ FILE_NAME_SIZE ];   // ������, ��� �����
    FIL fil[ FILE_N ];                     // fatfs File object structure
    queue_buf_t q_buf[ FILE_N ];           // ��������� ����� ������ ������
    uint32_t wr_data_size[ FILE_N ];       // ���������� ������ ���������� � ����
    uint8_t buf_w[ BUF_W_SIZE ];           // ������������� ����� ��� ������ ������ � ���� COPY(q_data)->buf_w[ BUF_W_SIZE ]-> FILE to disk
} tfile_st;

#define BUF_DUMP_SIZE   (512)
typedef struct {
    uint8_t file_name[ FILE_NAME_SIZE ];   // ������, ��� �����
    FIL fil;                               // fatfs File object structure
    uint32_t lseek;                        // ������� ������ � �����
    uint32_t fsize;                        // ������ �����
    uint8_t buf_r[ BUF_DUMP_SIZE ];        // ������������� ����� ��� ������ ������ � ���� COPY(q_data)->buf_w[ BUF_W_SIZE ]-> FILE to disk
} tfile_dump_st;


// Backup ������ ��� ���������� ��������
#define ADR_CFG         BKP_DR1 // 0xA5A5 - �������� ������������ RTC ����������
#define ADR_SPEED_L     BKP_DR2 //- ��. uint32_t �������� ������ ������
#define ADR_SPEED_H     BKP_DR3 //- ��. uint32_t �������� ������ ������

#define FILE_DATA_SIZE_MAX  (10*1024*1024) // ������������ ������ ����� ��� ������ ������


#endif /* MAIN_H_ */
