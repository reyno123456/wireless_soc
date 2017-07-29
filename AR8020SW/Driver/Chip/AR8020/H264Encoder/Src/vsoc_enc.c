/*====================================================*/
/*              Include         Files                 */
/*====================================================*/
#include "enc_internal.h"
#include "vsoc_enc.h"
#include "debuglog.h"

/*====================================================*/
/*              Encoder   Functions                   */
/*====================================================*/
void vsoc_enc_init(){

    //============   PAGE_0 REGS Setting ===========//
    //change_2_page0
    WRITE_WORD(VSOC_ENC_REG_BASE + (52<<2), 0x00000000) ;

    //ENC View0 Disable 
    WRITE_WORD(VSOC_ENC_REG_BASE + (0<<2), VIEW0_WORD_REG0) ;
    //
    WRITE_WORD(VSOC_ENC_REG_BASE + (0<<2), VIEW0_WORD_REG0) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (1<<2), VIEW0_WORD_REG1) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (2<<2), VIEW0_WORD_REG2) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (3<<2), VIEW0_WORD_REG3) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (4<<2), VIEW0_WORD_REG4) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (5<<2), VIEW0_WORD_REG5) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (6<<2), VIEW0_WORD_REG6) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (7<<2), VIEW0_WORD_REG7) ;

    WRITE_WORD(VSOC_ENC_REG_BASE + (8 <<2), VIEW0_WORD_REG8 ) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (9 <<2), VIEW0_WORD_REG9 ) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (10<<2), VIEW0_WORD_REG10) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (11<<2), VIEW0_WORD_REG11) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (12<<2), VIEW0_WORD_REG12) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (13<<2), VIEW0_WORD_REG13) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (14<<2), VIEW0_WORD_REG14) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (15<<2), VIEW0_WORD_REG15) ;

    WRITE_WORD(VSOC_ENC_REG_BASE + (16<<2), VIEW0_WORD_REG16) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (17<<2), VIEW0_WORD_REG17) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (18<<2), VIEW0_WORD_REG18) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (19<<2), VIEW0_WORD_REG19) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (20<<2), VIEW0_WORD_REG20) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (21<<2), VIEW0_WORD_REG21) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (22<<2), VIEW0_WORD_REG22) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (23<<2), VIEW0_WORD_REG23) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (24<<2), VIEW0_WORD_REG24) ;

    //ENC View1 Disable
    WRITE_WORD(VSOC_ENC_REG_BASE + (25<<2), VIEW1_WORD_REG0) ;
    //
    WRITE_WORD(VSOC_ENC_REG_BASE + (25<<2), VIEW1_WORD_REG0) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (26<<2), VIEW1_WORD_REG1) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (27<<2), VIEW1_WORD_REG2) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (28<<2), VIEW1_WORD_REG3) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (29<<2), VIEW1_WORD_REG4) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (30<<2), VIEW1_WORD_REG5) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (31<<2), VIEW1_WORD_REG6) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (32<<2), VIEW1_WORD_REG7) ;

    WRITE_WORD(VSOC_ENC_REG_BASE + (33<<2), VIEW1_WORD_REG8 ) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (34<<2), VIEW1_WORD_REG9 ) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (35<<2), VIEW1_WORD_REG10) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (36<<2), VIEW1_WORD_REG11) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (37<<2), VIEW1_WORD_REG12) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (38<<2), VIEW1_WORD_REG13) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (39<<2), VIEW1_WORD_REG14) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (40<<2), VIEW1_WORD_REG15) ;

    WRITE_WORD(VSOC_ENC_REG_BASE + (41<<2), VIEW1_WORD_REG16) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (42<<2), VIEW1_WORD_REG17) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (43<<2), VIEW1_WORD_REG18) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (44<<2), VIEW1_WORD_REG19) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (45<<2), VIEW1_WORD_REG20) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (46<<2), VIEW1_WORD_REG21) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (47<<2), VIEW1_WORD_REG22) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (48<<2), VIEW1_WORD_REG23) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (49<<2), VIEW1_WORD_REG24) ;
    
    //PUBLIC
    WRITE_WORD(VSOC_ENC_REG_BASE + (50<<2), PUBLIC_WORD_REG0) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (51<<2), PUBLIC_WORD_REG1) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (52<<2), PUBLIC_WORD_REG2) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (53<<2), PUBLIC_WORD_REG3) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (54<<2), PUBLIC_WORD_REG4) ;

    //DEBUG_REGS
    WRITE_WORD(VSOC_ENC_REG_BASE + (55<<2), DEBUG_WORD_REG5 ) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (56<<2), DEBUG_WORD_REG6 ) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (57<<2), DEBUG_WORD_REG7 ) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (58<<2), DEBUG_WORD_REG8 ) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (59<<2), DEBUG_WORD_REG9 ) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (60<<2), DEBUG_WORD_REG10) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (61<<2), DEBUG_WORD_REG11) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (62<<2), DEBUG_WORD_REG12) ;
    WRITE_WORD(VSOC_ENC_REG_BASE + (63<<2), DEBUG_WORD_REG13) ;

    //============   PAGE_2 REGS Setting ===========//
    //change_2_page2
    //WRITE_WORD(VSOC_ENC_REG_BASE + 52<<2, 0x00020000) ;
}


void vsoc_enc_enable(){
    unsigned int rd_enc_reg = 0 ;
    //ENC View0 Enable 
    encoder_delay(2000);

    //
    READ_WORD(VSOC_ENC_REG_BASE + (0<<2), rd_enc_reg) ;

    rd_enc_reg = rd_enc_reg | 0x01000000 ;

    encoder_delay(2000) ;
   
    WRITE_WORD(VSOC_ENC_REG_BASE + (0<<2), rd_enc_reg) ;
}

const int bridx2br[19] = { // update brindex_2_br look-up table, lhu, 2017/06/06
    8000000 ,
    600000  ,
    1200000 ,
    2400000 ,
    3000000 ,
    3500000 ,
    4000000 ,
    4800000 ,
    5000000 ,
    6000000 ,
    7000000 ,
    7500000 ,
    9000000 ,
    10000000,
    11000000,
    12000000,
    13000000,
    14000000,
    15000000
};
void init_view0(unsigned int width, unsigned int height, unsigned int gop, unsigned int fps, unsigned int br, ENUM_ENCODER_INPUT_SRC src) {
    if ((src >= ENCODER_INPUT_SRC_HDMI_0) && (src <= ENCODER_INPUT_SRC_DVP_1))
    {
        WRITE_WORD((ENC_REG_ADDR+(0x00<<2)),0x00900736);
    }
    else if (ENCODER_INPUT_SRC_MIPI == src)
    {
        WRITE_WORD((ENC_REG_ADDR+(0x00<<2)),0x00D40736);
    }
    else
    {
        ;
    }
    
    WRITE_WORD((ENC_REG_ADDR+(0x01<<2)),((width<<16)+(height&0xffff)));
    WRITE_WORD((ENC_REG_ADDR+(0x02<<2)),(0x00004D28|((gop&0xff)<<24)|((fps&0xff)<<16)));

    if ((width == 720) && (height == 480))
    {
        WRITE_WORD((ENC_REG_ADDR+(0x03<<2)),0x003C000F);
        if ((src >= ENCODER_INPUT_SRC_HDMI_0) && (src <= ENCODER_INPUT_SRC_DVP_1))
        {
            WRITE_WORD((ENC_REG_ADDR+(0x04<<2)),0xF01E8008);
        }
        else if (ENCODER_INPUT_SRC_MIPI == src)
        {
            WRITE_WORD((ENC_REG_ADDR+(0x04<<2)),0xF01E4008);
        }
        else
        {
            ;
        }
    }
    else
    {
        WRITE_WORD((ENC_REG_ADDR+(0x03<<2)),0x00000000);
        if ((src >= ENCODER_INPUT_SRC_HDMI_0) && (src <= ENCODER_INPUT_SRC_DVP_1))
        {
            WRITE_WORD((ENC_REG_ADDR+(0x04<<2)),0xF0008000);
        }
        else if (ENCODER_INPUT_SRC_MIPI == src)
        {
            WRITE_WORD((ENC_REG_ADDR+(0x04<<2)),0xF0004000);
        }
        else
        {
            ;
        }
    }

    WRITE_WORD((ENC_REG_ADDR+(0x05<<2)),(0x00030000|(((width+15)/16)&0xffff))); //bu

    if( br == 8 || br == 1 || br == 2)  // <= 2Mbps 
        WRITE_WORD((ENC_REG_ADDR+(0x06<<2)),0x20330806);
    else
        WRITE_WORD((ENC_REG_ADDR+(0x06<<2)),0x20300806);

    WRITE_WORD((ENC_REG_ADDR+(0x07<<2)),bridx2br[br]);
    WRITE_WORD((ENC_REG_ADDR+(0x08<<2)),0x04040f00); // default ipratiomax=15, lhu
    WRITE_WORD((ENC_REG_ADDR+(0x09<<2)),0x00005900);
    if ((src >= ENCODER_INPUT_SRC_HDMI_0) && (src <= ENCODER_INPUT_SRC_DVP_1))
    {
        WRITE_WORD((ENC_REG_ADDR+(0x0a<<2)),(0x030258FE | ((br & 0x1F)<<26) ));
    }
    else if (ENCODER_INPUT_SRC_MIPI == src)
    {
        WRITE_WORD((ENC_REG_ADDR+(0x0a<<2)),(0x0302589E | ((br & 0x1F)<<26) ));
    }
    else
    {
        ;
    }
    WRITE_WORD((ENC_REG_ADDR+(0x0b<<2)),0x12469422);
    WRITE_WORD((ENC_REG_ADDR+(0x0c<<2)),0x00FFFF02); // bs_info enable, aciprato enable, lhu

    if( height <= 720 ) {
        dlog_info("v0 <= 720p\n");
        WRITE_WORD((ENC_REG_ADDR+(0x0d<<2)),0x00002E00);
        WRITE_WORD((ENC_REG_ADDR+(0x0e<<2)),0x18004600);
        WRITE_WORD((ENC_REG_ADDR+(0x0f<<2)),0x20004E00);
        WRITE_WORD((ENC_REG_ADDR+(0x10<<2)),0x22005000);
        WRITE_WORD((ENC_REG_ADDR+(0x11<<2)),0x28005600);
    }
    else {
        dlog_info("v0 == 1080p\n");
        WRITE_WORD((ENC_REG_ADDR+(0x0d<<2)),0x00004700);
        WRITE_WORD((ENC_REG_ADDR+(0x0e<<2)),0x22006900);
        WRITE_WORD((ENC_REG_ADDR+(0x0f<<2)),0x32007900);
        WRITE_WORD((ENC_REG_ADDR+(0x10<<2)),0x36007d00);
        WRITE_WORD((ENC_REG_ADDR+(0x11<<2)),0x3e808580);
    }

    WRITE_WORD((ENC_REG_ADDR+(0x12<<2)),0x1D1E0401);
#ifdef ARCAST
    WRITE_WORD((ENC_REG_ADDR+(0x18<<2)),0x0005AA74); // HBitsRatioAbits_level=5, AbitsRatioTargetBits_level=170, psnr_drop_level=3db, insertOneIFrame mode enable, wireless_screen mode enable, lhu
#else
    WRITE_WORD((ENC_REG_ADDR+(0x18<<2)),0x0005AA60); // HBitsRatioAbits_level=5, AbitsRatioTargetBits_level=170, psnr_drop_level=3db, insertOneIFrame mode disable, wireless_screen mode disable, lhu
#endif

    WRITE_WORD((ENC_REG_ADDR+(0x32<<2)),0x18212931);
    
#ifdef RUN_IN_EMULATOR    // Test in emulator      
    WRITE_WORD((ENC_REG_ADDR+(0x33<<2)),0x39414A08);
    WRITE_WORD((ENC_REG_ADDR+(0x34<<2)),0x00000000);
    WRITE_WORD((ENC_REG_ADDR+(0x35<<2)),0x00000000);
    WRITE_WORD((ENC_REG_ADDR+(0x36<<2)),0x00000000);
    WRITE_WORD((ENC_REG_ADDR+(0x37<<2)),0x00000000);
#else
    WRITE_WORD((ENC_REG_ADDR+(0x33<<2)),0x39414A80);
#endif
}

void init_view1(unsigned int width, unsigned int height, unsigned int gop, unsigned int fps, unsigned int br, ENUM_ENCODER_INPUT_SRC src) {
    if ((src >= ENCODER_INPUT_SRC_HDMI_0) && (src <= ENCODER_INPUT_SRC_DVP_1))
    {
        WRITE_WORD((ENC_REG_ADDR+(0x19<<2)),0x00900736);
    }
    else if (ENCODER_INPUT_SRC_MIPI == src)
    {
        WRITE_WORD((ENC_REG_ADDR+(0x19<<2)),0x00D40736);
    }
    else
    {
        ;
    }

    WRITE_WORD((ENC_REG_ADDR+(0x1a<<2)),((width<<16)+(height&0xffff)));
    WRITE_WORD((ENC_REG_ADDR+(0x1b<<2)),(0x00004D28|((gop&0xff)<<24)|((fps&0xff)<<16)));

    if ((width == 720) && (height == 480))
    {
        WRITE_WORD((ENC_REG_ADDR+(0x1c<<2)),0x003C000F);
        if ((src >= ENCODER_INPUT_SRC_HDMI_0) && (src <= ENCODER_INPUT_SRC_DVP_1))
        {
            WRITE_WORD((ENC_REG_ADDR+(0x1d<<2)),0xF01E8008);
        }
        else if (ENCODER_INPUT_SRC_MIPI == src)
        {
            WRITE_WORD((ENC_REG_ADDR+(0x1d<<2)),0xF01E4008);
        }
        else
        {
            ;
        }
    }
    else
    {
        WRITE_WORD((ENC_REG_ADDR+(0x1c<<2)),0x00000000);
        if ((src >= ENCODER_INPUT_SRC_HDMI_0) && (src <= ENCODER_INPUT_SRC_DVP_1))
        {
            WRITE_WORD((ENC_REG_ADDR+(0x1d<<2)),0xF0008000);
        }
        else if (ENCODER_INPUT_SRC_MIPI == src)
        {
            WRITE_WORD((ENC_REG_ADDR+(0x1d<<2)),0xF0004000);
        }
        else
        {
            ;
        }
    }

    WRITE_WORD((ENC_REG_ADDR+(0x1e<<2)),(0x00030000|(((width+15)/16)&0xffff))); //bu

    if( br == 8 || br == 1 || br == 2)  // <= 2Mbps 
        WRITE_WORD((ENC_REG_ADDR+(0x1f<<2)),0x20330806);
    else
        WRITE_WORD((ENC_REG_ADDR+(0x1f<<2)),0x20300806);

    WRITE_WORD((ENC_REG_ADDR+(0x20<<2)),bridx2br[br]);
    WRITE_WORD((ENC_REG_ADDR+(0x21<<2)),0x04040f00); // default ipratiomax=15, lhu
    WRITE_WORD((ENC_REG_ADDR+(0x22<<2)),0x00003000);
    if ((src >= ENCODER_INPUT_SRC_HDMI_0) && (src <= ENCODER_INPUT_SRC_DVP_1))
    {
        WRITE_WORD((ENC_REG_ADDR+(0x23<<2)),(0x030258FE | ((br & 0x1F)<<26) ));
    }
    else if (ENCODER_INPUT_SRC_MIPI == src)
    {
        WRITE_WORD((ENC_REG_ADDR+(0x23<<2)),(0x0302589E | ((br & 0x1F)<<26) ));
    }
    else
    {
        ;
    }
    WRITE_WORD((ENC_REG_ADDR+(0x24<<2)),0x12469422);
    WRITE_WORD((ENC_REG_ADDR+(0x25<<2)),0x01FFFF02); // bs_info enable, acipratio enable, lhu

    if( height <= 720 ) {
        dlog_info("v1 <= 720p\n");

        WRITE_WORD((ENC_REG_ADDR+(0x26<<2)),0x5C008A00);
        WRITE_WORD((ENC_REG_ADDR+(0x27<<2)),0x7400A200);
        WRITE_WORD((ENC_REG_ADDR+(0x28<<2)),0x7C00AA00);
        WRITE_WORD((ENC_REG_ADDR+(0x29<<2)),0x7E00AC00);
        WRITE_WORD((ENC_REG_ADDR+(0x2a<<2)),0x8400B200);
    }
    else {
        dlog_info("v1 == 1080p\n");

        WRITE_WORD((ENC_REG_ADDR+(0x26<<2)),0x00004700);
        WRITE_WORD((ENC_REG_ADDR+(0x27<<2)),0x22006900);
        WRITE_WORD((ENC_REG_ADDR+(0x28<<2)),0x32007900);
        WRITE_WORD((ENC_REG_ADDR+(0x29<<2)),0x36007d00);
        WRITE_WORD((ENC_REG_ADDR+(0x2a<<2)),0x3e808580);
    }
    WRITE_WORD((ENC_REG_ADDR+(0x2b<<2)),0x11120401);
#ifdef ARCAST
    WRITE_WORD((ENC_REG_ADDR+(0x31<<2)),0x0005AA74); // HBitsRatioAbits_level=5, AbitsRatioTargetBits_level=170, psnr_drop_level=3db, insertOneIFrame mode enable, wireless_screen mode enable, lhu
#else
    WRITE_WORD((ENC_REG_ADDR+(0x31<<2)),0x0005AA60); // HBitsRatioAbits_level=5, AbitsRatioTargetBits_level=170, psnr_drop_level=3db, insertOneIFrame mode disable, wireless_screen mode disable, lhu
#endif

    WRITE_WORD((ENC_REG_ADDR+(0x32<<2)),0x18212931);
#ifdef RUN_IN_EMULATOR    // Test in emulator  
    WRITE_WORD((ENC_REG_ADDR+(0x33<<2)),0x39414A08);
    WRITE_WORD((ENC_REG_ADDR+(0x34<<2)),0x00000000);
    WRITE_WORD((ENC_REG_ADDR+(0x35<<2)),0x00000000);
    WRITE_WORD((ENC_REG_ADDR+(0x36<<2)),0x00000000);
    WRITE_WORD((ENC_REG_ADDR+(0x37<<2)),0x00000000);
#else
    WRITE_WORD((ENC_REG_ADDR+(0x33<<2)),0x39414A80);
#endif
    // Patch: view0 settings must be same as view1 when 1080P input. 
    if (height > 720)
    {
        init_view0(width, height, gop, fps, br, src);
    }
}
void open_view0( unsigned int rc_en ){ // hold until SDRAM initial done
    unsigned int i,t0,t1;
    while(1){
        for(i=0;i<2000;i++);
        READ_WORD(0xa0030024,t1);
        if((t1&0x1)==1){
            READ_WORD(0xa0010000,t0);
            t0=t0|0x01000000;
            WRITE_WORD(0xa0010000,t0);
            break;
        }
    }

    if( rc_en ) {
        READ_WORD((ENC_REG_ADDR+(0x05<<2)),t0);
        t0=t0|0x01000000;
        WRITE_WORD((ENC_REG_ADDR+(0x05<<2)),t0);
    }

}

void close_view0(void)
{
    unsigned int t0;

    READ_WORD(0xa0010000,t0);
    t0 &= ~0x01000000;
    WRITE_WORD(0xa0010000,t0);

    READ_WORD((ENC_REG_ADDR+(0x05<<2)),t0);
    t0 &= ~0x01000000;
    WRITE_WORD((ENC_REG_ADDR+(0x05<<2)),t0);
}

void open_view1( unsigned int rc_en ){ // hold until SDRAM initial done
    unsigned int i,t0,t1;
    while(1){
        for(i=0;i<2000;i++);
        READ_WORD(0xa0030024,t1);
        if((t1&0x1)==1){
            READ_WORD(0xa0010064,t0);
            t0=t0|0x01000000;
            WRITE_WORD(0xa0010064,t0);
            break;
        }
    }

    if( rc_en ) {
        READ_WORD((ENC_REG_ADDR+(0x1e<<2)),t0);
        t0=t0|0x01000000;
        WRITE_WORD((ENC_REG_ADDR+(0x1e<<2)),t0);
    }
}

void close_view1(void)
{
    unsigned int t0;

    READ_WORD(0xa0010064,t0);
    t0 &= ~0x01000000;
    WRITE_WORD(0xa0010064,t0);

    READ_WORD((ENC_REG_ADDR+(0x1e<<2)),t0);
    t0 &= ~0x01000000;
    WRITE_WORD((ENC_REG_ADDR+(0x1e<<2)),t0);
}
