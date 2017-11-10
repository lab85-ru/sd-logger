#ifndef _RTC_H_
#define _RTC_H_

void rtc_show( uint32_t TimeVar );
void rtc_setup(void);
void rtc_configuration(void);
void rtc_time_adjust( const uint32_t t);
uint32_t rtc_get_count( void );
DWORD get_fattime(void);        // for FatFS

#endif