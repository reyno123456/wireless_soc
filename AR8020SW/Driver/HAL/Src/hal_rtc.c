/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_rtc.c
Description: this module contains the helper fucntions necessary to control the general
             purpose rtc block
Author: Artosy Software Team
Version: 0.0.1
Date: 2017/06/30
History:
         0.0.1    2016/12/19    The initial version of hal_rtc.c
*****************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "hal_ret_type.h"
#include "debuglog.h"
#include "hal_rtc.h"
#include "rtc.h"

HAL_RET_T HAL_RTC_INIT(void)
{
    RTC_INIT();
    STRU_HAL_UTC_CALENDAR *pst_halRtcCalendar = (STRU_HAL_UTC_CALENDAR *)malloc(sizeof(STRU_HAL_UTC_CALENDAR) * sizeof(uint8_t));
    HAL_UTC_Get(pst_halRtcCalendar);
    dlog_info("utc %d_%d_%d %d:%d:%d ", pst_halRtcCalendar->u16_year,
                                        pst_halRtcCalendar->u8_month,
                                        pst_halRtcCalendar->u8_day,
                                        pst_halRtcCalendar->u8_hour,
                                        pst_halRtcCalendar->u8_minute,
                                        pst_halRtcCalendar->u8_second);
    free(pst_halRtcCalendar);
    return HAL_OK;
}

HAL_RET_T HAL_UTC_Get(STRU_HAL_UTC_CALENDAR *pst_halRtcCalendar)
{
    RTC_Get((STRU_RTC_CALENDAR *)pst_halRtcCalendar);
    return HAL_OK;
}

HAL_RET_T HAL_UTC_Set(STRU_HAL_UTC_CALENDAR *pst_halRtcCalendar)
{
    RTC_Set((STRU_RTC_CALENDAR *)pst_halRtcCalendar);
    return HAL_OK;
}

HAL_RET_T HAL_RTC_GlobalTimerINIT(void)
{
    RTC_GlobalTimerINIT();
    return HAL_OK;
}

