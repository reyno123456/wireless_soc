/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_bb.c
Description: The external HAL APIs to use the I2C controller.
Author: Artosy Software Team
Version: 0.0.1
Date: 2016/12/20
History:
        0.0.1    2017/02/06    The initial version of hal_bb_sky.c
*****************************************************************************/


#include <stdio.h>
#include <stdint.h>
#include "sys_event.h"
#include "bb_spi.h"
#include "bb_ctrl.h"
#include "bb_sky_ctrl.h"
#include "hal_bb.h"

#include "debuglog.h"
#include "bb_customerctx.h"


/** 
 * @brief   init baseband to sky mode
 * @param   NONE
 * @return  HAL_OK:                         means init baseband 
 *          HAL_BB_ERR_INIT:                means some error happens in init session 
 */
HAL_RET_T HAL_BB_InitSky(STRU_CUSTOMER_CFG *stru_customerCfg )
{
    if (NULL == stru_customerCfg|| NULL == stru_customerCfg->pstru_boardCfg)
    {
        BB_init( BB_SKY_MODE, &stru_boardCfg, stru_customerCfg);
    }
    else
    {
        BB_init( BB_SKY_MODE, stru_customerCfg->pstru_boardCfg , stru_customerCfg );
    }

    BB_SKY_start();

    return HAL_OK;
}


/** 
 * @brief   Set baseband sky to auto search the ground RC id
 * @param   NONE
 * @retval  HAL_OK:                    means command is sent sucessfully. 
            HAL_BB_ERR_EVENT_NOTIFY:   means error happens in sending the command to cpu2
 * @note    
 */
HAL_RET_T HAL_BB_SetAutoSearchRcIdProxy(void)
{
    uint8_t u8_ret;
    STRU_WIRELESS_CONFIG_CHANGE st_cmd;

    st_cmd.u8_configClass  = WIRELESS_AUTO_SEARCH_ID;
    st_cmd.u8_configItem   = 0;
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
