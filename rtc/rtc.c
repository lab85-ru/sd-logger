#include <stdint.h>
#include "stm32f10x.h"
#include "main.h"
#include "xprintf.h"
#include "time.h"
#include "config_mem.h"
#include "timer_lib.h"

struct tm rtc_tm;     // ��������� ����,�����

extern uint8_t char_buf[];   // ����� ��� ����������� � ������
extern uint32_t fat_time(const struct tm * timeptr);

/* Private function prototypes -----------------------------------------------*/
void rtc_configuration(void);
void rtc_time_adjust( const uint32_t t);
uint32_t rtc_get_count( void );
//------------------------------------------------------------------------------

//******************************************************************************
// ������ �������� ������� � � ��� ������
//******************************************************************************
void rtc_show( const uint32_t rtc_timer_sec )
{
    gmtime_r( &rtc_timer_sec, &rtc_tm);                  // rtc_timer_sec -> tm
    asctime_r( (const struct tm *)&rtc_tm, (char*)char_buf );   // tm -> ascii string
    xprintf( "%s\r\n", char_buf );                      // print data-time
}

//******************************************************************************
// �������� ������ (�������) � ��������������� ��������� ������� RTC (���� ��������)
//******************************************************************************
void rtc_setup(void)
{

    if (BKP_ReadBackupRegister(ADR_CFG) != 0xA5A5){
    /* Backup data register value is not correct or not yet programmed (when
       the first time the program is executed) */

        printf_d("STM32 RTC not yet configured....\r\n");

        /* RTC Configuration */
        rtc_configuration();

        printf_d("STM32 RTC configured....\r\n");

        // ���������� ����� �� ���������, 1 ������ 1900 ���� �.�. = 0
        RTC_SetCounter(0x0);
        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();

        BKP_WriteBackupRegister(ADR_CFG, 0xA5A5);
        
        config_mem_store_uint16(ADR_SPEED_L, 0x0); // ������ �������� ������ ����� = 0
        config_mem_store_uint16(ADR_SPEED_H, 0x0); // ������ �������� ������ ����� = 0
        
    }else{
        /* Check if the Power On Reset flag is set */
        if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET){
            printf_d("Power On Reset occurred....\r\n");
        }
        /* Check if the Pin Reset flag is set */
        else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET){
            printf_d("External Reset occurred....\r\n");
        }

        printf_d("No need to configure RTC....\r\n");
        /* Wait for RTC registers synchronization */
        RTC_WaitForSynchro();

        /* Enable the RTC Second */
        //RTC_ITConfig(RTC_IT_SEC, ENABLE);
        /* Wait until last write operation on RTC registers has finished */
        //RTC_WaitForLastTask();
        
    }

    /* Clear reset flags */
    RCC_ClearFlag();

  /* Display time in infinite loop */
    rtc_show( RTC_GetCounter() );
}


//******************************************************************************
// ��������� RTC ��� ������ ���������
//******************************************************************************
void rtc_configuration(void)
{
    uint32_t t = 0; // ��� ���c���� ��������
    
    /* Enable PWR and BKP clocks */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

    /* Allow access to BKP Domain */
    PWR_BackupAccessCmd(ENABLE);

    /* Reset Backup Domain */
    BKP_DeInit();

    /* Enable LSE */
    RCC_LSEConfig(RCC_LSE_ON);
    /* Wait till LSE is ready */
    t = 0;
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
    {
        _delay_ms(1000);
        t++;
        if (t == 6){  // ������(������) ��������� ��������� 32768 �� ����������� !
            while(1){
                printf_d("ERROR: 32768 Hz not started.\r\n");
                _delay_ms( 1000 );
            }// while
        }// if
    }

    /* Select LSE as RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

    /* Enable RTC Clock */
    RCC_RTCCLKCmd(ENABLE);

    /* Wait for RTC registers synchronization */
    RTC_WaitForSynchro();

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

    /* Enable the RTC Second */
    //RTC_ITConfig(RTC_IT_SEC, ENABLE);

    /* Wait until last write operation on RTC registers has finished */
    //RTC_WaitForLastTask();

    /* Set RTC prescaler: set RTC period to 1sec */
    RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
}

//******************************************************************************
// ��������� �������� ������
//******************************************************************************
void rtc_time_adjust( const uint32_t t)
{
    /* Enable PWR and BKP clocks */
    // �.�. ����� ������ ����� ������� � ������ �� ���������� !!! ����� ����� �������.
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

    /* Allow access to BKP Domain */
    PWR_BackupAccessCmd(ENABLE);

    /* Wait until last write operation on RTC registers has finished */
    //RTC_WaitForLastTask();
    /* Change the current time */
    RTC_SetCounter( t );
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
}

//******************************************************************************
// �������� ������� �������� �������� ������
//******************************************************************************
uint32_t rtc_get_count( void )
{
    return RTC_GetCounter();
}

//******************************************************************************
// date+time for FatFS
//******************************************************************************
DWORD get_fattime(void)
{
    uint32_t t;
    
    t = rtc_get_count();
    gmtime_r( &t, &rtc_tm);
    
    return (DWORD)fat_time( &rtc_tm );// ���������� ������� �����
}


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
