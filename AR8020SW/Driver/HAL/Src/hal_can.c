/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_can.c
Description: The external HAL APIs to use the CAN controller.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/29
History: 
        0.0.1    2016/12/29    The initial version of hal_can.c
*****************************************************************************/
#include "hal_can.h"
#include "hal_nvic.h"
#include "can.h"
#include "systicks.h"
#include "hal.h"
#include "debuglog.h"
#include <stdio.h>
#include "driver_mutex.h"

/**
* @brief   can controller initialization. 
* @param   st_halCanConfig        init need info.  
*                           
* @retval HAL_OK                  init can controller successed. 
*         HAL_CAN_ERR_INIT        init can controller failed.
*         HAL_CAN_ERR_COMPONENT   can channel error.init failed.
* @note   None. 
*/
HAL_RET_T HAL_CAN_Init(STRU_HAL_CAN_CONFIG *st_halCanConfig)
{
    uint8_t u8_canCh;

    if ((st_halCanConfig->e_halCanComponent) > HAL_CAN_COMPONENT_3)
    {
       return  HAL_CAN_ERR_COMPONENT;
    }

    if ( -1 == COMMON_driverMutexGet(MUTEX_CAN, st_halCanConfig->e_halCanComponent) )
    {
        dlog_error("fail, channel = %d", st_halCanConfig->e_halCanComponent);
        return HAL_OCCUPIED;
    }
    COMMON_driverMutexSet(MUTEX_CAN, (uint32_t)(st_halCanConfig->e_halCanComponent));

    u8_canCh = (uint8_t)(st_halCanConfig->e_halCanComponent);

    //first ,disable can_x interrupt.
    HAL_NVIC_DisableIrq(u8_canCh + HAL_NVIC_CAN_IRQ0_VECTOR_NUM);
    
    //record user function
    CAN_RegisterUserRxHandler(u8_canCh,\
                   (CAN_RcvMsgHandler)(st_halCanConfig->pfun_halCanRcvMsg));
    
    // hardware init
    CAN_InitHw((st_halCanConfig->e_halCanComponent), 
               (st_halCanConfig->e_halCanBaudr), 
               (st_halCanConfig->u32_halCanAcode), 
               (st_halCanConfig->u32_halCanAmask), 
               0x88,
               (st_halCanConfig->e_halCanFormat));

    //connect can interrupt service function
    HAL_NVIC_RegisterHandler(u8_canCh + HAL_NVIC_CAN_IRQ0_VECTOR_NUM, 
                           CAN_IntrSrvc, 
                           NULL);

    // enable can_x interrupt.
    if (NULL != (st_halCanConfig->pfun_halCanRcvMsg)) 
    {
        HAL_NVIC_EnableIrq(u8_canCh + HAL_NVIC_CAN_IRQ0_VECTOR_NUM);
    }

    return HAL_OK;
}

/**
* @brief  send can frame.include standard data frame,standard remote frame,
*         extended data frame, extended remote frame
* @param  st_halCanMsg       pointer to can message for send. 
*         u32_timeOut            timeout threshold, unit:ms                    
* @retval HAL_OK                can message send successed. 
*         HAL_CAN_ERR_SEND_MSG  can message send failed.
* @note   None. 
*/
HAL_RET_T HAL_CAN_Send(STRU_HAL_CAN_MSG *st_halCanMsg, uint32_t u32_timeOut)
{
    uint32_t start;
    volatile uint32_t tmpCnt = 100;
    
    if (CAN_GetTxBusyStatus(st_halCanMsg->e_halCanComponent))
    {
        return HAL_BUSY; 
    }
    
    CAN_Send((st_halCanMsg->e_halCanComponent), 
              (st_halCanMsg->u32_halCanId), 
              &(st_halCanMsg->u8_halCanDataArray[0]), 
              (st_halCanMsg->u8_halCanDataLen), 
              (st_halCanMsg->e_halCanFormat), 
              (st_halCanMsg->e_halCanType));

    if (0 != u32_timeOut)
    {
        while(tmpCnt--);
        
        start = SysTicks_GetTickCount();
        while (CAN_GetTxBusyStatus(st_halCanMsg->e_halCanComponent))
        {
            if ((SysTicks_GetDiff(start, SysTicks_GetTickCount())) >= u32_timeOut)
            {
                 dlog_info("can %dtime out.", st_halCanMsg->e_halCanComponent);
                 return HAL_TIME_OUT;
            }
            SysTicks_DelayUS(100);
        }
    }
    
    return HAL_OK;
}
