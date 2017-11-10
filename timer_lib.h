#ifndef _TIMER_LIB_H_
#define _TIMER_LIB_H_

typedef struct {
    uint32_t interval;     // ����� � ��, ����� �������� �����������
    uint32_t t_event;      // ����� ������ ������������
    void (*event_call)();  // ���������� ������������
} timer_event_st;

void timer_event( timer_event_st *tim );
void _delay_ms( uint32_t delay_ms );

#endif