/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: 
Description: 
Author: Artosy Software Team
Version: 
Date: 
History: 
        
*****************************************************************************/


#ifndef __HAL_BB_DEBUG___
#define __HAL_BB_DEBUG___

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "bb_types.h"
#include "bb_customerctx.h"
#include "hal_ret_type.h"

/** 
 * @brief       Set the image transmit LDPC coderate
 * @param[in]   e_ldpc:                  ldpc coderate 
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1   
 */
HAL_RET_T HAL_BB_SetItLdpcProxy(ENUM_BB_LDPC e_ldpc);

/** 
 * @brief       Set the rc transmit LDPC coderate
 * @param[in]   e_ldpc:                  ldpc coderate 
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1   
 */
HAL_RET_T HAL_BB_SetRcLdpcProxy(ENUM_BB_LDPC e_ldpc);

/** 
 * @brief   Set board enter or out debug mode
 * @param   mode	    0:  set Baseband to enter debug mode, 
                        1:  set Baseband to out debug mode.
 * @note    The function can only be called by cpu0,1 
 */
HAL_RET_T HAL_BB_SetBoardDebugModeProxy(uint8_t mode);

/** 
 * @brief   set Baseband to It only mode
 * @param   mode                            1: enter It only mode
 * @retval  HAL_OK,                         means command is sent sucessfully. 
 * @retval  HAL_BB_ERR_EVENT_NOTIFY         means error happens in sending the command to cpu2                        
 * @note    The function can only be called by cpu0,1, and only call for debug.
 */
HAL_RET_T HAL_BB_SetItOnlyFreqProxy(uint8_t mode);

/** 
 * @brief       Set It(image transmit) QAM
 * @param[in]   e_qam:                  the modulation QAM mode for image transmit.
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        None
 */
HAL_RET_T HAL_BB_SetFreqBandQamSelectionProxy(ENUM_BB_QAM e_qam);

/** 
 * @brief       Set rc QAM
 * @param[in]   e_qam:                  the modulation QAM mode for image transmit.
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        None
 */
HAL_RET_T HAL_BB_SetRcQamSelectionProxy(ENUM_BB_QAM e_qam);

/** 
 * @brief       switch encoder channel on/off
 * @param[in]   u8_ch:   channel,0:ch1,1:ch2
 * @param[in]   u8_data: 0:off, 1:on 
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        None
 */
HAL_RET_T HAL_BB_SwitchOnOffChProxy(uint8_t u8_ch, uint8_t u8_data);


#ifdef __cplusplus
}
#endif

#endif
