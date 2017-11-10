#include <stdint.h>
#include <string.h>
#include "main.h"
#include "ff.h"
#include "xprintf.h"
#include "data_write_to_file.h"
#include "time.h"
#include "rtc.h"
#include "hardware.h"

extern device_config_st dev_cfg;
extern tfile_st file_st;
extern struct tm rtc_tm;

static FRESULT fres;
static uint32_t w_bytes = 0;  // количество записанных байт в файл
static uint32_t d_len = 0;    // колдичество данных в кольцевом буфере
static uint32_t w_len = 0;    // количество записываемых данных

//******************************************************************************
// Запись данных в файл (главный автомат обработки-записи данных в файлы)
//******************************************************************************
void data_processing( void )
{
    
    switch( dev_cfg.wr_data_st ){
//------------------------------------------------------------------------------
    case W_STOP:{
        if (dev_cfg.rec == REC_ON){ // начинаем запись данных
            dev_cfg.wr_data_st = W_CREATEFILE;
            for (int i=0; i<FILE_N; i++) reset_queue( &file_st.q_buf[ i ] );        // Очистили приемные буферы
            // включили порты на прием данных
            if (uart_rx_port_enable( dev_cfg.speed ) == 0){
                dev_cfg.error_dev_st |= ER_UART_SPEED;   // выставляем флаг ошибка неверная скорость uart порта
                dev_cfg.wr_data_st = W_ERROR;            // выход и переход в состояние ОШИБКА
                break;
            }
        }
        break;
    }
    
//------------------------------------------------------------------------------
    case W_CREATEFILE:{
        for (int i=0; i<FILE_N; i++){
            generate_file_name( DATA_PATH, i, (char*)file_st.file_name[ i ], FILE_NAME_SIZE); // генерим имя файла 1,2
            file_st.wr_data_size[ i ] = 0;             // количество записанных данных = 0
        }
        
        // открываем файлы на запись
        for (int i=0; i<FILE_N; i++){        
            fres = f_open( &file_st.fil[ i ], (char const*)file_st.file_name[ i ], FA_CREATE_ALWAYS | FA_WRITE);
            if (fres != FR_OK){
                dev_cfg.fres = fres;                  // сохранили номер ошибки
                printf_d("ERROR num = %d\r\n", fres);
                dev_cfg.error_dev_st |= ER_CR_FILE;   // выставляем флаг ошибка создания файла
                dev_cfg.wr_data_st = W_ERROR;         // выход и переход в состояние ОШИБКА
                break;
            } else {
                printf_d("OK\r\n");
            }
        }
        
        dev_cfg.wr_data_st = W_WRDATA;
        break;
    }
    
//------------------------------------------------------------------------------
    case W_WRDATA:{
        // сколько есть данных на запись
        for (int i=0; i<FILE_N; i++){
            while (get_data_size_queue( &file_st.q_buf[ i ] ) >= BUF_W_SIZE){
                fres = write_data_to_file( i, BUF_W_SIZE, &w_bytes );
                if (w_bytes != BUF_W_SIZE){
                    dev_cfg.error_dev_st |= ER_WR_DATAM;
                    dev_cfg.wr_data_st = W_ERROR;
                    break;
                }
                if (fres != FR_OK){
                    dev_cfg.fres = fres;
                    dev_cfg.error_dev_st |= ER_WR_DATA;
                    dev_cfg.wr_data_st = W_ERROR;
                    break;
                }
                file_st.wr_data_size[ i ] += w_bytes;
                if ( file_st.wr_data_size[ i ] >= FILE_DATA_SIZE_MAX ){ // файл достиг максимального размера - Пересоздаем файл без прерывания входного потока
                    printf_d("\r\n RE-create file.\r\n");
                    dev_cfg.wr_data_st = W_CLOSEFILE;
                    break;
                }
            }// if -------------------------------------------------------------
        } // for ---------------------------------------------------------------
        
        if ( dev_cfg.rec == REC_OFF ){
            dev_cfg.wr_data_st = W_CLOSEFILE;
        }
        break;
    }
    
//------------------------------------------------------------------------------
    case W_CLOSEFILE:{
        if ( dev_cfg.rec == REC_OFF){
            uart_rx_port_disable();    // останавливаем прием данных
        }
        
        // записываем данные на диск (все что содержат буферы)
        for (int i=0; i<FILE_N; i++){
            while( (d_len = get_data_size_queue( &file_st.q_buf[ i ] )) > 0){
                if (d_len >= BUF_W_SIZE){
                    w_len = BUF_W_SIZE;
                    fres = write_data_to_file( i, w_len, &w_bytes );
                }else{
                    w_len = d_len;
                    fres = write_data_to_file( i, w_len, &w_bytes );
                }
                if (w_bytes != w_len){
                    dev_cfg.error_dev_st |= ER_WR_DATAM;
                    dev_cfg.wr_data_st = W_ERROR;
                    break;
                }
                if (fres != FR_OK){
                    dev_cfg.fres = fres;
                    dev_cfg.error_dev_st |= ER_WR_DATA;
                    dev_cfg.wr_data_st = W_ERROR;
                    break;
                }
                file_st.wr_data_size[ i ] += w_bytes;
            }// while ----------------------------------------------------------
        } // for ---------------------------------------------------------------

        
        close_file();    // Закрываем все открытые файлы
        if ( dev_cfg.rec == REC_ON){ // если запись включена значит Производим процедуру пересоздания файла т.к. записываемый файл достиг максимального размера
            dev_cfg.wr_data_st = W_CREATEFILE;
        } else {
            dev_cfg.wr_data_st = W_STOP;
        }
        break;
    }
    
//------------------------------------------------------------------------------
    case W_ERROR:{

        close_file();    // Закрываем все открытые файлы
        
        // ожидаем перехода в режим REC_OFF
        if (dev_cfg.rec == REC_OFF){
            dev_cfg.wr_data_st = W_STOP;
        }
        break;
    }

//------------------------------------------------------------------------------    
    default:{
        dev_cfg.wr_data_st = W_STOP;
        break;
    }
    }// switch
}


//******************************************************************************
// Создание строки - имя файла
// uint8_t *path                 - начальный путь (0:/)-например
// uint8_t id                    - номер файла
// uint8_t *fname                - строка имя файла
// const uint16_t FNAME_SIZE_MAX - максимальный размер имени файла
//******************************************************************************
void generate_file_name(char *path, int id, char *fname, const uint16_t fname_size_max)
{
    uint32_t t = 0;
    char * p = (char*)fname; // указатель на конец строки ( добавляем в конец строки, др. строки)
    
    memset(fname, 0, fname_size_max); // очищаем поле имени файла
    if ( strlen((char*)path) != 0){ // копируем путь перед именем файла
        strcpy(p, path);
    }
    
    p = fname + strlen(fname);
    
    // генерить имя файла будем на основе текущего времени
    t = rtc_get_count();
    gmtime_r( &t, &rtc_tm);
    
    xsprintf(p, "/%02d%02d%02d_%02d%02d%02d_%d.bin", rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900, rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec, id);// дата + номер файла + расширение файла .BIN
    
    printf_d("\nGENERATE FILE NAME = %s\r\n", fname); // DEBUG
}

//******************************************************************************
// Закрываем все открытые файлы
//******************************************************************************
void close_file( void )
{
    for (int i=0; i<FILE_N; i++){
        if (file_st.fil[ i ].obj.fs != 0)
            f_close( &file_st.fil[ i ] );
    }
}

//******************************************************************************
// Записываем данные на диск
//******************************************************************************
FRESULT write_data_to_file( uint8_t file_n, uint16_t d_size, uint32_t *w_size )
{
    // извлекаем данные из кольцевого буфера в промежуточный
    file_st.q_buf[ file_n ].rw_buf = file_st.buf_w;
    file_st.q_buf[ file_n ].rw_len = d_size;
    pop_data_queue( &file_st.q_buf[ file_n ] );
                
    // Производим запись буфера = BUF_W_SIZE байт в файл
    return f_write( &file_st.fil[ file_n ], file_st.buf_w, d_size, w_size);
}
