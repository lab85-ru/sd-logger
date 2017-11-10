#include "main.h"
#include "ff.h"
#include "xprintf.h"
#include "string.h"

extern uint8_t char_buf[];
extern device_config_st dev_cfg;

//******************************************************************************
// Print table files in working directory
//******************************************************************************
FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
//    UINT i;
    static FILINFO fno;


    res = f_opendir(&dir, path);                       /* Open the directory */
    xprintf("PATH = %s\r\n", path);
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                xprintf(" DIR  %s\n\r", fno.fname);
            } else {                                       /* It is a file. */
                xprintf(" FILE %s  %ld\n\r", fno.fname, fno.fsize);             // выводим в консоль БЕЗ вызова cn->call_print !!!!
            }
        }
        f_closedir(&dir);
    }

    return res;
}

//******************************************************************************
// Delete file or directory
//******************************************************************************
void rm_files( const char * path, const char * fname )
{
    FRESULT res;
    
    if (dev_cfg.mount_status != MOUNT_OK){
        xprintf("ERROR: Drive no mount.\r\n");
        return;
    }
        
    strcpy((char *)char_buf, (const char*)path);
    if (char_buf[ strlen((const char*)char_buf) - 1] != '/') strcpy((char *)&char_buf[ strlen((const char *)char_buf) ], "/");
    strcpy((char *)&char_buf[ strlen((char *)&char_buf) ], (const char*)fname);
    
    res = f_unlink( (const char *)char_buf );
    if (res != FR_OK){
        xprintf("ERROR: Delete => %s\r\n", fname);
    }
}

//******************************************************************************
// CD
//******************************************************************************
void cd_dir( char * path, const char * fname )
{
    FRESULT res;
    char *p = 0;
    uint8_t cd = 0;   // =1 UP(to dir), =0 down(..)
    
    if (dev_cfg.mount_status != MOUNT_OK){
        xprintf("ERROR: Drive no mount.\r\n");
        return;
    }
    
    if (strcmp(fname, "..") == 0){    // переход в сторону корня
        cd = 0;
        if (strcmp(path, "/") == 0){  // уже находимся в корне !
            xprintf("PATH = %s\r\n", path);
            return;
        }
        
        // ВЫрезаем строку с конца до символа '/' если '/'-первй не трогаем иначе удаляем его
        for(int i=strlen( path ) - 1; i>=0; i--){
            if (path[ i ] == '/'){
                if ( i == 0 )
                    path[ i + 1 ] = '\0';
                else
                    path[ i ] = '\0';
                p = path;
                break;
            }
        }
    }else{ // переход в сторону ОТ корня
        cd = 1;
        if (fname[0] == '/'){ // если первый символ / - значит все остальные игнорируем и приводим строку пути к корню.
            char_buf[ 0 ] = '/';
            char_buf[ 1 ] = '\0';
        }else{
            char_buf[ 0 ] = '\0';
            strcpy((char *)char_buf, (const char*)path);
            if (char_buf[ strlen((const char*)char_buf) - 1] != '/') strcpy((char *)&char_buf[ strlen((const char *)char_buf) ], "/");
            strcpy((char *)&char_buf[ strlen((const char *)char_buf) ], (const char*)fname);
        }
        p = (char *)char_buf;
    }
    
    xprintf("PATH = %s\r\n", p);    

    res = f_chdir( (const char *)p );
    if (res != FR_OK){
        xprintf("ERROR: path => %s\r\n", fname);
        return;
    }
    
    if (cd == 1){// cd -> DIR
        path[ 0 ] = '\0';
        strcpy((char *)path, (const char*)char_buf); // копируем строку нового пути(только что собранную) в текущий путь
    }
        
}
