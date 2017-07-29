/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: ov5640.h
Description: the ov5640 driver.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2017/01/05
History: 
        0.0.1    2017/01/05    The initial version of ov5640.h
*****************************************************************************/


#ifndef __OV5640_H__
#define __OV5640_H__

#include <stdint.h>
#include "data_type.h"
#include "i2c.h"
#include "debuglog.h"


/*******************Macro define**************************/
#define OV5640_I2C_ADDR         (0x78 >> 1)
#define OV5640_COMPONENT        (I2C_Component_1)
#define  OV5640_I2C_MAX_DELAY   (2)


typedef enum 
{
    OV5640_MODE_MIN             = 0,
    OV5640_MODE_VGA_640_480     = 0,
    OV5640_MODE_QVGA_320_240    = 1,
    OV5640_MODE_NTSC_720_480    = 2,
    OV5640_MODE_PAL_720_576     = 3,
    OV5640_MODE_720P_1280_720   = 4,
    OV5640_MODE_1080P_1920_1080 = 5,
    OV5640_MODE_QSXGA_2592_1944 = 6,
    OV5640_MODE_QCIF_176_144    = 7,
    OV5640_MODE_XGA_1024_768    = 8,
    OV5640_MODE_MAX             = 8,
    OV5640_MODE_INIT            = 0xff, /*only for sensor init*/
} ENUM_OV5640_MODE;

typedef enum  
{
    OV5640_15_FPS,
    OV5640_30_FPS
} ENUM_OV5640_FRAME_RATE;

typedef struct
{
    uint16_t u16_regAddr;
    uint8_t  u8_val;
    uint8_t  u8_mask;
    uint32_t u32_delayMs;
} STRU_OV5640_REG_VALUE;

typedef struct
{
    uint16_t u16_ov5640Width;
    uint16_t u16_ov5640Hight;
    uint8_t u8_ov5640FrameRate;
} STRU_OV5640_INFO;


/**
* @brief     
* @param     
* @retval   
* @note   
*/
void OV5640_stream_on(void);

/**
* @brief     
* @param     
* @retval   
* @note   
*/
void OV5640_stream_off(void);

/**
* @brief     
* @param     
* @retval   
* @note   
*/
void OV5640_set_night_mode(void);

/**
* @brief     
* @param     
* @retval   
* @note   
*/
int32_t OV5640_get_HTS(void);

/**
* @brief     
* @param     
* @retval   
* @note   
*/
int32_t OV5640_get_VTS(void);

/**
* @brief     
* @param     
* @retval   
* @note   
*/
int32_t OV5640_set_VTS(int32_t VTS);

/**
* @brief     
* @param     
* @retval   
* @note   
*/
int32_t OV5640_get_shutter(void);

/**
* @brief     
* @param     
* @retval   
* @note   
*/
int32_t OV5640_set_shutter(int32_t shutter);

/**
* @brief     
* @param     
* @retval   
* @note   
*/
int32_t OV5640_get_gain16(void);

/**
* @brief     
* @param     
* @retval   
* @note   
*/
int32_t OV5640_set_gain16(int32_t gain16);

/**
* @brief     
* @param     
* @retval   
* @note   
*/
int32_t OV5640_get_light_freq(void);

/**
* @brief    write OV5640 register with value u8_ov5640Val 
* @param    u16_ov5640Reg OV5640 register address
* @param    u8_ov5640Val data need to write  
* @retval   0:write success, -1:write failed
* @note   
*/
int32_t OV5640_WriteReg(uint16_t u16_ov5640Reg, uint8_t u8_ov5640Val);

/**
* @brief    read OV5640 register 
* @param    u16_ov5640Reg OV5640 register address
* @param    pu8_ov5640Val read data  
* @retval   0:read success, -1:read failed
* @note   
*/
int32_t OV5640_ReadReg(uint16_t u16_ov5640Reg, uint8_t *pu8_ov5640Val);

/**
* @brief     
* @param     
* @retval   
* @note   
*/
int32_t OV5640_Init(ENUM_OV5640_FRAME_RATE e_ov5640FrameRate, ENUM_OV5640_MODE e_ov5640Mode);


/**
* @brief    
* @param     
* @retval  
* @note    
*/
int32_t OV5640_GetImageInfo(uint16_t *u16_width, uint16_t *u16_hight, uint8_t *u8_frameRate);


#endif
