#ifndef __RF9363_H__
#define __RF9363_H__

#define     AAGC_GAIN_FAR                   (0)
#define     AAGC_GAIN_NEAR                  (1)


#include "boardParameters.h"

int RF_SPI_WriteReg(uint16_t u8_addr, uint8_t u8_data);

int RF_SPI_ReadReg(uint16_t u8_addr, uint8_t *pu8_rxValue);

void RF_init(ENUM_BB_MODE en_mode);

void RF_CaliProcess(ENUM_BB_MODE en_mode);

void BB_RF_band_switch(ENUM_RF_BAND rf_band);


void BB_grd_notify_it_skip_freq(ENUM_RF_BAND band, uint8_t u8_ch);


void BB_write_ItRegs(uint32_t u32_it);
uint8_t BB_write_ItRegsByArr(uint8_t *pu8_it);


uint8_t BB_set_ItFrqByCh(ENUM_RF_BAND band, uint8_t ch);


uint8_t BB_write_RcRegs(uint32_t u32_rc);


uint8_t BB_set_Rcfrq(ENUM_RF_BAND band, uint8_t ch);


uint8_t BB_set_SweepFrq(ENUM_RF_BAND band, ENUM_CH_BW e_bw, uint8_t ch);


uint8_t BB_GetRcFrqNum(ENUM_RF_BAND e_rfBand);

uint8_t BB_GetItFrqNum(ENUM_RF_BAND e_rfBand);

uint8_t BB_SetAgcGain(uint8_t gain);

int32_t BB_SweepEnergyCompensation(int32_t data);

#endif
