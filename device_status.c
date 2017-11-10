#include "stm32f10x.h"
#include "sdio_stm32f1.h"
#include "ff.h"
#include "xprintf.h"
#include "main.h"
#include "hardware.h"


extern device_config_st dev_cfg;

//******************************************************************************
// состояние устройства ВКЛ-ВЫКЛ
//******************************************************************************
void device_status( void )
{
    if ( dev_cfg.dev_state == DEV_POWER_OFF ){ // команда на отключение питания -> закрываем файлы
        dev_cfg.rec = REC_OFF;
        if ( dev_cfg.wr_data_st == W_STOP ){
            printf_d("\r\nSD CARD: unmount . . .\r\n");
            f_mount( 0, "", 1);
            dev_cfg.mount_status = MOUNT_NO;

            printf_d("SD CARD: De Init . . .\r\n");
            SD_DeInit();
            
            printf_d("SD CARD: POWER OFF . . .\r\n");
            SDIOPowerOff();     // Отключаем питания с SD карты памяти
            
            led_set_status( LED_PW_OFF );
            
            printf_d("DEVICE: stop . . .\r\n");
            while(1);
        }// if
    } else { // DEV_POWER_ON
        if (dev_cfg.error_dev_st != ER_NO){ // ERROR
            dev_cfg.led_st = LED_ERROR;
        } else { // NO ERROR
            if ( dev_cfg.rec == REC_ON )
                dev_cfg.led_st = LED_REC_ON;
            else
                dev_cfg.led_st = LED_READY;
        }
    }
}