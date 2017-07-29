#include <string.h>
#include <stdint.h>
#include "debuglog.h"
#include "hal_i2c.h"
#include "pmu_rtp5903.h"

#define PMU_RTP5903_SLAADDR                 (0x30)
#define PMU_RTP5903A_I2C_TIMEOUT_MS         (3)
#define ARRAY_COUNT_OF(x)                   ((sizeof(x)/sizeof(0[x])) / ((!(sizeof(x) % sizeof(0[x])))))

typedef struct 
{
    uint8_t u8_subAddr;
    uint8_t u8_value;
}STRU_RTP5903_I2C_CONFIGURE;


STRU_RTP5903_I2C_CONFIGURE ARCastRTP5903Array[2] = 
{
    {0x21, 0x10}, //DC2 1.2 -> 1.1
    {0x23, 0x17}  //DC4 3.3 -> 3.0
};

void Pmu_Rtp5903Configure(void)
{
    uint32_t i = 0;

    HAL_I2C_MasterInit(HAL_I2C_COMPONENT_2, PMU_RTP5903_SLAADDR, HAL_I2C_FAST_SPEED);

    for (i = 0; i < ARRAY_COUNT_OF(ARCastRTP5903Array); i++)
    {        
        HAL_I2C_MasterWriteData(HAL_I2C_COMPONENT_2, 
                                PMU_RTP5903_SLAADDR, 
                                (uint8_t *)(&ARCastRTP5903Array[i]), 
                                2, 
                                PMU_RTP5903A_I2C_TIMEOUT_MS);        
    }
}

