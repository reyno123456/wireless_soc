/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_bb.c
Description: The external HAL APIs to use the I2C controller.
Author: Artosy Software Team
Version: 0.0.1
Date: 2016/12/20
History:
        0.0.1    2016/12/20    The initial version of hal_bb.c
*****************************************************************************/


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "sys_event.h"
#include "bb_spi.h"
#include "bb_ctrl.h"
#include "rf_if.h"
#include "hal_bb.h"
#include "debuglog.h"


/** 
 * @brief       set channel Bandwidth 10M/20M
 * @param[in]   e_bandwidth              channel bandwidth setting 10M/20M
 * @retval      HAL_OK                   means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1 
 */
HAL_RET_T HAL_BB_SetFreqBandwidthSelectionProxy(ENUM_CH_BW e_bandwidth)
{
    uint8_t u8_ret;
    STRU_WIRELESS_CONFIG_CHANGE st_cmd;
    
    st_cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
    st_cmd.u8_configItem   = FREQ_BAND_WIDTH_SELECT;
    st_cmd.u32_configValue = (uint32_t)e_bandwidth;

	u8_ret = SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&st_cmd);
    if( u8_ret )
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_EVENT_NOTIFY;
    }
}



/** 
 * @brief       Set frequency band (2G/5G) selection mode (ATUO / Manual)
 * @param[in]   e_mode:                  selection mode (ATUO / Manual)
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1  
 */
HAL_RET_T HAL_BB_SetFreqBandSelectionModeProxy(ENUM_RF_BAND e_mode)
{
    uint8_t u8_ret;
    STRU_WIRELESS_CONFIG_CHANGE st_cmd;

    st_cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
    st_cmd.u8_configItem   = FREQ_BAND_MODE;
    st_cmd.u32_configValue = (uint32_t)e_mode;

    u8_ret = SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&st_cmd);
    if( u8_ret )
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_EVENT_NOTIFY;
    }
}



/**
 * @brief       Set frequency band (2G/5G)
 * @param[in]   e_band:                  RF band selection
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1   
 */
HAL_RET_T HAL_BB_SetFreqBandProxy(ENUM_RF_BAND e_band)
{
    uint8_t u8_ret;
    STRU_WIRELESS_CONFIG_CHANGE st_cmd;

    st_cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
    st_cmd.u8_configItem   = FREQ_BAND_SELECT;
    st_cmd.u32_configValue = (uint32_t)e_band;

	u8_ret = SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&st_cmd);
    if( u8_ret )
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_EVENT_NOTIFY;
    }    
}



/** 
 * @brief       Set It(image transmit) channel selection RUN mode(AUTO/Manual)
 * @param[in]   e_mode:                  the modulation QAM mode for image transmit.
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1  
 */
HAL_RET_T HAL_BB_SetItChannelSelectionModeProxy(ENUM_RUN_MODE e_mode)
{
    uint8_t u8_ret;
    STRU_WIRELESS_CONFIG_CHANGE st_cmd;
    
    st_cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
    st_cmd.u8_configItem   = FREQ_CHANNEL_MODE;
    st_cmd.u32_configValue = (uint32_t)e_mode;

    u8_ret = SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&st_cmd);
    if( u8_ret )
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_EVENT_NOTIFY;
    }    
}


/** 
 * @brief       API for set IT(image transmit) channel Number
 * @param[in]   u8_channelNum:           the current channel number int current Frequency band(2G/5G)
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1 
 */
HAL_RET_T HAL_BB_SetItChannelProxy(uint8_t u8_channelNum)
{
    uint8_t u8_ret;
    STRU_WIRELESS_CONFIG_CHANGE st_cmd;
    
    st_cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
    st_cmd.u8_configItem   = FREQ_CHANNEL_SELECT;
    st_cmd.u32_configValue = u8_channelNum;

	u8_ret = SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&st_cmd);
    if( u8_ret )
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_EVENT_NOTIFY;
    }     
}



////////////////// handlers for WIRELESS_MCS_CHANGE //////////////////

/** 
 * @brief       Set MCS(modulation, coderate scheme) mode, the function can only be called by cpu0,1
 * @param[in]   e_mode:                  auto or manual selection.
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1   
 */
HAL_RET_T HAL_BB_SetMcsModeProxy(ENUM_RUN_MODE e_mode)
{
    uint8_t u8_ret;
    STRU_WIRELESS_CONFIG_CHANGE st_cmd;
    
    st_cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
    st_cmd.u8_configItem   = MCS_MODE_SELECT;
    st_cmd.u32_configValue = (uint32_t)e_mode;

	u8_ret = SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&st_cmd);

    if( u8_ret )
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_EVENT_NOTIFY;
    }        
}


/** 
 * @brief       Set the image transmit QAM mode
 * @param[in]   e_qam:                   modulation qam mode
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1   
 */
HAL_RET_T HAL_BB_SetItQamProxy(ENUM_BB_QAM e_qam)
{
    uint8_t u8_ret;
    STRU_WIRELESS_CONFIG_CHANGE st_cmd;
    
    st_cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
    st_cmd.u8_configItem   = MCS_MODULATION_SELECT;
    st_cmd.u32_configValue = (uint32_t)e_qam;

	u8_ret = SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&st_cmd);
    if( u8_ret )
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_EVENT_NOTIFY;
    }     
}


/** 
 * @brief       Set the image transmit LDPC coderate
 * @param[in]   e_ldpc:                  ldpc coderate 
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1   
 */
HAL_RET_T HAL_BB_SetItLdpcProxy(ENUM_BB_LDPC e_ldpc)
{
    uint8_t u8_ret;
    STRU_WIRELESS_CONFIG_CHANGE st_cmd;

    st_cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
    st_cmd.u8_configItem   = MCS_IT_CODE_RATE_SELECT;
    st_cmd.u32_configValue = (uint32_t)e_ldpc;

    u8_ret = SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&st_cmd);
    if( u8_ret )
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_EVENT_NOTIFY;
    }
}

/** 
 * @brief       Set the rc transmit LDPC coderate
 * @param[in]   e_ldpc:                  ldpc coderate 
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1   
 */
HAL_RET_T HAL_BB_SetRcLdpcProxy(ENUM_BB_LDPC e_ldpc)
{
    uint8_t u8_ret;
    STRU_WIRELESS_CONFIG_CHANGE st_cmd;

    st_cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
    st_cmd.u8_configItem   = MCS_RC_CODE_RATE_SELECT;
    st_cmd.u32_configValue = (uint32_t)e_ldpc;

    u8_ret = SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&st_cmd);
    if( u8_ret )
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_EVENT_NOTIFY;
    }
}


/** 
 * @brief       Set the encoder bitrate control mode
 * @param[in]   e_mode: auto or manual selection.
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1  
 */
HAL_RET_T HAL_BB_SetEncoderBrcModeProxy(ENUM_RUN_MODE e_mode)
{
    uint8_t u8_ret;
    STRU_WIRELESS_CONFIG_CHANGE st_cmd;

    st_cmd.u8_configClass  = WIRELESS_ENCODER_CHANGE;
    st_cmd.u8_configItem   = ENCODER_DYNAMIC_BIT_RATE_MODE;
    st_cmd.u32_configValue = (uint32_t)e_mode;

	u8_ret = SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&st_cmd);
    if( u8_ret )
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_EVENT_NOTIFY;
    }    
}


/** 
 * @brief       Set the encoder bitrate Unit:Mbps
 * @param[in]   u8_bitrateMbps: select the bitrate unit: Mbps
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        the function can only be called by cpu0,1
 */
HAL_RET_T HAL_BB_SetEncoderBitrateProxy(uint8_t u8_ch, uint8_t u8_bitrateMbps)
{
    uint8_t u8_ret;
    STRU_WIRELESS_CONFIG_CHANGE st_cmd;

    st_cmd.u8_configClass  = WIRELESS_ENCODER_CHANGE;
    if (0 == u8_ch)
    {
        st_cmd.u8_configItem   = ENCODER_DYNAMIC_BIT_RATE_SELECT_CH1;
    }
    else
    {
        st_cmd.u8_configItem   = ENCODER_DYNAMIC_BIT_RATE_SELECT_CH2;
    }
    st_cmd.u32_configValue = u8_bitrateMbps;

    u8_ret = SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&st_cmd);
    if( u8_ret )
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_EVENT_NOTIFY;
    }
}


/** 
 * @brief   Set board enter or out debug mode
 * @param   u8_mode	    0:  set Baseband to enter debug mode, 
                        1:  set Baseband to out debug mode.
 * @retval  HAL_OK,                  means command is sent sucessfully. 
 * @retval  HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2                        
 * @note    The function can only be called by cpu0,1, and only call for debug.
 */
HAL_RET_T HAL_BB_SetBoardDebugModeProxy(uint8_t u8_mode)
{
    STRU_WIRELESS_CONFIG_CHANGE st_cmd;

    st_cmd.u8_configClass  = WIRELESS_DEBUG_CHANGE;
    st_cmd.u8_configItem   = 0;
    st_cmd.u32_configValue = (u8_mode & 1);

    return SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&st_cmd);
}


/** 
 * @brief   init the uart remote session
 * @param   None
 * @retval  HAL_OK                          means the uart sessiion init OK
 *          HAL_BB_ERR_INIT_SESSION         means some error happens in the uart session init.
 * @note    The function can only be called by cpu0,1 
 */
HAL_RET_T HAL_BB_UartComRemoteSessionInit(void)
{
    BB_UARTComRemoteSessionInit();

    return HAL_OK;
}


/** 
 * @brief   register one uart session for send or receive message
 * @param   e_sessionId:                    the session id to request
 * @param   sessionEventHandler             the handler for uart session
 * @return  HAL_OK:                         means register session OK
 *          HAL_BB_ERR_SESSION_OCCUPIED:    session ID is already occupied
 */
HAL_RET_T HAL_BB_UartComRegisterSession(ENUM_BBUARTCOMSESSIONID e_sessionId,
                                      ENUM_BB_UART_SESSION_PRIORITY e_sessionPriority,
                                      ENUM_BB_UART_SESSION_DATA_TYPE e_sessionDataType,
                                      SYS_Event_Handler rcvDataEventHandler)
{
    uint8_t u8_ret;

    u8_ret = BB_UARTComRegisterSession(e_sessionId, e_sessionPriority, e_sessionDataType);
    if ( u8_ret == 1 )
    {
        uint32_t u32_rcv_event;
        uint32_t u32_snd_event;

        if ( get_session_eventid(e_sessionId, &u32_rcv_event))
        {
            if ( rcvDataEventHandler )
            {
                //dlog_info("register: %d %x", u32_event, rcvDataEventHandler);
                SYS_EVENT_RegisterHandler(u32_rcv_event, rcvDataEventHandler);
            }

            return HAL_OK;        
        }
    }

    return HAL_BB_ERR_SESSION_OCCUPIED;
}


/** 
 * @brief   unregister one uart session
 * @param   e_sessionId:                    the session id has already requested.
 * @return  HAL_OK:                         means unrequest session OK 
 *          HAL_BB_ERR_UNREGISTER_SESSION:  means some error happens in unregister session
 */
HAL_RET_T HAL_BB_UartComUnRegisterSession(ENUM_BBUARTCOMSESSIONID e_sessionId, SYS_Event_Handler rcvDataEventHandler)
{
    uint32_t u32_rcv_event;
    uint32_t u32_snd_event;

    BB_UARTComUnRegisterSession( e_sessionId );

    if ( get_session_eventid(e_sessionId, &u32_rcv_event))
    {
        if ( rcvDataEventHandler )
        {
            SYS_EVENT_UnRegisterHandler(u32_rcv_event, rcvDataEventHandler);
        }

        return HAL_OK;        
    }

    return HAL_BB_ERR_UNREGISTER_SESSION;
}


/** 
 * @brief   send out messages from uart session
 * @param   e_sessionId:                    the session id has already requested.
 *          pu8_dataBuf:                    buffer pointer to the data to be sent
 *          u32_length:                     data size to be sent
 *
 * @return  HAL_OK:                         means ungister session OK 
 *          HAL_BB_ERR_UNREGISTER_SESSION:  means some error happens in unregister session 
 */
HAL_RET_T HAL_BB_UartComSendMsg(ENUM_BBUARTCOMSESSIONID e_sessionId, 
                                uint8_t  *pu8_dataBuf, 
                                uint32_t u32_length)
{
    uint8_t u8_ret = BB_UARTComSendMsg(e_sessionId, pu8_dataBuf, u32_length);
    if ( u8_ret )
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_SESSION_SEND;
    }
}


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
                                   uint32_t *pu32_dataLen)
{
    uint32_t u32_rcvCnt = BB_UARTComReceiveMsg(e_sessionId, pu8_dataBuf, u32_lengthMax);
    *pu32_dataLen = u32_rcvCnt;

    return HAL_OK;
}


/** 
 * @brief   set Baseband Rc frequency setting registers
 * @param   u32_freqSetting:                the registers 
 * @retval  HAL_OK,                         means command is sent sucessfully. 
 * @retval  HAL_BB_ERR_EVENT_NOTIFY         means error happens in sending the command to cpu2                        
 * @note    The function can only be called by cpu0,1, and only call for debug.
 */
HAL_RET_T HAL_BB_SetRcFreqProxy(uint32_t u32_freqSetting)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;

    cmd.u8_configClass   = WIRELESS_FREQ_CHANGE;
    cmd.u8_configItem    = RC_CHANNEL_FREQ;
    cmd.u32_configValue  = u32_freqSetting;

	return SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&cmd);
}


/** 
 * @brief   set Baseband It frequency setting registers
 * @param   u32_freqSetting:                the registers setting
 * @retval  HAL_OK,                         means command is sent sucessfully. 
 * @retval  HAL_BB_ERR_EVENT_NOTIFY         means error happens in sending the command to cpu2                        
 * @note    The function can only be called by cpu0,1, and only call for debug.
 */
HAL_RET_T HAL_BB_SetItFreqProxy(uint32_t u32_freqSetting)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;

    cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
    cmd.u8_configItem   = IT_CHANNEL_FREQ;
    cmd.u32_configValue  = u32_freqSetting;

	return SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&cmd);
}


/** 
 * @brief   set Baseband to It only mode
 * @param   mode                            1: enter It only mode  0: exit from the debug mode
 * @retval  HAL_OK,                         means command is sent sucessfully. 
 * @retval  HAL_BB_ERR_EVENT_NOTIFY         means error happens in sending the command to cpu2                        
 * @note    The function can only be called by cpu0,1, and only call for debug.
 */
HAL_RET_T HAL_BB_SetItOnlyFreqProxy(uint8_t mode)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;

    cmd.u8_configClass  = WIRELESS_MISC;
    cmd.u8_configItem   = MICS_IT_ONLY_MODE;
    cmd.u32_configValue = mode;

	return SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&cmd);
}


/** 
 * @brief   write RF 8003s register by spi
 * @param   u8_addr:                        rf 8003s register register address 
 * @param   u8_data:                        the data value to write to register 
 * @retval  HAL_OK,                         means write succesfully
 * @retval  HAL_BB_ERR_SPI_WRITE            spi write fail
 * @note    The function can only be called by cpu0,1, and only call for debug.
 */
HAL_RET_T HAL_RF8003S_WriteReg(uint16_t u16_addr, uint8_t u8_data)
{ 
    if (0 == RF_SPI_WriteReg(u16_addr, u8_data))
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_SPI_WRITE;
    }
}

/** 
 * @brief   read RF 8003s register by spi
 * @param   u8_addr:                        rf 8003s register register address 
 * @param   pu8_regValue:                   pointer to the address to store rf 8003 register value
 * @retval  HAL_OK,                         means read succesfully
 * @retval  HAL_BB_ERR_SPI_READ             spi read fail
 * @note    The function can only be called by cpu0,1, and only call for debug.
 */
HAL_RET_T HAL_RF8003S_ReadByte(uint16_t u16_addr, uint8_t *pu8_regValue)
{
    if ( 0 == RF_SPI_ReadReg(u16_addr, pu8_regValue))
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_SPI_READ;
    }
}


/** 
 * @brief   write baseband register by spi
 * @param   e_page                          register in the page
 * @param   u8_addr:                        rf 8003s register register address 
 * @param   u8_data:                        the data value to write to register 
 * @retval  HAL_OK,                         means write succesfully
 * @retval  HAL_BB_ERR_SPI_WRITE            spi write fail
 * @note    The function can only be called by cpu0,1, and only call for debug.
 */
HAL_RET_T HAL_BB_WriteByte(ENUM_REG_PAGES e_page, uint8_t u8_addr, uint8_t u8_data)
{
    if (0 == BB_SPI_WriteByte(e_page, u8_addr, u8_data))
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_SPI_READ;
    }
}


/** 
 * @brief   write current page baseband register by spi
 * @param   u8_addr:                        rf 8003s register register address 
 * @param   u8_data:                        the data value to write to register 
 * @retval  HAL_OK,                         means write succesfully
 * @retval  HAL_BB_ERR_SPI_WRITE            spi write fail
 * @note    The function can only be called by cpu0,1, and only call for debug.
 */
HAL_RET_T HAL_BB_CurPageWriteByte(uint8_t u8_addr, uint8_t u8_data)
{
    if (0 == BB_SPI_curPageWriteByte(u8_addr, u8_data))
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_SPI_READ;
    }
}

/** 
 * @brief   read current page baseband register by spi
 * @param   u8_addr:                        rf 8003s register register address 
 * @param   pu8_regValue:                   pointer to the address to store rf 8003 register value 
 * @retval  HAL_OK,                         means write succesfully
 * @retval  HAL_BB_ERR_SPI_WRITE            spi write fail
 * @note    The function can only be called by cpu0,1, and only call for debug.
 */
HAL_RET_T HAL_BB_CurPageReadByte(uint8_t u8_addr, uint8_t *pu8_regValue)
{
    *pu8_regValue = BB_SPI_curPageReadByte(u8_addr);
    
    return HAL_OK;
}


/** 
 * @brief       Set RC channel selection RUN mode(AUTO/Manual)
 * @param[in]   e_mode:                  the modulation QAM mode for rc.
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1  
 */
HAL_RET_T HAL_BB_SetRcChannelSelectionModeProxy(ENUM_RUN_MODE e_mode)
{
    uint8_t u8_ret;
    STRU_WIRELESS_CONFIG_CHANGE st_cmd;
    
    st_cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
    st_cmd.u8_configItem   = RC_CHANNEL_MODE;
    st_cmd.u32_configValue = (uint32_t)e_mode;

    u8_ret = SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&st_cmd);
    if( u8_ret )
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_EVENT_NOTIFY;
    }    
}

/** 
 * @brief       switch encoder channel on/off
 * @param[in]   u8_ch:   channel,0:ch1,1:ch2
 * @param[in]   u8_data: 0:off, 1:on 
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1        
 */
HAL_RET_T HAL_BB_SwitchOnOffChProxy(uint8_t u8_ch, uint8_t u8_data)
{
    uint8_t u8_ret;
    STRU_WIRELESS_CONFIG_CHANGE st_cmd;
    
    st_cmd.u8_configClass  = WIRELESS_OTHER;
    if (0 == u8_ch)
    {
        st_cmd.u8_configItem   = SWITCH_ON_OFF_CH1;
    }
    else
    {
        st_cmd.u8_configItem   = SWITCH_ON_OFF_CH2;
    }

    st_cmd.u32_configValue = u8_data;

    u8_ret = SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&st_cmd);
    if( u8_ret )
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_EVENT_NOTIFY;
    }
}

/** 
 * @brief       Set It(image transmit) QAM
 * @param[in]   e_qam:                  the modulation QAM mode for image transmit.
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1  
 */
HAL_RET_T HAL_BB_SetFreqBandQamSelectionProxy(ENUM_BB_QAM e_qam)
{
    uint8_t u8_ret;
    STRU_WIRELESS_CONFIG_CHANGE st_cmd;
    
    st_cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
    st_cmd.u8_configItem   = MCS_IT_QAM_SELECT;
    st_cmd.u32_configValue = (uint32_t)e_qam;

    u8_ret = SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&st_cmd);
    if( u8_ret )
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_EVENT_NOTIFY;
    }
}

/** 
 * @brief       Set rc QAM
 * @param[in]   e_qam:                  the modulation QAM mode for image transmit.
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1  
 */
HAL_RET_T HAL_BB_SetRcQamSelectionProxy(ENUM_BB_QAM e_qam)
{
    uint8_t u8_ret;
    STRU_WIRELESS_CONFIG_CHANGE st_cmd;
    
    st_cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
    st_cmd.u8_configItem   = MCS_RC_QAM_SELECT;
    st_cmd.u32_configValue = (uint32_t)e_qam;

    u8_ret = SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&st_cmd);
    if( u8_ret )
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_EVENT_NOTIFY;
    }
}

/** 
 * @brief       force baseband reset.
 * @param       none.
 * @retval      HAL_OK,                  means command is sent sucessfully. 
 * @retval      HAL_BB_ERR_EVENT_NOTIFY  means error happens in sending the command to cpu2
 * @note        The function can only be called by cpu0,1         
 */
HAL_RET_T HAL_BB_SoftResetProxy(void)
{
    uint8_t u8_ret;
    STRU_WIRELESS_CONFIG_CHANGE st_cmd;
    
    st_cmd.u8_configClass  = WIRELESS_OTHER;
    st_cmd.u8_configItem   = BB_SOFT_RESET;
    st_cmd.u32_configValue = 0;

    u8_ret = SYS_EVENT_Notify(SYS_EVENT_ID_USER_CFG_CHANGE, (void *)&st_cmd);
    if( u8_ret )
    {
        return HAL_OK;
    }
    else
    {
        return HAL_BB_ERR_EVENT_NOTIFY;
    }
}

/** 
 * @brief   
 * @param   
 * @retval            
 */
HAL_RET_T HAL_BB_SPI_DisableEnable(uint8_t u8_flag)
{
    BB_SPI_DisableEnable(u8_flag);
    
    return HAL_OK;
}

