#ifndef BB_SNR_SERVICE_H__
#define BB_SNR_SERVICE_H__

#include "bb_types.h"
typedef enum
{
    QAMUP,
    QAMDOWN,
    QAMKEEP
}QAMUPDONW;

uint8_t snr_static_for_qam_change(uint16_t threshod_left_section,uint16_t threshold_right_section);

uint16_t get_snr_average(void);

uint16_t grd_get_it_snr();

int grd_check_piecewiseSnrPass(uint8_t u8_flag_start, uint16_t u16_thld);

int grd_check_piecewiseSnrPass(uint8_t u8_flag_start, uint16_t u16_thld);

void grd_set_txmsg_mcs_change(ENUM_CH_BW bw, uint8_t index);

void grd_judge_qam_mode(void);

void grd_set_mcs_registers(ENUM_BB_QAM e_qam, ENUM_BB_LDPC e_ldpc, ENUM_CH_BW e_bw);


#endif /* __CONFIG_H__ */

/************************ (C) COPYRIGHT Artosyn *****END OF FILE****/
