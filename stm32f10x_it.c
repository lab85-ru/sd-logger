/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "stm32f10x_it.h"
#include "hardware.h"
#include "main.h"
#include "queue_buf.h"
#include "xprint_fault.h"
#include "sys_timer.h"

void hard_fault_handler_c(unsigned int * hardfault_args);

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

extern queue_buf_t consol_rx;
extern tfile_st file_st;
extern queue_buf_t consol_tx;
extern uint8_t flag_consol_tx;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/
/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
/*	asm volatile (
        	"TST LR, #4           \n"
	        "ITE EQ               \n"
        	"MRSEQ R0, MSP        \n"
	        "MRSNE R0, PSP        \n"
        	"B nmi_fault_handler_c    "
	);
  */
	while (1);
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
	asm volatile (
        	"TST LR, #4           \n"
	        "ITE EQ               \n"
        	"MRSEQ R0, MSP        \n"
	        "MRSNE R0, PSP        \n"
        	"B hard_fault_handler_c"
	);
     
	while (1);
}

void hard_fault_handler_c(unsigned int * hardfault_args)
{
//	xprint_str("\r\nhard_fault_handler!\r\nPC= 0x");
//	xprint_hexbyte( hardfault_args[6]>>24 );
//	xprint_hexbyte( hardfault_args[6]>>16 );
//	xprint_hexbyte( hardfault_args[6]>>8 );
//	xprint_hexbyte( hardfault_args[6] );
	unsigned int stacked_r0;
	unsigned int stacked_r1;
	unsigned int stacked_r2;
	unsigned int stacked_r3;
	unsigned int stacked_r12;
	unsigned int stacked_lr;
	unsigned int stacked_pc;
	unsigned int stacked_psr;

	stacked_r0 = ((unsigned long) hardfault_args[0]);
	stacked_r1 = ((unsigned long) hardfault_args[1]);
	stacked_r2 = ((unsigned long) hardfault_args[2]);
	stacked_r3 = ((unsigned long) hardfault_args[3]);

	stacked_r12 = ((unsigned long) hardfault_args[4]);
	stacked_lr = ((unsigned long) hardfault_args[5]);
	stacked_pc = ((unsigned long) hardfault_args[6]);
	stacked_psr = ((unsigned long) hardfault_args[7]);

	xprint_str("\r\n[Hard fault handler]\r\n");

	xprint_str("R0 = ");
	xprint_hexuint32(stacked_r0);
	xprint_str("\r\n");

	xprint_str("R1 = ");
	xprint_hexuint32(stacked_r1);
	xprint_str("\r\n");

	xprint_str("R2 = ");
	xprint_hexuint32(stacked_r2);
	xprint_str("\r\n");

	xprint_str("R3 = ");
	xprint_hexuint32(stacked_r3);
	xprint_str("\r\n");

	xprint_str("R12 = ");
	xprint_hexuint32(stacked_r12);
	xprint_str("\r\n");

	xprint_str("LR [R14] subroutine call return address = ");
	xprint_hexuint32(stacked_lr);
	xprint_str("\r\n");

	xprint_str("PC [R15] program counter = ");
	xprint_hexuint32(stacked_pc);
	xprint_str("\r\n");

	xprint_str("PSR = "); 
	xprint_hexuint32(stacked_psr); 
	xprint_str("\r\n");
/*
printf ("BFAR = %x\n", (*((volatile unsigned long *)(0xE000ED38))));
printf ("CFSR = %x\n", (*((volatile unsigned long *)(0xE000ED28))));
printf ("HFSR = %x\n", (*((volatile unsigned long *)(0xE000ED2C))));
printf ("DFSR = %x\n", (*((volatile unsigned long *)(0xE000ED30))));
printf ("AFSR = %x\n", (*((volatile unsigned long *)(0xE000ED3C))));
printf ("SCB_SHCSR = %x\n", SCB->SHCSR);
*/
	while(1);
}
/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
/*	asm volatile (
        	"TST LR, #4           \n"
	        "ITE EQ               \n"
        	"MRSEQ R0, MSP        \n"
	        "MRSNE R0, PSP        \n"
        	"B mem_manage_fault_handler_c    "
	);*/
	while (1);
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
/* 	asm volatile (
        	"TST LR, #4           \n"
	        "ITE EQ               \n"
        	"MRSEQ R0, MSP        \n"
	        "MRSNE R0, PSP        \n"
        	"B bus_fault_handler_c    "
	);*/
	while (1);
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
/*	asm volatile (
        	"TST LR, #4           \n"
	        "ITE EQ               \n"
        	"MRSEQ R0, MSP        \n"
	        "MRSNE R0, PSP        \n"
        	"B usage_fault_handler_c    "
	);
  */
	while (1);
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
	//vPortSVCHandler();
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
	//xPortPendSVHandler();
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	//xPortSysTickHandler();
	sys_tick_handler();
}

/*******************************************************************************
* Function Name  : SDIO_IRQHandler
* Description    : This function handles SDIO global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
// SDIO_IRQHandler(void)  ->  sdio_stm32f1
//void SDIO_IRQHandler(void)
//{
	/* Process All SDIO Interrupt Sources */
//	SD_ProcessIRQSrc();
//}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
//------------------------------------------------------------------------------
// Consol
//------------------------------------------------------------------------------
void USART1_IRQHandler(void)
{
    uint8_t c,t;
    QUEUE_VAR_TYPE dl;

    if(USART_GetITStatus(H_COM1, USART_IT_RXNE) != RESET){
        c = (USART_ReceiveData(H_COM1) & 0xFF);
        dl = get_free_size_queue( &consol_rx );
	if ( dl >=  2 ){
            push_data_queue_b( &consol_rx, c);
        }
    }

    if(USART_GetITStatus(H_COM1, USART_IT_TXE) != RESET){
        if ( get_data_size_queue( &consol_tx ) > 0 ){
            pop_data_queue_b( &consol_tx, &t );
            USART_SendData(H_COM1, t);
        }else{
            flag_consol_tx = 0;
            USART_ITConfig(H_COM1, USART_IT_TXE, DISABLE);
        }
    }
}

//------------------------------------------------------------------------------
// rx_2
//------------------------------------------------------------------------------
void USART2_IRQHandler(void)
{
    uint8_t c;
    QUEUE_VAR_TYPE dl;

    if(USART_GetITStatus(H_COM2, USART_IT_RXNE) != RESET){
        c = (USART_ReceiveData(H_COM2) & 0xFF);
        dl = get_free_size_queue( &file_st.q_buf[ 0 ] );
	if ( dl >=  2 ){
            push_data_queue_b( &file_st.q_buf[ 0 ], c);
        }
    }

    if(USART_GetITStatus(H_COM2, USART_IT_TXE) != RESET){   
        USART_ITConfig(H_COM2, USART_IT_TXE, DISABLE);
    }
}

//------------------------------------------------------------------------------
// rx_3
//------------------------------------------------------------------------------
void USART3_IRQHandler(void)
{
    uint8_t c;
    QUEUE_VAR_TYPE dl;

    if(USART_GetITStatus(H_COM3, USART_IT_RXNE) != RESET){
        c = (USART_ReceiveData(H_COM3) & 0xFF);
        dl = get_free_size_queue( &file_st.q_buf[ 1 ] );
	if ( dl >=  2 ){
            push_data_queue_b( &file_st.q_buf[ 1 ], c);
        }
    }

    if(USART_GetITStatus(H_COM3, USART_IT_TXE) != RESET){   
        USART_ITConfig(H_COM3, USART_IT_TXE, DISABLE);
    }
}


/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
