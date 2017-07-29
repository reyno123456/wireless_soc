/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_sd.h
Description: The external HAL APIs to use the SDMMC controller.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/26
History: 
        0.0.1    2016/12/26    The initial version of hal_sd.h
*****************************************************************************/
#ifndef __HAL_SD_H__
#define __HAL_SD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "hal_ret_type.h"

typedef enum
{
	HAL_SD_GET_CSD_VERSION = 0,  /* 0: CSD version */
	HAL_SD_GET_SECTOR_COUNT,     /* 1: the SD card capacity */
	HAL_SD_GET_SECTOR_SIZE,      /* 2: maximum read/write data block length in bytes */
	HAL_SD_GET_TRAN_SPEED,       /* 3: maximum data transfer rate per one data line */
	HAL_SD_GET_CARD_STATUS,      /* 4: the current card state */
	HAL_SD_GET_MANUID,           /* 5: manufacturer ID */
	HAL_SD_GET_OEMID             /* 6: card OEM information */
} ENUM_HAL_SD_CTRL;

typedef enum
{
	SDR12 = 0,  
	SDR25,     
	SDR50,      
	SDR104       /*TODO this mode is not supported by current level translator chip*/
} ENUM_HAL_SD_SPEED_MODE;

/**
* @brief  Initializes the SD card according to the specified parameters in the 
            SD_HandleTypeDef and create the associated handle
* @param  none
* @retval HAL_OK            means the initializtion is well done
*         HAL_SD_ERR_ERROR  means some error happens in the initializtion
*/
HAL_RET_T HAL_SD_Init(void);

/**
* @brief  Writes block(s) to a specified address in a card. The Data transfer 
  *         is managed by DMA mode
* @param  u32_dstStartAddr   pointer to the buffer that will contain the data to transmit
*         u32_srcStartAddr   Address from where data is to be read   
*         u32_sectorNum      the SD card Data block size 
* @retval HAL_OK            write to sd card succeed
*         HAL_SD_ERR_ERROR  means some error happens during the writing
*/
HAL_RET_T HAL_SD_Write(uint32_t u32_dstBlkAddr, uint32_t u32_srcStartAddr, uint32_t u32_sectorNum);

/**
* @brief  Reads block(s) from a specified address in a card. The Data transfer 
  *         is managed by DMA mode. 
* @param  u32_dstStartAddr   Pointer to the buffer that will contain the received data
*         u32_srcStartAddr   Address from where data is to be read  
*         u32_sectorNum      SD card Data block size
* @retval HAL_OK             read from sd card succeed
*         HAL_SD_ERR_ERROR   means some error happens during the reading
*/
HAL_RET_T HAL_SD_Read(uint32_t u32_dstStartAddr, uint32_t u32_srcBlkAddr, uint32_t u32_sectorNum);
/**
* @brief  Erases the specified memory area of the given SD card
* @param  u32_startBlock    start byte address
*         u32_sectorNum     the SD card data block size
* @retval HAL_OK            erase SD card succeed
*         HAL_SD_ERR_ERROR  means some error happens during the erasing
*/
HAL_RET_T HAL_SD_Erase(uint32_t u32_startBlock, uint32_t u32_sectorNum);

/**
* @brief  IO ctrl function to acquire SD card related information
* @param  e_sdCtrl          the SD IO ctrl enumetation variable to specify the corresponding function
*         pu32_info         IO ctrl function output 
* @retval HAL_OK            get the information succeed
*         HAL_SD_ERR_ERROR  means some error happens in the function
*/
HAL_RET_T HAL_SD_Ioctl(ENUM_HAL_SD_CTRL e_sdCtrl, uint32_t *pu32_info);

/**
* @brief  De-Initializes the SD card
* @param  none
* @retval HAL_OK      means the de-initialization is well done
		  HAL_SD_ERR_ERROR means de-initialization
*/
HAL_RET_T HAL_SD_Deinit();

HAL_RET_T HAL_SD_Fatfs_Init(void);

HAL_RET_T HAL_SD_Write_test(uint32_t u32_dstBlkAddr, uint32_t u32_srcStartAddr, uint32_t u32_sectorNum);
HAL_RET_T HAL_SD_Read_test(uint32_t u32_dstStartAddr, uint32_t u32_srcBlkAddr, uint32_t u32_sectorNum);
HAL_RET_T HAL_SD_GetPresent(void);

#ifdef __cplusplus
}
#endif

#endif
