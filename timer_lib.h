#ifndef _TIMER_LIB_H_
#define _TIMER_LIB_H_

typedef struct {
    uint32_t interval;     // גנולÿ ג לס, לוזהף גûחמגאלט ןמהןנמדנאלל
    uint32_t t_event;      // גנולÿ גûחמגא ןמהןנמדנאללû
    void (*event_call)();  // גûחûגאולאÿ ןמהןנמדנאללא
} timer_event_st;

void timer_event( timer_event_st *tim );
void _delay_ms( uint32_t delay_ms );

#endif