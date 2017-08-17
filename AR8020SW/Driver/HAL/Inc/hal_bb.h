/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_i2c.h
Description: The external HAL APIs to use the I2C controller.
Author: Artosy Software Team
Version: 0.0.1
Date: 2016/12/20
History: 
        0.0.1    2016/12/20    The initial version of hal_i2c.h
*****************************************************************************/


#ifndef __HAL_BB___
#define __HAL_BB___

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "bb_types.h"
#include "bb_uart_com.h"
#include "bb_customerctx.h"
#include "hal_ret_type.h"


/** 
 * @brief   init baseband to ground mode
 * @param   NONE
 * @return  HAL_OK:                         means init baseband 
 *          HAL_BB_ERR_INIT:                means some error happens in init session 
 */
HAL_RET_T HAL_BB_InitGround( STRU_CUSTOMER_CFG *stru_customerCfg );


/** 
 * @brief   init baseband to sky mode
 * @param   NONE
 * @return  HAL_OK:                         means init baseband 
 *          HAL_BB_ERR_INIT:                means some error happens in init session 
 */
HAL_RET_T HAL_BB_InitSky( STRU_CUSTOMER_CFG *stru_customerCfg );




/** 
 * @brief       set channel Bandwidth 10M/20M
 * @param[in]   e_bandwidth              channel bandwidth setting 10M/20M
 * @retval      HAL_OK                   means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1 
 */
HAL_RET_T HAL_BB_SetFreqBandwidthSelectionProxy(ENUM_CH_BW e_bandwidth);



/** 
 * @brief       Set frequency band (2G/5G) selection mode (ATUO / Manual)
 * @param[in]   e_mode:                  selection mode (ATUO / Manual)
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1  
 */
HAL_RET_T HAL_BB_SetFreqBandSelectionModeProxy(ENUM_RUN_MODE e_mode);


/**
 * @brief       Set frequency band (2G/5G)
 * @param[in]   e_band:                  RF band selection
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1   
 */
HAL_RET_T HAL_BB_SetFreqBandProxy(ENUM_RF_BAND e_band);





/** 
 * @brief       Set It(image transmit) channel selection RUN mode(AUTO/Manual)
 * @param[in]   e_mode:                  the modulation QAM mode for image transmit.
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1  
 */
HAL_RET_T HAL_BB_SetItChannelSelectionModeProxy(ENUM_RUN_MODE e_mode);




/** 
 * @brief       API for set IT(image transmit) channel Number
 * @param[in]   u8_channelNum:           the current channel number int current Frequency band(2G/5G)
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1 
 */
HAL_RET_T HAL_BB_SetItChannelProxy(uint8_t u8_channelNum);


/** 
 * @brief       Set MCS(modulation, coderate scheme) mode, the function can only be called by cpu0,1
 * @param[in]   e_mode:                  auto or manual selection.
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1   
 */
HAL_RET_T HAL_BB_SetMcsModeProxy(ENUM_RUN_MODE e_mode);


/** 
 * @brief       Set the image transmit QAM mode
 * @param[in]   e_qam:                   modulation qam mode
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1   
 */
HAL_RET_T HAL_BB_SetItQamProxy(ENUM_BB_QAM e_qam);

/** 
 * @brief       Set the encoder bitrate control mode
 * @param[in]   e_mode: auto or manual selection.
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1  
 */
HAL_RET_T HAL_BB_SetEncoderBrcModeProxy(ENUM_RUN_MODE e_mode);



/** 
 * @brief       Set the encoder bitrate Unit:Mbps
 * @param[in]   u8_ch: channel, 0:ch1 1:ch2
 * @param[in]   u8_bitrateMbps: select the bitrate unit: Mbps
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        the function can only be called by cpu0,1
 */
HAL_RET_T HAL_BB_SetEncoderBitrateProxy(uint8_t u8_ch, uint8_t u8_bitrateMbps);

/** 
 * @brief   init the uart remote session
 * @param   None
 * @retval  HAL_OK                          means the uart sessiion init OK
 *          HAL_BB_ERR_INIT_SESSION         means some error happens in the uart session init.
 * @note    The function can only be called by cpu0,1 
 */
HAL_RET_T HAL_BB_UartComRemoteSessionInit(void);



/** 
 * @brief   register one uart session for send or receive message
 * @param   e_sessionId:                    the session id to request
 * @return  HAL_OK:                         means request session OK
 *          HAL_BB_ERR_SESSION_OCCUPIED:    session ID is already occupied
 */
HAL_RET_T HAL_BB_UartComRegisterSession(ENUM_BBUARTCOMSESSIONID e_sessionId,
                                      ENUM_BB_UART_SESSION_PRIORITY e_sessionPriority,
                                      ENUM_BB_UART_SESSION_DATA_TYPE e_sessionDataType,
                                      SYS_Event_Handler rcvDataEventHandler);


/** 
 * @brief   unregister one uart session
 * @param   e_sessionId:                    the session id has already requested.
 * @return  HAL_OK:                         means unrequest session OK 
 *          HAL_BB_ERR_UNREGISTER_SESSION:  means some error happens in unregister session
 */
HAL_RET_T HAL_BB_UartComUnRegisterSession(ENUM_BBUARTCOMSESSIONID e_sessionId, SYS_Event_Handler sessionEventHandler);



/** 
 * @brief   send out messages from uart session
 * @param   e_sessionId:                    the session id has already requested.
 *          pu8_dataBuf:                    buffer pointer to the data to be sent
 *          u32_length:                     data size to be sent
 *
 * @return  HAL_OK:                         means unrequest session OK 
 *          HAL_BB_ERR_UNREGISTER_SESSION:  means some error happens in unregister session 
 */
HAL_RET_T HAL_BB_UartComSendMsg(ENUM_BBUARTCOMSESSIONID e_sessionId, 
                                uint8_t  *pu8_dataBuf, 
                                uint32_t u32_length);


/** 
 * @brief   receive messages from uart session
 * @param   e_sessionId:                    the session id has already requested.
 *          pu8_dataBuf:                    buffer pointer to the data to be stored
 *          u32_lengthMax:                  maximum data size for store session uart data
 *          pu32_dataLen:                   actual data len has received from session
 * @return  HAL_OK:                         means unrequest session OK 
 *          HAL_BB_ERR_SESSION_RCV:         means some error happens in receving data from session
 */
HAL_RET_T HAL_BB_UartComReceiveMsg(ENUM_BBUARTCOMSESSIONID e_sessionId, 
                                   uint8_t  *pu8_dataBuf, 
                                   uint32_t u32_lengthMax,
                                   uint32_t *pu32_dataLen);


/** 
 * @brief   set Baseband Rc frequency setting registers
 * @param   u32_freqSetting:                the registers 
 * @retval  HAL_OK,                         means command is sent sucessfully. 
 * @retval  HAL_BB_ERR_EVENT_NOTIFY         means error happens in sending the command to cpu2                        
 * @note    The function can only be called by cpu0,1, and only call for debug.
 */
HAL_RET_T HAL_BB_SetRcFreqProxy(uint32_t u32_freqSetting);



/** 
 * @brief   set Baseband It frequency setting registers
 * @param   u32_freqSetting:                the registers setting
 * @retval  HAL_OK,                         means command is sent sucessfully. 
 * @retval  HAL_BB_ERR_EVENT_NOTIFY         means error happens in sending the command to cpu2                        
 * @note    The function can only be called by cpu0,1, and only call for debug.
 */
HAL_RET_T HAL_BB_SetItFreqProxy(uint32_t u32_freqSetting);

/** 
 * @brief   Set baseband sky to auto search the ground RC id
 * @param   NONE
 * @retval  HAL_OK:                    means command is sent sucessfully. 
            HAL_BB_ERR_EVENT_NOTIFY:   means error happens in sending the command to cpu2
 * @note    
 */
HAL_RET_T HAL_BB_SetAutoSearchRcIdProxy(void);


/** 
 * @brief       Set RC channel selection RUN mode(AUTO/Manual)
 * @param[in]   e_mode:                  the modulation QAM mode for rc.
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        None
 */
HAL_RET_T HAL_BB_SetRcChannelSelectionModeProxy(ENUM_RUN_MODE e_mode);

/** 
 * @brief       force baseband reset.
 * @param       none.
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        None
 */
HAL_RET_T HAL_BB_SoftResetProxy(void);

/** 
 * @brief       force baseband calculation distance zero calibration.
 * @param       none.
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        None
 */
HAL_RET_T HAL_BB_CalcDistZeroCalibration(void);

/** 
 * @brief       set baseband calculation distance zero point.
 * @param       none.
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        None
 */
HAL_RET_T HAL_BB_SetCalcDistZeroPoint(uint32_t value);


/** 
 * @brief       request ground to disconnect from the sky by rcid in searching mode
 * @param       pu8_rcid:                sky rc id request to disconnect
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        None
 */
HAL_RET_T HAL_BB_GroundDisConnectSkyByRcId(uint8_t * pu8_rcid);


/** 
 * @brief       request ground to connect with the sky by rcid
 * @param       pu8_rcid:                sky rc id request to connect
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        None
 */
HAL_RET_T HAL_BB_GroundConnectToSkyByRcId(uint8_t *pu8_rcId);


/** 
 * @brief       request ground to search rc id
 * @param       flag_followGroundInSearching:  not usefull for ground. For sky, it means sky will follow ground in rc id searching mode until ground request disconnect
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        None
 */
HAL_RET_T HAL_BB_SearchRcId(uint8_t flag_skyFollowGroundInSearching);


/** 
 * @brief       request to save and use the rc id
 * @param       pu8_rcId: rc id setting, size is Five
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        None
 */
HAL_RET_T HAL_BB_SaveRcId(uint8_t *pu8_rcId);


/** 
 * @brief       get the current rc id
 * @param       pu8_rcId: the pointer to save the rc id
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        None
 */
HAL_RET_T HAL_BB_GetRcId(uint8_t *pu8_rcId, uint8_t bufsize);


/** 
 * @brief       get baseband info storage address, details refer to structrue 
 *              STRU_WIRELESS_INFO_DISPLAY, in header file bb_types.h 
 * @param       pst_bbInfoAddr: the pointer to baseband info storage address
 * @retval      HAL_OK:     means get baseband info sucessfully. 
 * @retval      HAL_BUSY:   means baseband info is updating.
 * @note        None
 */
HAL_RET_T HAL_BB_GetInfo(STRU_WIRELESS_INFO_DISPLAY **pst_bbInfoAddr);


/** 
 ** @brief       HAL_BB_UpgradeMode
 ** @param       1: enter into upgrade    0: exit upgrade
 ** @retval      HAL_OK,                  means command is sent sucessfully. 
 ** @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 **/
HAL_RET_T HAL_BB_UpgradeMode(uint8_t u8_flag);

#ifdef __cplusplus
}
#endif

#endif
