/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_norflash.h
Description: The external HAL APIs to use the nor flash controller.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2017/03/23
History: 
        0.0.1    2017/03/23    The initial version of hal_norflash.h
*****************************************************************************/

#include <stdint.h>
#include "nor_flash.h"
#include "debuglog.h"
#include "hal_ret_type.h"
#include "hal_norflash.h"
#include "driver_mutex.h"

HAL_RET_T HAL_NORFLASH_Init(void)
{
    if ( -1 == COMMON_driverMutexGet(MUTEX_NOR_FLASH, 0) )
    {
        dlog_error("fail");
        return HAL_OCCUPIED;
    }
    COMMON_driverMutexSet(MUTEX_NOR_FLASH, 0);

    NOR_FLASH_Init();
}


HAL_RET_T HAL_NORFLASH_Erase(ENUM_HAL_NORFLASH_EraseIndex e_index, uint32_t u32_startAddr)
{
    switch (e_index)
    {
        case 0:
            NOR_FLASH_EraseSector(u32_startAddr);
            break;
        case 1:
            NOR_FLASH_EraseBlock(u32_startAddr);
            break;
        case 2:
            NOR_FLASH_EraseChip();
            break;

        default:
            return HAL_NORFLASH_ERR_ERASE;
            break;
    }
    return HAL_OK;
}


HAL_RET_T HAL_NORFLASH_WriteByteBuffer(uint32_t u32_startAddr, uint8_t* pu8_dataBuff, uint32_t u32_size)
{
     NOR_FLASH_WriteByteBuffer(u32_startAddr, pu8_dataBuff, u32_size);
     return HAL_OK;
}

HAL_RET_T HAL_NORFLASH_ReadByteBuffer(uint32_t u32_startAddr,uint8_t* pu8_dataBuff, uint32_t u32_size)
{
     NOR_FLASH_ReadByteBuffer(u32_startAddr, pu8_dataBuff, u32_size);
     return HAL_OK;
}
