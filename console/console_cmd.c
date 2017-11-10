#include <string.h>

#include "stm32f10x.h"
#include "git_commit.h"
#include "console.h"
#include "xprintf.h"
#include "ff.h"
#include "main.h"
#include "fat_functions.h"
#include "rtc/rtc.h"
#include "time.h"
#include "conv_data.h"
#include "config_mem.h"
#include "hardware.h"
#include "dump.h"

extern device_config_st dev_cfg;
extern tfile_st file_st;
extern FATFS *fs_p;
extern uint8_t char_buf[];
extern const char txt_device_ver_soft[];
extern const char txt_device_ver_hard[];
extern const char txt_device_name[];
extern uint8_t path_buf[];


static DWORD fre_clust, fre_sect, tot_sect;
static tfile_dump_st f_dump;                // структура дл€ просмотра(вывода на консоль) файла


static const char ERROR_PARAM[] = { "\r\nERROR: parameter.\r\n" };
static const char TIRE_STR1[] = {
"\r\n"
"-------------------------------------------------------\r\n"
};

static const char HELP_STR2[] = {
"----------------- HELP --------------------------------\r\n"
"help or ?    - this help.\r\n"
"status       - status, information this device.\r\n"
"ls           - to list files this directory.\r\n"
"rec          - on|off  write data to file.\r\n"
"date         - Set date-time, format: DDMMYYYY CCMMSS\r\n"
"speed        - Set UART RX speed. =0 off, 1200, 4800,\r\n"
"               9600, 19200, 38400, 57600, 115200\r\n"
"reboot       - Reboot device.\r\n"
"poweroff     - Device: rec=off, file close, power off.\r\n"
"pwd          - Print working directory.\r\n"
"rm           - Delete file or directory.\r\n"
"cd           - Changes the current directory.\r\n"
"dump         - Dump file, block = 512 bytes\r\n"
"               1. DUMP <file name> - start dump file from 0 position,\r\n"
"               2. DUMP - dump next 512 bytes and etc.\r\n"
"recreate     - if rec=on then CLOSE FILE -> CREATE NEW FILE, Rx buf no clear!\r\n"
"seek         - Set start position for cmd DUMP.\r\n"
"               1. DUMP etc.\r\n"
"               2. seek <start position>\r\n"
"               if seek -no argument- then seek=0.\r\n"
"-------------------------------------------------------\r\n"
};

//-FUNCTIONS--------------------------------------------------------------------
static void call_cmd_help(const console_var_type *cn);
static void call_cmd_status(const console_var_type *cn);
static void call_cmd_ls(const console_var_type *cn);
static void call_cmd_rec(const console_var_type *cn);
static void call_cmd_date(const console_var_type *cn);
static void call_cmd_speed(const console_var_type *cn);
static void call_cmd_reboot(const console_var_type *cn);
static void call_cmd_pwd(const console_var_type *cn);
static void call_cmd_rm(const console_var_type *cn);
static void call_cmd_cd(const console_var_type *cn);
static void call_cmd_dump(const console_var_type *cn);
static void call_cmd_recreate(const console_var_type *cn);
static void call_cmd_seek(const console_var_type *cn);
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// cmd date
// Set date-time, format: DDMMYYYY CCMMSS
//                        12345678 123456
//------------------------------------------------------------------------------
// длинны строк даты и времени
#define DATE_STR_SIZE   (8)
#define TIME_STR_SIZE   (6)

// позиции в строке, смещение от начала строки
#define POS_DH          (0) 
#define POS_DL          (1)
#define POS_MH          (2) 
#define POS_ML          (3)
#define POS_YHH         (4) 
#define POS_YHL         (5)
#define POS_YLH         (6) 
#define POS_YLL         (7)

#define POS_CH          (0) 
#define POS_CL          (1)
#define POS_MIH         (2) 
#define POS_MIL         (3)
#define POS_SH          (4) 
#define POS_SL          (5)

static void call_cmd_date(const console_var_type *cn)
{
    struct tm tmi;
    char c;
    uint8_t n8;
    uint16_t n16;
    uint32_t n32;
    
    if (cn->argc != 3){
        goto _cmd_date_error;
    }

    if (strlen((const char *)cn->argv[1]) != DATE_STR_SIZE){
        goto _cmd_date_error;    }
    
    if (strlen((const char *)cn->argv[2]) != TIME_STR_SIZE){
        goto _cmd_date_error;    }
    
    // провер€ем, все символы должны быть числами
    for(int i=0; i<strlen((const char *)cn->argv[1]);i++){
        c = cn->argv[1][i];
        if ( c < '0' && c > '9' ){
            goto _cmd_date_error;            
        }
    }
    
    for(int i=0; i<strlen((const char *)cn->argv[2]);i++){
        c = cn->argv[2][i];
        if ( c < '0' && c > '9' ){
            goto _cmd_date_error;
        }
    }
    
    // заполн€ем структуру времени и провер€ем на правильный диапазон значени€
    // DDMMYYYY
    
    conv_2char_to_uint8(cn->argv[1][POS_DH],cn->argv[1][POS_DL], &n8);
    if (n8 > 31 || n8 == 0){
        goto _cmd_date_error;
    }
    tmi.tm_mday = n8;

    conv_2char_to_uint8(cn->argv[1][POS_MH],cn->argv[1][POS_ML], &n8);
    n8 = n8 - 1; // т.к. значение будет в диапазоне 0-11
    if (n8 > 11){
        goto _cmd_date_error;
    }
    tmi.tm_mon = n8;
    
    conv_2char_to_uint8(cn->argv[1][POS_YHH],cn->argv[1][POS_YHL], &n8);
    n16 = n8 * 100;
    
    conv_2char_to_uint8(cn->argv[1][POS_YLH],cn->argv[1][POS_YLL], &n8);
    n16 += n8;
    
    if (n16 < 2000 || n16 > 2136){
        goto _cmd_date_error;
    }
    
//    tmi.tm_year = n16 - 2000;
    tmi.tm_year = n16 - 1900;
    
    conv_2char_to_uint8(cn->argv[2][POS_CH],cn->argv[2][POS_CL], &n8);
    if (n8 > 23){
        goto _cmd_date_error;
    }
    tmi.tm_hour = n8;
    
    conv_2char_to_uint8(cn->argv[2][POS_MIH],cn->argv[2][POS_MIL], &n8);
    if (n8 > 59){
        goto _cmd_date_error;
    }
    tmi.tm_min = n8;
    
    conv_2char_to_uint8(cn->argv[2][POS_SH],cn->argv[2][POS_SL], &n8);
    if (n8 > 59){
        goto _cmd_date_error;
    }
    tmi.tm_sec = n8;

    tmi.tm_wday = 0; /**< days since Sunday - [ 0 to 6 ] */
    tmi.tm_yday = 0; /**< days since January 1 - [ 0 to 365 ] */
    tmi.tm_isdst = 0; /**< Daylight Saving Time flag */
    
    cn->call_print( "\r\n" );
    asctime_r( (const struct tm *)&tmi, (char*)char_buf );   // tm -> ascii string
    xprintf( "SET TIME: %s\r\n", char_buf );    
    
    n32 = mktime( &tmi );
    xprintf( "counter : %ld\r\n", n32);    
    
    rtc_time_adjust( n32 );
    
    return;
    
_cmd_date_error:
    cn->call_print( ERROR_PARAM );
}


//------------------------------------------------------------------------------
// cmd REC
//------------------------------------------------------------------------------
static void call_cmd_rec(const console_var_type *cn)
{
    if (cn->argc != 2){
        cn->call_print( ERROR_PARAM );
        return;
    }
    
    if (strcmp((const char*)cn->argv[1], "on") == 0){
        cn->call_print("\r\nEnable Recording data . . .\r\n");
        dev_cfg.rec = REC_ON;
        return;
    }
    
    if (strcmp((const char*)cn->argv[1], "off") == 0){
        cn->call_print("\r\nSTOP Recording data . . .\r\n");
        dev_cfg.rec = REC_OFF;
        return;
    }
    
    // неизвестный аргумент ...
    cn->call_print( ERROR_PARAM );
}



//------------------------------------------------------------------------------
// cmd LS
//------------------------------------------------------------------------------
static void call_cmd_ls(const console_var_type *cn)
{
    cn->call_print( TIRE_STR1 );
    
    if (dev_cfg.mount_status == MOUNT_OK){
        //strcpy((char *)char_buf, (const char*)path_buf);
        scan_files( (char*)path_buf );
    } else {
        cn->call_print("ERROR: DIST NOT MOUNT.\r\n");
    }
    
    cn->call_print( TIRE_STR1 );
}

//------------------------------------------------------------------------------
// cmd status
//------------------------------------------------------------------------------
static void call_cmd_status(const console_var_type *cn)
{
    uint32_t rtc_count;
    
    cn->call_print( TIRE_STR1 );
    cn->call_print( "DEVICE STATUS:\r\n\r\n");

    cn->call_print( "Power          = " );
    if (dev_cfg.dev_state == DEV_POWER_ON)
        cn->call_print( "ON.\r\n" );
    else
        cn->call_print( "OFF.\r\n" );
    
    // наличие карты в слоте ---------------------------------------------------
    cn->call_print("SDCARD in slot = ");
    if (dev_cfg.sd_slot == SD_LOAD){
        cn->call_print("YES.\r\n");
    }else{
        cn->call_print("NO.\r\n");
        return;
    }
    
    // —татус монтировани€ карты -----------------------------------------------
    cn->call_print("FAT MOUNT      = ");
    switch( dev_cfg.mount_status ){
    case MOUNT_NO:{
        cn->call_print("NO.\r\n");
        return;
    }
    case MOUNT_OK:{
        cn->call_print("OK.\r\n");
        break;
    }
    case MOUNT_ERROR:{
        cn->call_print("ERROR.\r\n");
        return;
    }
    default:{
        cn->call_print("????\r\n");
        return;
    }
    } //switch
    
    // текуща€ скорость работы портов приема
    cn->call_print("UART SPEED     = ");
    switch( dev_cfg.speed ){
    case 0:{
        cn->call_print("OFF\r\n");
        break;
    }
    case 9600:{
        cn->call_print("9600\r\n");
        break;
    }
    case 19200:{
        cn->call_print("19200\r\n");
        break;
    }
    case 38400:{
        cn->call_print("38400\r\n");
        break;
    }
    case 57600:{
        cn->call_print("57600\r\n");
        break;
    }
    case 115200:{
        cn->call_print("115200\r\n");
        break;
    }
    default:{
        cn->call_print("???? = OFF\r\n");
        break;
    }
    }
    
    // состо€ние записи данных в файл
    cn->call_print("RECORDING      = ");
    if ( dev_cfg.rec == REC_OFF){
        cn->call_print("OFF\r\n");
    }else{
        cn->call_print("ON\r\n");
        // выводим текущею информацию по записываемым файлам
        for (int i=0; i<FILE_N; i++){
            cn->call_print("File : ");
            cn->call_print( (const char*)file_st.file_name[ i ] );
            cn->call_print("    size = ");
            xsprintf((char *)char_buf , "%ld bytes.\r\n", file_st.wr_data_size[ i ]);
            cn->call_print( (const char *)char_buf );
        }
        
    }

    cn->call_print("\r\nDisk space :\r\n");
    
    /* Get volume information and free clusters of drive 1 */
    if (f_getfree("0:", &fre_clust, &fs_p) == FR_OK){
        /* Get total sectors and free sectors */
        tot_sect = (fs_p->n_fatent - 2) * fs_p->csize;
        fre_sect = fre_clust * fs_p->csize;

        /* Print the free space (assuming 512 bytes/sector) */
        xsprintf((char*)char_buf, "Total drive space = %10lu KiB.\n\rFree space        = %10lu KiB.\n\r",
            tot_sect / 2, fre_sect / 2);
        cn->call_print( (const char*)char_buf );
    }else{
        cn->call_print("ERROR\r\n");
    }
    
    cn->call_print("\r\nTIME:\r\n");
    rtc_count = RTC_GetCounter();
    xprintf("Counter = %ld sec.\r\n", rtc_count );
    rtc_show( rtc_count );

    // ошибки ------------------------------------------------------------------
    cn->call_print( "\r\n" );
    if ( dev_cfg.error_dev_st != ER_NO){
        if ( dev_cfg.error_dev_st & ER_SD_PRES) cn->call_print( "ERROR: Not SD CARD in slot !\r\n" );
        if ( dev_cfg.error_dev_st & ER_INIT) cn->call_print( "ERROR: INIT SD CARD !\r\n" );
        if ( dev_cfg.error_dev_st & ER_MOUNT) cn->call_print( "ERROR: MOUNT SD CARD !\r\n" );
        if ( dev_cfg.error_dev_st & ER_CR_FILE) cn->call_print( "ERROR: Create file !\r\n" );
        if ( dev_cfg.error_dev_st & ER_WR_DATA) cn->call_print( "ERROR: Write data to file !\r\n" );
        if ( dev_cfg.error_dev_st & ER_WR_DATAM) cn->call_print( "ERROR: data for write to file <> data write to file !\r\n" );
        if ( dev_cfg.error_dev_st & ER_FREE_SPACE) cn->call_print( "ERROR: SDCARD FULL ! NO FREESPACE !\r\n" );
        if ( dev_cfg.error_dev_st & ER_RX_BUF_FULL) cn->call_print( "ERROR: uart rx buffer is FULL !\r\n" );
        if ( dev_cfg.error_dev_st & ER_UART_SPEED) cn->call_print( "ERROR: Set uart speed !\r\n" );
    }else cn->call_print( "Status error: NO.\r\n" );
    
    cn->call_print( TIRE_STR1 );
}

//------------------------------------------------------------------------------
// cmd HELP
//------------------------------------------------------------------------------
static void call_cmd_help(const console_var_type *cn)
{
    cn->call_print( TIRE_STR1 );
    cn->call_print("\r\n-------------- SD Logger ----------------\r\n");
    cn->call_print("Software Ver = ");
    cn->call_print(txt_device_ver_soft);
    cn->call_print("\r\n");
    cn->call_print("Hardware Ver = ");
    cn->call_print(txt_device_ver_hard);
    cn->call_print("\r\n");
    cn->call_print("Git Ver      = ");
    cn->call_print(git_commit_str);
    cn->call_print("\r\n");
    cn->call_print("Device name  = ");
    cn->call_print(txt_device_name);
    cn->call_print("\r\n");
    cn->call_print( HELP_STR2 );
}

//------------------------------------------------------------------------------
// cmd speed
//------------------------------------------------------------------------------
static void call_cmd_speed(const console_var_type *cn)
{
    uint8_t s = 0;
    uint32_t sp = 0;
        
    cn->call_print( TIRE_STR1 );
    
    if (cn->argc != 2){
        cn->call_print( ERROR_PARAM );
        return;
    }
    
    // производим поиск по таблице 
    sp = atol( (const char *)cn->argv[1] );
    s = uart_rx_find_speed( sp );
    if (s == 0){
        cn->call_print( "ERROR: Parameter speed.\r\n" );
        return;
    }
    
    cn->call_print( "Store to config memory.\r\n" );
    config_mem_store_uint16( ADR_SPEED_L, sp & 0xffff );
    config_mem_store_uint16( ADR_SPEED_H, (sp & 0xffff0000) >> 16 );
    
    dev_cfg.speed = sp;
    dev_cfg.error_dev_st &= ~ER_UART_SPEED; // стереть бит ошибки
    
}

//------------------------------------------------------------------------------
// cmd REBOOT
//------------------------------------------------------------------------------
static void call_cmd_reboot(const console_var_type *cn)
{
    cn->call_print("\r\nREBOOT . . .\r\n");
    __disable_irq();
}

//------------------------------------------------------------------------------
// cmd poweroff
//------------------------------------------------------------------------------
static void call_cmd_poweroff(const console_var_type *cn)
{
    cn->call_print("\r\nPOWER OFF . . .\r\n");
    dev_cfg.dev_state = DEV_POWER_OFF;
}

//------------------------------------------------------------------------------
// cmd pwd
//------------------------------------------------------------------------------
static void call_cmd_pwd(const console_var_type *cn)
{
    cn->call_print( "\r\n" );
    cn->call_print( "PATH= " );
    cn->call_print( (const char*)path_buf );
    cn->call_print( "\r\n" );
}

//------------------------------------------------------------------------------
// cmd rm
//------------------------------------------------------------------------------
static void call_cmd_rm(const console_var_type *cn)
{
    
    if (cn->argc != 2){
        cn->call_print( ERROR_PARAM );
        return;
    }
    
    cn->call_print( "\r\n" );
    rm_files( (const char *)path_buf, (const char *)cn->argv[ 1 ] );
}

//------------------------------------------------------------------------------
// cmd cd
//------------------------------------------------------------------------------
static void call_cmd_cd(const console_var_type *cn)
{
    
    if (cn->argc != 2){
        cn->call_print( ERROR_PARAM );
        return;
    }
    
    cn->call_print( "\r\n" );
    cd_dir( (char *)path_buf, (const char *)cn->argv[ 1 ] );
}

//------------------------------------------------------------------------------
// cmd dump
//------------------------------------------------------------------------------
static void call_cmd_dump(const console_var_type *cn)
{
    FRESULT fres;
    FILINFO fno;
    uint32_t r_bytes = 0;
    
    cn->call_print( "\r\n" );
    
    if (cn->argc == 2){
        if (strlen((const char*)cn->argv[ 1 ]) > FILE_NAME_SIZE - 1 ){
            cn->call_print( "ERROR: File name is Long.\r\n" );
            return;
        }
        strcpy((char*)f_dump.file_name, (const char*)cn->argv[ 1 ]);
        f_dump.lseek = 0;
        
        fres = f_stat( (const char*)f_dump.file_name, &fno);
        if (fres != FR_OK){
            cn->call_print( "ERROR: Get status file.\r\n" );
        }
        f_dump.fsize = fno.fsize;
    }
 
    // открываем и читаем файл по 512 байт, смешение измен€ет на 512 байт.
    fres = f_open( &f_dump.fil, (const char*)f_dump.file_name, FA_OPEN_EXISTING || FA_READ);
    if (fres != FR_OK){
        cn->call_print( "ERROR: Open file.\r\n" );
        return;
    }
    
    xprintf("SEEK = 0x%08X\r\n", f_dump.lseek);
        
    fres = f_lseek( &f_dump.fil, f_dump.lseek);
    if (fres != FR_OK){
        cn->call_print( "ERROR: lseek.\r\n" );
        f_close( &f_dump.fil );
        return;
    }
    
    fres = f_read ( &f_dump.fil, f_dump.buf_r, BUF_DUMP_SIZE, &r_bytes);
    if (fres != FR_OK){
        cn->call_print( "ERROR: read.\r\n" );
        f_close( &f_dump.fil );
        return;
    }
    
    f_close( &f_dump.fil );
    
    if (r_bytes == 0){
        cn->call_print( "File of EOF.\r\n" );
        return;
    }
    
    dump_2((const uint8_t*)f_dump.buf_r, r_bytes);
    
    f_dump.lseek += BUF_DUMP_SIZE;
}

//------------------------------------------------------------------------------
// cmd recreate
// подаем принудительную команду на закрытие файлов т.к. rec=on,
// то произойдет автоматическо содание новых файлов,
// при этом входные буфферы продолжат принимать данные.
//------------------------------------------------------------------------------
static void call_cmd_recreate(const console_var_type *cn)
{
    if (dev_cfg.rec == REC_ON){
        dev_cfg.wr_data_st = W_CLOSEFILE;
        cn->call_print("\r\n Re-Create NEW file for log.\r\n");
    }
}

//------------------------------------------------------------------------------
// cmd seek
// установить позицию начала просматриваемого файла
//------------------------------------------------------------------------------
static void call_cmd_seek(const console_var_type *cn)
{
    uint32_t pos = 0;
    
    if (cn->argc == 1){
        xprintf("\r\nSet seek = 0.\r\n");
        f_dump.lseek = 0;
        return;
    }

    pos = atol( (const char *)cn->argv[1] );
    xprintf("\r\nSet seek = %d.\r\n", pos);
    f_dump.lseek = pos;
    return;
}



//------------------------------------------------------------------------------
// tablicha CMD -> CALL
const console_cmd_type consol_cmd_tab[] = {
{
    .cmd = "?",
    .call = &call_cmd_help,
},
{
    .cmd = "help",
    .call = &call_cmd_help,
},
{
    .cmd = "status",
    .call = &call_cmd_status,
},
{
    .cmd = "ls",
    .call = &call_cmd_ls,
},
{
    .cmd = "rec",
    .call = &call_cmd_rec,
},
{
    .cmd = "date",
    .call = &call_cmd_date,
},
{
    .cmd = "speed",
    .call = &call_cmd_speed,
},
{
    .cmd = "reboot",
    .call = &call_cmd_reboot,
},
{
    .cmd = "poweroff",
    .call = &call_cmd_poweroff,
},
{
    .cmd = "pwd",
    .call = &call_cmd_pwd,
},
{
    .cmd = "rm",
    .call = &call_cmd_rm,
},
{
    .cmd = "cd",
    .call = &call_cmd_cd,
},
{
    .cmd = "dump",
    .call = &call_cmd_dump,
},
{
    .cmd = "recreate",
    .call = &call_cmd_recreate,
},
{
    .cmd = "seek",
    .call = &call_cmd_seek,
}

};

// razmer tablichi
const uint32_t CONSOLE_CMD_TAB_SIZE = sizeof(consol_cmd_tab) / sizeof(struct console_cmd_st);
//------------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
