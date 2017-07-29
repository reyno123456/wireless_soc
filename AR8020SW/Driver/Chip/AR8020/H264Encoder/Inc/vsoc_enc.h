#ifndef _VSOC_ENC_
#define _VSOC_ENC_

#include "sys_event.h"


typedef struct ENC_REG_STRUCT {
   
    struct { 
        struct { 
            unsigned int L1_mul_factor  : 3;
            unsigned int L2_mul_factor  : 3;
            unsigned int Reserved0      : 2;
        } byte3;

        struct { 
            unsigned int L0             : 1;
            unsigned int L1             : 1;
            unsigned int L2             : 1;
            unsigned int Reserved0      : 4;
            unsigned int L12_preload_mode : 1;
        } byte2;

        struct {
            unsigned int polarity       : 3;
            unsigned int ps_mode        : 2;
            unsigned int yuv_8bit       : 1;
            unsigned int mipi_en        : 1;
            unsigned int view_pd_en     : 1;
        } byte1;
        struct {
            unsigned int view0_en       : 1;
            unsigned int image_wr_en    : 1;
            unsigned int reserved0      : 6;
        } byte0;
    } dw0;

    
    struct { 
        struct { 
            unsigned int height_low     : 8;
        } byte3;
        struct { 
            unsigned int height_high    : 8;
        } byte2;
        struct {
            unsigned int width_low      : 8;
        } byte1;
        struct {
            unsigned int width_high     : 8;
        } byte0;
    } dw1;

    
    struct { 
        struct { 
            unsigned int level          : 8;
        } byte3;
        struct { 
            unsigned int profile        : 8;
        } byte2;
        struct {
            unsigned int FPS            : 8;
        } byte1;
        struct {
            unsigned int GOP            : 8;
        } byte0;
    } dw2;

     
    struct { 
        struct { 
            unsigned int hbp_low        : 8;
        } byte3;
        struct { 
            unsigned int hbp_high       : 4;
            unsigned int reserved0      : 4;
        } byte2;
        struct {
            unsigned int hfp_low        : 8;
        } byte1;
        struct {
            unsigned int hfp_high       : 4;
            unsigned int reserved0      : 4;
        } byte0;
    } dw3;

    
    struct { 
        struct { 
            unsigned int vbp_low        : 8;
        } byte3;
        struct { 
            unsigned int vbp_hight      : 4;
            unsigned int field_mode     : 1;
            unsigned int luma_chroma_sw : 1;
            unsigned int de_mode        : 2;
        } byte2;
        struct {
            unsigned int vfp_low        : 8;
        } byte1;
        struct {
            unsigned int vfp_high       : 4;
            unsigned int dvp_wd_cnt     : 4;    // dvp watch dog cnt
        } byte0;
    } dw4;


    struct { 
        struct { 
            unsigned int bu_low         : 8;
        } byte3;
        struct { 
            unsigned int bu_high        : 8;
        } byte2;
        struct {
            unsigned int rc_mode        : 2;
            unsigned int reserved       : 6;
        } byte1;
        struct {
            unsigned int rc_en          : 1;
            unsigned int reserved       : 4;    // dvp watch dog cnt
        } byte0;
    } dw5;


} ENC_REG;

//=== VSOC_ENC REG Base_Addr Definition ===//
#define     VSOC_ENC_REG_BASE   0xA0010000  // master
#define     ENC_REG_ADDR        0xA0010000  // master

#define     VSOC_SOFT_RESET     0xA0030034

//=== VSOC_ENC Register Mapping ===//
//#define VIEW0_IMG_WR_EN         0
//#define VIEW0_EN                0
//#define VIEW0_PRELD_EN          0
//#define VIEW0_MIPI_EN           0
//#define VIEW0_YUV_8BIT          0 
//#define VIEW0_PS_MODE           0 
//#define VIEW0_POLARITY          0 
//#define VIEW0_L12_PLD_MODE      0
//#define VIEW0_L012_LD_EN        7
//#define VIEW0_L2_MUL_FAC        6
//#define VIEW0_L1_MUL_FAC        6
//#define VIEW0_PIC_WIDTH         720
//#define VIEW0_PIC_HEIGHT        480
//#define VIEW0_GOP               30
//#define VIEW0_FPS               30
//#define VIEW0_PROFILE           77
//#define VIEW0_LEVEL             40 
//#define VIEW0_HF_PROCH          0 
//#define VIEW0_HB_PORCH          0   

//VIEW0_REGS
#define VIEW0_WORD_REG0     0x01100736
#define VIEW0_WORD_REG1     0x02D001E0
#define VIEW0_WORD_REG2     0x1E3C4D28
#define VIEW0_WORD_REG3     0x003C000F
#define VIEW0_WORD_REG4     0xF01E0008
#define VIEW0_WORD_REG5     0x0003002D
#define VIEW0_WORD_REG6     0x1C300506
#define VIEW0_WORD_REG7     0x003D0900
                            
#define VIEW0_WORD_REG8     0x01040000
#define VIEW0_WORD_REG9     0x00003904
#define VIEW0_WORD_REG10    0x130258BC
#define VIEW0_WORD_REG11    0x12469422
#define VIEW0_WORD_REG12    0x00FFFF00
#define VIEW0_WORD_REG13    0x00002E00
#define VIEW0_WORD_REG14    0x18004600
#define VIEW0_WORD_REG15    0x20004E00
                            
#define VIEW0_WORD_REG16    0x22005000
#define VIEW0_WORD_REG17    0x28005600
#define VIEW0_WORD_REG18    0x0B0D0401
#define VIEW0_WORD_REG19    0x000050BF
#define VIEW0_WORD_REG20    0x00000018
#define VIEW0_WORD_REG21    0x00000004
#define VIEW0_WORD_REG22    0x000002B0
#define VIEW0_WORD_REG23    0x0023F0B8
#define VIEW0_WORD_REG24    0x00000000

//VIEW1_REGS
#define VIEW1_WORD_REG0     0x00100736
#define VIEW1_WORD_REG1     0x02D001E0
#define VIEW1_WORD_REG2     0x0F1E4D28
#define VIEW1_WORD_REG3     0x00000000
#define VIEW1_WORD_REG4     0xF0008000
#define VIEW1_WORD_REG5     0x0003002D
#define VIEW1_WORD_REG6     0x28300606
#define VIEW1_WORD_REG7     0x00500000
                            
#define VIEW1_WORD_REG8     0x01040000
#define VIEW1_WORD_REG9     0x00000F05
#define VIEW1_WORD_REG10    0x000258FD
#define VIEW1_WORD_REG11    0x12469422
#define VIEW1_WORD_REG12    0x00FFFF00
#define VIEW1_WORD_REG13    0x5C008A00
#define VIEW1_WORD_REG14    0x7400A200
#define VIEW1_WORD_REG15    0x7C00AA00
                            
#define VIEW1_WORD_REG16    0x7E00AC00
#define VIEW1_WORD_REG17    0x8400B200
#define VIEW1_WORD_REG18    0x28280401
#define VIEW1_WORD_REG19    0x0001AC0F
#define VIEW1_WORD_REG20    0x00000141
#define VIEW1_WORD_REG21    0x00000479
#define VIEW1_WORD_REG22    0x00002270
#define VIEW1_WORD_REG23    0x0AA2A464
#define VIEW1_WORD_REG24    0x00000000

//PUBLIC_REGS
#define PUBLIC_WORD_REG0    0x18212931
#define PUBLIC_WORD_REG1    0x39414A00
#define PUBLIC_WORD_REG2    0x00000000
#define PUBLIC_WORD_REG3    0x20B10000
#define PUBLIC_WORD_REG4    0x00000000
                            
//DEBUG_REGS                
#define DEBUG_WORD_REG5     0x01000000
#define DEBUG_WORD_REG6     0x28E48BC6
#define DEBUG_WORD_REG7     0x01E002D0
#define DEBUG_WORD_REG8     0x00030001
#define DEBUG_WORD_REG9     0x18003428
#define DEBUG_WORD_REG10    0x0A28A38E
#define DEBUG_WORD_REG11    0x01030020
#define DEBUG_WORD_REG12    0x00002A59
#define DEBUG_WORD_REG13    0xE0000101
                                        
//PAGE_2 REGS
#define PAGE2_WORD_REG0     0x01000000
#define PAGE2_WORD_REG1     0x00000100


//PAGE_3 REGS 
#define PAGE3_WORD_REG0     0x00000000


void init_view0(unsigned int width, unsigned int height, unsigned int gop, unsigned int fps, unsigned int br, ENUM_ENCODER_INPUT_SRC src);
void init_view1(unsigned int width, unsigned int height, unsigned int gop, unsigned int fps, unsigned int br, ENUM_ENCODER_INPUT_SRC src);
void open_view0( unsigned int rc_en );
void close_view0(void);
void open_view1( unsigned int rc_en );
void close_view1(void);


#endif

