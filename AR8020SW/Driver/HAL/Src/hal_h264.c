/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_h264.c
Description: The external HAL APIs to use the H264 encoder.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/20
History: 
        0.0.1    2016/12/20    The initial version of hal_h264.c
*****************************************************************************/

#include <stdint.h>
#include "h264_encoder.h"
#include "hal_h264.h"
#include "hal_ret_type.h"

/**
* @brief  The H264 encoder initialization function.
* @param  st_h264Cfg    The H264 encoder configuration parameters
*                       u8_view0En:   0 - view0 disable; 1 - view0 enable.
*                       u8_view0Gop:  view0 GOP.
*                       e_view0Br:    view0 default bitrate.
*                       u8_view0BrEn: 0 - view0 dynamic bitrate disable; 0 - view0 dynamic bitrate enable. 
*                       u8_view1En:   0 - view1 disable; 1 - view1 enable.
*                       u8_view1Gop:  view1 GOP.
*                       e_view0Br:    view1 default bitrate.
*                       u8_view1BrEn: 0 - view1 dynamic bitrate disable; 0 - view1 dynamic bitrate enable.
*         u16_i2cAddr       16 bit I2C address of the target device.
*         e_i2cSpeed        The I2C speed of the I2C clock of the I2C controller, the right value
*                           should be standard (<100Kb/s), fast (<400Kb/s) and high(<3.4Mb/s).
* @retval HAL_OK            means the initializtion is well done.
*         HAL_I2C_ERR_INIT  means some error happens in the initializtion.
* @note   Master mode only
*         High speed mode has some system dependency and is especially affected by the circuit capacity
*         and the I2C slave device ability.
*/

HAL_RET_T HAL_H264_Init(STRU_HAL_H264_CONFIG st_h264Cfg)
{
    H264_Encoder_Init(st_h264Cfg.u8_view0Gop, 
                      st_h264Cfg.e_view0Br, 
                      st_h264Cfg.u8_view0BrEn, 
                      st_h264Cfg.u8_view1Gop,
                      st_h264Cfg.e_view1Br,
                      st_h264Cfg.u8_view1BrEn);

    return HAL_OK;
}

