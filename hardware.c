/*
 * Подпрограммы работы с аппаратурой
 *
 */

#include <stdint.h>
#include <string.h>
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "main.h"
#include "hardware.h"
#include "xprintf.h"
#include "sys_timer.h"
#include "queue_buf.h"

#ifndef DEBUG
#define DEBUG 0
#endif
   
GPIO_TypeDef* GPIO_PORT[LEDn] = {LEDR_GPIO_PORT, LEDG_GPIO_PORT};
const uint16_t GPIO_PIN[LEDn] = {LEDR_PIN, LEDG_PIN};
const uint32_t GPIO_CLK[LEDn] = {LEDR_GPIO_CLK, LEDG_GPIO_CLK};

USART_TypeDef* const COM_USART[COMn]  = {H_COM1, H_COM2, H_COM3};
GPIO_TypeDef* const COM_TX_PORT[COMn] = {H_COM1_TX_GPIO_PORT, H_COM2_TX_GPIO_PORT, H_COM3_TX_GPIO_PORT};
GPIO_TypeDef* const COM_RX_PORT[COMn] = {H_COM1_RX_GPIO_PORT, H_COM2_RX_GPIO_PORT, H_COM3_RX_GPIO_PORT};
const uint32_t COM_USART_CLK[COMn]    = {H_COM1_CLK, H_COM2_CLK, H_COM3_CLK};
const uint32_t COM_TX_PORT_CLK[COMn]  = {H_COM1_TX_GPIO_CLK, H_COM2_TX_GPIO_CLK, H_COM3_TX_GPIO_CLK};
const uint32_t COM_RX_PORT_CLK[COMn]  = {H_COM1_RX_GPIO_CLK, H_COM2_RX_GPIO_CLK, H_COM3_RX_GPIO_CLK};
const uint16_t COM_TX_PIN[COMn]       = {H_COM1_TX_PIN, H_COM2_TX_PIN, H_COM3_TX_PIN};
const uint16_t COM_RX_PIN[COMn]       = {H_COM1_RX_PIN, H_COM2_RX_PIN, H_COM3_RX_PIN};

// таблица стандартных скоростей, для настройки работы портов на прием
const uint32_t tab_u_speed[] = { 0, 1200, 4800, 9600, 19200, 38400, 57600, 115200};
// количество элементов в таблице
const uint32_t UART_RX_SPEED_TAB_SIZE = sizeof(tab_u_speed) / sizeof(uint32_t);
//------------------------------------------------------------------------------

extern tfile_st file_st;
extern queue_buf_t consol_tx;
extern uint8_t flag_consol_tx;

//******************************************************************************
// Настройка портов Светодиодов
//******************************************************************************
void led_init( led_t led )
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* Enable the GPIO_LED Clock */
  RCC_APB2PeriphClockCmd(GPIO_CLK[ led ], ENABLE);

  /* Configure the GPIO_LED pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_PIN[ led ];
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIO_PORT[ led ], &GPIO_InitStructure);
}

//******************************************************************************
// Включение светодиодов
//******************************************************************************
void led_on( led_t led)
{
  GPIO_PORT[ led ]->BSRR = GPIO_PIN[ led ];   
}

//******************************************************************************
// ВЫключение светодиода
//******************************************************************************
void led_off( led_t led)
{
  GPIO_PORT[ led ]->BRR = GPIO_PIN[ led ];   
}

//******************************************************************************
// Смена состояния светодиода
//******************************************************************************
void led_toggle( led_t led )
{
  GPIO_PORT[ led ]->ODR ^= GPIO_PIN[ led ];
}

//******************************************************************************
// Начальная инициальзация переферии
//******************************************************************************
void init_hardware(void)
{
    USART_InitTypeDef USART_InitStructure;

   	led_init( LED_R );
	led_init( LED_G );

	led_off( LED_R );
	led_off( LED_G );
        
    SDIOPowerDetectPinInit();               // настройка пинов для работы с sd картой
    
    key_init();
        
    // UART Init CONSOL
    USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;

	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	stm32_uart_init(COM1, &USART_InitStructure);
    
    backup_init();  // включаем доступ к backup регистрам

    printf_d("Start IWDG(watchdog) interval 280ms.\r\n");
    
	/* IWDG timeout equal to 280 ms (the timeout may varies due to LSI frequency dispersion) */
	/* Enable write access to IWDG_PR and IWDG_RLR registers */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	/* IWDG counter clock: 40KHz(LSI) / 32 = 1.25 KHz */
	IWDG_SetPrescaler(IWDG_Prescaler_32);

	/* Set counter reload value to 349 */
	IWDG_SetReload(349);

	/* Reload IWDG counter */
	IWDG_ReloadCounter();

	/* Enable IWDG (the LSI oscillator will be enabled by hardware) */
	IWDG_Enable();
}

//******************************************************************************
// Настройка контролера прерываний для USART1
//******************************************************************************
void nvic_config_com1(FunctionalState st)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	#ifdef SOFTOFFSET
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x20000);
	#endif
	/* Enable the USART1 Interrupt */
  	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = st;
  	NVIC_Init(&NVIC_InitStructure);
}

//******************************************************************************
// Настройка контролера прерываний для USART2
//******************************************************************************
void nvic_config_com2(FunctionalState st)
{
  	NVIC_InitTypeDef NVIC_InitStructure;
	#ifdef SOFTOFFSET
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x20000);
	#endif
  	/* Enable the USART2 Interrupt */
  	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = st;
  	NVIC_Init(&NVIC_InitStructure);
}

//******************************************************************************
// Настройка контролера прерываний для USART3
//******************************************************************************
void nvic_config_com3(FunctionalState st)
{
  	NVIC_InitTypeDef NVIC_InitStructure;
	#ifdef SOFTOFFSET
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x20000);
	#endif
  	/* Enable the USART2 Interrupt */
  	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = st;
  	NVIC_Init(&NVIC_InitStructure);
}

//******************************************************************************
// Настройка uart портов в работу
//******************************************************************************
void stm32_uart_init(com_num_t COM, USART_InitTypeDef* USART_InitStruct)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    switch (COM){
    case COM1:
	    nvic_config_com1(ENABLE);
	    /* Enable GPIO clock */
	    RCC_APB2PeriphClockCmd(COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM] | RCC_APB2Periph_AFIO, ENABLE);
	    /* Enable UART clock */
	    RCC_APB2PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
	    /* Configure USART Tx as alternate function push-pull */
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
        GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);
	    /* Configure USART Rx as input floating */
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
        GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);
	    break;

    case COM2: // RX Only
	    nvic_config_com2(ENABLE);
	    /* Enable GPIO clock */
	    //RCC_APB2PeriphClockCmd(COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM] | RCC_APB2Periph_AFIO, ENABLE);
        RCC_APB2PeriphClockCmd( COM_RX_PORT_CLK[COM] | RCC_APB2Periph_AFIO, ENABLE);
	    /* Enable UART clock */
	    RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
        
	    /* Configure USART Tx as alternate function push-pull */
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	    //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	    //GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
	    //GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);
        
	    /* Configure USART Rx as input floating */
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	    GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
	    GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);
	    break;

    case COM3: // RX Only
	    nvic_config_com3(ENABLE);
	    /* Enable GPIO clock */
	    //RCC_APB2PeriphClockCmd(COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM] | RCC_APB2Periph_AFIO, ENABLE);
        RCC_APB2PeriphClockCmd( COM_RX_PORT_CLK[COM] | RCC_APB2Periph_AFIO, ENABLE);
	    /* Enable UART clock */
	    RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
        
	    /* Configure USART Tx as alternate function push-pull */
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	    //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	    //GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
	    //GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);
        
	    /* Configure USART Rx as input floating */
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	    GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
	    GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);
        break;

  default:
	  return;
	  break;
  }

    /* USART configuration */
    USART_Init(COM_USART[COM], USART_InitStruct);
    /* Enable USART */
    USART_Cmd(COM_USART[COM], ENABLE);
    /* Enable Int to RX char */
    USART_ITConfig(COM_USART[COM], USART_IT_RXNE, ENABLE);

}

//******************************************************************************
// Управление питанием SD карты
//******************************************************************************
void SDIOPowerDetectPinInit( void )
{
	 // включение тактирования порта вывода  SDIO_CARD_POWER_DETECT
	 RCC_APB2PeriphClockCmd(SDIO_CARD_POWER_DETECT_APB2Periph_GPIO , ENABLE);
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 // инициализация вывода сигнала питания uSD сокета
	 GPIO_InitStructure.GPIO_Pin = SDIO_CARD_POWER_PIN ;
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	 GPIO_Init(SDIO_CARD_POWER_DETECT_PORT, &GPIO_InitStructure);
	 // подключаем питание к накапителю
	 SDIO_CARD_POWER_DETECT_PORT->BRR = SDIO_CARD_POWER_PIN ;

	 // инициализация вывода детектирования наличия uSD карты в сокете
	 //GPIO_InitStructure.GPIO_Pin = SDIO_CARD_DETECT_PIN ;
	 //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	 //GPIO_Init(SDIO_CARD_POWER_DETECT_PORT, &GPIO_InitStructure);
}
//******************************************************************************
// подача питания на карту
//******************************************************************************
void SDIOPowerOn( void )
{
    SDIO_CARD_POWER_DETECT_PORT->BRR = SDIO_CARD_POWER_PIN ;
}

//******************************************************************************
// снимаем питания с карты
//******************************************************************************
void SDIOPowerOff( void )
{
    SDIO_CARD_POWER_DETECT_PORT->BSRR = SDIO_CARD_POWER_PIN ;
}

//******************************************************************************
// Настройка доступа к backup registers
//******************************************************************************
void backup_init( void )
{
    /* Enable PWR and BKP clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

    /* Enable write access to Backup domain */
    PWR_BackupAccessCmd(ENABLE);
}
//******************************************************************************
// поиск значения скорости в таблице стандартных скоростей
//******************************************************************************
uint8_t uart_rx_find_speed( uint32_t speed)
{
    uint8_t i=0;
    
    for(i=0; i<UART_RX_SPEED_TAB_SIZE; i++){
        if ( speed == tab_u_speed[ i ] )
            return i;
    }
    
    return 0;
}

//******************************************************************************
// Включение портов
//******************************************************************************
uint32_t uart_rx_port_enable( uint32_t speed )
{
    USART_InitTypeDef USART_InitStructure;
    
    if ( speed == 0 || uart_rx_find_speed(speed) == 0 ) return 0; // проверка на существование скорости в таблице

    // очистка входных буферов от предыдущих данных
    for (int i=0; i<FILE_N; i++){
        reset_queue( &file_st.q_buf[ i ] );
    }
        
    // UART Init CONSOL
    USART_InitStructure.USART_BaudRate = speed;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;

	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx;  // включаем только прием

	stm32_uart_init(COM2, &USART_InitStructure);
    stm32_uart_init(COM3, &USART_InitStructure);
    
    return speed;
}

//******************************************************************************
// выключение портов uart 2,3 (с данных портов собираем логи)
//******************************************************************************
void uart_rx_port_disable( void )
{
    __disable_irq();
    
    USART_DeInit( H_COM2 );
    USART_ITConfig( H_COM2, USART_IT_RXNE, DISABLE);
    USART_Cmd( H_COM2, DISABLE);

    USART_DeInit( H_COM3 );
    USART_ITConfig( H_COM3, USART_IT_RXNE, DISABLE);
    USART_Cmd( H_COM3, DISABLE);
    
    __enable_irq();
}

//******************************************************************************
// Передача одного байта в порт (консоль)
// - теперь кладем в буфер и запускаем передачу !
//******************************************************************************
void tx_byte(uint8_t ch)
{
    //while (USART_GetFlagStatus(H_COM1, USART_FLAG_TC) == RESET);
    //USART_SendData(H_COM1, ch);
    
    while( get_free_size_queue( &consol_tx ) <= 1); // ожидаем когда освободится место
    
    push_data_queue_b( &consol_tx, ch);
    
    if ( flag_consol_tx == 0 ){
        flag_consol_tx = 1;
        USART_ITConfig( COM_USART[ COM1 ], USART_IT_TXE, ENABLE);// start tx
    }
    
}

//******************************************************************************
// Print for console. (cn->call_print)
//******************************************************************************
void print_console(const char * s)
{
    uint32_t len = 0, i = 0;

    len = strlen(s);

    if (len){
        while( i != len ){
            tx_byte( s[i] );
            i++;
        }
    }
}

//******************************************************************************
// Настроить входа кнопки
//******************************************************************************
void key_init( void )
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    // включение тактирования порта вывода KEY
	RCC_APB2PeriphClockCmd(KEY_CLK, ENABLE);
	
	// инициализация вывода KEY
    GPIO_InitStructure.GPIO_Pin = KEY_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(KEY_PORT, &GPIO_InitStructure);
}

//******************************************************************************
// Получить состояние кнопки
//******************************************************************************
uint8_t key_pressed( void )
{

	if ( KEY_PORT->IDR & KEY_PIN )
		return KEY_UP;

    return KEY_DOWN;
}
