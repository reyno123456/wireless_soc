#ifndef _BRC_H_
#define _BRC_H_

#define ARMCM7_RC //!!!!!!!!!! IMPORTANT !!!!!!!!!!!

#ifdef ARMCM7_RC //###########
    #define RC_MODE_0 0
    #define RC_MODE_1 1
    #define RC_MODE_2 2
    #define RC_MODE_3 3
    #define I_SLICE 2
    #define P_SLICE 0
    #define RC_MODEL_HISTORY 21
    #define MIN_INT64 0x8000000000000000
    #define MIN_INT   0x80000000
    #define MAX_INT   0x7fffffff
    typedef long long int64;
    typedef unsigned char Boolean;
#endif	  //###########

#define MIN_INT64 0x8000000000000000
#define MIN_INT   0x80000000
#define MAX_INT   0x7fffffff

//// generic rate control variables
typedef struct my_rc{
//// top set(img,params)
  unsigned char     enable;                  // 0x00
  unsigned char     rc_enable;
  unsigned char     RCUpdateMode;
  unsigned char     framerate;
  int               height;                  // 0x04
  int               width;                   // 0x08
  int               basicunit;               // 0x0c
  int               bit_rate;                // 0x10
  int               size;                    // 0x14
  unsigned char     PMaxQpChange;            // 0x18
  unsigned char     intra_period;
  unsigned char     RCISliceBitRatio;
  unsigned char     RCISliceBitRatioMax;
  int               no_frm_base;             // 0x1c
  unsigned char     RCMinQP;                 // 0x20
  unsigned char     RCMaxQP;             
  unsigned char     RCIoverPRatio;
  unsigned char     c1_over;                 //
  int               FrameSizeInMbs;          // 0x24
  int               FrameHeightInMbs;        // 0x28
  int               type;                    // 0x2c
  int               BasicUnit;               // 0x30
  int               MBPerRow;                // 0x34
  int               qp;                      // 0x38
  int               header_bits;             // 0x3c
  int               new_bitrate;             // 0x40
  int               PrevBitRate;// lhumod    // 0x44
  int               TotalFrameMAD;// lhumod  // 0x48
  int               RCISliceTargetBits;//@lhu// 0x4c
  unsigned char     re_bitrate;              // 0x50
  unsigned char     cmadequ0;// lhumod
  unsigned char     prev_ac_br_index;// @lhu
  unsigned char     wireless_screen;// @lhu  
  unsigned char     changeToIFrame;// @lhu   // 0x54
  unsigned char     insertOneIFrame;// @lhu  
  unsigned char     PrevFrmPSNRLow; // @lhu
  unsigned char     HBitsRatioABits_level; // @lhu
  unsigned char     PrevIntraPeriod;// @lhu  // 0x58
  unsigned char     gop_change_NotResetRC;// @lhu
  unsigned char     nextPFgotoIF;// @lhu
  unsigned char     IFduration;// @lhu
  unsigned char     PrevRCMinQP;// lhupsnr   	 // 0x5c
  unsigned char     PSNRDropSharply;
  long long         ifrm_ymse, min_pfrm_ymse;    // 0x60
  long long         frm_ymse[2];                 // 0x70
  int               frm_fbits[2];                // 0x80
  int               frm_hbits[2];                // 0x88
  int               frm_abits[2];                // 0x90
  int               RCSliceBits;                 // 0x98
  int               PrevFbits;                   // 0x9c
//
  int               gop_cnt;                     // 0xa0
  int               frame_cnt;                   // 0xa4
  int               bu_cnt;                      // 0xa8
  int               mb_cnt;                      // 0xac
  int               frame_mad;                   // 0xb0
  int               frame_tbits;                 // 0xb4
  int               frame_hbits;                 // 0xb8
  int               frame_abits;                 // 0xbc
  
  int               frame_bs;                    // 0xc0
  int               slice_qp;                    // 0xc4
  int               codedbu_cnt;                 // 0xc8
  int               NumberofHeaderBits;          // 0xcc
  int               NumberofTextureBits;         // 0xd0
  int               NumberofBasicUnitHeaderBits; // 0xd4
  int               NumberofBasicUnitTextureBits;// 0xd8
  int               NumberofGOP;                 // 0xdc
  int               TotalMADBasicUnit;           // 0xe0
  int               CurrentBufferFullness;       // 0xe4        //LIZG 25/10/2002
  int               RemainingBits;               // 0xe8
  int               RCPSliceBits;                // 0xec
  int               RCISliceBits;                // 0xf0
  int               NPSlice;                     // 0xf4
  int               NISlice;                     // 0xf8
  int               GAMMAP_1p;                   // 0xfc         //LIZG, JVT019r1
  int               BETAP_1p;                    // 0x100        //LIZG, JVT019r1
  int               TargetBufferLevel;           // 0x104        //LIZG 25/10/2002
  int               MyInitialQp;                 // 0x108
  int               PAverageQp;                  // 0x10c
  int               PreviousPictureMAD_8p;       // 0x110
  int               MADPictureC1_12p;            // 0x114
  int               MADPictureC2_12p;            // 0x118
  int               PMADPictureC1_12p;           // 0x11c
  int               PMADPictureC2_12p;           // 0x120

  int               PPictureMAD_8p;              // 0x124
  int               PictureMAD_8p  [20];         // 0x174
  int               ReferenceMAD_8p[20];         // 0x1c4
  int               mad_tmp0 [20];               // 0x214
  int               mad_tmp0_valid [20];         // 0x264
  int               mad_tmp1 [20];               // 0x2b4
  int               mad_tmp2 [20];               // 0x304
  int               m_rgQp_8p [20];              // 0x354
  int               m_rgRp_8p [20];              // 0x3A4
  int               m_rgRp_8prr8 [20];           // 0x3f4
  int               rc_tmp0 [20];                // 0x444
  int               rc_tmp1 [20];                // 0x494
  int               rc_tmp2 [20];                // 0x4d4
  int               rc_tmp3 [20];                // 0x534
  int               rc_tmp4 [20];                // 0x584
  
  unsigned char     rc_hold;                     // 0x5d4
  unsigned char     mad_hold;                    // 0x5d5

  char              rc_rgRejected [20];          // 0x5d6
  char              mad_rgRejected [20];         // 0x5ea
  int               m_X1_8p;                     // 0x600
  int               m_X2_8p;                     // 0x604
  int               Pm_X1_8p;                    // 0x608
  int               Pm_X2_8p;                    // 0x60c
  int               Pm_Qp;                       // 0x610
  int               Pm_Hp;                       // 0x614
  int               MADm_windowSize;             // 0x618
  int               m_windowSize;                // 0x61c
  int               m_Qc;                        // 0x620
  int               PPreHeader;                  // 0x624
  int               PrevLastQP;                  // 0x628    // QP of the second-to-last coded frame in the primary layer
  int               CurrLastQP;                  // 0x62c        // QP of the last coded frame in the primary layer
  int               TotalFrameQP;                // 0x630
  int               PAveHeaderBits1;             // 0x634
  int               PAveHeaderBits2;             // 0x638
  int               PAveHeaderBits3;             // 0x63c
  int               PAveFrameQP;                 // 0x640
  int               TotalNumberofBasicUnit;      // 0x644
  int               NumberofCodedPFrame;         // 0x648
  int               TotalQpforPPicture;          // 0x64c
  int               CurrentMAD_8p;               // 0x650
  int               TotalBUMAD_12p;              // 0x654
  int               PreviousMAD_8p;              // 0x658
  int               PreviousWholeFrameMAD_8p;    // 0x65c
  int               DDquant;                     // 0x660
  int               QPLastPFrame;                // 0x664
  int               QPLastGOP;                   // 0x668
  int               FrameQPBuffer;               // 0x66c
  int               FrameAveHeaderBits;          // 0x670
  int               BUPFMAD_8p[70];// lhumod     // 0x674
  int               BUCFMAD_8p[70];// lhumod     // 0x78c
  int               RCISliceActualBits;// lhugop // 0x8a4
  
  int               GOPOverdue;                  // 0x8a8
  // rate control variables
  int               Xp;                          // 0x8ac
  int               Xb;                          // 0x8b0
  int               Target;                      // 0x8b4
  int               Np;                          // 0x8b8
  int               UpperBound1;                 // 0x8bc
  int               UpperBound2;                 // 0x8c0
  int               LowerBound;                  // 0x8c4
  int               DeltaP;                      // 0x8c8
  int               TotalPFrame;                 // 0x8cc
//// for feedback
  int               aof_inc_qp;                  // 0x8d0
  unsigned char     fd_row_cnt;                  // 0x8d4
  unsigned char     fd_last_row;                 
  unsigned char     fd_last_p;
  unsigned char     fd_iframe;
  unsigned char     fd_reset;                    // 0x8d8 
  unsigned char     fd_irq_en;                   
//int    fd_cpu_test;
//// for test
// int    v0_PrevImgType;// added by lhulhu
// int    v0_RCInitialQP;// added by lhulhu
// int    v0_NumberofBasicUnit;// added by lhumod
// int    v0_SceneChange_Simp2Comp;// added by lhusc
// int    v0_PrevFrmBits; // added by lhusc
// int    v0_TargetBitsPerFrm;// added by lhusc

  //s int *pp_BUPFMAD_8p;                                        
  //s int *pp_BUCFMAD_8p;                                        
  int tbits_tmp;                                 // 0x8dc
  int hbits_tmp;                                 // 0x8e0
  int fbits_tmp;                                 // 0x8e4
  int mad_tmp;                                   // 0x8e8
  
//  int qp;                                        // 0x8ec        
  int ymsel_tmp;                                 // 0x8f0
  int ymseh_tmp; // @lhu                         // 0x8f4
  long long ymse_frame;                          // 0x8f8
  Boolean last_p_gop_change;                     // 0x900     
  int last_p_prev_gop;                           // 0x904                         
  int poweron_rc_params_set;                     // 0x908
  unsigned char firstpframe_coming;				 // 
  unsigned char ac_br_index;
  unsigned int dvp_lb_freesize;                
} RC_DATA;


int aof_v0_frame;
int aof_v1_frame;

int  my_imin(int a, int b);
int  my_iequmin(int a, int b);
int  my_imax(int a, int b);
int  my_iClip3(int low, int high, int x);
int  my_sqrt(int x);
int  my_sqrt64(int64 x);
int  Qstep2QP_8p(int Qstep_8p);
int  QP2Qstep_8p(int QP);
void my_rc_ac_br(int view); // @lhu
//void my_calc_prevFramePSNR(unsigned char psnr_level, int view);
unsigned short my_divider2psnr(int my_divider);
unsigned char my_trace_PSNRDropSharply(unsigned char psnr_drop_level, unsigned char view);
void my_criteria_decide_changeToIFrame(unsigned char HBitsRatioABits_level, unsigned char ABitsRatioTargetBits_level, unsigned char PSNRDrop_level, unsigned char view);
void my_decide_backtoNormalGOP(int view);
void my_ac_RCISliceBitRatio(unsigned char RCISliceBitRatioMax, int view);

void my_rc_params( unsigned int view_idx );
void my_rc_init_seq( unsigned int view_idx );
void my_rc_init_GOP( unsigned int view_idx, int np);
void my_rc_init_pict( unsigned int view_idx, int mult);
void my_rc_update_pict(unsigned int view_idx, int nbits);
void my_updatePparams( unsigned int view_idx );
void my_rc_update_pict_frame( unsigned int view_idx );
void my_updateRCModel( unsigned int view_idx );
void my_RCModelEstimator(unsigned int view_idx, int n_realSize, int n_windowSize, char *rc_rgRejected);
void my_updateMADModel( unsigned int view_idx );
void my_MADModelEstimator(unsigned int view_idx, int n_realSize, int n_windowSize, char *mad_rgRejected);
void my_updateQPNonPicAFF( unsigned int view_idx );
int  my_updateFirstP( unsigned int view_idx );
int  my_updateNegativeTarget(unsigned int view_idx, int m_Qp);
int  my_updateFirstBU( unsigned int view_idx );
void my_updateLastBU( unsigned int view_idx );
void my_predictCurrPicMAD( unsigned int view_idx );
void my_updateModelQPBU( unsigned int view_idx, int m_Qp);
void my_updateModelQPFrame( unsigned int view_idx, int m_Bits);
int  my_rc_handle_mb( unsigned int view_idx );
void my_rc_update_bu_stats( unsigned int view_idx );
void my_rc_update_frame_stats( unsigned int view_idx );
void my_rc_init_gop_params( unsigned int view_idx );
void my_rc_init_frame( unsigned int view_idx );
void my_rc_store_header_bits( unsigned int view_idx );
//int  my_updateQPRC1( );
int  my_updateQPRC3( unsigned int view_idx );
//int  my_updateQPRC0( );
void my_initial_all( unsigned int view_idx );
void my_feedback(unsigned int view_idx, unsigned int feedback);
void my_hold( unsigned int view_idx );
int  (*my_updateQP)( unsigned int view_idx );

void update_aof_cycle();
void update_aof();


#ifdef ARMCM7_RC //###########
int VEBRC_IRQ_Handler(unsigned int view0_feedback, unsigned int view1_feedback);
#endif


#endif
