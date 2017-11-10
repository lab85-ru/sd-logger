/*
 * consol.c
 * Обертка для console.c
 * Данная обертка предназначена для передачи байт из приемного буфера UART порта в consol 
 */

#include <string.h>
#include <time.h>

#include "main.h"
#include "queue_buf.h"
#include "conv_data.h"
#include "console/console.h"


extern queue_buf_t consol_rx;

extern const char txt_device_name[];
extern const char txt_device_ver_soft[];
extern const char txt_device_ver_hard[];

extern console_var_type cn;

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// обработка даннных пришедших с консоли
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void consol( void )
{

	uint8_t c;

	if ( get_data_size_queue( &consol_rx ) == 0 )
	    return;

	pop_data_queue_b( &consol_rx , &c );
    console ( &cn, c);

}
