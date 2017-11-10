#include <stdint.h>
#include "hardware.h"
#include "xprintf.h"
#include "main.h"
#include "keybord_cmd.h"
#include "sys_timer.h"

extern device_config_st dev_cfg;

static uint32_t t_start = 0;        // Время начала нажатия кнопки
static uint32_t t_stop = 0;         // Время отжатия кнопки
static uint32_t t_down = 0;         // интервал между нажатием и отжатием кнопки
static uint8_t key_event_flag = KEY_UP;  // событие, на нажатие кнопки, обработку кнопки. После обработки надо сбрасывать флаг в исходное сосятоние = KEY_UP
static uint8_t key_h = KEY_UP;      // предыдущее состояние кнопки

//******************************************************************************
// обработка команд от клавиш
//******************************************************************************
void keybord_cmd( void )
{
    uint8_t key;
    uint32_t t = 0;
    
    key = key_status( &t );
    
    if (key == KEY_UP)
        return;
    
    printf_d("\r\nKey press Time = %ld ms.\r\n", t);
    
    if (t >= 150 && t <=1500){ // rec on/off
        if (dev_cfg.rec == REC_ON){
            dev_cfg.rec = REC_OFF;
            printf_d("set REC_OFF\r\n");
        }else{
            dev_cfg.rec = REC_ON;
            printf_d("set REC_ON\r\n");
        }
        key_event_flag = KEY_UP;
        return;
    }
    
    if (t >= 3000){ // power off
        printf_d("set POWER_OFF\r\n");
        dev_cfg.rec = REC_OFF;
        dev_cfg.dev_state = DEV_POWER_OFF;

        key_event_flag = KEY_UP;
        return;
    }

    key_event_flag = KEY_UP;
}

//******************************************************************************
// обработка кнопки
//******************************************************************************
uint8_t key_status( uint32_t *t )
{
    uint8_t key;
    
    key = key_pressed();
    
    if (key_event_flag == KEY_DOWN){
        //printf_d("EVENT Ne obrabotan.... exit.\r\n");
        key_event_flag = KEY_UP;
        //return KEY_UP;
    }
    
    switch( key ){
    case KEY_DOWN:{
        if (key_h == KEY_UP){
            t_start = get_sys_counter(); // запоминаем текушее время нажатия кнопки
            //printf_d("t_start = %ld\r\n", t_start);
            key_h = KEY_DOWN;
        }
        break;
    }
        
    case KEY_UP:{
        if (key_h == KEY_DOWN){
            t_stop = get_sys_counter(); // запоминаем текушее время отжатия кнопки
            //printf_d("t_stop = %ld\r\n", t_stop);
            key_h = KEY_UP;
            if (t_start <= t_stop){ // расчет интервала нажатия
                t_down = t_stop - t_start;
            }else{
                t_down = 0xffffffff - t_start + t_stop;
            }
            key_event_flag = KEY_DOWN;
            //printf_d("t_down = %ld\r\n", t_down);
            *t = t_down;
        }// if -----------------------------
        break;
    }
    
    default:
        break;
    }
    
    return key_event_flag;
}
