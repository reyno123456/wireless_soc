/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_camera.c
Description: The external HAL APIs to use the camera.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2017/01/05
History: 
        0.0.1    2017/01/05    The initial version of hal_camera.c
*****************************************************************************/

#include "hal_camera.h"
#include "ov5640.h"
#include "debuglog.h"
#include "hal_i2c.h"


/**
* @brief   
* @param   
*                           
* @retval HAL_OK             means the initializtion is well done.
*         HAL_CAMERA_ERR_INIT  means some error happens in the initializtion.
* @note   
*/
HAL_RET_T HAL_CAMERA_Init(ENUM_HAL_CAMERA_FRAME_RATE e_cameraFrameRate, 
                          ENUM_HAL_CAMERA_MODE e_cameraMode)
{
    HAL_I2C_MasterInit(OV5640_COMPONENT, OV5640_I2C_ADDR, HAL_I2C_FAST_SPEED);
    OV5640_Init((ENUM_OV5640_FRAME_RATE)e_cameraFrameRate,
                (ENUM_OV5640_MODE)e_cameraMode);

    return HAL_OK;
}

/**
* @brief   
* @param   
* @retval 
* @note   
*/
HAL_RET_T HAL_CAMERA_ReadReg(uint16_t u16_regAddr, uint8_t *pu8_val)
{
    OV5640_ReadReg(u16_regAddr, pu8_val);

    return HAL_OK;
}

/**
* @brief   
* @param   
* @retval 
* @note   
*/
HAL_RET_T HAL_CAMERA_WriteReg(uint16_t u16_regAddr, uint8_t u8_val)
{
    OV5640_WriteReg(u16_regAddr, u8_val);

    return HAL_OK;
}

/**
* @brief    
* @param     
* @retval  
* @note    
*/
HAL_RET_T HAL_CAMERA_GetImageInfo(uint16_t *u16_width, uint16_t *u16_hight, uint8_t *u8_frameRate)
{
    int32_t s_result;
    
    s_result = OV5640_GetImageInfo(u16_width, u16_hight, u8_frameRate);
    
    if (0 == s_result)
    {
        return HAL_OK;
    }
    else
    {
        return HAL_CAMERAQ_ERR;
    }
}

