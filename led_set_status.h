#ifndef _LED_SET_STATUS_H_
#define _LED_SET_STATUS_H_

typedef enum {LED_PW_OFF = 0, LED_ERROR, LED_READY, LED_REC_ON} led_state_e;


void led_set_status( led_state_e led_st );

#endif