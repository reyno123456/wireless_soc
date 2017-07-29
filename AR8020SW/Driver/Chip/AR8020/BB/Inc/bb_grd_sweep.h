#ifndef _SWEEP_FREQ_SERVICE_H__
#define _SWEEP_FREQ_SERVICE_H__

#include <stdint.h>
#include "bb_ctrl_internal.h"

typedef enum _ENUM_CH_SEL_OPT
{
    SELECT_OPT      = 0,
    SELECT_MAIN_OPT = 1,
    SELECT_OTHER    = 2
}ENUM_CH_SEL_OPT;


#define MAIN_CH_CYCLE   (0)
#define OPT_CH_CYCLE    (1)
#define OTHER_CH_CYCLE  (2)
#define OTHER_BAND_CH_CYCLE     (3)

#define CMP_POWER_AVR_LEVEL             (3)


void BB_SweepStart( ENUM_RF_BAND e_rfBand, ENUM_CH_BW e_bw);

uint8_t BB_GetSweepedChResult( uint8_t flag );

void BB_GetSweepNoise(int16_t *ptr_noise_power);

int16_t compare_chNoisePower(ENUM_RF_BAND e_band,
                             uint8_t u8_itCh1, uint8_t u8_itCh2, 
                             int16_t *ps16_averdiff, int16_t *ps16_fluctdiff, 
                             uint8_t log);


uint8_t BB_selectBestCh(ENUM_RF_BAND e_band, ENUM_CH_SEL_OPT e_opt,
                        uint8_t *pu8_mainCh, uint8_t *u8_optCh, uint8_t *u8_other,
                        uint8_t log);

int32_t BB_Sweep_updateCh(ENUM_RF_BAND e_band, uint8_t mainch);

uint8_t get_opt_channel( void );

int32_t BB_CompareCh1Ch2ByPowerAver(ENUM_RF_BAND e_rfBand, uint8_t u8_itCh1, uint8_t u8_itCh2, int32_t level);

int32_t BB_CompareCh1Ch2ByPower(ENUM_RF_BAND e_rfBand, uint8_t u8_itCh1, uint8_t u8_itCh2, uint8_t u8_cnt);

int32_t grd_doRfbandChange( uint8_t *pu8_mainCh, uint8_t *pu8_optCh );

int32_t grd_checkBackTo2G(void);

void grd_checkBlockMode(void);

void grd_notifyRfbandChange( ENUM_RF_BAND e_band, uint8_t u8_itCh, uint8_t u8_optCh, uint8_t u8_delayCnt );


#endif

