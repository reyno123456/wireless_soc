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


void BB_GRD_start(void);

void grd_SetRCId(uint8_t *pu8_id);

void grd_SetSaveRCId(uint8_t *pu8_id);

uint8_t grd_is_bb_fec_lock(void);

uint8_t snr_static_for_qam_change(uint16_t threshod_left_section,uint16_t threshold_right_section);

void grd_set_txmsg_mcs_change(ENUM_CH_BW bw, uint8_t index);

static void grd_handle_RC_mode_cmd(ENUM_RUN_MODE mode);

#endif
