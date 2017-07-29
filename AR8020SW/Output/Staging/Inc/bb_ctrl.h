#ifndef __BB_API_
#define __BB_API_

#include "bb_types.h"
#include "boardParameters.h"


/** 
 * @brief       API for Baseband initialize.
 * @param[in]   en_mode: brief @ENUM_BB_MODE
 */

void BB_init(ENUM_BB_MODE en_mode, STRU_BoardCfg *boardCfg);


/** 
 * @brief       API for set frequency band (2G/5G) selection mode (ATUO / Manual)
 * @param[in]   en_mode: selection mode (ATUO / Manual)
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetFreqBandSelectionMode(ENUM_RUN_MODE en_mode);

/** 
 * @brief       API for set frequency band (2G/5G)
 * @param[in]   en_mode: selection mode (ATUO / Manual)
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetFreqBand(ENUM_RF_BAND band);


/** 
 * @brief       API for set IT(image transmit) channel selection RUN mode(AUTO/Manual).
 * @param[in]   qam: the modulation QAM mode for image transmit.
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetITChannelSelectionMode(ENUM_RUN_MODE en_mode);


/** 
 * @brief       API for set IT(image transmit) channel Number.
 * @param[in]   channelNum: the current channel number int current Frequency band(2G/5G)
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetITChannel(uint8_t channelNum);


/** 
 * @brief       API for set MCS(modulation, coderate scheme) mode
 * @param[in]   en_mode: auto or manual selection.
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetMCSmode(ENUM_RUN_MODE en_mode);


/** 
 * @brief       API for set the image transmit QAM mode
 * @param[in]   qam: modulation qam mode
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetITQAM(ENUM_BB_QAM qam);



/** 
 * @brief       API for set the image transmit LDPC coderate
 * @param[in]   ldpc:  ldpc coderate 
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetITLDPC(ENUM_BB_LDPC ldpc);



/** 
 * @brief       API for set the encoder bitrate control mode
 * @param[in]   en_mode: auto or manual selection.
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetEncoderBrcMode(ENUM_RUN_MODE en_mode);



/** 
 * @brief       API for set the encoder bitrate Unit:Mbps
 * @param[in]   bitrate_Mbps: select the bitrate unit: Mbps
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetEncoderBitrateCh1(uint8_t bitrate_Mbps);
int BB_SetEncoderBitrateCh2(uint8_t bitrate_Mbps);



/** 
 * @brief       API for set the encoder bitrate Unit:Mbps
 * @param[in]   bitrate_Mbps: select the bitrate unit: Mbps
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
void BB_uart10_spi_sel(uint32_t sel_dat);


uint8_t BB_write_RcRegs(uint32_t u32_rc);


/** 
 * @brief       
 * @param   
 * @retval      
 * @note      
 */
int BB_GetDevInfo(void);

/** 
 * @brief       
 * @param   
 * @retval      
 * @note      
 */
int BB_SwtichOnOffCh(uint8_t u8_ch, uint8_t u8_data);


void BB_grd_notify_it_skip_freq(ENUM_RF_BAND band, uint8_t u8_ch);

void BB_grd_notify_rc_skip_freq(uint32_t u32_rcfrq);


void BB_grd_notify_it_skip_freq_1(void);

uint8_t BB_get_bitrateByMcs(ENUM_CH_BW bw, uint8_t u8_mcs);

int BB_InsertCmd(STRU_WIRELESS_CONFIG_CHANGE *p);

int BB_WrSpiChkFlag(void);

int BB_ChkSpiFlag(void);


#endif
