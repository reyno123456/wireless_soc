#ifndef __RTC_H__
#define __RTC_H__


#define RTC_CLK                             (32000)
#define RTC_BASE_ADDR                       (0x41000000)
#define RTC_CCVR_OFFSET                     (0x0)
#define RTC_CMR_OFFSET                      (0x4)
#define RTC_CLR_OFFSET                      (0x8)
#define RTC_CCR_OFFSET                      (0xC)
#define RTC_STAT_OFFSET                     (0x10)
#define RTC_RSTAT_OFFSET                    (0x14)
#define RTC_EOI_OFFSET                      (0x18)
#define RTC_INTER_COUNT                     (RTC_CLK)
#define RTC_UTC_ADDR                        (0x41100000)

typedef struct
{
    uint16_t  u16_year;
    uint8_t  u8_month;
    uint8_t  u8_day;
    uint8_t  u8_hour;
    uint8_t  u8_minute;
    uint8_t  u8_second;
    uint8_t  u8_week;
} STRU_RTC_CALENDAR;


void RTC_INIT(void);
uint8_t RTC_Get(STRU_RTC_CALENDAR *pst_rtcCalendar);
uint8_t RTC_Set(STRU_RTC_CALENDAR *pst_rtcCalendar);
uint32_t RTC_GlobalTimerCount(void);
void RTC_GlobalTimerINIT(void);

#endif