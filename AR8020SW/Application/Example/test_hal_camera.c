#include "hal_camera.h"
#include "test_hal_camera.h"

void command_TestHalCameraInit(unsigned char *rate, unsigned char *mode)
{
    uint32_t u32_rate = strtoul(rate, NULL, 0);
    uint32_t u32_mode = strtoul(mode, NULL, 0);

    HAL_CAMERA_Init((ENUM_HAL_CAMERA_FRAME_RATE)u32_rate, 
                    (ENUM_HAL_CAMERA_MODE)u32_mode);
}

void command_TestCameraWrite(unsigned char *subAddr, unsigned char *value)
{
    uint16_t reg = strtoul(subAddr, NULL, 0);
    uint8_t val = strtoul(value, NULL, 0);
    uint32_t delay = 200;
    uint8_t u8_val;

    HAL_CAMERA_ReadReg(reg, &u8_val);
    dlog_info("before write red:0x%x val:0x%x",reg,u8_val);
    
    HAL_CAMERA_WriteReg(reg, val);
        
    HAL_CAMERA_ReadReg(reg, &u8_val);
    dlog_info("after write red:0x%x val:0x%x",reg,u8_val);
}

void command_TestCameraRead(unsigned char *subAddr)
{
    uint16_t reg = strtoul(subAddr, NULL, 0);
    uint8_t u8_val;
        
    HAL_CAMERA_ReadReg(reg, &u8_val);
    dlog_info("red:0x%x val:0x%x",reg,u8_val);

}

