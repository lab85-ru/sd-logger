#include <stdint.h>
#include "sys_timer.h"
#include "timer_lib.h"

//******************************************************************************
// Timer
//******************************************************************************
void timer_event( timer_event_st *tim )
{
    if ( tim->t_event <= get_sys_counter() ){
        tim->t_event = get_sys_counter() + tim->interval;  // задаем время следующего запуска подпрограммы
        if ( tim->event_call != 0 ) (*tim->event_call)();
    }
}

//******************************************************************************
// _delay_ms For sdio_stm32f1
//******************************************************************************
void _delay_ms( uint32_t delay_ms )
{
    uint32_t t = 0;
    t = get_sys_counter();
    t = t + delay_ms;
    while ( get_sys_counter() < t );
}
