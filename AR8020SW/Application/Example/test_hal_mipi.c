#include "debuglog.h"
#include "hal_mipi.h"
#include "test_hal_mipi.h"
#include "test_hal_camera.h"


void command_TestHalMipiInit(char *toEncoderCh)
{
    uint16_t u16_width;
    uint16_t u16_hight;
    uint8_t u8_frameRate;
    uint8_t ch = strtoul(toEncoderCh, NULL, 0);

    HAL_CAMERA_GetImageInfo(&u16_width, &u16_hight, &u8_frameRate);
    dlog_info("width:%d hight:%d frameRate:%d",u16_width,u16_hight,u8_frameRate);
    HAL_MIPI_Init(ch, u16_width, u16_hight, u8_frameRate);
}
