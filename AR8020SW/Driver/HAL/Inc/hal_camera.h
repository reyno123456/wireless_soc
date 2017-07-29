/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_camera.h
Description: The external HAL APIs to use the camera.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2017/01/05
History: 
        0.0.1    2017/01/05    The initial version of hal_camera.h
*****************************************************************************/
#ifndef __HAL_CAMERA_H__
#define __HAL_CAMERA_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "hal_ret_type.h"


typedef enum
{
    HAL_CAMERA_MODE_VGA_640_480      = 0,
    HAL_CAMERA_MODE_QVGA_320_240     = 1,
    HAL_CAMERA_MODE_NTSC_720_480     = 2,
    HAL_CAMERA_MODE_PAL_720_576      = 3,
    HAL_CAMERA_MODE_720P_1280_720    = 4,
    HAL_CAMERA_MODE_1080P_1920_1080  = 5,
    HAL_CAMERA_MODE_QSXGA_2592_1944  = 6,
    HAL_CAMERA_MODE_QCIF_176_144     = 7,
    HAL_CAMERA_MODE_XGA_1024_768     = 8,
} ENUM_HAL_CAMERA_MODE;


typedef enum  
{
    HAL_CAMERA_15_FPS,
    HAL_CAMERA_30_FPS
} ENUM_HAL_CAMERA_FRAME_RATE;

/**
* @brief   
* @param   
*                           
* @retval HAL_OK             means the initializtion is well done.
*         HAL_CAMERA_ERR_INIT  means some error happens in the initializtion.
* @note   
*/
HAL_RET_T HAL_CAMERA_Init(ENUM_HAL_CAMERA_FRAME_RATE e_cameraFrameRate, 
                          ENUM_HAL_CAMERA_MODE e_cameraMode);

/**
* @brief   
* @param   
* @retval 
* @note   
*/
HAL_RET_T HAL_CAMERA_ReadReg(uint16_t u16_regAddr, uint8_t *pu8_val);

/**
* @brief   
* @param   
* @retval 
* @note   
*/
HAL_RET_T HAL_CAMERA_WriteReg(uint16_t u16_regAddr, uint8_t u8_val);

/**
* @brief    
* @param     
* @retval  
* @note    
*/
HAL_RET_T HAL_CAMERA_GetImageInfo(uint16_t *u16_width, uint16_t *u16_hight, uint8_t *u8_frameRate);


#ifdef __cplusplus
}
#endif

#endif
