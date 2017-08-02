/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_i2c.c
Description: The external HAL APIs to use the I2C controller.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/20
History: 
        0.0.1    2016/12/20    The initial version of hal_i2c.c
*****************************************************************************/

#include <stdint.h>
#include "i2c.h"
#include "hal_i2c.h"
#include "hal_ret_type.h"
#include "hal_nvic.h"
#include "systicks.h"
#include "hal.h"
#include "interrupt.h"
#include "debuglog.h"
#include "driver_mutex.h"


/**
* @brief  The I2C initialization function which must be called before using the I2C controller.
* @param  e_i2cComponent    The I2C controller number, the right number should be 0-4 and totally
*                           5 I2C controllers can be used by application.
*         u16_i2cAddr       16 bit I2C address of the target device.
*         e_i2cSpeed        The I2C speed of the I2C clock of the I2C controller, the right value
*                           should be standard (<100Kb/s), fast (<400Kb/s) and high(<3.4Mb/s).
* @retval HAL_OK            means the initializtion is well done.
*         HAL_I2C_ERR_INIT  means some error happens in the initializtion.
* @note   Master mode only
*         High speed mode has some system dependency and is especially affected by the circuit capacity
*         and the I2C slave device ability.
*/

HAL_RET_T HAL_I2C_MasterInit(ENUM_HAL_I2C_COMPONENT e_i2cComponent, 
                             uint16_t u16_i2cAddr,
                             ENUM_HAL_I2C_SPEED e_i2cSpeed)
{
    EN_I2C_COMPONENT en_component;
    ENUM_I2C_Speed en_speed;
    uint8_t u8_i2cVecNum;

    switch (e_i2cComponent)
    {
        case HAL_I2C_COMPONENT_0:
            en_component = I2C_Component_0;
            break;
        case HAL_I2C_COMPONENT_1:
            en_component = I2C_Component_1;
            break;
        case HAL_I2C_COMPONENT_2:
            en_component = I2C_Component_2;
            break;
        case HAL_I2C_COMPONENT_3:
            en_component = I2C_Component_3;
            break;
        case HAL_I2C_COMPONENT_4:
            en_component = I2C_Component_4;
            break;
        case HAL_I2C_COMPONENT_5:
            en_component = I2C_Component_5;
            break;
        default:
            return HAL_I2C_ERR_INIT;
    }

    switch (e_i2cSpeed)
    {
    case HAL_I2C_STANDARD_SPEED:
        en_speed = I2C_Standard_Speed;
        break;
    case HAL_I2C_FAST_SPEED:
        en_speed = I2C_Fast_Speed;
        break;
    case HAL_I2C_HIGH_SPEED:
        en_speed = I2C_High_Speed;
        break;
    default:
        return HAL_I2C_ERR_INIT;
    }

    if ( -1 == driver_mutex_get(mutex_i2c, e_i2cComponent) )
    {
        dlog_error("fail, channel = %d", e_i2cComponent);
        return HAL_OCCUPIED;
    }
    driver_mutex_set(mutex_i2c, (uint32_t)e_i2cComponent);

    if (HAL_I2C_COMPONENT_5 == e_i2cComponent)
    {
        u8_i2cVecNum = HAL_NVIC_VIDEO_I2C_INTR_VIDEO_VECTOR_NUM;
    }
    else if (HAL_I2C_COMPONENT_4 == e_i2cComponent)
    {
        u8_i2cVecNum = HAL_NVIC_I2C_SLV_INTR_VECTOR_NUM;
    }
    else
    {
        u8_i2cVecNum = e_i2cComponent + HAL_NVIC_I2C_INTR0_VECTOR_NUM;
    }
    
    HAL_NVIC_SetPriority(u8_i2cVecNum, INTR_NVIC_PRIORITY_I2C_DEFAULT, 0);
    HAL_NVIC_RegisterHandler(u8_i2cVecNum, I2C_Master_IntrSrvc, NULL);
    I2C_Init(en_component, I2C_Master_Mode, u16_i2cAddr, en_speed);
    HAL_NVIC_EnableIrq(u8_i2cVecNum);
    
    return HAL_OK;
}

/**
* @brief  The I2C data write function which can be used to send out I2C data by the I2C controller.
* @param  e_i2cComponent          The I2C controller number, the right number should be 0-4 and totally
*                                 5 I2C controllers can be used by application.
*         u16_i2cAddr             16 bit I2C address of the target device.
*         pu8_wrData              The transmit buffer pointer to be sent out by I2C bus. In normal case, the first 1 
*                                 or 2 bytes should be the sub address to be accessed.
*         u16_wrSize              The transmit buffer size in byte. 
* @retval HAL_OK                  means the I2C data write is well done.
*         HAL_I2C_ERR_WRITE_DATA  means some error happens in the I2C data write.
* @note   u32_wrSize should be less than 6, this is the I2C fifo limit. There is some fifo full risk when u32_wrSize 
          is larger than 6.
          High speed mode has some system dependency and is especially affected by the circuit capacity.
*/

HAL_RET_T HAL_I2C_MasterWriteData(ENUM_HAL_I2C_COMPONENT e_i2cComponent, 
                                  uint16_t u16_i2cAddr,
                                  uint8_t *pu8_wrData,
                                  uint32_t u32_wrSize,
                                  uint32_t u32_timeOut)
{
    EN_I2C_COMPONENT en_component;
    uint32_t start;

    switch (e_i2cComponent)
    {
        case HAL_I2C_COMPONENT_0:
            en_component = I2C_Component_0;
            break;
        case HAL_I2C_COMPONENT_1:
            en_component = I2C_Component_1;
            break;
        case HAL_I2C_COMPONENT_2:
            en_component = I2C_Component_2;
            break;
        case HAL_I2C_COMPONENT_3:
            en_component = I2C_Component_3;
            break;
        case HAL_I2C_COMPONENT_4:
            en_component = I2C_Component_4;
            break;
        case HAL_I2C_COMPONENT_5:
            en_component = I2C_Component_5;
            break;
        default:
            return HAL_I2C_ERR_WRITE_DATA;
    }

    I2C_Master_ClrTxAbrt(en_component);

    if (I2C_Master_GetBusyStatus(en_component))
    {
        return HAL_BUSY;
    }

    I2C_Master_WriteData(en_component, u16_i2cAddr, pu8_wrData, u32_wrSize);

    if (0 != u32_timeOut)
    {
        start = SysTicks_GetTickCount();
        while (I2C_Master_GetBusyStatus(en_component))
        {
            if ((SysTicks_GetDiff(start, SysTicks_GetTickCount())) >= u32_timeOut)
            {
                 return HAL_TIME_OUT;
            }

            SysTicks_DelayUS(100);
        }
    }
    
    return HAL_OK;
}

/**
* @brief  The I2C data read function which can be used to read I2C data by the I2C controller.
* @param  e_i2cComponent          The I2C controller number, the right number should be 0-4 and totally
*                                 5 I2C controllers can be used by application.
*         u16_i2cAddr             16 bit I2C address of the target device.
*         pu8_wrData              The transmit buffer pointer to be sent out by I2C bus before read starts.
                                  In normal case, the first 1 or 2 bytes should be the sub address to be accessed.
*         u16_wrSize              The transmit buffer size in byte. 
          pu8_rdData              The receive buffer pointer to hold the data in read operation.
          u16_rdSize              The receive buffer size in byte.
* @retval HAL_OK                  means the initializtion is well done.
*         HAL_I2C_ERR_READ_DATA   means some error happens in the data read.
* @note   High speed mode has some system dependency and is especially affected by the circuit capacity.
*/

HAL_RET_T HAL_I2C_MasterReadData(ENUM_HAL_I2C_COMPONENT e_i2cComponent, 
                                 uint16_t u16_i2cAddr,
                                 uint8_t *pu8_wrData,
                                 uint8_t  u8_wrSize,
                                 uint8_t *pu8_rdData,
                                 uint32_t u32_rdSize,
                                 uint32_t u32_timeOut)
{
    EN_I2C_COMPONENT en_component;
    uint32_t start;

    switch (e_i2cComponent)
    {
        case HAL_I2C_COMPONENT_0:
            en_component = I2C_Component_0;
            break;
        case HAL_I2C_COMPONENT_1:
            en_component = I2C_Component_1;
            break;
        case HAL_I2C_COMPONENT_2:
            en_component = I2C_Component_2;
            break;
        case HAL_I2C_COMPONENT_3:
            en_component = I2C_Component_3;
            break;
        case HAL_I2C_COMPONENT_4:
            en_component = I2C_Component_4;
            break;
        case HAL_I2C_COMPONENT_5:
            en_component = I2C_Component_5;
            break;
        default:
            return HAL_I2C_ERR_READ_DATA;
    }

    I2C_Master_ClrTxAbrt(en_component);
    
    if (I2C_Master_GetBusyStatus(en_component))
    {
        return HAL_BUSY;
    }

    I2C_Master_ReadData(en_component, u16_i2cAddr, pu8_wrData, u8_wrSize, pu8_rdData, u32_rdSize);

    if (0 != u32_timeOut)
    {
        start = SysTicks_GetTickCount();
        while (I2C_Master_GetBusyStatus(en_component))
        {
            if ((SysTicks_GetDiff(start, SysTicks_GetTickCount())) >= u32_timeOut)
            {
                 return HAL_TIME_OUT;
            }

            SysTicks_DelayUS(100);
        }
    }

    return HAL_OK;
}

