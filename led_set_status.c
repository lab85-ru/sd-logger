#include "hardware.h"
#include "led_set_status.h"

//******************************************************************************
// Установка значений на светодиодах
//******************************************************************************
void led_set_status( led_state_e led_st )
{

    switch ( led_st ){
    case LED_PW_OFF:
      	led_off(LED_R);
    	led_off(LED_G);
        break;
        
    case LED_ERROR:
      	led_on(LED_R);
    	led_off(LED_G);
        break;
        
    case LED_READY:
      	led_off(LED_R);
    	led_on(LED_G);
        break;
        
    case LED_REC_ON:
      	led_on(LED_R);
    	led_on(LED_G);
        break;
        
    default:
      	led_off(LED_R);
    	led_off(LED_G);
        break;
    }
}