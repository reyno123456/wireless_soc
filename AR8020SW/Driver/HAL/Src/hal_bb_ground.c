/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_bb.c
Description: The external HAL APIs to use the I2C controller.
Author: Artosy Software Team
Version: 0.0.1
Date: 2016/12/20
History:
        0.0.1    2017/02/06    The initial version of hal_bb_ground.c
*****************************************************************************/


#include <stdio.h>
#include <stdint.h>

#include "debuglog.h"
#include "sys_event.h"
#include "bb_ctrl.h"
#include "bb_grd_ctrl.h"
#include "hal_bb.h"
#include "bb_customerctx.h"


/** 
 * @brief   init baseband to ground mode
 * @param   NONE
 * @return  HAL_OK:                         means init baseband 
 *          HAL_BB_ERR_INIT:                means some error happens in init session 
 */
HAL_RET_T HAL_BB_InitGround( STRU_CUSTOMER_CFG *stru_customerCfg )
{
    if (NULL == stru_customerCfg || NULL == stru_customerCfg->pstru_boardCfg)
    {
        BB_init(BB_GRD_MODE, &stru_boardCfg, stru_customerCfg);
    }
    else
    {
        BB_init(BB_GRD_MODE, stru_customerCfg->pstru_boardCfg , stru_customerCfg);
    }

    BB_GRD_start();

    return HAL_OK;
}

/** 
 * @brief       force baseband calculation distance zero calibration.
 * @param       none.
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1         
 */
HAL_RET_T HAL_BB_CalcDistZeroCalibration(void)
{
    uint8_t u8_ret;
    STRU_WIRELESS_CONFIG_CHANGE st_cmd;
    
    st_cmd.u8_configClass  = WIRELESS_OTHER;
    st_cmd.u8_configItem   = CALC_DIST_ZERO_CALI;
    st_cmd.u32_configValue = 0;

    u8_ret = SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&st_cmd);
    if( u8_ret )
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_EVENT_NOTIFY;
    }
}

/** 
 * @brief       set baseband calculation distance zero point.
 * @param       none.
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1         
 */
HAL_RET_T HAL_BB_SetCalcDistZeroPoint(uint32_t value)
{
    uint8_t u8_ret;
    STRU_WIRELESS_CONFIG_CHANGE st_cmd;
    
    st_cmd.u8_configClass  = WIRELESS_OTHER;
    st_cmd.u8_configItem   = SET_CALC_DIST_ZERO_POINT;
    st_cmd.u32_configValue = value;

    u8_ret = SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&st_cmd);
    if( u8_ret )
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_EVENT_NOTIFY;
    }
}
