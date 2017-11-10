#ifndef INIT_HARDWARE_H_
#define INIT_HARDWARE_H_
     
#include <stdint.h>
#include "stm32f10x.h"
#include "ff/integer.h"  // for DWORD

//------------------------------------------------------------------------------

#define LEDn	2

#define LEDR_PIN                         GPIO_Pin_4
#define LEDR_GPIO_PORT                   GPIOA
#define LEDR_GPIO_CLK                    RCC_APB2Periph_GPIOA

#define LEDG_PIN                         GPIO_Pin_5
#define LEDG_GPIO_PORT                   GPIOA
#define LEDG_GPIO_CLK                    RCC_APB2Periph_GPIOA

//#define LED3_PIN                       GPIO_Pin_6
//#define LED3_GPIO_PORT                 GPIOA
//#define LED3_GPIO_CLK                  RCC_APB2Periph_GPIOA


#define SDIO_CARD_POWER_PIN                      GPIO_Pin_0
//#define SDIO_CARD_DETECT_PIN                   GPIO_Pin_1
#define SDIO_CARD_POWER_DETECT_PORT              GPIOB
#define SDIO_CARD_POWER_DETECT_APB2Periph_GPIO   RCC_APB2Periph_GPIOB

     
#define KEY_UP    (1)  // кнопка НЕ нажата
#define KEY_DOWN  (0)  // кнопка нажата

#define KEY_PIN               GPIO_Pin_6                  /* PB.6 */
#define KEY_PORT              GPIOB                       /* GPIOB */
#define KEY_CLK               RCC_APB2Periph_GPIOB

typedef enum {
    LED_R = 0,
    LED_G = 1
} led_t;


#define COMn      (3) // Количество портов занесенных в структуру
typedef enum {
    COM1 = 0,
    COM2 = 1,
    COM3 = 2
} com_num_t;


/**
 * @brief Definition for COM port1, connected to USART1
 * Consol
 */ 
#define H_COM1                        USART1
#define H_COM1_CLK                    RCC_APB2Periph_USART1
#define H_COM1_TX_PIN                 GPIO_Pin_9
#define H_COM1_TX_GPIO_PORT           GPIOA
#define H_COM1_TX_GPIO_CLK            RCC_APB2Periph_GPIOA
#define H_COM1_RX_PIN                 GPIO_Pin_10
#define H_COM1_RX_GPIO_PORT           GPIOA
#define H_COM1_RX_GPIO_CLK            RCC_APB2Periph_GPIOA
#define H_COM1_IRQn                   USART1_IRQn

/**
 * @brief Definition for COM port2, connected to USART2
 */ 
#define H_COM2                        USART2
#define H_COM2_CLK                    RCC_APB1Periph_USART2
#define H_COM2_TX_PIN                 GPIO_Pin_2
#define H_COM2_TX_GPIO_PORT           GPIOA
#define H_COM2_TX_GPIO_CLK            RCC_APB2Periph_GPIOA
#define H_COM2_RX_PIN                 GPIO_Pin_3
#define H_COM2_RX_GPIO_PORT           GPIOA
#define H_COM2_RX_GPIO_CLK            RCC_APB2Periph_GPIOA
#define H_COM2_IRQn                   USART2_IRQn

/**
 * @brief Definition for COM port3, connected to USART3
 */ 
#define H_COM3                        USART3
#define H_COM3_CLK                    RCC_APB1Periph_USART3
#define H_COM3_TX_PIN                 GPIO_Pin_10
#define H_COM3_TX_GPIO_PORT           GPIOB
#define H_COM3_TX_GPIO_CLK            RCC_APB2Periph_GPIOB
#define H_COM3_RX_PIN                 GPIO_Pin_11
#define H_COM3_RX_GPIO_PORT           GPIOB
#define H_COM3_RX_GPIO_CLK            RCC_APB2Periph_GPIOB
#define H_COM3_IRQn                   USART3_IRQn

void init_hardware(void);
void nvic_config_com1(FunctionalState st);
void nvic_config_com2(FunctionalState st);
void nvic_config_com3(FunctionalState st);
void stm32_uart_set(com_num_t COM, USART_InitTypeDef* USART_InitStruct, FunctionalState st);
void stm32_uart_init(com_num_t COM, USART_InitTypeDef* USART_InitStruct);
void led_init( led_t _led );
void led_on( led_t _led );
void led_off( led_t _led );
void led_toggle( led_t _led );
void SDIOPowerDetectPinInit( void );
void SDIOPowerOn( void );
void SDIOPowerOff( void );
void backup_init( void );
uint8_t uart_rx_find_speed( uint32_t speed );
void uart_rx_port_disable( void );
uint32_t uart_rx_port_enable( uint32_t speed );
void tx_byte(uint8_t ch);
void print_console(const char * s);
void key_init( void );
uint8_t key_pressed( void );

#endif
