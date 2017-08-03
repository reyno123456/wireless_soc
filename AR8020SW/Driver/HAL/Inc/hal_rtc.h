/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_rtc.h
Description: this module contains the helper fucntions necessary to control the general
             purpose rtc block, rtc only run 37 hour on un-power. 
Author: Artosy Software Team
Version: 0.0.1
Date: 2017/06/30
History:
         0.0.1    2016/12/19    The initial version of hal_rtc.h
*****************************************************************************/

#ifndef __HAL_RTC_H__
#define __HAL_RTC_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "hal.h"

typedef struct
{
    uint16_t  u16_year;
    uint8_t  u8_month;
    uint8_t  u8_day;
    uint8_t  u8_hour;
    uint8_t  u8_minute;
    uint8_t  u8_second;
    uint8_t  u8_week;
} STRU_HAL_UTC_CALENDAR;

/**
* @brief  The rtc initialization function,detect RTC Enable or disable.if rtc disable ,set define value(2017/1/1 0:0:0).
* @param  none.
* @retval HAL_OK            means the initializtion is well done.  
*/
HAL_RET_T HAL_RTC_INIT(void);
/**
* @brief    get utc value
* @param    UTC struct
* @note     none
*/
HAL_RET_T HAL_UTC_Get(STRU_HAL_UTC_CALENDAR *pst_rtcCalendar);
/**
* @brief    set utc value
* @param    UTC struct
* @note     none
*/
HAL_RET_T HAL_UTC_Set(STRU_HAL_UTC_CALENDAR *pst_rtcCalendar);

/**
* @brief  The rtc initialization global timer.
* @param  none.
* @retval HAL_OK            means the initializtion is well done.  
*/
HAL_RET_T HAL_RTC_GlobalTimerINIT(void);

#ifdef __cplusplus
}
#endif

#endif