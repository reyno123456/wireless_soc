/*****************************************************************************
 * Copyright: 2016-2020, Artosyn. Co., Ltd.
 * File name: canDef.h
 * Description: can controller hardware address definition and macro definition 
 * Author: Artosyn FW
 * Version: V0.01 
 * Date: 2016.11.29
 * History: 
 * 2016.11.29 the first edition
 * *****************************************************************************/

#ifndef  __CAN_DEF_H__
#define  __CAN_DEF_H__


#include <stdint.h>
#include <stdio.h>
#include "debuglog.h"
#include "interrupt.h"


/*******************Macro define**************************/
#define BASE_ADDR_CAN0 (0x40300000)
#define BASE_ADDR_CAN1 (0x40340000)
#define BASE_ADDR_CAN2 (0x40380000)
#define BASE_ADDR_CAN3 (0x403C0000)

//define register RTIE
#define CAN_RTIE_RIE    (1<<7)
#define CAN_RTIE_ROIE   (1<<6)
#define CAN_RTIE_RFIE   (1<<5)
#define CAN_RTIE_RAFIE  (1<<4)
#define CAN_RTIE_TPIE   (1<<3)
#define CAN_RTIE_TSIE   (1<<2)
#define CAN_RTIE_EIE    (1<<1)

//define register TCMD
#define CAN_TCMD_TBSEL  (1<<7)
#define CAN_TCMD_LOM    (1<<6)
#define CAN_TCMD_STBY   (1<<5)
#define CAN_TCMD_TPE    (1<<4)
#define CAN_TCMD_TPA    (1<<3)
#define CAN_TCMD_TSONE  (1<<2)
#define CAN_TCMD_TSALL  (1<<1)
#define CAN_TCMD_TSA    (1<<0)

//define register TBUF
#define CAN_TBUF_IDE    (1<<7)  //std or ext
#define CAN_TBUF_RTR    (1<<6)  //data or remote
#define CAN_TBUF_EDL    (1<<5)
#define CAN_TBUF_BRS    (1<<4)

//define register CFG_STAT
#define CFG_STAT_RESET     (1<<7) // 
#define CFG_STAT_LBME      (1<<6) // 
#define CFG_STAT_LBMI      (1<<5) // 
#define CFG_STAT_TPSS      (1<<4) // 
#define CFG_STAT_TSSS      (1<<3) // 
#define CFG_STAT_RACTIVE   (1<<2) // Reception ACTIVE (Receive Status bit)
#define CFG_STAT_TACTIVE   (1<<1) // Transmission ACTIVE (Transmit Status bit)
#define CFG_STAT_BUSOFF    (1<<0) // 


/*******************can register define**************************/
typedef struct{
    volatile uint32_t    u32_rxBuf[18];          // 0x00-0x47
    volatile uint32_t    u32_txBuf[18];          // 0x48-0x8f
    volatile uint32_t    u32_reg3;               // 0x90 --> RCTRL     TCTRL    TCMD      CFG_STAT
    volatile uint32_t    u32_reg4;               // 0x94 --> LIMIT     ERRINT   RTIF      RTIE
    volatile uint32_t    u32_reg5;               // 0x98 -->  --       BITTIME2 BITTIME1  BITTIME0
    volatile uint32_t    u32_reg6;               // 0x9c -->  --       TDC      F_PRESC   S_PRESC
    volatile uint32_t    u32_reg7;               // 0xa0 -->  TECNT    RECNT    --        EALCAP
    volatile uint32_t    u32_reg8;               // 0xa4 -->  ACF_EN1  ACF_EN0  --        ACFCTRL
    volatile uint32_t    u32_reg9;               // 0xa8 -->  ACF3     ACF2     ACF1      ACF0
    volatile uint32_t    u32_reg10;              // 0xac -->  --       --       VER1      VER0
    volatile uint32_t    u32_reg11;              // 0xb0 -->  REF_MSG3 REF_MSG2 REF_MSG1  REF_MSG0
    volatile uint32_t    u32_reg12;              // 0xb4 -->  TT_TRIG1 TT_TRIG0 TRIG_CFG1 TRIG_CFG0
    volatile uint32_t    u32_reg13;              // 0xb8 -->  --       --       TT_WTRIG1 TT_WTRIG0
} STRU_CAN_TYPE;


#endif
