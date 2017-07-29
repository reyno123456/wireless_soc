/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_dma.h
Description: The external HAL APIs to use the I2C controller.
Author: Artosy Software Team
Version: 0.0.1
Date: 2017/1/4
History: 
        0.0.1    2016/12/20    The initial version of hal_dma.h
*****************************************************************************/


#ifndef __HAL_DMA___
#define __HAL_DMA___

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "hal_ret_type.h"

/* Flag to identify which channel to be used */
typedef enum {
	DMA_CHAN0 = 0,
	DMA_CHAN1,
	DMA_CHAN2,
	DMA_CHAN3,
	DMA_CHAN4,
	DMA_CHAN5,
	DMA_CHAN6,
	DMA_CHAN7,
	DMA_AUTO
} ENUM_DMA_chan;

/* Transfer types */
typedef enum {
	DMA_LINK_LIST_ITEM,
	DMA_AUTO_RELOAD
} ENUM_DMA_TransferType;


HAL_RET_T HAL_DMA_init(void);

/** 
 * @brief   Start the DMA Transfer
 * @param   u32_srcAddress: The source memory Buffer address
 * @param   u32_dstAddress: The destination memory Buffer address
 * @param   u32_dataLength: The length of data to be transferred from source to destination
 * @param   u8_channel: The channel index from 0 to 7
 * @return  none
 */
HAL_RET_T HAL_DMA_Transfer(uint32_t u32_srcAddr, 
								uint32_t u32_dstAddr, 
								uint32_t u32_transByteNum, 
								uint32_t u32_timeOut);


#ifdef __cplusplus
}
#endif

#endif
