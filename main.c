//******************************************************************************
// Устройство записи данных получаемых из 2-х usart портов в файлы на SD карту.
// 
// 
// 
//******************************************************************************
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#include "main.h"

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "hardware.h"
#include "sys_timer.h"
#include "xprintf.h"
#include "git_commit.h"
#include "console/console.h"
#include "consol.h"
#include "queue_buf.h"
#include "data_write_to_file.h"
#include "ff/sdio_stm32f1.h" // sdcard
#include "ff/ff.h"
#include "rtc/rtc.h"
#include "config_mem.h"
#include "keybord_cmd.h"
#include "device_status.h"
#include "timer_lib.h"

const char txt_device_ver_soft[] = {"SV : 2.2.16"};        // версия программы
const char txt_device_ver_hard[] = {"HV : stm32f103ret6"}; // version hardware
const char txt_device_name[]={"SDLogger-2"};               // название устройства

device_config_st dev_cfg;    // текущее состояние работы устройства
console_var_type cn;         // консоль (на UART порт)

queue_buf_t consol_rx;
uint8_t consol_rx_buf[ QUEUE_CONSOL_RX_SIZE ];

FATFS fs;      // fatfs structure
FATFS * fs_p;  // указатель на fatfs structure
FRESULT fres;  // результата IO операции

#define CHAR_BUF_SIZE (256)
uint8_t char_buf[ CHAR_BUF_SIZE ];// буфер для sprintf

#define PATH_BUF_SIZE (256)
uint8_t path_buf[ PATH_BUF_SIZE ];// строка текущий путь ! внимание длинна строки имеет ограниченный размер !

// буфер-а приема данных из uart порта -----------------------------------------
uint8_t rx_buf[ FILE_N ][ RX_BUF_SIZE ];

tfile_st file_st;          // структура параметры записи данных в файл
timer_event_st timer_led;  // структура для вызова подпрограммы управления светодиодами по таймеру

#define QUEUE_CONSOL_TX_BUF_SIZE (2048)
uint8_t consol_tx_buf[ QUEUE_CONSOL_TX_BUF_SIZE ]; // буфер для вывода сообщений на консоль
queue_buf_t consol_tx;
uint8_t flag_consol_tx = 0;  // флаг идет передача = 1, =0 нет передачи.
//******************************************************************************
// Установка значений на светодиодах (обертка)
//******************************************************************************
void led_set( void )
{
    led_set_status( dev_cfg.led_st );
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
int main(void) {
    
    path_buf[ 0 ] = '/';  // задаем начальный путь - корень
    path_buf[ 1 ] = '\0';
       
    // начальная инициализация состояния устройства    
    dev_cfg.error_dev_st = ER_NO;
    dev_cfg.sd_slot      = SD_EMPTY;
    dev_cfg.mount_status = MOUNT_NO;
    dev_cfg.speed        = 0;
    dev_cfg.rec          = REC_OFF;
    dev_cfg.wr_data_st   = W_STOP;
    dev_cfg.led_st       = LED_READY;
    dev_cfg.dev_state    = DEV_POWER_ON;
    
    timer_led.event_call = &led_set;  // параметры запуска подпрограммы отображения состояния на светодиодах
    timer_led.interval   = 100; //ms
    timer_led.t_event    = 0;
    
    // инит кольцевой буфер для консоли
    consol_rx.queue = consol_rx_buf;
    consol_rx.in    = 0;
    consol_rx.out   = 0;
    consol_rx.len   = QUEUE_CONSOL_RX_SIZE;
        
    reset_queue( &consol_rx );
    
    consol_tx.queue = consol_tx_buf;
    consol_tx.in    = 0;
    consol_tx.out   = 0;
    consol_tx.len   = QUEUE_CONSOL_TX_BUF_SIZE;
    
    reset_queue( &consol_tx );
    
    for (int i=0; i<FILE_N; i++){
        file_st.wr_data_size[ i ] = 0;
        file_st.q_buf[ i ].in    = 0;
        file_st.q_buf[ i ].out   = 0;
        file_st.q_buf[ i ].len   = RX_BUF_SIZE;
        file_st.q_buf[ i ].queue = &rx_buf[ i ][0];
        
        memset( &file_st.file_name[ i ][0], 0, FILE_NAME_SIZE );
        
        file_st.fil[ i ].obj.fs = 0; // =0 значит файл закрыт
    }
    
    fs_p = &fs;
        
    SystemInit();                   // this sets up the oscilator

    init_hardware();
    sys_timer_setup();
    
    led_set_status( dev_cfg.led_st );
        
    xfunc_out = tx_byte;           // настройка указателя на функцию вывода (для xprintf.c)
    
    cn.call_print = print_console;
    console_init( &cn );

    printf_d("\r\n--------------- SD Logger -----------------------------\r\n");
    printf_d("Software Ver = %s\r\n", txt_device_ver_soft);
    printf_d("Hardware Ver = %s\r\n", txt_device_ver_hard);
    printf_d("Git Ver      = %s\r\n", git_commit_str);
    printf_d("Device name  = %s\r\n", txt_device_name);
    printf_d("\r\n");
    
    rtc_setup(); // настройка внутреннего stm32(RTC)
    
    // считываем значение скорости работы uart порта из BACKUP памяти
    dev_cfg.speed = config_mem_load_uint16(ADR_SPEED_L) | (config_mem_load_uint16(ADR_SPEED_H) << 16);
    
    // проверка наличия карты в слоте
    if (SD_Detect() == SD_NOT_PRESENT){
        led_set_status( LED_ERROR );
        while(1){
            printf_d("ERROR: SD CARD is not present to SLOT !!!\n\r");
            _delay_ms( 1000 );
            if (SD_Detect() == SD_PRESENT) { // Карту вставили ! Перезагрузка устройства.
                _delay_ms( 1000 );
                __disable_irq();             // Перезагрузка произойдет от WDT
                while(1);
            }
        }// while --------------------------------------------------------------
    } // if
     
    dev_cfg.sd_slot = SD_LOAD;
    printf_d("SD CARD in slot.\r\n");

    SDIOPowerOn();     // Подача питания на слот карты памяти
    
    _delay_ms( 500 );  // задержка для стабилизации напряженния на карте
        
    printf_d("Init . . . ");
    Status = SD_Init();
    if (Status != SD_OK){
        led_set_status( LED_ERROR );   // вывод текущего состояния на светодиоды
        while(1){
            printf_d("ERROR: Init sd, error num = %d\r\n", Status);
            _delay_ms( 1000 );
        }
    }
    printf_d("OK.\r\n");
    
    // Монтирование диска
    printf_d("Mount . . . ");
    fres = f_mount(fs_p, "", 1);
    if (fres != FR_OK){
        led_set_status( LED_ERROR );
        while(1){
            printf_d("ERROR: Mount SD, error num = %d\r\n", fres);
            _delay_ms( 1000 );
        }
    }
    dev_cfg.mount_status = MOUNT_OK;
    printf_d("OK.\r\n");

    // Создаем директорию в которой будем создовать файлы
    printf_d("Mkdir ( %s ) ", DATA_PATH);
    fres = f_mkdir( DATA_PATH );
    if (fres != FR_OK && fres != FR_EXIST){
        printf_d("ERROR\r\n");
        led_set_status( LED_ERROR );
        while(1){
            printf_d("ERROR: Create directory for DATA => %s\r\n", DATA_PATH);
            _delay_ms( 1000 );
        }
    }
    printf_d("OK.\r\n");
    
	while(1){
        consol();            // консоль устройства - обертка для console()
        data_processing();   // обработка и запись данных в файл
        keybord_cmd();       // обработка команд от клавиш
        timer_event( &timer_led );  // вывод текущего состояния на светодиоды
        device_status();            // состояние устройства ВКЛ-ВЫКЛ
        
        if (SD_Detect() == SD_NOT_PRESENT){ // нет карты, перезагружает устройство
            __disable_irq();                // Перезагрузка произойдет от WDT
            while(1);
        }// if -----------------------------------------------------------------
    }//while
}
//******************************************************************************
