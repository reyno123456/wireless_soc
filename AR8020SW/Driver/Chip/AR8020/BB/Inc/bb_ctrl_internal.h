#ifndef __BB_INTERNAL_CTRL_
#define __BB_INTERNAL_CTRL_

#include <stdint.h>

#include "bb_ctrl.h"
#include "bb_spi.h"
#include "rf_if.h"
#include "bb_regs.h"


#define MAX_2G_RC_FRQ_SIZE          (34)
#define MAX_2G_IT_FRQ_SIZE          (8)

#define MAX_5G_RC_FRQ_SIZE          (40)
#define MAX_5G_IT_FRQ_SIZE          (13)


#define RC_FRQ_MAX_MASK_NUM         (10)
#define RC_FRQ_MASK_THRESHOLD       (5)
#define RF_FRQ_MAX_NUM              ((MAX_2G_RC_FRQ_SIZE>=MAX_5G_RC_FRQ_SIZE)?MAX_2G_RC_FRQ_SIZE:MAX_5G_RC_FRQ_SIZE)


#define OTHER_BAND(band)    ( (band == RF_2G)?  RF_5G : RF_2G )
#define BAND_CHANGE_DELAY   (40)
typedef enum
{
    IDLE,
    INIT_DATA,
    FEC_UNLOCK,
    FEC_LOCK,
    DELAY_14MS,
    CHECK_FEC_LOCK,
    ID_MATCH_LOCK,
    SEARCH_ID,
    CHECK_ID_MATCH
}DEVICE_STATE;


#define FALSE   (0)
#define TRUE    (1)


#define  BLUE_LED_GPIO      (67)
#define  RED_LED_GPIO       (71)


typedef struct _STRU_FRQ_CHANNEL           //  Remote Controller Freq Channnel
{
    uint8_t frq1;
    uint8_t frq2;
    uint8_t frq3;
    uint8_t frq4;
}STRU_FRQ_CHANNEL;


typedef enum
{
    MISC_READ_RF_REG,
    MISC_WRITE_RF_REG,
    MISC_READ_BB_REG,
    MISC_WRITE_BB_REG,
} ENUM_WIRELESS_MISC_ITEM;



#define  SKY_LOCK_STATUS        (0)
#define  RC_MASK_CODE           (1)
#define  SKY_AGC_STATUS         (2)
#define  RF_BAND_SWITCH         (3)

typedef struct _SysEvent_SkyStatus
{
    uint8_t pid;
    union
    {
        struct
        {
            uint8_t u8_rcNrLockCnt;
            uint8_t u8_rcCrcLockCnt;
        } rcLockCnt;

        struct
        {
            uint8_t u8_skyagc1;
            uint8_t u8_skyagc2;
        }skyAgc;

        uint64_t u64_rcMask;
    } par;
} STRU_SysEventSkyStatus;

typedef struct
{
    uint8_t u8_rcNrLockCnt;
    uint8_t u8_rcCrcLockCnt;
    uint8_t u8_skyagc1;
    uint8_t u8_skyagc2;
} STRU_SkyStatus;


#define QAM_CHANGE_THRESHOLD_COUNT (6)

typedef struct _STRU_BandChange
{
    uint8_t             flag_bandchange;
    uint8_t             u8_bandChangecount;
    uint8_t             u8_ItCh;
    uint8_t             u8_optCh;
    uint8_t             u8_unlockLoopCnt;
    //uint8_t             u8_frqoffset[3];   //3bytes frequency offset
    //uint8_t             u8_softfrqoffset;
}STRU_BandChange;

typedef struct
{
    uint8_t             cur_IT_ch;
    uint8_t             next_IT_ch;
    uint8_t             it_manual_ch;
    ENUM_RUN_MODE       e_rfbandMode;
    uint8_t             fec_unlock_cnt;
    uint16_t            rc_unlock_cnt;

    ENUM_RUN_MODE       it_skip_freq_mode;
    ENUM_RUN_MODE       rc_skip_freq_mode;
    ENUM_RUN_MODE       qam_skip_mode;
    ENUM_RUN_MODE       brc_mode;
    ENUM_TRX_CTRL       trx_ctrl;
    uint8_t             brc_bps[2];             //unit: Mbps

    ENUM_RF_BAND        e_bandsupport;          //2.4G, 5.8G
    ENUM_RF_BAND        e_curBand;              //current band: 2.4G, 5.8G
    ENUM_CH_BW          CH_bandwidth;    //10M, 20M
    ENUM_BB_QAM         qam_mode;
    ENUM_BB_QAM         rc_qam_mode;
    ENUM_BB_LDPC        ldpc;
    ENUM_BB_LDPC        rc_ldpc;
    
    uint8_t             qam_ldpc;
    uint8_t             enable_plot;
    uint8_t             id[5];
    uint8_t             need_write_flash;
    uint8_t             pwr;

    uint8_t             locked;
    uint8_t             rc_status;
    
    uint16_t            qam_threshold_range[QAM_CHANGE_THRESHOLD_COUNT][2];
    uint16_t            mosaic;
    uint8_t             default_fac_setting;
    uint8_t             search_id_enable;
    uint8_t             bb_power;    
    ENUM_RUN_MODE       rf_power_mode;
    uint8_t             enable_freq_offset;
    uint8_t             flag_mrc;
    uint8_t             flag_updateRc; //need to update Remote controller frequency

    uint16_t            cycle_count; 

    uint32_t            u32_rcValue;
    uint32_t            qam_switch_time;

    uint8_t             ldpc_error_move_cnt;
    DEVICE_STATE        dev_state;

    uint8_t             u8_idSrcSel; /* 0:id comes from flash,this is default value.
                                        1:id comes from automatic search*/
    uint8_t             u8_flashId[6]; // 5B(rc_id) + 1B(check)
    uint8_t             u8_debugMode;
    uint8_t             u8_flagdebugRequest;
    STRU_FRQ_CHANNEL    stru_rcRegs;
    STRU_FRQ_CHANNEL    stru_itRegs;
    ENUM_BB_MODE        en_bbmode;
    uint8_t             agclevel;

    uint8_t             u8_bbStartMcs;
    uint8_t             u8_harqcnt_lock;

    uint8_t             flag_signalBlock;
    uint8_t             u8_agc[4];
    STRU_BandChange     stru_bandChange;
}CONTEXT;


typedef struct param
{
    ENUM_RUN_MODE   it_skip_freq_mode;
    ENUM_RUN_MODE   rc_skip_freq_mode;
    ENUM_RUN_MODE   qam_skip_mode;
    uint16_t        qam_change_threshold[QAM_CHANGE_THRESHOLD_COUNT][2];
}PARAM;


#define CALC_DIST_RAW_DATA_MAX_RECORD    (100)

typedef enum
{
    INVALID,
    CALI_ZERO,
    CALC_DIST_PREPARE,
    CALC_DIST
}ENUM_CALC_DIST_STATUS;


typedef struct
{
    ENUM_CALC_DIST_STATUS e_status;
    uint8_t     u8_rawData[CALC_DIST_RAW_DATA_MAX_RECORD][3];
    uint32_t    u32_calcDistValue;
    uint32_t    u32_calcDistZero;
    uint32_t    u32_cnt;
    uint32_t    u32_lockCnt;
} STRU_CALC_DIST_DATA;




typedef struct
{
    uint64_t    u64_mask; // bit0 <-> freq0  ... bit63 <-> freq63
    uint64_t    u64_rcvGrdMask; // received mask from spi, send by grd
    uint8_t     u8_unLock[RF_FRQ_MAX_NUM]; // continue unlock cnt 
    uint8_t     u8_maskOrderIndex;
    uint8_t     u8_maskOrder[RC_FRQ_MAX_MASK_NUM]; // u8_maskOrder[0] earlist masked frq's channel                                      
} STRU_RC_FRQ_MASK;


extern volatile CONTEXT context;
extern volatile DEVICE_STATE dev_state;



#define MAX(a,b) (((a) > (b)) ?  (a) :  (b) )

void BB_uart10_spi_sel(uint32_t sel_dat);

int BB_softReset(ENUM_BB_MODE en_mode);




uint8_t BB_ReadReg(ENUM_REG_PAGES page, uint8_t addr);

uint8_t BB_WriteReg(ENUM_REG_PAGES page, uint8_t addr, uint8_t data);

int BB_WriteRegMask(ENUM_REG_PAGES page, uint8_t addr, uint8_t data, uint8_t mask);


uint8_t BB_map_modulation_to_br(uint8_t mod);

ENUM_BB_LDPC BB_get_LDPC(void);

ENUM_BB_QAM BB_get_QAM(void);

void BB_set_RF_Band(ENUM_BB_MODE sky_ground, ENUM_RF_BAND rf_band);

void BB_set_RF_bandwitdh(ENUM_BB_MODE sky_ground, ENUM_CH_BW rf_bw);


int BB_GetCmd(STRU_WIRELESS_CONFIG_CHANGE *pconfig);

void BB_HandleEventsCallback(void *p);

void BB_handle_misc_cmds(STRU_WIRELESS_CONFIG_CHANGE* pcmd);

int BB_add_cmds(uint8_t type, uint32_t param0, uint32_t param1, uint32_t param2, uint32_t param3);

void BB_set_QAM(ENUM_BB_QAM mod);

void BB_set_LDPC(ENUM_BB_LDPC ldpc);

uint8_t BB_write_ItRegs(uint32_t u32_it);

void sky_set_McsByIndex(ENUM_CH_BW bw, uint8_t idx);

int BB_WriteRegMask(ENUM_REG_PAGES page, uint8_t addr, uint8_t data, uint8_t mask);

PARAM * BB_get_sys_param(void);

int grd_cal_frqOffset( uint8_t ch, ENUM_RF_BAND e_band );

int grd_GetDistAverage(int *pDist);

#endif
