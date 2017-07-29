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

#ifndef __HAL_NORFLASH_H__
#define __HAL_NORFLASH_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    HAL_NORFLASH_Sector = 0,
    HAL_NORFLASH_Block,
    HAL_NORFLASH_Chip
} ENUM_HAL_NORFLASH_EraseIndex;

/**
* @brief  The nor flash initialization function which must be called before using the nor flash controller.
* @param  none
* @retval HAL_OK            means the initializtion is well done.
* @note   none
*/
HAL_RET_T HAL_NORFLASH_Init(void);

/**
* @brief  The nor flash erase。
* @param  e_index           The erase flash index.
*         u32_startAddr     start address must align.
*         HAL_NORFLASH_ERR_ERASE  means don't support type.
* @note   HAL_NORFLASH_Sector align 4K.
          HAL_NORFLASH_Block  align 64K.          
*/
HAL_RET_T HAL_NORFLASH_Erase(ENUM_HAL_NORFLASH_EraseIndex e_index, uint32_t u32_startAddr);

/**
* @brief  The write byte flash.
* @param  u32_startAddr    start address.
*         pu8_dataBuff     buff point.
*         u32_size         write size.
* @retval HAL_OK           means the write is well done.
* @note   none.
*/
HAL_RET_T HAL_NORFLASH_WriteByteBuffer(uint32_t u32_startAddr, uint8_t* pu8_dataBuff, uint32_t u32_size);

/**
* @brief  The read byte flash.
* @param  u32_startAddr    start address.
*         pu8_dataBuff     buff point.
*         u32_size         read size.
* @retval HAL_OK           means the read is well done.
* @note   none.
*/
HAL_RET_T HAL_NORFLASH_ReadByteBuffer(uint32_t u32_startAddr,uint8_t* pu8_dataBuff, uint32_t u32_size);



#ifdef __cplusplus
}
#endif



#endif/*__HAL_NORFLASH_H__*/
