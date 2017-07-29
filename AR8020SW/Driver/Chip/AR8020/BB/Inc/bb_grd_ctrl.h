/**
  ******************************************************************************
  * @file    ground_controller.h
  * @author  Artosyn AE/FAE Team
  * @version V1.0
  * @date    03-21-2016
  * @brief
  *
  *
  ******************************************************************************
  */
#ifndef __GRD_CONTROLLER_H
#define __GRD_CONTROLLER_H

#include <stdint.h>
#include "debuglog.h"

//*************************  Data Structure Define  **********************

void grd_add_snr_daq(void);

void BB_GRD_start(void);

void BB_Grd_SetRCId(uint8_t *pu8_id);

void wimax_vsoc_tx_isr(uint32_t u32_vectorNum);


void grd_rc_hopfreq(void);

void reset_it_span_cnt(void);

uint8_t grd_is_bb_fec_lock(void);

uint8_t is_it_need_skip_freq(uint8_t qam_ldpc);

void grd_set_txmsg_qam_change(ENUM_BB_QAM qam, ENUM_CH_BW bw, ENUM_BB_LDPC ldpc);

uint8_t snr_static_for_qam_change(uint16_t threshod_left_section,uint16_t threshold_right_section);

void grd_handle_IT_mode_cmd(ENUM_RUN_MODE mode);

void grd_handle_IT_CH_cmd(uint8_t ch);

static void Grd_Timer2_7_Init(void);

static void Grd_Timer2_6_Init(void);

static void grd_handle_RF_band_cmd(ENUM_RF_BAND rf_band);

static void grd_handle_CH_bandwitdh_cmd(ENUM_CH_BW bw);

static void grd_handle_MCS_mode_cmd(ENUM_RUN_MODE mode);

static void grd_handle_MCS_cmd(ENUM_BB_QAM qam, ENUM_BB_LDPC ldpc);

static void grd_handle_brc_mode_cmd(ENUM_RUN_MODE mode);

static void grd_handle_brc_cmd(uint8_t coderate);

static void grd_handle_all_cmds(void);


void grd_set_txmsg_mcs_change(ENUM_CH_BW bw, uint8_t index);

static void grd_handle_RC_mode_cmd(ENUM_RUN_MODE mode);

void grd_handle_IT_mode_cmd(ENUM_RUN_MODE mode);

void grd_calc_dist_zero_calibration(void);

void grd_set_calc_dist_zero_point(uint32_t value);

#endif
