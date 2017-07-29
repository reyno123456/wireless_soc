/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_h264.h
Description: The external HAL APIs to use the H264 encoder.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/20
History: 
        0.0.1    2016/12/20    The initial version of hal_h264.h
*****************************************************************************/

#ifndef __HAL_H264_H__
#define __HAL_H264_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "hal_ret_type.h"

typedef enum
{
    HAL_H264_BITRATE_8M = 0,
    HAL_H264_BITRATE_1M,
    HAL_H264_BITRATE_2M,
    HAL_H264_BITRATE_3M,
    HAL_H264_BITRATE_4M,
    HAL_H264_BITRATE_5M,
    HAL_H264_BITRATE_6M,
    HAL_H264_BITRATE_7M,
    HAL_H264_BITRATE_500K,
    HAL_H264_BITRATE_9M,
    HAL_H264_BITRATE_10M,
    HAL_H264_BITRATE_11M,
    HAL_H264_BITRATE_12M,
    HAL_H264_BITRATE_13M,
    HAL_H264_BITRATE_14M,
    HAL_H264_BITRATE_15M,
    HAL_H264_BITRATE_16M,
    HAL_H264_BITRATE_17M,
    HAL_H264_BITRATE_18M,
    HAL_H264_BITRATE_19M,
    HAL_H264_BITRATE_20M,
    HAL_H264_BITRATE_21M,
    HAL_H264_BITRATE_22M,
    HAL_H264_BITRATE_23M,
    HAL_H264_BITRATE_24M,
    HAL_H264_BITRATE_25M,
    HAL_H264_BITRATE_26M,
    HAL_H264_BITRATE_27M,
    HAL_H264_BITRATE_28M,
    HAL_H264_BITRATE_29M,
    HAL_H264_BITRATE_30M
} ENUM_HAL_H264_BITRATE;

typedef struct
{
    uint8_t               u8_view0En;
    uint8_t               u8_view0Gop;
    ENUM_HAL_H264_BITRATE e_view0Br;
    uint8_t               u8_view0BrEn;
    uint8_t               u8_view1En;
    uint8_t               u8_view1Gop;
    ENUM_HAL_H264_BITRATE e_view1Br;
    uint8_t               u8_view1BrEn;
} STRU_HAL_H264_CONFIG;

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

HAL_RET_T HAL_H264_Init(STRU_HAL_H264_CONFIG st_h264Cfg);


#ifdef __cplusplus
}
#endif

#endif

