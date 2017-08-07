#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "interrupt.h"
#include "reg_rw.h"
#include "debuglog.h"
#include "rtc.h"


const uint8_t table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5};

const uint8_t leapmon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};
const uint8_t mon_table[12]={31,29,31,30,31,30,31,31,30,31,30,31};

static void rtc_handle(uint32_t u32_vectorNum) 
{
    write_reg32((uint32_t *)(RTC_BASE_ADDR + RTC_CLR_OFFSET), 0);    
    read_reg32((uint32_t *)(RTC_BASE_ADDR + RTC_EOI_OFFSET));
    uint32_t u32_UTC = read_reg32((uint32_t *)(RTC_UTC_ADDR));
    u32_UTC += RTC_INTER_COUNT/RTC_CLK;

    write_reg32((uint32_t *)(RTC_UTC_ADDR), u32_UTC); 

}

void RTC_disable(void)
{
    INTR_NVIC_DisableIRQ(RTC_INTR_VECTOR_NUM);
    write_reg32((uint32_t *)(RTC_BASE_ADDR + RTC_CCR_OFFSET), 0x00);
    write_reg32((uint32_t *)(RTC_BASE_ADDR + RTC_CLR_OFFSET), 0x00);
    write_reg32((uint32_t *)RTC_UTC_ADDR, 0);
}

void RTC_enable(void)
{
    uint32_t u32_priorityGroup = 0x00;

    u32_priorityGroup = INTR_NVIC_GetPriorityGrouping();

    INTR_NVIC_SetIRQPriority(RTC_INTR_VECTOR_NUM, INTR_NVIC_EncodePriority(u32_priorityGroup, INTR_NVIC_PRIORITY_RTC, 0));

    reg_IrqHandle(RTC_INTR_VECTOR_NUM, rtc_handle, NULL);

    write_reg32((uint32_t *)(RTC_BASE_ADDR + RTC_CCR_OFFSET), 0x00);
    write_reg32((uint32_t *)(RTC_BASE_ADDR + RTC_CLR_OFFSET), 0x00);
    write_reg32((uint32_t *)(RTC_BASE_ADDR + RTC_CMR_OFFSET), RTC_INTER_COUNT);
    write_reg32((uint32_t *)(RTC_BASE_ADDR + RTC_CCR_OFFSET), 0x0D);

    INTR_NVIC_EnableIRQ(RTC_INTR_VECTOR_NUM);

}


uint8_t RTC_detectRTCEnable(void)
{
    uint32_t u32_UTC = read_reg32((uint32_t *)(RTC_UTC_ADDR));
    uint32_t u32_rtcCountCCVR = read_reg32((uint32_t *)(RTC_BASE_ADDR + RTC_CCVR_OFFSET));
    if ((0 != u32_UTC) || (0 != u32_rtcCountCCVR))
    {     
        return 1;
    }
    else
    {
        return 0;
    }
}



void RTC_SetCounter(uint32_t u32_startTime)
{
    RTC_disable();
    write_reg32((uint32_t *)RTC_UTC_ADDR, u32_startTime);
    RTC_enable();
}

uint8_t RTC_Leap_Year(uint16_t u16_year)
{
    if(u16_year%4 == 0)
    {
        if(u16_year%100 == 0)
        {
            if(u16_year%400 == 0)
            {
                return 1;
            }
            else 
            {
                return 0;
            }
        }
        else 
        {
            return 1;
        }
    }
    else 
    {
        return 0;
    }
}


uint8_t RTC_Set(STRU_RTC_CALENDAR *pst_rtcCalendar)
{
    uint16_t t;
    uint32_t seccount=0;
    if((pst_rtcCalendar->u16_year < 1970 )|| (pst_rtcCalendar->u16_year > 2099))
    {
        return 1;
    }
    for(t = 1970; t < pst_rtcCalendar->u16_year; t++)
    {
        if(RTC_Leap_Year(t))
        {
            seccount+=31622400;
        }
        else 
        {
            seccount+=31536000;
        }
    }

    pst_rtcCalendar->u8_month-=1;

    if (RTC_Leap_Year(pst_rtcCalendar->u16_year))
    {
        for(t = 0; t < pst_rtcCalendar->u8_month; t++)
        {
            seccount+=(uint32_t)leapmon_table[t]*86400;
        }
    }
    else
    {
        for(t = 0; t < pst_rtcCalendar->u8_month; t++)
        {
            seccount+=(uint32_t)mon_table[t]*86400;
        }
    }

    seccount+=(uint32_t)(pst_rtcCalendar->u8_day-1)*86400;
    seccount+=(uint32_t)pst_rtcCalendar->u8_hour*3600;
    seccount+=(uint32_t)pst_rtcCalendar->u8_minute*60;
    seccount+=pst_rtcCalendar->u8_second;

    RTC_SetCounter(seccount);
    dlog_info("seccount %d",seccount);

    return 0;
}

uint8_t RTC_Get(STRU_RTC_CALENDAR *pst_rtcCalendar)
{
    uint16_t u32_daycnt=0;
    uint32_t u32_timecount=0;
    uint32_t temp=0;
    uint16_t temp1=0;

    u32_timecount =  read_reg32((uint32_t *)(RTC_UTC_ADDR));    
    temp=u32_timecount/86400;

    if(u32_daycnt != temp)
    {
        u32_daycnt = temp;
        temp1 = 1970; 
        while(temp >= 365)
        {
            if(RTC_Leap_Year(temp1))
            {
                if(temp >= 366)
                {
                    temp -= 366;
                }
                else 
                {
                    temp1++;
                    break;
                }
            }
            else 
            {
                temp-=365; 
            }
            temp1++;
        }

        pst_rtcCalendar->u16_year = temp1;
        pst_rtcCalendar->u8_month = 0;

        if (RTC_Leap_Year(pst_rtcCalendar->u16_year))
        {
            while(temp >= leapmon_table[pst_rtcCalendar->u8_month])
            {
                temp-=leapmon_table[pst_rtcCalendar->u8_month];
                pst_rtcCalendar->u8_month++;
            }
        }
        else
        {
            while(temp >= mon_table[pst_rtcCalendar->u8_month])
            {
                temp-=mon_table[pst_rtcCalendar->u8_month];
                pst_rtcCalendar->u8_month++;
            }
        }
        pst_rtcCalendar->u8_month++;
        pst_rtcCalendar->u8_day = temp + 1;
    }

    temp=u32_timecount%86400; 
    pst_rtcCalendar->u8_hour=temp/3600; 
    pst_rtcCalendar->u8_minute=(temp%3600)/60; 
    pst_rtcCalendar->u8_second=(temp%3600)%60; 
    
    return 0;
}


void RTC_INIT(void)
{
    uint32_t u32_priorityGroup = 0x00;

    u32_priorityGroup = INTR_NVIC_GetPriorityGrouping();

    INTR_NVIC_SetIRQPriority(RTC_INTR_VECTOR_NUM, INTR_NVIC_EncodePriority(u32_priorityGroup, INTR_NVIC_PRIORITY_RTC, 0));

    reg_IrqHandle(RTC_INTR_VECTOR_NUM, rtc_handle, NULL);

    write_reg32((uint32_t *)(RTC_BASE_ADDR + RTC_CCR_OFFSET), 0x00);
    write_reg32((uint32_t *)(RTC_BASE_ADDR + RTC_CLR_OFFSET), 0x00);
    write_reg32((uint32_t *)(RTC_BASE_ADDR + RTC_CMR_OFFSET), RTC_INTER_COUNT);
    write_reg32((uint32_t *)(RTC_BASE_ADDR + RTC_CCR_OFFSET), 0x0D);

    INTR_NVIC_EnableIRQ(RTC_INTR_VECTOR_NUM);
    
    if (1 != RTC_detectRTCEnable())
    {        
        STRU_RTC_CALENDAR *pst_rtcCalendar = (STRU_RTC_CALENDAR *)malloc(sizeof(STRU_RTC_CALENDAR) * sizeof(uint8_t));
        pst_rtcCalendar->u16_year = 2017;
        pst_rtcCalendar->u8_month = 1;
        pst_rtcCalendar->u8_day = 1;
        pst_rtcCalendar->u8_hour = 0;
        pst_rtcCalendar->u8_minute = 0;
        pst_rtcCalendar->u8_second = 0;
        RTC_Set(pst_rtcCalendar);
        free(pst_rtcCalendar);
    }

}

void RTC_GlobalTimerINIT(void)
{   
    write_reg32((uint32_t *)(RTC_BASE_ADDR + RTC_CCR_OFFSET), 0x00);
    write_reg32((uint32_t *)(RTC_BASE_ADDR + RTC_CLR_OFFSET), 0x00);
    write_reg32((uint32_t *)(RTC_BASE_ADDR + RTC_CCR_OFFSET), 0x0C);
}

uint32_t RTC_GlobalTimerCount(void)
{   
    uint32_t u32_rtcCountCCVR = read_reg32((uint32_t *)(RTC_BASE_ADDR + RTC_CCVR_OFFSET));
    return (u32_rtcCountCCVR/32);
}
