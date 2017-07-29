////////////////////////////////////////////////////////////////////////////
// brief: Rate Control algorithm for two video view
// author: Li Hu
// date: 2015-11-8
////////////////////////////////////////////////////////////////////////////
#include "data_type.h"
#include "brc.h"
#include "log10.h"
#include "debuglog.h"

extern const int bridx2br[19];

#ifdef ARMCM7_RC  //###########
    #include "enc_internal.h"
#else  //###########
    #include <math.h>
    #include <limits.h>
    #include "global.h"
    #include "ratectl.h"
#endif    //###########

#ifdef ARMCM7_RC //###########


enum REG_ADDR {
    ENABLE_ADDR = 0,
    FRAME_XY_ADDR,
    GOPFPS_ADDR,   
    RCEN_BU_ADDR,  
    RCSET1_ADDR, 
    BR_ADDR,  
    RCSET2_ADDR,
    FEEDBACK_ADDR,
    RC_ACBR_ADDR,
    ZW_BSINFO_ADDR,
    QP_ADDR   ,
    MAD_ADDR  ,
    HBITS_ADDR,
    TBITS_ADDR,
    ABITS_ADDR,
    YMSEL_ADDR,
    YMSEH_ADDR,
    REG_TOTAL,
};

enum {
	VIEW0 = 0,
	VIEW1 = 1,
	VIEW_NUM = 2,
};

unsigned char reg[2][REG_TOTAL] = {
    {(0<<2),        
     (1<<2),        
     (2<<2),        
     (5<<2),        
     (6<<2),        
     (7<<2),        
     (8<<2),        
     (9<<2),        
     (10<<2),       
     (12<<2),       
     (18<<2),       
     (19<<2),       
     (20<<2),       
     (21<<2),       
     (22<<2),       
     (23<<2),       
     (24<<2)},
     
     {0x64 + (0<<2),  
      0x64 + (1<<2),  
      0x64 + (2<<2),  
      0x64 + (5<<2),  
      0x64 + (6<<2),  
      0x64 + (7<<2),  
      0x64 + (8<<2),  
      0x64 + (9<<2),  
      0x64 + (10<<2), 
      0x64 + (12<<2), 
      0x64 + (18<<2), 
      0x64 + (19<<2), 
      0x64 + (20<<2), 
      0x64 + (21<<2), 
      0x64 + (22<<2), 
      0x64 + (23<<2), 
      0x64 + (24<<2)}     
};
#endif    //###########

RC_DATA rca[VIEW_NUM];

#define CHECK_VIEW_IDX( view_idx ) \
    if( view_idx >= VIEW_NUM ) \
    { \
        dlog_info( "view_idx %d is wrong.\n", view_idx ); \
        return 0; \
    }
     

static const int OMEGA_4p = 0xe;
static const int MINVALUE = 4;




static const int QP2QSTEP_8p[6]={0x0a0,0x0b0,0x0d0,0x0e0,0x100,0x120};
int QP2Qstep_8p(int QP) {
    int i,Qstep_8p;

    Qstep_8p=QP2QSTEP_8p[QP%6];
    for(i=0;i<(QP/6);i++) Qstep_8p*=2;
    return Qstep_8p;
}

int Qstep2QP_8p(int Qstep_8p) {
    int tmp,q_per=0,q_rem=0;

    if     (Qstep_8p<QP2Qstep_8p(0 )) return 0;
    else if(Qstep_8p>QP2Qstep_8p(51)) return 51;

    tmp=QP2Qstep_8p(5);
    while(Qstep_8p>tmp){tmp=tmp<<1; q_per+=1;}

    if     (Qstep_8p<=(0x0a8<<q_per)) q_rem=0;
    else if(Qstep_8p<=(0x0c0<<q_per)) q_rem=1;
    else if(Qstep_8p<=(0x0d8<<q_per)) q_rem=2;
    else if(Qstep_8p<=(0x0f0<<q_per)) q_rem=3;
    else if(Qstep_8p<=(0x110<<q_per)) q_rem=4;
    else                              q_rem=5;

    return ((q_per*6)+q_rem);
}


int my_sqrt32(int x)
{
    int temp=0,v_bit=15,n=0,b=0x8000;

    if(x<=1)
        return x;
    else if(x<0x10000)
    {
        v_bit=7;
        b=0x80;
    }

    do{
        temp=((n<<1)+b)<<(v_bit--);
        if(x>=temp)
        {
            n+=b;
            x-=temp;
        }
    }while(b>>=1);

    return n;
}
int my_sqrt64(int64 x)
{
    int v_bit=31,nn = x>>32;
    int64 temp=0,n=0,b=0x80000000;

    if(x<=1)
        return ((int)x);

    if(nn==0)
        return(my_sqrt32((int)x));

    do{
        temp=((n<<1)+b)<<(v_bit--);
        if(x>=temp)
        {
            n+=b;
            x-=temp;
        }
    }while(b>>=1);

    return ((int)n);
}

int my_imin(int a, int b) { return((a<b)? a:b); }
int my_iequmin(int a, int b) { return((a<=b)? a:b); }
int my_imax(int a, int b) { return((a>b)? a:b); }
int my_iClip3(int low, int high, int x) {
  x = (x>low)? x:low;
  x = (x<high)? x:high;
  return x;
}




//==============================================================================
//==============================================================================
#ifdef ARMCM7_RC //###########
extern int v0_poweron_rc_params_set,v1_poweron_rc_params_set;
int VEBRC_IRQ_Handler(unsigned int view0_feedback, unsigned int view1_feedback)
{
    int i,run_case=0;
	RC_DATA *prca;
	unsigned int view_idx;
	int qp;

    view_idx = VIEW0;
    my_feedback( view_idx, view0_feedback); //// view0 irq
    if((rca[view_idx].fd_irq_en==1) && (rca[view_idx].rc_enable==1) && (rca[view_idx].enable==1)) {
        run_case=0;
        // only for gop change at last_p_frame. At this circumstance, Hardware use the updated gop for frame_cnt increment immediately,
        // but software use the un-updated one for its frame_cnt counting. Thus would iccur mismatch for both side.
        if (rca[view_idx].fd_last_p==1 && rca[view_idx].fd_iframe!=1) { // fix the false decision when inserting OneIFrame
            READ_WORD_ENC(reg[view_idx][GOPFPS_ADDR], i); //read view0 gop
            if (rca[view_idx].fd_row_cnt==0)                   rca[view_idx].last_p_prev_gop = (i>>24)&0xff;
            if (((i>>24)&0xff)!=rca[view_idx].last_p_prev_gop)  {
				rca[view_idx].last_p_gop_change = TRUE; // check view0's GOP change or not at last_p_frame
            }
			else {
				rca[view_idx].last_p_gop_change = FALSE;
			}
            rca[view_idx].last_p_prev_gop = (i>>24)&0xff;
        }
    }
    else {
		view_idx = VIEW1;
        my_feedback(view_idx, view1_feedback ); //// view1 irq
        if((rca[view_idx].fd_irq_en==1) && (rca[view_idx].rc_enable==1) && (rca[view_idx].enable==1)) {
            READ_WORD_ENC(reg[view_idx][FRAME_XY_ADDR],i); //check x/y first
            if( ((i&0xffff)>720) || (((i>>16)&0xffff)>1280) )
                //// only for 1080p input from view1 (encoder used view0)
                //1080P input view1 but encoder used view0,
                //for rc read parameter from view1 / use V1 compute / read enc-data from view0 / write QP to view0
                run_case=2;
            else
                run_case=1;
            // only for gop change at last_p_frame. At this circumstance, Hardware use the updated gop for frame_cnt increment immediately,
            // but software use the un-updated one for its frame_cnt counting. Thus would iccur mismatch for both side.
            if (rca[view_idx].fd_last_p==1 && rca[view_idx].fd_iframe!=1) { // fix the false decision when inserting OneIFrame
                READ_WORD_ENC(reg[view_idx][GOPFPS_ADDR],i); //read view1 gop
                if (rca[view_idx].fd_row_cnt==0) rca[view_idx].last_p_prev_gop = (i>>24)&0xff;
                if (((i>>24)&0xff)!=rca[view_idx].last_p_prev_gop) {
					rca[view_idx].last_p_gop_change = TRUE; // check view1's GOP change or not at last_p_frame
                }
				else {
					rca[view_idx].last_p_gop_change = FALSE;
				}
                rca[view_idx].last_p_prev_gop = (i>>24)&0xff;
            }
        }
        else
            run_case=3;
    }

    switch(run_case) {
        case 0:
			view_idx = VIEW0;
            prca = &rca[view_idx];
            if((prca->fd_reset==1) && (prca->fd_last_p==1) && (prca->fd_last_row==1) && (prca->last_p_gop_change==FALSE) && (prca->poweron_rc_params_set==0) && (prca->gop_change_NotResetRC==0)) { // lhu, 2017/04/17
                update_aof (); //lyu
                my_initial_all( view_idx );
            }
            else if(prca->rc_enable==1)
            {
                if ((prca->fd_iframe==1) && (prca->fd_row_cnt==0) && (prca->last_p_gop_change==TRUE) && (prca->gop_change_NotResetRC==0)) { // lhu, 2017/04/17
                  // release last_p_gop_change to FALSE and execute initial_all operation until the first bu of iframe reached!!!
                    update_aof (); //lyu
                    my_initial_all( view_idx );
                    prca->last_p_gop_change=FALSE;
                }
                if ((prca->gop_cnt!=0) && (prca->fd_iframe==1) && (prca->fd_last_row==1) && (prca->poweron_rc_params_set==1)) {
                    prca->poweron_rc_params_set = 0; //lhuemu
                }
                my_rc_ac_br(view_idx);
                if(prca->re_bitrate==1 && prca->fd_last_row==1) // change bit rate
                    READ_WORD_ENC(reg[view_idx][BR_ADDR],prca->bit_rate); //read br

                READ_WORD_ENC(reg[view_idx][MAD_ADDR],prca->mad_tmp); //read mad
                READ_WORD_ENC(reg[view_idx][HBITS_ADDR],prca->hbits_tmp); //read hbits
                //READ_WORD_ENC(reg[VIEW0][TBITS_ADDR],v0_tbits_tmp); //read tbits
                //Fix the HeaderBits and TextureBits assignment according to lyu's statistics === begin
                if (prca->type==I_SLICE) prca->hbits_tmp = prca->hbits_tmp - ((prca->MBPerRow*3)>>1); // decrease 1.5 bit every MB for I SLICE
                else                     prca->hbits_tmp = prca->hbits_tmp - 5;                     // decrease 5 bit every BU for P SLICE
                READ_WORD_ENC(reg[view_idx][ABITS_ADDR],prca->fbits_tmp);
                prca->tbits_tmp = (prca->fbits_tmp - prca->PrevFbits) - prca->hbits_tmp;
                if (prca->bu_cnt==0) prca->PrevFbits = 0;
                else                  prca->PrevFbits = prca->fbits_tmp;
                //Fix the HeaderBits and TextureBits assignment according to lyu's statistics === end
                if(prca->fd_last_row==1) {
                    READ_WORD_ENC(reg[view_idx][ABITS_ADDR],prca->fbits_tmp); //read abits
                    READ_WORD_ENC(reg[view_idx][YMSEL_ADDR],prca->ymsel_tmp); // read frame's y-mse
                    READ_WORD_ENC(reg[view_idx][YMSEH_ADDR],prca->ymseh_tmp);
                    prca->ymse_frame = (((long long)((prca->ymseh_tmp>>24)&0x3f)<<32) + prca->ymsel_tmp);
                    if ((prca->ymseh_tmp>>2)&0x1) prca->wireless_screen=1; else prca->wireless_screen=0;
                    if ((prca->ymseh_tmp>>3)&0x1) prca->changeToIFrame=1; else prca->changeToIFrame=0; // lhu, 2017/03/09
                    if ((prca->ymseh_tmp>>4)&0x1) prca->insertOneIFrame=1; else prca->insertOneIFrame=0; // lhu, 2017/03/09
                    prca->frm_ymse[0]  = prca->ymse_frame; // lhu, 2017/03/27
                    prca->frm_fbits[0] = prca->fbits_tmp;  // lhu, 2017/03/27
                    prca->RCSliceBits = (prca->type==P_SLICE)? prca->RCPSliceBits:prca->RCISliceBits; // lhu, 2017/03/27
                    prca->frm_hbits[0] = prca->frame_hbits; // lhu, 2017/03/27
                    prca->frm_abits[0] = prca->frame_abits; // lhu, 2017/03/27
                    if (prca->fd_iframe==1) {
						prca->ifrm_ymse = prca->ymse_frame; // lhu, 2017/04/13
						prca->firstpframe_coming = 1;
                    }
                    else { // obtain the mininum ymse value for all the P Slice, lhu, 2017/05/26
                    	if(prca->firstpframe_coming == 1) {
							prca->min_pfrm_ymse = prca->ymse_frame;
							prca->firstpframe_coming = 0;
                    	}
						prca->min_pfrm_ymse = my_iequmin(prca->ymse_frame, prca->min_pfrm_ymse);
                        if (prca->fd_last_p==1) {
                            READ_WORD_ENC(reg[view_idx][ZW_BSINFO_ADDR],i);
                            if ((i>>1)&0x1) my_ac_RCISliceBitRatio(prca->RCISliceBitRatioMax,view_idx);
                            else {READ_WORD_ENC(reg[view_idx][RCSET2_ADDR],i); prca->RCISliceBitRatio = (i>>24)&0xf;}
                        }
                    }
                }
                qp = my_rc_handle_mb( view_idx ); // update QP once
                if(prca->fd_last_row==1) //frame last
                    prca->slice_qp = prca->PAveFrameQP;

                READ_WORD_ENC(reg[view_idx][QP_ADDR],i);
                WRITE_WORD_ENC(reg[view_idx][QP_ADDR],((qp<<24)+(prca->slice_qp<<16)+(i&0xffff))); //write qp & sqp, also maintain ac_gop and ac_iopratio
                my_hold( view_idx );
            }
            return 1; //// view0 done

        case 1:
			view_idx = VIEW1;
			prca = &rca[view_idx];

            if ((prca->fd_reset==1) && (prca->fd_last_p==1) && (prca->fd_last_row==1) && (prca->last_p_gop_change==FALSE) && (prca->poweron_rc_params_set==0) && (prca->gop_change_NotResetRC==0)) { // lhu, 2017/04/17
                update_aof (); //lyu
                my_initial_all( view_idx );
            }
            else if(prca->rc_enable==1)
            {
                if ((prca->fd_iframe==1) && (prca->fd_row_cnt==0) && (prca->last_p_gop_change==TRUE) && (prca->gop_change_NotResetRC==0)) { // lhu, 2017/04/17
                    // release last_p_gop_change to FALSE and execute initial_all operation until the first bu of iframe reached!!!
                    update_aof (); //lyu
                    my_initial_all( view_idx );
                    prca->last_p_gop_change=FALSE;
                }
                if ((prca->gop_cnt!=0) && (prca->fd_iframe==1) && (prca->fd_last_row==1) && (prca->poweron_rc_params_set==1)) {
                    prca->poweron_rc_params_set = 0; //lhuemu
                }
                my_rc_ac_br( view_idx );
                if(prca->re_bitrate==1 && prca->fd_last_row==1) // change bit rate
                    READ_WORD_ENC(reg[view_idx][BR_ADDR],prca->bit_rate); //read br

                READ_WORD_ENC(reg[view_idx][MAD_ADDR],   prca->mad_tmp); //read mad
                READ_WORD_ENC(reg[view_idx][HBITS_ADDR], prca->hbits_tmp); //read hbits
                //READ_WORD_ENC(reg[view_idx][TBITS_ADDR],v1_tbits_tmp); //read tbits
                //Fix the HeaderBits and TextureBits assignment according to lyu's statistics === begin
                if (prca->type==I_SLICE) prca->hbits_tmp = prca->hbits_tmp - (prca->MBPerRow*3/2); // decrease 1.5 bit every MB for I SLICE
                else                     prca->hbits_tmp = prca->hbits_tmp - 5;                     // decrease 5 bit every BU for P SLICE
                READ_WORD_ENC(reg[view_idx][ABITS_ADDR],prca->fbits_tmp);
                prca->tbits_tmp = (prca->fbits_tmp - prca->PrevFbits) - prca->hbits_tmp;
                if (prca->bu_cnt==0) prca->PrevFbits = 0;
                else                  prca->PrevFbits = prca->fbits_tmp;
                //Fix the HeaderBits and TextureBits assignment according to lyu's statistics === end
                if(prca->fd_last_row==1) {
                    READ_WORD_ENC(reg[view_idx][ABITS_ADDR],prca->fbits_tmp); //read abits
                    READ_WORD_ENC(reg[view_idx][YMSEL_ADDR],prca->ymsel_tmp); // read frame's y-mse
                    READ_WORD_ENC(reg[view_idx][YMSEH_ADDR],prca->ymseh_tmp);
                    prca->ymse_frame = (((long long)((prca->ymseh_tmp>>24)&0x3f)<<32) + prca->ymsel_tmp);
                    if ((prca->ymseh_tmp>>2)&0x1) prca->wireless_screen=1; else prca->wireless_screen=0;
                    if ((prca->ymseh_tmp>>3)&0x1) prca->changeToIFrame=1; else prca->changeToIFrame=0; // lhu, 2017/03/09
                    if ((prca->ymseh_tmp>>4)&0x1) prca->insertOneIFrame=1; else prca->insertOneIFrame=0; // lhu, 2017/03/09
                    prca->frm_ymse[0]  = prca->ymse_frame; // lhu, 2017/03/27
                    prca->frm_fbits[0] = prca->fbits_tmp;  // lhu, 2017/03/27
                    prca->RCSliceBits = (prca->type==P_SLICE)? prca->RCPSliceBits:prca->RCISliceBits; // lhu, 2017/03/27
                    prca->frm_hbits[0] = prca->frame_hbits; // lhu, 2017/03/27
                    prca->frm_abits[0] = prca->frame_abits; // lhu, 2017/03/27
                    if (prca->fd_iframe==1) {
						prca->ifrm_ymse = prca->ymse_frame; // lhu, 2017/04/13
						prca->firstpframe_coming = 1;
                    }
                    else { // obtain the mininum ymse value for all the P Slice, lhu, 2017/05/26
                    	if(prca->firstpframe_coming == 1) {
							prca->min_pfrm_ymse = prca->ymse_frame;
							prca->firstpframe_coming = 0;
                    	}
						prca->min_pfrm_ymse = my_iequmin(prca->ymse_frame, prca->min_pfrm_ymse);
                        if (prca->fd_last_p==1) {;
                            READ_WORD_ENC(reg[view_idx][ZW_BSINFO_ADDR],i);
                            if ((i>>1)&0x1) my_ac_RCISliceBitRatio(prca->RCISliceBitRatioMax,view_idx);
                            else {READ_WORD_ENC(reg[view_idx][RCSET2_ADDR],i); prca->RCISliceBitRatio = (i>>24)&0xf;}
                        }
                    }
                }
                qp = my_rc_handle_mb( view_idx ); // update QP once
                if(prca->fd_last_row==1) //frame last
                    prca->slice_qp = prca->PAveFrameQP;

                READ_WORD_ENC(reg[view_idx][QP_ADDR],i);
                WRITE_WORD_ENC(reg[view_idx][QP_ADDR],((qp<<24)+(prca->slice_qp<<16)+(i&0xffff))); //write qp & sqp, also maintain ac_gop and ac_iopratio
                my_hold( view_idx );
            }
            return 1; //// view1 done

        case 2: // 1080p used view0
            view_idx = VIEW1;
			prca = &rca[view_idx];
            if ((prca->fd_reset==1) && (prca->fd_last_p==1) && (prca->fd_last_row==1) && (prca->last_p_gop_change==FALSE) && (prca->poweron_rc_params_set==0) && (prca->gop_change_NotResetRC==0)) { // lhu, 2017/04/17
                update_aof (); //lyu
                my_initial_all( view_idx );
            }
            else if(prca->rc_enable==1)
            {
                if ((prca->fd_iframe==1) && (prca->fd_row_cnt==0) && (prca->last_p_gop_change==TRUE) && (prca->gop_change_NotResetRC==0)) { // lhu, 2017/04/17
                    // release last_p_gop_change to FALSE and execute initial_all operation until the first bu of iframe reached!!!
                    update_aof (); //lyu
                    my_initial_all( view_idx );
                    prca->last_p_gop_change=FALSE;
                }
                if ((prca->gop_cnt!=0) && (prca->fd_iframe==1) && (prca->fd_last_row==1) && (prca->poweron_rc_params_set==1)) {
                    prca->poweron_rc_params_set = 0; //lhuemu
                }
                my_rc_ac_br(view_idx);
                if(prca->re_bitrate==1 && prca->fd_last_row==1) // change bit rate
                    READ_WORD_ENC(reg[VIEW1][BR_ADDR],prca->bit_rate); //read br

                READ_WORD_ENC(reg[VIEW0][MAD_ADDR],prca->mad_tmp); //read mad
                READ_WORD_ENC(reg[VIEW0][HBITS_ADDR],prca->hbits_tmp); //read hbits
                //READ_WORD_ENC(reg[VIEW0][TBITS_ADDR],v1_tbits_tmp); //read tbits
                //Fix the HeaderBits and TextureBits assignment according to lyu's statistics === begin
                if (prca->type==I_SLICE) prca->hbits_tmp = prca->hbits_tmp - (prca->MBPerRow*3/2); // decrease 1.5 bit every MB for I SLICE
                else                     prca->hbits_tmp = prca->hbits_tmp - 5;                     // decrease 5 bit every BU for P SLICE
                READ_WORD_ENC(reg[VIEW0][ABITS_ADDR],prca->fbits_tmp);
                prca->tbits_tmp = (prca->fbits_tmp - prca->PrevFbits) - prca->hbits_tmp;
                if (prca->bu_cnt==0) prca->PrevFbits = 0;
                else                  prca->PrevFbits = prca->fbits_tmp;
                //Fix the HeaderBits and TextureBits assignment according to lyu's statistics === end
                if(prca->fd_last_row==1) {
                    READ_WORD_ENC(reg[VIEW0][ABITS_ADDR],prca->fbits_tmp); //read abits
                    READ_WORD_ENC(reg[VIEW0][YMSEL_ADDR],prca->ymsel_tmp); // read frame's y-mse
                    READ_WORD_ENC(reg[VIEW0][YMSEH_ADDR],prca->ymseh_tmp);
                    prca->ymse_frame = (((long long)((prca->ymseh_tmp>>24)&0x3f)<<32) + prca->ymsel_tmp);
                    if ((prca->ymseh_tmp>>2)&0x1) prca->wireless_screen=1; else prca->wireless_screen=0;
                    if ((prca->ymseh_tmp>>3)&0x1) prca->changeToIFrame=1; else prca->changeToIFrame=0; // lhu, 2017/03/09
                    if ((prca->ymseh_tmp>>4)&0x1) prca->insertOneIFrame=1; else prca->insertOneIFrame=0; // lhu, 2017/03/09
                    prca->frm_ymse[0]  = prca->ymse_frame; // lhu, 2017/03/27
                    prca->frm_fbits[0] = prca->fbits_tmp;  // lhu, 2017/03/27
                  prca->RCSliceBits = (prca->type==P_SLICE)? prca->RCPSliceBits:prca->RCISliceBits; // lhu, 2017/03/27
                    prca->frm_hbits[0] = prca->frame_hbits; // lhu, 2017/03/27
                    prca->frm_abits[0] = prca->frame_abits; // lhu, 2017/03/27
                    if (prca->fd_iframe==1) {
						prca->ifrm_ymse = prca->ymse_frame; // lhu, 2017/04/13
						prca->firstpframe_coming = 1;
                    }
                    else { // obtain the mininum ymse value for all the P Slice, lhu, 2017/05/26
                    	if(prca->firstpframe_coming == 1) {
							prca->min_pfrm_ymse = prca->ymse_frame;
							prca->firstpframe_coming = 0;
                    	}
						prca->min_pfrm_ymse = my_iequmin(prca->ymse_frame, prca->min_pfrm_ymse);
                        if (prca->fd_last_p==1) {
                            READ_WORD_ENC(reg[VIEW1][ZW_BSINFO_ADDR],i);
                            if ((i>>1)&0x1) my_ac_RCISliceBitRatio(prca->RCISliceBitRatioMax,1);
                            else {READ_WORD_ENC(reg[VIEW1][RCSET2_ADDR],i); prca->RCISliceBitRatio = (i>>24)&0xf;}
                        }
                    }
                }
                qp = my_rc_handle_mb( VIEW1 ); // update QP once
                if(prca->fd_last_row==1) //frame last
                    prca->slice_qp = prca->PAveFrameQP;

                READ_WORD_ENC(reg[VIEW1][QP_ADDR],i);
                WRITE_WORD_ENC(reg[VIEW1][QP_ADDR],((qp<<24)+(prca->slice_qp<<16)+(i&0xffff))); //write qp & sqp, also maintain ac_gop and ac_iopratio
                my_hold( VIEW1 );
            }
            return 1; //// view1 done

        default: return 1;
    }
}
#endif   //###########
//==============================================================================
//==============================================================================


//-----------------------------------------------------------------------------
// add by bomb for aof target cycles
//-----------------------------------------------------------------------------
void update_aof_cycle(){
    unsigned int v0_on=0,v0_mbs=0, v1_on=0,v1_mbs=0, tmp=0,i=0;

    READ_WORD((REG_BASE_ADDR+(0x00<<2)),v0_on); //view0
    v0_on=(v0_on>>24)&0x1;
    if(v0_on==1){
        READ_WORD((REG_BASE_ADDR+(0x01<<2)),i);
        v0_mbs=((((i>>16)&0xffff)+15)/16)*(((i&0xffff)+15)/16);
        READ_WORD((REG_BASE_ADDR+(0x02<<2)),i);
        v0_mbs=v0_mbs*((i>>16)&0xff);
    }
    READ_WORD((REG_BASE_ADDR+(0x19<<2)),v1_on); //view1
    v1_on=(v1_on>>24)&0x1;
    if(v1_on==1){
        READ_WORD((REG_BASE_ADDR+(0x1a<<2)),i);
        v1_mbs=((((i>>16)&0xffff)+15)/16)*(((i&0xffff)+15)/16);
        READ_WORD((REG_BASE_ADDR+(0x1b<<2)),i);
        v1_mbs=v1_mbs*((i>>16)&0xff);
    }

    i=v0_mbs+v1_mbs;
    if(i>0){
        i=IN_CLK/i;
        if(i>0xff00) i=0xff00; else if(i<540) i=540;
        if(i>580) i=i-30;

        READ_WORD((REG_BASE_ADDR+(0x0a<<2)),tmp); //write back view0
        tmp=(tmp&0xff0000ff)|((i&0xffff)<<8);
        WRITE_WORD((REG_BASE_ADDR+(0x0a<<2)),tmp);
        READ_WORD((REG_BASE_ADDR+(0x23<<2)),tmp); //write back view1
        tmp=(tmp&0xff0000ff)|((i&0xffff)<<8);
        WRITE_WORD((REG_BASE_ADDR+(0x23<<2)),tmp);
    }
}
void update_aof(){
    if(aof_v0_frame!=rca[0].frame_cnt){
        update_aof_cycle();
        aof_v0_frame=rca[0].frame_cnt;
    }
    else if(aof_v1_frame!=rca[1].frame_cnt){
        update_aof_cycle();
        aof_v1_frame=rca[1].frame_cnt;
    }
    else { 
        ;
    }
}
//-----------------------------------------------------------------------------


#ifdef ARMCM7_RC  //###########
//==============================================================================
void my_initial_all( unsigned int view_idx )
{
    int i;
    unsigned int qp;
    my_rc_params( view_idx );
    //if(prca->rc_enable==1) //lhuemu
    {
        qp = my_rc_handle_mb( view_idx ); // update QP initial
        READ_WORD_ENC   (reg[view_idx][QP_ADDR],i);
        WRITE_WORD_ENC  (reg[view_idx][QP_ADDR],((qp<<24)+(rca[view_idx].slice_qp<<16)+(i&0xffff))); //after RC initial..
    }
}

//// read feedback data for restart rc and etc
void my_feedback(unsigned int view_idx, unsigned int feedback)
{
    RC_DATA *prca = &rca[view_idx];
  
    //READ_WORD_ENC(reg[VIEW0][FEEDBACK_ADDR],i); //read feedback-data
    prca->aof_inc_qp = (feedback>>16)&0xffff;
    prca->fd_row_cnt = (feedback>>9)&0x7f;
    prca->fd_last_row = (feedback>>8)&0x1;
    //prca->fd_cpu_test = (feedback>>7)&0x1;
    prca->fd_irq_en = (feedback>>4)&0x1;
    //if(prca->v0_re_bitrate==0)
    prca->re_bitrate = (feedback>>3)&0x1;
    //if(prca->v0_fd_reset==0)
    prca->fd_reset = (feedback>>2)&0x1;
    prca->fd_iframe = (feedback>>1)&0x1;
    prca->fd_last_p = feedback&0x1;
    READ_WORD_ENC(reg[view_idx][RCEN_BU_ADDR],feedback); //read rc_en, rc_mode & bu
    prca->rc_enable = (feedback>>24)&0x1;
    READ_WORD_ENC(reg[view_idx][ENABLE_ADDR],feedback); //read view enable
    prca->enable = (feedback>>24)&0x1;
}
#endif   //###########



//-----------------------------------------------------------------------------------
// \brief
//    Initialize rate control parameters
//-----------------------------------------------------------------------------------
void my_rc_params( unsigned int view_idx ) {
    int i,j,m;
    RC_DATA *prca = &rca[view_idx];   

    prca->mad_tmp   = 0; // @lhu, initial value
    prca->hbits_tmp = 0;
    prca->tbits_tmp = 0;
    prca->enable    = 0;
    prca->gop_cnt   = 0;
    prca->frame_cnt = 0;
    prca->bu_cnt    = 0;
    prca->mb_cnt    = 0;
    prca->fd_reset  = 0; ////
    prca->aof_inc_qp = 0;
    prca->fd_last_row= 0;
    prca->fd_row_cnt = 0;
    prca->fd_last_p  = 0;
    prca->fd_iframe  = 0;
    prca->re_bitrate = 0;
    prca->prev_ac_br_index = 0;
    prca->wireless_screen = 0; // lhu, 2017/02/27
    prca->changeToIFrame = 0; // lhu, 2017/03/09
    prca->insertOneIFrame= 0; // lhu, 2017/03/09
    prca->PrevFrmPSNRLow= 0; // lhu, 2017/03/15
    prca->gop_change_NotResetRC = 0; // lhu, 2017/03/07
    prca->nextPFgotoIF = 0; // lhu, 2017/03/07
    prca->IFduration = 0; // lhu, 2017/03/07
    prca->PrevFbits = 0; // lhu, 2017/04/05
    prca->last_p_gop_change = FALSE; // @lhu, initial value

    READ_WORD_ENC(reg[view_idx][FRAME_XY_ADDR],m); //read frame-x & frame-y
        i=(m>>16)&0xffff;
        j=m&0xffff;
        prca->MBPerRow = (i+15)/16;
        prca->FrameSizeInMbs = prca->MBPerRow*((j+15)/16);
        prca->size = prca->FrameSizeInMbs<<8;
        prca->width = i;
        prca->height = j;

    READ_WORD_ENC(reg[view_idx][GOPFPS_ADDR],i); //read gop and fps
        i=(i>>16)&0xffff;
        prca->intra_period = (i>>8)&0xff;
        prca->framerate = i&0xff;

    READ_WORD_ENC(reg[view_idx][RCEN_BU_ADDR],i); //read rc_en, rc_mode & bu
        prca->rc_enable = (i>>24)&0x1;
        prca->RCUpdateMode = (i>>16)&0x3;
        prca->BasicUnit = (i&0xffff);
        prca->basicunit = (i&0xffff);

    READ_WORD_ENC(reg[view_idx][RCSET1_ADDR],i); //read rcset1
        prca->PMaxQpChange  = i&0x3f;
        prca->RCMinQP       = (i>>8)&0x3f;
        prca->RCMaxQP       = (i>>16)&0x3f;


    READ_WORD_ENC(reg[view_idx][BR_ADDR],i); //read br
        prca->bit_rate = i;

    READ_WORD_ENC(reg[view_idx][RCSET2_ADDR],i); //read rcset2
        prca->RCISliceBitRatioMax= (i>>8)&0x3f;
        prca->RCIoverPRatio = (i>>16)&0xf;
        prca->RCISliceBitRatio = (i>>24)&0xf;
        unsigned int width_in256 = ((prca->width + 255) & (~(0xFF)));
        prca->dvp_lb_freesize = 24576 - (( (width_in256 << 2)  + (width_in256<< 1) )); // 24576 - picture width * 16*3/2/4
        //dlog_info("dvp_lb_freesize %08x\n", prca->dvp_lb_freesize);
}


#ifdef ARMCM7_RC //###########
//===== Auto-config Bit-Rate =====
void my_rc_ac_br(int view) {
    int m,ac_br;
    unsigned char ac_br_index;
	RC_DATA *prca = &rca[view];

    READ_WORD_ENC(reg[view][RC_ACBR_ADDR],m);
    prca->ac_br_index = (m>>26)&0x3f;

    ac_br_index = prca->ac_br_index;
    ac_br = bridx2br[ac_br_index]; // use look-up table for bitrate generation, lhu, 2017/06/12

    if (ac_br_index != prca->prev_ac_br_index)
    {
         WRITE_WORD_ENC(reg[view][BR_ADDR], ac_br);
    }
    prca->prev_ac_br_index = ac_br_index;

}

#if 0
unsigned short my_divider2psnr(int my_divider) {
    unsigned short my_psnr;
    if      (my_divider>=1000000)                      my_psnr = 60;// 10^6.0=10000
    else if (my_divider>=794328 && my_divider<1000000) my_psnr = 59;// 10^5.9=794328
    else if (my_divider>=630957 && my_divider<794328)  my_psnr = 58;// 10^5.8=630957
    else if (my_divider>=501187 && my_divider<630957)  my_psnr = 57;// 10^5.7=501187
    else if (my_divider>=398107 && my_divider<501187)  my_psnr = 56;// 10^5.6=398107
    else if (my_divider>=316227 && my_divider<398107)  my_psnr = 55;// 10^5.5=316227
    else if (my_divider>=251188 && my_divider<316227)  my_psnr = 54;// 10^5.4=251188
    else if (my_divider>=199526 && my_divider<251188)  my_psnr = 53;// 10^5.3=199526
    else if (my_divider>=158489 && my_divider<199526)  my_psnr = 52;// 10^5.2=158489
    else if (my_divider>=125892 && my_divider<158489)  my_psnr = 51;// 10^5.1=125892
    else if (my_divider>=100000 && my_divider<125892)  my_psnr = 50;// 10^5.0=100000
    else if (my_divider>=79432  && my_divider<100000)  my_psnr = 49;// 10^4.9=79432
    else if (my_divider>=63095  && my_divider<79432 )  my_psnr = 48;// 10^4.8=63095
    else if (my_divider>=50118  && my_divider<63095 )  my_psnr = 47;// 10^4.7=50118
    else if (my_divider>=39810  && my_divider<50118 )  my_psnr = 46;// 10^4.6=39810
    else if (my_divider>=31622  && my_divider<39810 )  my_psnr = 45;// 10^4.5=31622
    else if (my_divider>=25118  && my_divider<31622 )  my_psnr = 44;// 10^4.4=25118
    else if (my_divider>=19952  && my_divider<25118 )  my_psnr = 43;// 10^4.3=19952
    else if (my_divider>=15848  && my_divider<19952 )  my_psnr = 42;// 10^4.2=15848
    else if (my_divider>=12589  && my_divider<15848 )  my_psnr = 41;// 10^4.1=12589
    else if (my_divider>=10000  && my_divider<12589 )  my_psnr = 40;// 10^4.0=10000
    else if (my_divider>=7943   && my_divider<10000 )  my_psnr = 39;// 10^3.9=7943
    else if (my_divider>=6309   && my_divider<7943  )  my_psnr = 38;// 10^3.8=6309
    else if (my_divider>=5011   && my_divider<6309  )  my_psnr = 37;// 10^3.7=5011
    else if (my_divider>=3981   && my_divider<5011  )  my_psnr = 36;// 10^3.6=3981
    else if (my_divider>=3162   && my_divider<3981  )  my_psnr = 35;// 10^3.5=3162
    else if (my_divider>=2511   && my_divider<3162  )  my_psnr = 34;// 10^3.4=2511
    else if (my_divider>=1995   && my_divider<2511  )  my_psnr = 33;// 10^3.3=1995
    else if (my_divider>=1584   && my_divider<1995  )  my_psnr = 32;// 10^3.2=1584
    else if (my_divider>=1258   && my_divider<1584  )  my_psnr = 31;// 10^3.1=1258
    else if (my_divider>=1000   && my_divider<1258  )  my_psnr = 30;// 10^3.0=1000
    else if (my_divider>=794    && my_divider<1000  )  my_psnr = 29;// 10^2.9=794
    else if (my_divider>=630    && my_divider<794   )  my_psnr = 28;// 10^2.8=630
    else if (my_divider>=501    && my_divider<630   )  my_psnr = 27;// 10^2.7=501
    else if (my_divider>=398    && my_divider<501   )  my_psnr = 26;// 10^2.6=398
    else if (my_divider>=316    && my_divider<398   )  my_psnr = 25;// 10^2.5=316
    else if (my_divider>=251    && my_divider<316   )  my_psnr = 24;// 10^2.4=251
    else if (my_divider>=199    && my_divider<251   )  my_psnr = 23;// 10^2.3=199
    else if (my_divider>=158    && my_divider<199   )  my_psnr = 22;// 10^2.2=158
    else if (my_divider>=125    && my_divider<158   )  my_psnr = 21;// 10^2.1=125
    else if (my_divider>=100    && my_divider<125   )  my_psnr = 20;// 10^2.0=100
    else if (my_divider>=79     && my_divider<100   )  my_psnr = 19;// 10^1.9=79
    else if (my_divider>=63     && my_divider<79    )  my_psnr = 18;// 10^1.8=63
    else if (my_divider>=50     && my_divider<63    )  my_psnr = 17;// 10^1.7=50
    else if (my_divider>=39     && my_divider<50    )  my_psnr = 16;// 10^1.6=39
    else if (my_divider>=31     && my_divider<39    )  my_psnr = 15;// 10^1.5=31
    else if (my_divider>=25     && my_divider<31    )  my_psnr = 14;// 10^1.4=25
    else                                               my_psnr = 13;
    
    return my_psnr;
}
#endif

// compare two consecutive P frame's psnr, if it drop sharply and the degree of drop is greater than psnr_drop_level, I frame should be inserted afterward.
unsigned char my_trace_PSNRDropSharply(unsigned char psnr_drop_level, unsigned char view) {
    long long whm255square,m;
    int prev_frame_divider,curr_frame_divider;
    unsigned short prev_frame_psnr, curr_frame_psnr;
	RC_DATA *prca = &rca[view];

    whm255square = (long long)(prca->width*255)*(long long)(prca->height*255);
    prev_frame_divider = (int)(whm255square/prca->frm_ymse[1]);
    curr_frame_divider = (int)(whm255square/prca->frm_ymse[0]);

    prev_frame_psnr = get_10log10(prev_frame_divider);
    curr_frame_psnr = get_10log10(curr_frame_divider);
    if ( (prev_frame_psnr>curr_frame_psnr) && ((prev_frame_psnr-curr_frame_psnr)>=psnr_drop_level) ) {
        prca->PSNRDropSharply = 1;
    } else {
        prca->PSNRDropSharply = 0;
    }
    return prca->PSNRDropSharply;
}
void my_criteria_decide_changeToIFrame(unsigned char HBitsRatioABits_level, unsigned char ABitsRatioTargetBits_level, unsigned char PSNRDrop_level, unsigned char view) {
	RC_DATA *prca = &rca[view];
	
    int IntraPeriod, NotResetRC, i, RCSliceBits;
    Boolean Condition1=FALSE,Condition2=FALSE,Condition3=FALSE;

    // Condition1: Hbits/(Hbits+Tbits) bigger or equal than HBitsRatioABits_level.
    if ( (prca->frm_hbits[0]*100/prca->frm_abits[0])>=HBitsRatioABits_level ) Condition1=TRUE;
    // Condition2: Abits/TargetBits of this frame is bigger or equal than ABitsRatioTargetBits_level.
    if ( (prca->frm_fbits[0]*100/prca->RCSliceBits)>=ABitsRatioTargetBits_level ) Condition2=TRUE;
    // Condition3: PSNR drop sharply and degree of the drop is greater than PSNRDrop_level.
    if ( my_trace_PSNRDropSharply(PSNRDrop_level,view) ) Condition3=TRUE;
    if ( Condition1==TRUE && Condition2==TRUE && Condition3==TRUE ) {prca->nextPFgotoIF=1; IntraPeriod=1; NotResetRC=1; prca->IFduration=1;}
    else                                                            {prca->nextPFgotoIF=0; NotResetRC=0; prca->IFduration=0;}


    prca->gop_change_NotResetRC = NotResetRC;
    if (prca->nextPFgotoIF==1) {
        READ_WORD_ENC(reg[view][GOPFPS_ADDR],i);
        prca->PrevIntraPeriod = (i>>24)&0xff; // save previous GOP before writing new GOP to it.
        WRITE_WORD_ENC(reg[view][GOPFPS_ADDR],((IntraPeriod<<24)+(i&0xffffff)));
        if (prca->insertOneIFrame==1) {
            READ_WORD_ENC(reg[view][GOPFPS_ADDR],i);
            prca->intra_period= (i>>24)&0xff;
        }
    }
}
// Go back to its normal GOP structure ===> After reach next GOP's I frame, release the IFduration.
void my_decide_backtoNormalGOP(int view) {
  RC_DATA *prca = &rca[view];
  int i;
  if ( (prca->frame_cnt==0) && (prca->IFduration==1) ) {
       READ_WORD_ENC(reg[view][GOPFPS_ADDR],i);
       WRITE_WORD_ENC(reg[view][GOPFPS_ADDR],((prca->PrevIntraPeriod<<24)+(i&0xffffff)));
       prca->IFduration=0;
       if (prca->insertOneIFrame==1) {
           READ_WORD_ENC(reg[view][GOPFPS_ADDR],i);
           prca->intra_period= (i>>24)&0xff; // go back to its normal IntraPeriod
       }
   }

}
/*===== Criteria for auto-config of RCISliceBitRatio=====
1> Depend on comparsion of I frame's psnr(Ipsnr) and MAX frame's psnr(Ppsnr) in current GOP.
2> Ppsnr-Ipsnr and RCISliceBitRatio_nextGOP and RCISliceBitRatio_currGOP's relation:
    ||                       ||
    \/                       \/
    -5             RCISliceBitRatio_currGOP-4
    ...                      ...
    -2             RCISliceBitRatio_currGOP-1
    -1             RCISliceBitRatio_currGOP
     0             RCISliceBitRatio_currGOP
    +1             RCISliceBitRatio_currGOP+1
    +2             RCISliceBitRatio_currGOP+2
    ...                      ...
    +5             RCISliceBitRatio_currGOP+5
3> Finally use RCISliceBitRatioMax value to clamp final output RCISliceBitRatio value.*/
void my_ac_RCISliceBitRatio(unsigned char RCISliceBitRatioMax, int view) {
    long long whm255square,m;
    int i,iframe_divider,pframe_divider_max;
    unsigned short iframe_psnr, pframe_psnr_max;
    signed short diffpsnr_PI;
	RC_DATA *prca = &rca[view];
    unsigned char RCISliceBitRatio_currGOP, RCISliceBitRatio_nextGOP;

    whm255square = (long long)(prca->width*255)*(long long)(prca->height*255);
    iframe_divider = (int)(whm255square/prca->ifrm_ymse);
    pframe_divider_max = (int)(whm255square/prca->min_pfrm_ymse);

    iframe_psnr     = get_10log10(iframe_divider);
    pframe_psnr_max = get_10log10(pframe_divider_max);

    diffpsnr_PI = pframe_psnr_max - iframe_psnr;
    RCISliceBitRatio_currGOP = prca->RCISliceBitRatio;
    
    if (diffpsnr_PI>=1)                         RCISliceBitRatio_nextGOP = RCISliceBitRatio_currGOP+diffpsnr_PI;
    else if (diffpsnr_PI>=-1 && diffpsnr_PI<=0) RCISliceBitRatio_nextGOP = RCISliceBitRatio_currGOP;
    else                                        RCISliceBitRatio_nextGOP = RCISliceBitRatio_currGOP+diffpsnr_PI+1;
    RCISliceBitRatio_nextGOP = my_imax(RCISliceBitRatio_nextGOP, 1);
    RCISliceBitRatio_nextGOP = my_iequmin(RCISliceBitRatio_nextGOP, RCISliceBitRatioMax);
    
    READ_WORD_ENC(reg[view][ZW_BSINFO_ADDR],i);
    WRITE_WORD_ENC(reg[view][ZW_BSINFO_ADDR],((i&0xffffff03)|((RCISliceBitRatio_nextGOP&0x3f)<<2)));
    prca->RCISliceBitRatio = RCISliceBitRatio_nextGOP;

}
#endif  //###########

void my_rc_init_seq( unsigned int view_idx )
{
//18  double L1,L2,L3;
  int bpp_p6,qp,i;
  RC_DATA *prca = &rca[view_idx];

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

  prca->type     = I_SLICE;
  prca->qp       = 0;
  prca->slice_qp = 0;
  prca->c1_over  = 0;
  prca->cmadequ0 = 0;// lhumad
  prca->frame_mad   = 0;
  prca->frame_tbits = 0;
  prca->frame_hbits = 0;
  prca->frame_abits = 0;

  //if(prca->intra_period==1)
  //  prca->RCUpdateMode = RC_MODE_1;

  if(prca->RCUpdateMode!=RC_MODE_0)
  {
    if (prca->RCUpdateMode==RC_MODE_1 && prca->intra_period==1) {// make sure it execute only once!!! lhumod
      prca->no_frm_base = prca->intra_period*50; //!!!
      prca->intra_period = prca->no_frm_base;// make fake for frame_cnt increment, lhumod
    }
    else if (prca->RCUpdateMode==RC_MODE_3) {
		prca->no_frm_base = prca->intra_period*1; // lhugop
    }
  }

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
  switch (prca->RCUpdateMode )
  {
     //case RC_MODE_0: my_v0_updateQP = my_v0_updateQPRC0; break;
     //case RC_MODE_1: my_v0_updateQP = my_v0_updateQPRC1; break;
     case RC_MODE_3: my_updateQP = my_updateQPRC3; break;
     default: my_updateQP = my_updateQPRC3; break;
  }

  prca->PreviousMAD_8p = (1<<8);
  prca->CurrentMAD_8p  = (1<<8);
  prca->Target        = 0;
  prca->LowerBound    = 0;
  prca->UpperBound1   = MAX_INT;
  prca->UpperBound2   = MAX_INT;
  prca->PAveFrameQP   = 0;
  prca->m_Qc          = 0;
  prca->PAverageQp    = 0;

  for(i=0;i<70;i++)
  {
    prca->BUPFMAD_8p[i] = 0;
    prca->BUCFMAD_8p[i] = 0;
  }

  for(i=0;i<2;i++) { // set ymse_pframe[i] to max value at begining of sequence, lhu, 2017/03/27
    prca->frm_ymse[i] = prca->height*prca->width*((1<<8)-1)*((1<<8)-1);
  }
  prca->PrevBitRate = prca->bit_rate; //lhumod
  //compute the total number of MBs in a frame
  if(prca->basicunit >= prca->FrameSizeInMbs)
    prca->basicunit = prca->FrameSizeInMbs;

  if(prca->basicunit < prca->FrameSizeInMbs)
    prca->TotalNumberofBasicUnit = prca->FrameSizeInMbs/prca->basicunit;
  else
    prca->TotalNumberofBasicUnit = 1;

  //initialize the parameters of fluid flow traffic model
  prca->CurrentBufferFullness = 0;
//  rca.GOPTargetBufferLevel = 0; //(double)rca.CurrentBufferFullness;

  //initialize the previous window size
  prca->m_windowSize = 0;
  prca->MADm_windowSize = 0;
  prca->NumberofCodedPFrame = 0;
  prca->NumberofGOP = 0;
  //remaining # of bits in GOP
  prca->RemainingBits = 0;

  prca->GAMMAP_1p=1;
  prca->BETAP_1p=1;

  //quadratic rate-distortion model
  prca->PPreHeader=0;

  prca->Pm_X1_8p = prca->bit_rate<<8;
  prca->Pm_X2_8p = 0;
  // linear prediction model for P picture
  prca->PMADPictureC1_12p = (1<<12);
  prca->PMADPictureC2_12p = 0;
  prca->MADPictureC1_12p = (1<<12);
  prca->MADPictureC2_12p = 0;

  // Initialize values
  for(i=0;i<20;i++)
  {
    prca->m_rgQp_8p[i] = 0;
    prca->m_rgRp_8p[i] = 0;
    prca->m_rgRp_8prr8[i] = 0;
    prca->rc_tmp0[i] = 0;
    prca->rc_tmp1[i] = 0;
    prca->rc_tmp2[i] = 0;
    prca->rc_tmp3[i] = 0;
    prca->rc_tmp4[i] = 0;

    prca->PictureMAD_8p[i]   = 0;
    prca->ReferenceMAD_8p[i] = 0;
    prca->mad_tmp0[i] = 0;
    prca->mad_tmp0_valid[i] = 1;
    prca->mad_tmp1[i] = 0;
    prca->mad_tmp2[i] = 0;

    prca->rc_rgRejected[i] = FALSE;
    prca->mad_rgRejected[i] = FALSE;
  }

  prca->rc_hold = 0;
  prca->mad_hold = 0;

  prca->PPictureMAD_8p = 0;
  //basic unit layer rate control
  prca->PAveHeaderBits1 = 0;
  prca->PAveHeaderBits3 = 0;
  prca->DDquant = (prca->TotalNumberofBasicUnit>=9? 1:2);

  prca->frame_bs = prca->bit_rate/prca->framerate;

  bpp_p6=(prca->frame_bs<<6)/prca->size; //for test
/*if     (bpp_p6<=0x26) qp=35;
  else if(bpp_p6<=0x39) qp=25;
  else if(bpp_p6<=0x59) qp=20;
  else                  qp=10;*/// test for more initial_qp assignment, lhuemu
  if     (bpp_p6<=0x6 ) {if (prca->height>=1080) qp=42; else if(prca->height>=720) qp=40; else qp=38;}
  else if(bpp_p6<=0x16) {if (prca->height>=1080) qp=39; else if(prca->height>=720) qp=37; else qp=35;}
  else if(bpp_p6<=0x26) qp=35;
  else if(bpp_p6<=0x39) qp=25;
  else if(bpp_p6<=0x59) qp=20;
  else                  qp=10;

  prca->MyInitialQp=qp;
}


void my_rc_init_GOP(unsigned int view_idx, int np)
{
  RC_DATA *prca = &rca[view_idx];
  Boolean Overum=FALSE;
  int OverBits,denom,i;
  int GOPDquant;
  int gop_bits;
  int v0_RCISliceBitsLow,v0_RCISliceBitsHigh,v0_RCISliceBitsLow2,v0_RCISliceBitsHigh2,v0_RCISliceBitsLow4,v0_RCISliceBitsHigh4;
  int v0_RCISliceBitsLow8,v0_RCISliceBitsHigh8,v0_RCISliceBitsLow9,v0_RCISliceBitsHigh9; // lhuqu1

    //if(prca->RCUpdateMode != RC_MODE_0) {// lhugop
    //  my_v0_rc_init_seq( );
    //}
    //Compute InitialQp for each GOP
    prca->TotalPFrame = np;
    if(prca->gop_cnt==0)
    {
        prca->QPLastGOP   = prca->MyInitialQp;
        prca->PAveFrameQP = prca->MyInitialQp;
        prca->PAverageQp  = prca->MyInitialQp;
        prca->m_Qc        = prca->MyInitialQp;
    }
    else
    {
        //compute the average QP of P frames in the previous GOP
        prca->PAverageQp=(prca->TotalQpforPPicture+(np>>1))/np;// + 0.5);
        #ifdef JM_RC_DUMP
        #ifdef USE_MY_RC
        // rc-related debugging info dump, lhulhu
        {
          jm_rc_info_dump = fopen("jm_rc_info_dump.txt","a+");
          fprintf(jm_rc_info_dump, "(init_GOP_s1)PAverageQp:%-d \t", rca.PAverageQp);
          fclose (jm_rc_info_dump);
        }
        #endif
        #endif
        if     (np>=22) GOPDquant=2; // GOPDquant=(int)((1.0*(np+1)/15.0) + 0.5);
        else if(np>=7 ) GOPDquant=1; // if(GOPDquant>2)
        else            GOPDquant=0; // GOPDquant=2;

        prca->PAverageQp -= GOPDquant;

        if(prca->PAverageQp > (prca->QPLastPFrame-2))
            prca->PAverageQp--;

        if(prca->RCUpdateMode == RC_MODE_3) {
            //69 gop_bits = rca.v0_no_frm_base * rca.v0_frame_bs;
            gop_bits = (!prca->intra_period? 1:prca->intra_period)*(prca->bit_rate/prca->framerate);
            if (prca->IFduration==1 && prca->insertOneIFrame==1) {
                // Initial QP value should be fixed to certain value when decide to insert I Frame, lhu, 2017/05/26
                switch(prca->ac_br_index) {
                    case 0 : if (prca->width>1280 || prca->height>720) prca->PAverageQp = 30; else prca->PAverageQp = 28; break; // 8Mbps
                    case 1 : if (prca->width>1280 || prca->height>720) prca->PAverageQp = 38; else prca->PAverageQp = 36; break; // 600kps
                    case 2 : if (prca->width>1280 || prca->height>720) prca->PAverageQp = 37; else prca->PAverageQp = 35; break; // 1.2Mbps
                    case 3 : if (prca->width>1280 || prca->height>720) prca->PAverageQp = 36; else prca->PAverageQp = 34; break; // 2.4Mbps
                    case 4 : if (prca->width>1280 || prca->height>720) prca->PAverageQp = 35; else prca->PAverageQp = 33; break; // 3Mbps
                    case 5 : if (prca->width>1280 || prca->height>720) prca->PAverageQp = 35; else prca->PAverageQp = 33; break; // 3.5Mbps
                    case 6 : if (prca->width>1280 || prca->height>720) prca->PAverageQp = 34; else prca->PAverageQp = 32; break; // 4Mbps
                    case 7 : if (prca->width>1280 || prca->height>720) prca->PAverageQp = 33; else prca->PAverageQp = 31; break; // 4.8Mbps
                    case 8 : if (prca->width>1280 || prca->height>720) prca->PAverageQp = 33; else prca->PAverageQp = 31; break; // 5Mbps
                    case 9 : if (prca->width>1280 || prca->height>720) prca->PAverageQp = 32; else prca->PAverageQp = 30; break; // 6Mbps
                    case 10: if (prca->width>1280 || prca->height>720) prca->PAverageQp = 31; else prca->PAverageQp = 29; break; // 7Mbps
                    case 11: if (prca->width>1280 || prca->height>720) prca->PAverageQp = 30; else prca->PAverageQp = 28; break; // 7.5Mbps
                    case 12: if (prca->width>1280 || prca->height>720) prca->PAverageQp = 29; else prca->PAverageQp = 27; break; // 9Mbps
                    case 13: if (prca->width>1280 || prca->height>720) prca->PAverageQp = 28; else prca->PAverageQp = 26; break; // 10Mbps
                    default: if (prca->width>1280 || prca->height>720) prca->PAverageQp = 30; else prca->PAverageQp = 28; break; // 8Mbps
                }
            } else {
                // Use the Previous ISliceBitRatio to calculate the initial QP value of current I Slice, lhu, 2017/05/25
                prca->RCISliceTargetBits = gop_bits * prca->RCISliceBitRatio/(prca->RCISliceBitRatio+(prca->intra_period-1));
                v0_RCISliceBitsLow    = prca->RCISliceTargetBits*9/10;
                v0_RCISliceBitsHigh   = prca->RCISliceTargetBits*11/10;
                v0_RCISliceBitsLow2   = prca->RCISliceTargetBits*8/10;
                v0_RCISliceBitsHigh2  = prca->RCISliceTargetBits*12/10;
                v0_RCISliceBitsLow4   = prca->RCISliceTargetBits*6/10;
                v0_RCISliceBitsHigh4  = prca->RCISliceTargetBits*14/10;
                v0_RCISliceBitsLow8   = prca->RCISliceTargetBits*2/10;
                v0_RCISliceBitsHigh8  = prca->RCISliceTargetBits*18/10;
                v0_RCISliceBitsLow9   = prca->RCISliceTargetBits*1/10;
                v0_RCISliceBitsHigh9  = prca->RCISliceTargetBits*19/10;
                if(prca->RCISliceActualBits  <= v0_RCISliceBitsLow9)                                                              prca->PAverageQp = prca->QPLastGOP-8;
                else if((v0_RCISliceBitsLow9  < prca->RCISliceActualBits) && (prca->RCISliceActualBits <= v0_RCISliceBitsLow8))  prca->PAverageQp = prca->QPLastGOP-6;
                else if((v0_RCISliceBitsLow8  < prca->RCISliceActualBits) && (prca->RCISliceActualBits <= v0_RCISliceBitsLow4))  prca->PAverageQp = prca->QPLastGOP-4;
                else if((v0_RCISliceBitsLow4  < prca->RCISliceActualBits) && (prca->RCISliceActualBits <= v0_RCISliceBitsLow2))  prca->PAverageQp = prca->QPLastGOP-2;
                else if((v0_RCISliceBitsLow2  < prca->RCISliceActualBits) && (prca->RCISliceActualBits <= v0_RCISliceBitsLow))   prca->PAverageQp = prca->QPLastGOP-1;
                else if((v0_RCISliceBitsLow   < prca->RCISliceActualBits) && (prca->RCISliceActualBits <= v0_RCISliceBitsHigh))  prca->PAverageQp = prca->QPLastGOP;
                else if((v0_RCISliceBitsHigh  < prca->RCISliceActualBits) && (prca->RCISliceActualBits <= v0_RCISliceBitsHigh2)) prca->PAverageQp = prca->QPLastGOP+1;
                else if((v0_RCISliceBitsHigh2 < prca->RCISliceActualBits) && (prca->RCISliceActualBits <= v0_RCISliceBitsHigh4)) prca->PAverageQp = prca->QPLastGOP+2;
                else if((v0_RCISliceBitsHigh4 < prca->RCISliceActualBits) && (prca->RCISliceActualBits <= v0_RCISliceBitsHigh8)) prca->PAverageQp = prca->QPLastGOP+4;
                else if((v0_RCISliceBitsHigh8 < prca->RCISliceActualBits) && (prca->RCISliceActualBits <= v0_RCISliceBitsHigh9)) prca->PAverageQp = prca->QPLastGOP+6;
                else if(prca->RCISliceActualBits > v0_RCISliceBitsHigh9)                                                          prca->PAverageQp = prca->QPLastGOP+8;
            }

        } else {
            // QP is constrained by QP of previous QP
            prca->PAverageQp = my_iClip3(prca->QPLastGOP-2, prca->QPLastGOP+2, prca->PAverageQp);
        }
        #ifdef JM_RC_DUMP
        #ifdef USE_MY_RC
        // rc-related debugging info dump, lhulhu
        {
          jm_rc_info_dump = fopen("jm_rc_info_dump.txt","a+");
          fprintf(jm_rc_info_dump, "(init_GOP_s2)PAverageQp:%-d \n", rca.PAverageQp);
          fclose (jm_rc_info_dump);
        }
        #endif
        #endif
        // Also clipped within range.
        prca->PAverageQp = my_iClip3(prca->RCMinQP,  prca->RCMaxQP,  prca->PAverageQp);

        prca->MyInitialQp = prca->PAverageQp;
        prca->Pm_Qp       = prca->PAverageQp;
        prca->PAveFrameQP = prca->PAverageQp; //(13)
        prca->QPLastGOP   = prca->PAverageQp;
    }

    prca->TotalQpforPPicture=0;//(13)

    // bit allocation for RC_MODE_3
    if(prca->RCUpdateMode == RC_MODE_3) // running this only once !!!
    {
        // calculate allocated bRCUpdateModeits for each type of frame
        // Fix the ISliceBitRatio when decide to insert I Frame and calculate bit target for I/P frame, lhu, 2017/05/25
        if (prca->IFduration==1 && prca->insertOneIFrame==1)
            prca->RCISliceBitRatio = 4;

        denom = (!prca->intra_period? 1:prca->intra_period) + prca->RCISliceBitRatio - 1;

        // set bit targets for each type of frame
//18      rca.RCPSliceBits = (int)floor(gop_bits/denom + 0.5F);
        prca->RCPSliceBits = gop_bits/denom ;
        prca->RCISliceBits = (prca->intra_period)? (prca->RCISliceBitRatio * prca->RCPSliceBits) : 0;

        prca->NISlice = (prca->intra_period)? (prca->intra_period/prca->intra_period):0; // totoal I-frame number
        prca->NPSlice = prca->intra_period - prca->NISlice;
    }

    // check if the last GOP over uses its budget. If yes, the initial QP of the I frame in
    // the coming  GOP will be increased.
    if(prca->RemainingBits<0)
        Overum=TRUE;
    OverBits=-prca->RemainingBits;

    prca->RemainingBits = 0; // set remainingbits as 0 at beginning of gop, lhu, 2017/02/08
    //initialize the lower bound and the upper bound for the target bits of each frame, HRD consideration
    prca->LowerBound  = prca->RemainingBits + (prca->bit_rate/prca->framerate);
    prca->UpperBound1 = prca->RemainingBits + (prca->bit_rate<<1); //2.048
    prca->UpperBound2  = ((OMEGA_4p*prca->UpperBound1) >> 4); // lhu, 2017/03/13

    //compute the total number of bits for the current GOP
    if (prca->IFduration!=1)
        gop_bits = (1+np)*(prca->bit_rate/prca->framerate);
    else {
        if (prca->changeToIFrame==1)
            gop_bits = ((1+np)*(prca->bit_rate/prca->framerate)*14)/10; // expand whole GOP target by 40%, lhu, 2017/03/07
        else if (prca->insertOneIFrame==1)
            gop_bits = (1+np)*(prca->bit_rate/prca->framerate); // maintain the original GOP target, lhu, 2017/03/09
    }
    prca->RemainingBits+= gop_bits;
    prca->Np = np;

    //  OverDuantQp=(int)(8 * OverBits/gop_bits+0.5);
    prca->GOPOverdue=FALSE;

}


void my_rc_init_pict(unsigned int view_idx, int mult)
{
  RC_DATA *prca = &rca[view_idx];
  int i,tmp_T;

    //if ( prca->type==P_SLICE ) //g1|| (rca.RCUpdateMode==RC_MODE_1 &&(rca.gop_cnt!=0 || rca.frame_cnt!=0)) ) // (rca.number !=0)
    if ( prca->type==P_SLICE || ((prca->RCUpdateMode==RC_MODE_1 || (prca->type==I_SLICE&&prca->RCUpdateMode==RC_MODE_3)) && (!(prca->gop_cnt==0 && prca->frame_cnt==0))) ) // lhuitune
    {
      //// for CBR ...
      if(prca->PrevBitRate!=prca->bit_rate)
        prca->RemainingBits += (prca->bit_rate - prca->PrevBitRate)*prca->Np/prca->framerate;
      /*if(prca->re_bitrate == 1)
      {
        prca->re_bitrate = 0;
        prca->RemainingBits += (prca->new_bitrate - prca->bit_rate)*prca->Np/prca->framerate;
        prca->bit_rate = prca->new_bitrate;
      }*/

      // Frame - Level
      if(prca->BasicUnit >= prca->FrameSizeInMbs)
      {
        if(prca->frame_cnt==2) //(rca.NumberofPPicture==1)
        {
          prca->TargetBufferLevel = prca->CurrentBufferFullness;
//18          rca.DeltaP = (rca.CurrentBufferFullness - rca.GOPTargetBufferLevel) / (rca.TotalPFrame-1);
          prca->DeltaP = prca->CurrentBufferFullness/(prca->TotalPFrame-1);
          prca->TargetBufferLevel -= prca->DeltaP;
        }
        else if(prca->frame_cnt>2) //(rca.NumberofPPicture>1)
          prca->TargetBufferLevel -= prca->DeltaP;
      }
      // BU - Level
      else
      {
        if(prca->NumberofCodedPFrame>0)
        {
          for(i=0;i<prca->TotalNumberofBasicUnit;i++)
             prca->BUPFMAD_8p[i] = prca->BUCFMAD_8p[i];
        }

        if(prca->gop_cnt==0) //(rca.NumberofGOP==1)
        {
          if(prca->frame_cnt==2) //(rca.NumberofPPicture==1)
          {
            prca->TargetBufferLevel = prca->CurrentBufferFullness;
//18            rca.DeltaP = (rca.CurrentBufferFullness - rca.GOPTargetBufferLevel)/(rca.TotalPFrame-1);
            prca->DeltaP = prca->CurrentBufferFullness/(prca->TotalPFrame-1);
            prca->TargetBufferLevel -= prca->DeltaP;
          }
          else if(prca->frame_cnt>2) //(rca.NumberofPPicture>1)
            prca->TargetBufferLevel -= prca->DeltaP;
        }
        else if(prca->gop_cnt>0) //(rca.NumberofGOP>1)
        {
          if(prca->frame_cnt==1) //(rca.NumberofPPicture==0)
          {
            prca->TargetBufferLevel = prca->CurrentBufferFullness;
//18            rca.DeltaP = (rca.CurrentBufferFullness - rca.GOPTargetBufferLevel) / rca.TotalPFrame;
            prca->DeltaP = prca->CurrentBufferFullness/prca->TotalPFrame;
            prca->TargetBufferLevel -= prca->DeltaP;
          }
          else if(prca->frame_cnt>1) //(rca.NumberofPPicture>0)
            prca->TargetBufferLevel -= prca->DeltaP;
        }
      }
    }

    // Compute the target bit for each frame
    if(prca->type==P_SLICE || ((prca->gop_cnt!=0 || prca->frame_cnt!=0) && (prca->RCUpdateMode==RC_MODE_1 || prca->RCUpdateMode==RC_MODE_3)))
    {
        // frame layer rate control
        if((prca->BasicUnit>=prca->FrameSizeInMbs || (prca->RCUpdateMode==RC_MODE_3)) && (prca->NumberofCodedPFrame>0))
        {
            if(prca->RCUpdateMode == RC_MODE_3)
            {
                int bitrate = (prca->type==P_SLICE)? prca->RCPSliceBits:prca->RCISliceBits;
                int denom = prca->NISlice*prca->RCISliceBits + prca->NPSlice*prca->RCPSliceBits;

                // target due to remaining bits
                prca->Target = ((long long)bitrate*(long long)prca->RemainingBits) / denom;

                // target given original taget rate and buffer considerations
//18            tmp_T = imax(0, (int)floor((double)bitrate - ((rca.CurrentBufferFullness-rca.TargetBufferLevel)/rca.GAMMAP) + 0.5) );
//s             tmp_T = imax(0, bitrate-((rca.CurrentBufferFullness-rca.TargetBufferLevel)/rca.GAMMAP_1p));
                tmp_T = my_imax(0, (bitrate-((prca->CurrentBufferFullness-prca->TargetBufferLevel)>>1)));

                if(prca->type == I_SLICE) {
                    //prca->Target = prca->Target/(prca->RCIoverPRatio); //lhulhu
                }
            }
            else
            {
//18              rca.Target = (int) floor( rca.RemainingBits / rca.Np + 0.5);
                prca->Target = prca->RemainingBits/prca->Np;
                tmp_T=my_imax(0, ((prca->bit_rate/prca->framerate) - ((prca->CurrentBufferFullness-prca->TargetBufferLevel)>>1)));
//s              rca.Target = ((rca.Target-tmp_T)/rca.BETAP) + tmp_T;
                prca->Target = (prca->Target+tmp_T)>>1;
            }
        }
      // basic unit layer rate control
      else
      {
        if(((prca->gop_cnt==0)&&(prca->NumberofCodedPFrame>0)) || (prca->gop_cnt>0))
        {
//18          rca.Target = (int)(floor(rca.RemainingBits/rca.Np + 0.5));
          prca->Target = prca->RemainingBits/prca->Np;
          tmp_T = my_imax(0, ((prca->bit_rate/prca->framerate) - ((prca->CurrentBufferFullness-prca->TargetBufferLevel)>>1)));

//s          rca.Target = ((rca.Target-tmp_T)*rca.BETAP) + tmp_T;
          prca->Target = ((prca->Target+tmp_T)>>1);
        }
      }
      prca->Target = mult * prca->Target;

      // HRD consideration
      if(prca->RCUpdateMode!=RC_MODE_3 || prca->type==P_SLICE) {
        if (prca->IFduration!=1)
          prca->Target = my_iClip3(prca->LowerBound, prca->UpperBound2, prca->Target);
        else {
          if (prca->changeToIFrame==1)
            prca->Target = (prca->Target*14)/10; // expand P frame target by 40%, lhu, 2017/03/07
        }
      }
    }

    #ifdef JM_RC_DUMP
    #ifdef USE_MY_RC
    // rc-related debugging info dump, lhulhu
    {
      jm_rc_info_dump = fopen("jm_rc_info_dump.txt","a+");
      fprintf(jm_rc_info_dump, "Target(init_pict):%-d \t", rca.Target);
      fclose (jm_rc_info_dump);
    }
    #endif
    #endif
    // frame layer rate control
    prca->NumberofHeaderBits  = 0;
    prca->NumberofTextureBits = 0;
    prca->TotalFrameMAD = 0;// lhumod
    // basic unit layer rate control
    if(prca->BasicUnit < prca->FrameSizeInMbs)
    {
      prca->TotalFrameQP = 0;
      prca->NumberofBasicUnitHeaderBits  = 0;
      prca->NumberofBasicUnitTextureBits = 0;
      prca->TotalMADBasicUnit = 0;
    }
    prca->PrevBitRate = prca->bit_rate; // lhumod
    prca->PrevRCMinQP = prca->RCMinQP; // lhupsnr
}


void my_rc_update_pict(unsigned int view_idx, int nbits) // after frame running once
{
  int delta_bits;
  RC_DATA *prca = &rca[view_idx];
/////////////////////////////////////////////////////  my_rc_update_pict_frame( );
  if((prca->RCUpdateMode==RC_MODE_0) || (prca->RCUpdateMode==RC_MODE_2)){
    if(prca->type==P_SLICE)
      my_updatePparams( view_idx );
  }
  else if(prca->RCUpdateMode==RC_MODE_1){
    if(prca->type==P_SLICE) //g1   (rca.gop_cnt!=0 || rca.frame_cnt!=0) //( rca.number != 0 )
      my_updatePparams( view_idx );
  }
  else if(prca->RCUpdateMode==RC_MODE_3){
    if(prca->type==I_SLICE && (prca->gop_cnt!=0 || prca->frame_cnt!=0)) //(rca.number != 0)
      prca->NISlice--;
    if(prca->type==P_SLICE)
    {
      my_updatePparams( view_idx );
      prca->NPSlice--;
    }
  }
/////////////////////////////////////////////////////
  if (prca->RCUpdateMode==RC_MODE_3 && prca->type==I_SLICE) { // lhugop, save bits number for I_SLICE every gop
    prca->RCISliceActualBits = nbits;
  }

  delta_bits=nbits - (prca->bit_rate/prca->framerate);
  // remaining # of bits in GOP
  prca->RemainingBits -= nbits;
  prca->CurrentBufferFullness += delta_bits;

  // update the lower bound and the upper bound for the target bits of each frame, HRD consideration
  prca->LowerBound  -= delta_bits;
  prca->UpperBound1 -= delta_bits;
  prca->UpperBound2  = ((OMEGA_4p*prca->UpperBound1) >> 4);

  // update the parameters of quadratic R-D model
  if(prca->type==P_SLICE || (prca->RCUpdateMode==RC_MODE_1 && (prca->gop_cnt!=0 || prca->frame_cnt!=0)))
  {
    my_updateRCModel( view_idx );
    if(prca->RCUpdateMode == RC_MODE_3)
        prca->PreviousWholeFrameMAD_8p = prca->frame_mad; // my_ComputeFrameMAD( ) * (1<<8);
//21      rca.PreviousWholeFrameMAD = my_ComputeFrameMAD( ); ////!!!!
  }
}

void my_updatePparams( unsigned int view_idx )
{
  RC_DATA *prca = &rca[view_idx];
  
  prca->Np--;
  if(prca->NumberofCodedPFrame<=1000)
    prca->NumberofCodedPFrame++;
}



void my_updateRCModel ( unsigned int view_idx )
{
  int n_windowSize;
  int i,n_realSize;
  RC_DATA *prca = &rca[view_idx];
  
  int m_Nc = prca->NumberofCodedPFrame;
  Boolean MADModelFlag = FALSE;
//1  static Boolean m_rgRejected[RC_MODEL_HISTORY];
  int error_0p[RC_MODEL_HISTORY];
  unsigned int std_0p=0, threshold_0p;

  if(prca->bu_cnt==0)
    prca->codedbu_cnt = prca->TotalNumberofBasicUnit;
  else
    prca->codedbu_cnt = prca->bu_cnt;

  //if(prca->type==P_SLICE)//g1 || (prca->RCUpdateMode==RC_MODE_1 && (prca->gop_cnt!=0 || prca->frame_cnt!=0)) ) //(prca->number != 0)
  if( prca->type==P_SLICE || ((prca->RCUpdateMode==RC_MODE_1 || (prca->type==I_SLICE&&prca->RCUpdateMode==RC_MODE_3)) && (!(prca->gop_cnt==0&&prca->frame_cnt==0))) ) //lhuitune
  {
    //frame layer rate control
    if(prca->BasicUnit >= prca->FrameSizeInMbs)
    {
        prca->CurrentMAD_8p = prca->frame_mad; //my_ComputeFrameMAD() * (1<<8);
        m_Nc=prca->NumberofCodedPFrame;
    }
    //basic unit layer rate control
    else
    {
        //compute the MAD of the current bu
        prca->CurrentMAD_8p = prca->TotalMADBasicUnit/prca->BasicUnit;
        prca->TotalMADBasicUnit=0;

        // compute the average number of header bits
        prca->PAveHeaderBits1=(prca->PAveHeaderBits1*(prca->codedbu_cnt-1) + prca->NumberofBasicUnitHeaderBits)/prca->codedbu_cnt;
        if(prca->PAveHeaderBits3 == 0)
            prca->PAveHeaderBits2 = prca->PAveHeaderBits1;
        else
        {
            prca->PAveHeaderBits2 = (prca->PAveHeaderBits1*prca->codedbu_cnt +
                prca->PAveHeaderBits3*(prca->TotalNumberofBasicUnit-prca->codedbu_cnt))/prca->TotalNumberofBasicUnit;
        }

//s        *(pp_BUCFMAD_8p+prca->codedbu_cnt-1) = prca->CurrentMAD_8p;
        prca->BUCFMAD_8p[prca->codedbu_cnt-1] = prca->CurrentMAD_8p;

        if(prca->codedbu_cnt >= prca->TotalNumberofBasicUnit)
            m_Nc = prca->NumberofCodedPFrame * prca->TotalNumberofBasicUnit;
        else
            m_Nc = prca->NumberofCodedPFrame * prca->TotalNumberofBasicUnit + prca->codedbu_cnt;
    }

    if(m_Nc > 1)
      MADModelFlag=TRUE;

    prca->PPreHeader = prca->NumberofHeaderBits;

    // hold to over
    prca->rc_hold = 1;

    prca->m_rgQp_8p[0] = QP2Qstep_8p(prca->m_Qc); //*1.0/prc->CurrentMAD;

    if(prca->BasicUnit >= prca->FrameSizeInMbs) {//frame layer rate control
        if(prca->CurrentMAD_8p==0) {// added by lhumad
            prca->cmadequ0 = 1;
            prca->m_rgRp_8p[0] = (long long)prca->NumberofTextureBits<<16;
        }
        else {
            prca->cmadequ0 = 0;
            prca->m_rgRp_8p[0] = ((long long)prca->NumberofTextureBits<<16)/prca->CurrentMAD_8p;
        }
    }
    else {//basic unit layer rate control
        if(prca->CurrentMAD_8p==0) {// added by lhumad
            prca->cmadequ0 = 1;
            prca->m_rgRp_8p[0] = (long long)prca->NumberofBasicUnitTextureBits<<16;
        }
        else {
            prca->cmadequ0 = 0;
            //prca->Pm_rgRp[0] = prca->NumberofBasicUnitTextureBits*1.0/prca->CurrentMAD;
            prca->m_rgRp_8p[0] = ((long long)prca->NumberofBasicUnitTextureBits<<16)/prca->CurrentMAD_8p;
        }
    }

    prca->rc_tmp0[0] = (prca->m_rgQp_8p[0]>>4)*(prca->m_rgRp_8p[0]>>4);
    prca->rc_tmp1[0] = (1<<24)/(prca->m_rgQp_8p[0]>>4);
    prca->rc_tmp4[0] = (prca->m_rgQp_8p[0]>>4)*(prca->m_rgQp_8p[0]>>4);
    prca->rc_tmp2[0] = (1<<28)/prca->rc_tmp4[0];
    prca->m_rgRp_8prr8[0] = prca->m_rgRp_8p[0]>>8;
    prca->rc_tmp3[0] = (prca->m_rgQp_8p[0]>>8)*prca->m_rgRp_8prr8[0];;
    prca->m_X1_8p = prca->Pm_X1_8p;
    prca->m_X2_8p = prca->Pm_X2_8p;

    //compute the size of window
    //n_windowSize = (prca->CurrentMAD>prca->PreviousMAD)? (int)(prca->PreviousMAD/prca->CurrentMAD * (RC_MODEL_HISTORY-1))
    //    :(int)(prca->CurrentMAD/prca->PreviousMAD * (RC_MODEL_HISTORY-1));
    n_windowSize = (prca->CurrentMAD_8p>prca->PreviousMAD_8p)? ((prca->PreviousMAD_8p*20)/prca->CurrentMAD_8p):
        ((prca->CurrentMAD_8p*20)/prca->PreviousMAD_8p);

    n_windowSize=my_iClip3(1, m_Nc, n_windowSize);
    n_windowSize=my_imin(n_windowSize,prca->m_windowSize+1); // m_windowSize:: previous_windowsize
    n_windowSize=my_imin(n_windowSize,20);

    //update the previous window size
    prca->m_windowSize = n_windowSize;
    n_realSize = n_windowSize;

    // initial RD model estimator
    my_RCModelEstimator(view_idx, n_windowSize, n_windowSize, prca->rc_rgRejected);

    n_windowSize = prca->m_windowSize;
    // remove outlier

    for(i=0; i<n_windowSize; i++)
    {
//a     error_4p[i] = prca->m_X1_8p/prca->m_rgQp_8p[i] + (prca->m_X2_8p)/((prca->m_rgQp_8p[i]>>4)*(prca->m_rgQp_8p[i]>>4)) - (prca->m_rgRp_8p[i]>>8);
        error_0p[i] = prca->m_X1_8p/prca->m_rgQp_8p[i] + (prca->m_X2_8p/prca->rc_tmp4[i]) - prca->m_rgRp_8prr8[i];
        std_0p += error_0p[i]*error_0p[i];
    }

    threshold_0p = (n_windowSize==2)? 0:my_sqrt32(std_0p/n_windowSize);

    for(i=1;i<n_windowSize;i++)
    {
      if(abs(error_0p[i]) > threshold_0p)
      {
        prca->rc_rgRejected[i] = TRUE;
        n_realSize--;
      }
    }
    // always include the last data point
//1    prca->rc_rgRejected[0] = FALSE;

    // second RD model estimator
    my_RCModelEstimator(view_idx, n_realSize, n_windowSize, prca->rc_rgRejected);

    if( MADModelFlag )
      my_updateMADModel( view_idx );
    else if(prca->type==P_SLICE)//g1 || (prca->RCUpdateMode==RC_MODE_1 && (prca->gop_cnt!=0 || prca->frame_cnt!=0)) ) //(prca->number != 0)
      prca->PPictureMAD_8p = prca->CurrentMAD_8p;
  }
}


void my_RCModelEstimator (unsigned int view_idx, int n_realSize, int n_windowSize, char *rc_rgRejected)
{
  int i;
  Boolean estimateX2 = FALSE;
  unsigned int  a00_20p=0,a01_20p=0,a11_20p=0,b0_0p=0,b1_0p=0;
  long long  MatrixValue_20p;
  int sum_rc_tmp0=0;
  RC_DATA *prca = &rca[view_idx];

    // default RD model estimation results
    prca->m_X1_8p = 0;
    prca->m_X2_8p = 0;

    for(i=0;i<n_windowSize;i++) // if all non-rejected Q are the same, take 1st order model
    {
        if(!rc_rgRejected[i])
        {
            if((prca->m_rgQp_8p[i]!=prca->m_rgQp_8p[0]))
            {
                estimateX2 = TRUE;
                break;
            }
            sum_rc_tmp0 += prca->rc_tmp0[i]; // ((prca->m_rgQp_8p[i]>>4) * (prca->m_rgRp_8p[i]>>4));
        }
    }
    if(estimateX2==FALSE)
        prca->m_X1_8p = sum_rc_tmp0/n_realSize;


  // take 2nd order model to estimate X1 and X2
  if(estimateX2)
  {
    a00_20p = n_realSize<<20;
    for (i = 0; i < n_windowSize; i++)
    {
      if (!rc_rgRejected[i])
      {
        a01_20p += prca->rc_tmp1[i];
        a11_20p += prca->rc_tmp2[i];
        b0_0p   += prca->rc_tmp3[i];
        b1_0p   += prca->m_rgRp_8prr8[i];
      }
    }
    MatrixValue_20p = (((long long)a00_20p*(long long)a11_20p)-((long long)a01_20p*(long long)a01_20p)+(1<<19))>>20;
    if(MatrixValue_20p > 1)
    {
      prca->m_X1_8p = (((long long)b0_0p*(long long)a11_20p - (long long)b1_0p*(long long)a01_20p)<<8)/MatrixValue_20p;
      prca->m_X2_8p = (((long long)b1_0p*(long long)a00_20p - (long long)b0_0p*(long long)a01_20p)<<8)/MatrixValue_20p;
    }
    else
    {
      prca->m_X1_8p = (b0_0p<<8)/(a00_20p>>20);
      prca->m_X2_8p = 0;
    }
  }

  //if(prca->type==P_SLICE)//g1 || (prca->RCUpdateMode==RC_MODE_1 && (prca->gop_cnt!=0 || prca->frame_cnt!=0))) //(prca->number != 0)
  if( prca->type==P_SLICE || ((prca->RCUpdateMode==RC_MODE_1 || (prca->type==I_SLICE&&prca->RCUpdateMode==RC_MODE_3)) && (!(prca->gop_cnt==0&&prca->frame_cnt==0))) ) //lhuitune
  {
    prca->Pm_X1_8p = prca->m_X1_8p;
    prca->Pm_X2_8p = prca->m_X2_8p;
  }
}


void my_updateMADModel( unsigned int view_idx )
{
  RC_DATA *prca = &rca[view_idx];
  int    n_windowSize;
  int    i, n_realSize;
  int    m_Nc = prca->NumberofCodedPFrame;
  static int error_8p[RC_MODEL_HISTORY];
  long long std_16p=0;
  int threshold_8p;
  int MADPictureC2_12prr4;

  if(prca->NumberofCodedPFrame>0)
  {
    //frame layer rate control
    if(prca->BasicUnit >= prca->FrameSizeInMbs)
      m_Nc = prca->NumberofCodedPFrame;
    else // basic unit layer rate control
      m_Nc=prca->NumberofCodedPFrame*prca->TotalNumberofBasicUnit+prca->codedbu_cnt; //prca->CodedBasicUnit;

    // hold to over
    prca->mad_hold=1;

    prca->PPictureMAD_8p = prca->CurrentMAD_8p;
    prca->PictureMAD_8p[0]  = prca->PPictureMAD_8p;

    if(prca->BasicUnit >= prca->FrameSizeInMbs)
        prca->ReferenceMAD_8p[0]=prca->PictureMAD_8p[1];
    else
        prca->ReferenceMAD_8p[0]=prca->BUPFMAD_8p[prca->codedbu_cnt-1];
//s        prca->ReferenceMAD_8p[0] = *(pp_BUPFMAD_8p+prca->codedbu_cnt-1);

    if(prca->ReferenceMAD_8p[0] == 0)
    {
        prca->mad_tmp0_valid[0] = 0;
        prca->mad_tmp0[0] = 0;
    }
    else
    {
        prca->mad_tmp0_valid[0] = 1;
        prca->mad_tmp0[0] = (prca->PictureMAD_8p[0]<<12)/prca->ReferenceMAD_8p[0];
    }
    prca->mad_tmp1[0] = (prca->ReferenceMAD_8p[0]>>4)*(prca->ReferenceMAD_8p[0]>>4);
    prca->mad_tmp2[0] = (prca->PictureMAD_8p[0]>>4)*(prca->ReferenceMAD_8p[0]>>4);


    prca->MADPictureC1_12p = prca->PMADPictureC1_12p;
    prca->MADPictureC2_12p = prca->PMADPictureC2_12p;

    //compute the size of window
    //n_windowSize = (prca->CurrentMAD>prca->PreviousMAD)? (int)((float)(RC_MODEL_HISTORY-1) * prca->PreviousMAD/prca->CurrentMAD)
    //    :(int)((float)(RC_MODEL_HISTORY-1) * prca->CurrentMAD/prca->PreviousMAD);
    n_windowSize = (prca->CurrentMAD_8p>prca->PreviousMAD_8p)? ((20*prca->PreviousMAD_8p)/prca->CurrentMAD_8p)
        :((20*prca->CurrentMAD_8p)/prca->PreviousMAD_8p);

    n_windowSize = my_iClip3(1, (m_Nc-1), n_windowSize);
    n_windowSize = my_imin(n_windowSize, my_imin(20, prca->MADm_windowSize+1));

    //update the previous window size
    prca->MADm_windowSize=n_windowSize;


    //update the MAD for the previous frame
    //if(prca->type==P_SLICE) {//g1 || (prca->RCUpdateMode==RC_MODE_1 && (prca->gop_cnt!=0 || prca->frame_cnt!=0)))//(prca->number != 0)
    if( prca->type==P_SLICE || ((prca->RCUpdateMode==RC_MODE_1 || (prca->type==I_SLICE&&prca->RCUpdateMode==RC_MODE_3)) && (!(prca->gop_cnt==0&&prca->frame_cnt==0))) ) {//lhuitune
      if (prca->CurrentMAD_8p==0) prca->PreviousMAD_8p=1;// lhumad, make fake for dividing by zero when PreviousMAD equal to 0
      else                         prca->PreviousMAD_8p = prca->CurrentMAD_8p;
    }

    // initial MAD model estimator
    my_MADModelEstimator (view_idx, n_windowSize, n_windowSize, prca->mad_rgRejected);

    MADPictureC2_12prr4 = prca->MADPictureC2_12p>>4;
    // remove outlier
    for (i = 0; i < n_windowSize; i++)
    {
      //error[i] = prca->MADPictureC1 * prca->ReferenceMAD[i] + prca->MADPictureC2 - prca->PictureMAD[i];
      error_8p[i] = ((prca->MADPictureC1_12p*prca->ReferenceMAD_8p[i])>>12) + MADPictureC2_12prr4 - prca->PictureMAD_8p[i];
      std_16p += error_8p[i]*error_8p[i];
    }

    threshold_8p = (n_windowSize==2)? 0:my_sqrt64(std_16p/n_windowSize);

    n_realSize = n_windowSize;
    for(i=1; i<n_windowSize; i++)
    {
      if(abs(error_8p[i]) > threshold_8p)
      {
        prca->mad_rgRejected[i] = TRUE;
        n_realSize--;
      }
    }

    // second MAD model estimator
    my_MADModelEstimator(view_idx, n_realSize, n_windowSize, prca->mad_rgRejected);
  }
}


void my_MADModelEstimator(unsigned int view_idx, int n_realSize, int n_windowSize, char *mad_rgRejected)
{
  RC_DATA *prca = &rca[view_idx];
  int     i;
  long long MatrixValue_20p; // change 4p to 20p, lhu, 2017/02/23
  Boolean estimateX2=FALSE;
  unsigned int a00_20p=0,a01_20p=0,a11_20p=0,b0_8p=0,b1_8p=0; // change 8p to 20p, lhu, 2017/02/23

    // default MAD model estimation results
    prca->MADPictureC1_12p = 0;
    prca->MADPictureC2_12p = 0;
    prca->c1_over = 0;

    for(i=0;i<n_windowSize;i++) // if all non-rejected MAD are the same, take 1st order model
    {
        if(!mad_rgRejected[i])
        {
            if(prca->PictureMAD_8p[i]!=prca->PictureMAD_8p[0])
            {
                estimateX2 = TRUE;
                    break;
            }
            prca->MADPictureC1_12p += prca->mad_tmp0[i]; // ((prca->PictureMAD_8p[i]<<12) / prca->ReferenceMAD_8p[i]) /n_realSize;
            if(prca->mad_tmp0_valid[i] == 0)
                prca->c1_over = 1;
        }
    }
    if(estimateX2==FALSE)
        prca->MADPictureC1_12p = prca->MADPictureC1_12p/n_realSize;

    // take 2nd order model to estimate X1 and X2
    if(estimateX2)
    {
        a00_20p = n_realSize<<20; // change 8 to 20, lhu, 2017/02/23
        for(i=0;i<n_windowSize;i++)
        {
            if(!mad_rgRejected[i])
            {
                a01_20p += (prca->ReferenceMAD_8p[i]<<12); // change 8p to 20p, lhu, 2017/02/23
                a11_20p += (prca->mad_tmp1[i]<<12); // change 8p to 20p, lhu, 2017/02/23
                b0_8p  += prca->PictureMAD_8p[i];
                b1_8p  += prca->mad_tmp2[i]; // (prca->PictureMAD_8p[i]>>4)*(prca->ReferenceMAD_8p[i]>>4);
            }
        }
        // solve the equation of AX = B
        MatrixValue_20p = ((long long)a00_20p*(long long)a11_20p - (long long)a01_20p*(long long)a01_20p + (1<<19))>>20; // change 4p to 20p, lhu, 2017/02/23

        //if(MatrixValue_4p != 0)  //if(fabs(MatrixValue) > 0.000001)
        if(abs(MatrixValue_20p) > 1)  // change 4p to 20p, lhu, 2017/02/23
        {
            prca->MADPictureC2_12p = (((long long)b0_8p*(long long)a11_20p - (long long)b1_8p*(long long)a01_20p)<<4)/MatrixValue_20p;
            prca->MADPictureC1_12p = (((long long)b1_8p*(long long)a00_20p - (long long)b0_8p*(long long)a01_20p)<<4)/MatrixValue_20p;
        }
        else
        {
            if (a01_20p==0) {// lhumad, make fake for dividing by zero when a01_20p equal to 0
                prca->MADPictureC1_12p = ((long long)b0_8p)<<4;
                prca->cmadequ0 = 1;
            }
            else {
                prca->MADPictureC1_12p = (((long long)b0_8p)<<24)/(long long)a01_20p; // lhu, 2017/02/23
                prca->cmadequ0 = 0;
            }
            prca->MADPictureC2_12p = 0;
        }
        prca->c1_over = 0;
    }
    //if(prca->type==P_SLICE)//g1 || (prca->RCUpdateMode==RC_MODE_1 && (prca->gop_cnt!=0 || prca->frame_cnt!=0)))  //(prca->number != 0)
    if( prca->type==P_SLICE || ((prca->RCUpdateMode==RC_MODE_1 || (prca->type==I_SLICE&&prca->RCUpdateMode==RC_MODE_3)) && (!(prca->gop_cnt==0&&prca->frame_cnt==0))) ) //lhuitune
    {
        prca->PMADPictureC1_12p = prca->MADPictureC1_12p;
        prca->PMADPictureC2_12p = prca->MADPictureC2_12p;
    }
}


void my_hold( unsigned int view_idx )
{
    RC_DATA *prca = &rca[view_idx];
	
    int i;
    if(prca->rc_hold==1)
    {
        for(i=(RC_MODEL_HISTORY-2); i>0; i--)
        {// update the history
            prca->m_rgQp_8p[i] = prca->m_rgQp_8p[i-1];
            prca->m_rgRp_8p[i] = prca->m_rgRp_8p[i-1];
            prca->rc_tmp0[i] = prca->rc_tmp0[i-1];
            prca->rc_tmp1[i] = prca->rc_tmp1[i-1];
            prca->rc_tmp2[i] = prca->rc_tmp2[i-1];
            prca->rc_tmp3[i] = prca->rc_tmp3[i-1];
            prca->rc_tmp4[i] = prca->rc_tmp4[i-1];
            prca->m_rgRp_8prr8[i] = prca->m_rgRp_8prr8[i-1];
        }
        for(i=0; i<(RC_MODEL_HISTORY-1); i++)
            prca->rc_rgRejected[i] = FALSE;

        prca->rc_hold=0;
    }

    if(prca->mad_hold==1)
    {
        for(i=(RC_MODEL_HISTORY-2);i>0;i--)
        {// update the history
            prca->PictureMAD_8p[i] = prca->PictureMAD_8p[i-1];
            prca->ReferenceMAD_8p[i] = prca->ReferenceMAD_8p[i-1];
            prca->mad_tmp0[i] = prca->mad_tmp0[i-1];
            prca->mad_tmp0_valid[i] = prca->mad_tmp0_valid[i-1];
            prca->mad_tmp1[i] = prca->mad_tmp1[i-1];
            prca->mad_tmp2[i] = prca->mad_tmp2[i-1];
        }
        for(i=0; i<(RC_MODEL_HISTORY-1); i++)
            prca->mad_rgRejected[i] = FALSE;

        prca->mad_hold=0;
    }
}


//////////////////////////////////////////////////////////////////////////////////////
// \brief
//    compute a  quantization parameter for each frame
//////////////////////////////////////////////////////////////////////////////////////
int my_updateQPRC3( unsigned int view_idx )
{
  RC_DATA *prca = &rca[view_idx];
  int m_Bits;
  int SumofBasicUnit;
  int MaxQpChange, m_Qp, m_Hp;

  /* frame layer rate control */
  //if(prca->BasicUnit == prca->FrameSizeInMbs || prca->type != P_SLICE )
  if( prca->BasicUnit == prca->FrameSizeInMbs ) //lhuitune
  {
      if(prca->gop_cnt==0 && prca->frame_cnt==0) // (prca->number == 0)
      {
        prca->m_Qc = prca->MyInitialQp;
        return prca->m_Qc;
      }
      else if(prca->type==P_SLICE &&  prca->frame_cnt==0) // prca->NumberofPPicture == 0 )
      {
        prca->m_Qc = prca->MyInitialQp;
        my_updateQPNonPicAFF( view_idx );
        return prca->m_Qc;
      }
      else
      {
        prca->m_X1_8p = prca->Pm_X1_8p;
        prca->m_X2_8p = prca->Pm_X2_8p;
        prca->MADPictureC1_12p = prca->PMADPictureC1_12p;
        prca->MADPictureC2_12p = prca->PMADPictureC2_12p;
//22        prca->PreviousPictureMAD = prca->PPictureMAD[0];
            prca->PreviousPictureMAD_8p = prca->PPictureMAD_8p;

        MaxQpChange = prca->PMaxQpChange;
        m_Qp = prca->Pm_Qp;
        m_Hp = prca->PPreHeader;

        if (prca->BasicUnit < prca->FrameSizeInMbs && prca->type != P_SLICE )
        {
          // when RC_MODE_3 is set and basic unit is smaller than a frame, note that:
          // the linear MAD model and the quadratic QP model operate on small units and not on a whole frame;
          // we therefore have to account for this
            prca->PreviousPictureMAD_8p = prca->PreviousWholeFrameMAD_8p;
        }
        if (prca->type == I_SLICE )
          m_Hp = 0; // it is usually a very small portion of the total I_SLICE bit budget

        /* predict the MAD of current picture*/
//20        prca->CurrentMAD=prca->MADPictureC1*prca->PreviousPictureMAD + prca->MADPictureC2;
//30        prca->CurrentMAD_8p=(prca->MADPictureC1_12p*prca->PreviousPictureMAD_8p)/(1<<12) + prca->MADPictureC2_12p/(1<<4);
        prca->CurrentMAD_8p=(prca->MADPictureC1_12p>>8)*(prca->PreviousPictureMAD_8p>>4) + (prca->MADPictureC2_12p>>4);

        /*compute the number of bits for the texture*/
        if(prca->Target < 0)
        {
          prca->m_Qc=m_Qp+MaxQpChange;
          prca->m_Qc = my_iClip3(prca->RCMinQP, prca->RCMaxQP, prca->m_Qc); // Clipping
        }
        else
        {
          if (prca->type != P_SLICE )
          {
            if (prca->BasicUnit < prca->FrameSizeInMbs )
              m_Bits =(prca->Target-m_Hp)/prca->TotalNumberofBasicUnit;
            else
              m_Bits =prca->Target-m_Hp;
          }
          else {
            m_Bits = prca->Target-m_Hp;
            m_Bits = my_imax(m_Bits, (int)(prca->bit_rate/(MINVALUE*prca->framerate)));
          }
          my_updateModelQPFrame( view_idx, m_Bits );
          prca->m_Qc = my_iClip3(prca->RCMinQP, prca->RCMaxQP, prca->m_Qc); // clipping
          if (prca->type == P_SLICE )
            prca->m_Qc = my_iClip3(m_Qp-MaxQpChange, m_Qp+MaxQpChange, prca->m_Qc); // control variation
        }

        if(prca->type == P_SLICE)  // && prca->FieldControl == 0
          my_updateQPNonPicAFF( view_idx );

        return prca->m_Qc;
      }
  }
  //// basic unit layer rate control
  else
  {
    if(prca->gop_cnt==0 && prca->frame_cnt==0) // (prca->number == 0)
    {
      prca->m_Qc = prca->MyInitialQp;
      return prca->m_Qc;
    }
    //else if( prca->type == P_SLICE )
    else if( prca->type == P_SLICE || prca->type == I_SLICE ) //lhuitune
    {
      if(prca->gop_cnt==0 && prca->frame_cnt==1) // ((prca->NumberofGOP==1)&&(prca->NumberofPPicture==0)) // gop==0; frameP==0
      {
          return my_updateFirstP( view_idx  );
      }
      else
      {
        prca->m_X1_8p = prca->Pm_X1_8p;
        prca->m_X2_8p = prca->Pm_X2_8p;
        prca->MADPictureC1_12p=prca->PMADPictureC1_12p;
        prca->MADPictureC2_12p=prca->PMADPictureC2_12p;

        m_Qp=prca->Pm_Qp;

        SumofBasicUnit=prca->TotalNumberofBasicUnit;

        if(prca->bu_cnt==0) //(prca->NumberofBasicUnit==SumofBasicUnit)
          return my_updateFirstBU( view_idx );
        else
        {
          /*compute the number of remaining bits*/
          prca->Target -= (prca->NumberofBasicUnitHeaderBits + prca->NumberofBasicUnitTextureBits);
          prca->NumberofBasicUnitHeaderBits  = 0;
          prca->NumberofBasicUnitTextureBits = 0;
          #ifdef JM_RC_DUMP
          #ifdef USE_MY_RC
          // jm rc-related debugging info dump, lhulhu
          {
            jm_rc_info_dump = fopen("jm_rc_info_dump.txt","a+");
            fprintf(jm_rc_info_dump, "Target(BU):%-d \t", prca->Target);
            fclose (jm_rc_info_dump);
          }
          #endif
          #endif
          if(prca->Target<0)
            return my_updateNegativeTarget(view_idx, m_Qp );
          else
          {
            /*predict the MAD of current picture*/
            my_predictCurrPicMAD( view_idx );

            /*compute the total number of bits for the current basic unit*/
            my_updateModelQPBU( view_idx, m_Qp );

            prca->TotalFrameQP +=prca->m_Qc;
            prca->Pm_Qp=prca->m_Qc;
            if((prca->bu_cnt==(prca->TotalNumberofBasicUnit-1)) && prca->type==P_SLICE) // lhu, 2017/03/23
            //if((prca->bu_cnt==(prca->TotalNumberofBasicUnit-1)) && (prca->type==P_SLICE || prca->type==I_SLICE) ) //lhuitune
              my_updateLastBU( view_idx );

            return prca->m_Qc;
          }
        }
      }
    }
  }
  return prca->m_Qc;
}


void my_updateQPNonPicAFF( unsigned int view_idx )
{
    RC_DATA *prca = &rca[view_idx];
    prca->TotalQpforPPicture +=prca->m_Qc;
    prca->Pm_Qp=prca->m_Qc;
}


int my_updateFirstP( unsigned int view_idx )
{
  RC_DATA *prca = &rca[view_idx];
  //top field of the first P frame
  prca->m_Qc=prca->MyInitialQp;
  prca->NumberofBasicUnitHeaderBits=0;
  prca->NumberofBasicUnitTextureBits=0;
  //bottom field of the first P frame
  if(prca->bu_cnt==(prca->TotalNumberofBasicUnit-1)) //(prca->NumberofBasicUnit==0)
  {
    prca->TotalQpforPPicture +=prca->m_Qc;
    prca->PAveFrameQP=prca->m_Qc;
    prca->PAveHeaderBits3=prca->PAveHeaderBits2;
  }
  prca->Pm_Qp = prca->m_Qc;
  prca->TotalFrameQP += prca->m_Qc;
  return prca->m_Qc;
}


int my_updateNegativeTarget( unsigned int view_idx, int m_Qp )
{
  RC_DATA *prca = &rca[view_idx];
  int PAverageQP;

  if(prca->GOPOverdue==TRUE)
    prca->m_Qc=m_Qp+2;
  else
    prca->m_Qc=m_Qp+prca->DDquant;//2

  prca->m_Qc = my_imin(prca->m_Qc, prca->RCMaxQP);  // clipping
  if(prca->basicunit>=prca->MBPerRow) {
    if (prca->wireless_screen!=1) { // added by lhu, 2017/02/27
      if (prca->type == P_SLICE) prca->m_Qc = my_imin(prca->PAveFrameQP+6, prca->m_Qc); // change +6 to +10, lhu, 2017/01/26
      else                        prca->m_Qc = my_imin(prca->PAveFrameQP+5, prca->m_Qc); // lower QP change range for I slice, lhu, 2017/02/07
    } else {
      if (prca->type == P_SLICE) prca->m_Qc = my_imin(prca->PAveFrameQP+3, prca->m_Qc); // change +6 to +3, lhu, 2017/04/25
      else                        prca->m_Qc = my_imin(prca->PAveFrameQP+2, prca->m_Qc); // change +6 to +2 for I slice, lhu, 2017/04/25
    }
  } else
    prca->m_Qc = my_imin(prca->m_Qc, prca->PAveFrameQP+3);

  prca->TotalFrameQP +=prca->m_Qc;
  if(prca->bu_cnt==(prca->TotalNumberofBasicUnit-1)) //(prca->NumberofBasicUnit==0)
  {
//18    PAverageQP=(int)((double)prca->TotalFrameQP/(double)prca->TotalNumberofBasicUnit+0.5);
    PAverageQP=(prca->TotalFrameQP+(prca->TotalNumberofBasicUnit>>1))/prca->TotalNumberofBasicUnit;
    if(prca->frame_cnt==(prca->intra_period-1)) //(prca->NumberofPPicture == (prca->intra_period - 2))
      prca->QPLastPFrame = PAverageQP;
    if (prca->type == P_SLICE) // not increase TotalQpforPPicture for I_SLICE, lhuitune
      prca->TotalQpforPPicture +=PAverageQP;
    prca->PAveFrameQP=PAverageQP;
    prca->PAveHeaderBits3=prca->PAveHeaderBits2;
  }
  if(prca->GOPOverdue==TRUE)
    prca->Pm_Qp=prca->PAveFrameQP;
  else
    prca->Pm_Qp=prca->m_Qc;

  return prca->m_Qc;
}


int my_updateFirstBU( unsigned int view_idx )
{
  RC_DATA *prca = &rca[view_idx];
  if(prca->frame_cnt==1) prca->PAveFrameQP = prca->QPLastPFrame; // first P frame's initial QP value equals to LastPFrame's average QP, lhu, 2017/03/23
  else                    prca->PAveFrameQP = prca->PAveFrameQP;
  if(prca->Target<=0)
  {
    prca->m_Qc = prca->PAveFrameQP + 2;
    if(prca->m_Qc > prca->RCMaxQP)
      prca->m_Qc = prca->RCMaxQP;

    prca->GOPOverdue=TRUE;
  }
  else
  {
    prca->m_Qc=prca->PAveFrameQP;
  }
  prca->TotalFrameQP +=prca->m_Qc;
  prca->Pm_Qp = prca->PAveFrameQP;

  return prca->m_Qc;
}


void my_updateLastBU( unsigned int view_idx )
{
  RC_DATA *prca = &rca[view_idx];
  int PAverageQP;

//18  PAverageQP=(int)((double)prca->TotalFrameQP/(double)prca->TotalNumberofBasicUnit+0.5);
  PAverageQP=(prca->TotalFrameQP+(prca->TotalNumberofBasicUnit>>1))/prca->TotalNumberofBasicUnit;
  if(prca->frame_cnt==(prca->intra_period-1)) // (prca->NumberofPPicture == (prca->intra_period - 2))  last P_FRAME in gop
    prca->QPLastPFrame = PAverageQP;
  if (prca->type == P_SLICE) // not increase TotalQpforPPicture for I_SLICE, lhuitune
    prca->TotalQpforPPicture +=PAverageQP;
  prca->PAveFrameQP=PAverageQP;
  prca->PAveHeaderBits3=prca->PAveHeaderBits2;
}


void my_updateModelQPFrame( unsigned int view_idx, int m_Bits )
{
  RC_DATA *prca = &rca[view_idx];
  long long dtmp_8p, qstep_tmp;
  int tmp_4p=0;
  int m_Qstep_8p;

  //dtmp_8p = (prca->CurrentMAD_8p>>6)*(prca->m_X1_8p>>6)*(prca->CurrentMAD_8p>>6)*(prca->m_X1_8p>>6) + \
  //    4*(prca->m_X2_8p>>4)*(prca->CurrentMAD_8p>>4)*m_Bits;
  dtmp_8p = ((long long)prca->CurrentMAD_8p>>6)*((long long)prca->CurrentMAD_8p>>6)*((long long)prca->m_X1_8p>>6)*((long long)prca->m_X1_8p>>6) + \
      4*((long long)prca->m_X2_8p>>4)*((long long)prca->CurrentMAD_8p>>4)*m_Bits;

  if(dtmp_8p>0)
      tmp_4p = my_sqrt64(dtmp_8p);

  if((prca->m_X2_8p==0) || (dtmp_8p<0) || ((tmp_4p-((prca->m_X1_8p>>6)*(prca->CurrentMAD_8p>>6)))<=0))
  {
    //m_Qstep = (float)((prca->m_X1*prca->CurrentMAD) / (double) m_Bits);
    m_Qstep_8p = ((prca->m_X1_8p>>4)*(prca->CurrentMAD_8p>>4)) / m_Bits;
  }
  else // 2nd order mode
  {
    //m_Qstep = (float)((2*prca->m_X2_8p*prca->CurrentMAD_8p)/(sqrt(dtmp)*(1<<16) - prca->m_X1_8p*prca->CurrentMAD_8p));
    qstep_tmp = (2*((long long)prca->m_X2_8p)*((long long)prca->CurrentMAD_8p)) / ((tmp_4p<<4) - (prca->m_X1_8p>>4)*(prca->CurrentMAD_8p>>4));
    m_Qstep_8p = qstep_tmp;
  }

  prca->m_Qc = Qstep2QP_8p(m_Qstep_8p);
}


void my_predictCurrPicMAD( unsigned int view_idx )
{
    int i,CurrentBUMAD_8p,MADPictureC1_12prr4,MADPictureC2_12prr4;
	RC_DATA *prca = &rca[view_idx];

    MADPictureC1_12prr4 = prca->MADPictureC1_12p>>4;
    MADPictureC2_12prr4 = prca->MADPictureC2_12p>>4;

    //prca->CurrentMAD=prca->MADPictureC1*prca->BUPFMAD[prca->bu_cnt]+prca->MADPictureC2;
    prca->CurrentMAD_8p=(MADPictureC1_12prr4*(prca->BUPFMAD_8p[prca->bu_cnt]>>8)) + MADPictureC2_12prr4;
    prca->TotalBUMAD_12p=0;

    for(i=prca->TotalNumberofBasicUnit-1; i>=prca->bu_cnt; i--)
    {
        //CurrentBUMAD = prca->MADPictureC1*prca->BUPFMAD[i]+prca->MADPictureC2;
        CurrentBUMAD_8p = (MADPictureC1_12prr4*(prca->BUPFMAD_8p[i]>>8)) + MADPictureC2_12prr4;
        prca->TotalBUMAD_12p += (CurrentBUMAD_8p*CurrentBUMAD_8p)>>4;
    }
}


void my_updateModelQPBU( unsigned int view_idx, int m_Qp )
{
  RC_DATA *prca = &rca[view_idx];
  int m_Bits;
  long long dtmp_8p,qstep_tmp;
  int tmp_4p=0;
  int m_Qstep_8p;

  //compute the total number of bits for the current basic unit
  //m_Bits =(int)(prca->Target * prca->CurrentMAD * prca->CurrentMAD / prca->TotalBUMAD);
  if((prca->TotalBUMAD_12p>>8) == 0)
    m_Bits = prca->Target;
  else
    m_Bits =(prca->Target*(prca->CurrentMAD_8p>>6)*(prca->CurrentMAD_8p>>6)) / (prca->TotalBUMAD_12p>>8);

  //compute the number of texture bits
  m_Bits -=prca->PAveHeaderBits2;

  m_Bits=my_imax(m_Bits,((prca->bit_rate/prca->framerate)/(MINVALUE*prca->TotalNumberofBasicUnit)));

  //dtmp = prca->CurrentMAD*prca->CurrentMAD*prca->m_X1*prca->m_X1 + 4*prca->m_X2*prca->CurrentMAD*m_Bits;
  dtmp_8p = ((long long)prca->CurrentMAD_8p>>6)*((long long)prca->CurrentMAD_8p>>6)*((long long)prca->m_X1_8p>>6)*((long long)prca->m_X1_8p>>6) + \
      4*((long long)prca->m_X2_8p>>4)*((long long)prca->CurrentMAD_8p>>4)*m_Bits;

  if(dtmp_8p>0)
    tmp_4p = my_sqrt64(dtmp_8p);

  //if((prca->m_X2==0) || (dtmp<0) || ((sqrt(dtmp)-(prca->m_X1*prca->CurrentMAD))<=0))  // fall back 1st order mode
  if((prca->m_X2_8p==0) || (dtmp_8p<0) || ((tmp_4p-((prca->m_X1_8p>>6)*(prca->CurrentMAD_8p>>6)))<=0))
  {
    //m_Qstep = (float)((prca->m_X1*prca->CurrentMAD) / (double) m_Bits);
    m_Qstep_8p = ((prca->m_X1_8p>>4)*(prca->CurrentMAD_8p>>4)) / m_Bits;
  }
  else // 2nd order mode
  {
      //m_Qstep = (float)((2*prca->m_X2_8p*prca->CurrentMAD_8p)/(sqrt(dtmp)*(1<<16) - prca->m_X1_8p*prca->CurrentMAD_8p));
      qstep_tmp = (2*((long long)prca->m_X2_8p)*((long long)prca->CurrentMAD_8p)) / ((tmp_4p<<4) - (prca->m_X1_8p>>4)*(prca->CurrentMAD_8p>>4));
      m_Qstep_8p = qstep_tmp;
  }

  prca->m_Qc = Qstep2QP_8p(m_Qstep_8p);
  //use the Qc by R-D model when non-wireless-screen application, lhu, 2017/02/27
  if (prca->wireless_screen==1) // added by lhu, 2017/02/27
    prca->m_Qc = my_imin(m_Qp+prca->DDquant,  prca->m_Qc); // control variation
  
  if(prca->basicunit>=prca->MBPerRow) {
    if (prca->wireless_screen!=1) { // added by lhu, 2017/02/27
      if (prca->type == P_SLICE) prca->m_Qc = my_imin(prca->PAveFrameQP+6, prca->m_Qc); // change +6 to +10, lhu, 2017/01/24
      else                        prca->m_Qc = my_imin(prca->PAveFrameQP+5, prca->m_Qc); // lower QP change range for I slice, lhu, 2017/02/07
    } else {
      if (prca->type == P_SLICE) prca->m_Qc = my_imin(prca->PAveFrameQP+3, prca->m_Qc); // change +6 to +3, lhu, 2017/04/25
      else {
			// Expand QP change range when decide to insert I Frame, lhu, 2017/05/26
			if (prca->IFduration==1 && prca->insertOneIFrame==1) prca->m_Qc = my_imin(prca->PAveFrameQP+6, prca->m_Qc);
			else												 prca->m_Qc = my_imin(prca->PAveFrameQP+2, prca->m_Qc); // change +6 to +2 for I slice, lhu, 2017/04/25
	  }
    }
  } else
    prca->m_Qc = my_imin(prca->PAveFrameQP+3, prca->m_Qc);

  /*if(prca->c1_over==1)
    //prca->m_Qc = my_imin(m_Qp-prca->DDquant, prca->RCMaxQP); // clipping
    prca->m_Qc = my_imin(m_Qp+prca->DDquant, prca->RCMaxQP-10); // not letting QP decrease when MAD equal 0, 2017/02/21
  else*/
    prca->m_Qc = my_iClip3(m_Qp-prca->DDquant, prca->RCMaxQP, prca->m_Qc); // clipping

  if(prca->basicunit>=prca->MBPerRow) {
    if (prca->wireless_screen!=1) { // added by lhu, 2017/04/18
      if (prca->type == P_SLICE) prca->m_Qc = my_imax(prca->PAveFrameQP-6, prca->m_Qc); // lhu, 2017/04/18
      else                        prca->m_Qc = my_imax(prca->PAveFrameQP-5, prca->m_Qc); // lhu, 2017/04/18
    } else {
      if (prca->type == P_SLICE) prca->m_Qc = my_imax(prca->PAveFrameQP-3, prca->m_Qc); // lhu, 2017/04/25
      else {
        // Expand QP change range when decide to insert I Frame, lhu, 2017/05/26
        if (prca->IFduration==1 && prca->insertOneIFrame==1) prca->m_Qc = my_imax(prca->PAveFrameQP-6, prca->m_Qc);
      	else                                                 prca->m_Qc = my_imax(prca->PAveFrameQP-2, prca->m_Qc); // lhu, 2017/04/25
      }
    }
  } else
    prca->m_Qc = my_imax(prca->PAveFrameQP-3, prca->m_Qc);

  prca->m_Qc = my_imax(prca->RCMinQP, prca->m_Qc);
}


void my_rc_update_bu_stats( unsigned int view_idx ) {
    RC_DATA *prca = &rca[view_idx];
    
    prca->NumberofHeaderBits  = prca->frame_hbits;
    prca->NumberofTextureBits = prca->frame_tbits;
    // basic unit layer rate control
    if(prca->BasicUnit < prca->FrameSizeInMbs) {
        prca->NumberofBasicUnitHeaderBits  = prca->hbits_tmp;  // add slice_header
        prca->NumberofBasicUnitTextureBits = prca->tbits_tmp;
    }
}
void my_rc_update_frame_stats( unsigned int view_idx ) {
	RC_DATA *prca = &rca[view_idx];
    prca->frame_mad   += prca->mad_tmp;
    prca->frame_tbits += prca->tbits_tmp;
    prca->frame_hbits += prca->hbits_tmp;
    prca->frame_abits = prca->frame_tbits+prca->frame_hbits;
    if(prca->bu_cnt==0) { //after calculate frame's status reset related status to zero, lhu, 2017/03/06
        prca->frame_mad   = 0;
        prca->frame_tbits = 0;
        prca->frame_hbits = 0;
        prca->frame_abits = 0;
    }
}



void my_rc_init_gop_params( unsigned int view_idx )
{
	RC_DATA *prca = &rca[view_idx];
    if(prca->RCUpdateMode==RC_MODE_1)
    {
        //if((rca.gop_cnt==0 && rca.frame_cnt==0) || ((rca.gop_cnt*rca.intra_period)==rca.no_frm_base))
        if(prca->frame_cnt==0 && prca->bu_cnt==0)
            my_rc_init_GOP( view_idx,  prca->no_frm_base - 1 );
    }
    else if((prca->RCUpdateMode==RC_MODE_0)|(prca->RCUpdateMode==RC_MODE_2)|(prca->RCUpdateMode==RC_MODE_3))
    {
        if(prca->frame_cnt==0 && prca->bu_cnt==0) {
            if (prca->IFduration==1 && prca->insertOneIFrame==1) {
                prca->intra_period = prca->PrevIntraPeriod; // use previous intra_period to calculate GOP TargetBits, lhu, 2017/03/13
            }
            my_rc_init_GOP( view_idx, prca->intra_period - 1 );
        }
    }
}


int my_rc_handle_mb( unsigned int view_idx )
{
    CHECK_VIEW_IDX( view_idx );
    
    RC_DATA *prca = &rca[view_idx];
    //// first update, update MB_state
    if(prca->gop_cnt!=0 || prca->frame_cnt!=0 || prca->bu_cnt!=0)// || rca.mb_cnt!=0
    {
        my_rc_update_bu_stats( view_idx ); 
        prca->TotalMADBasicUnit = prca->mad_tmp;
        prca->TotalFrameMAD    += prca->mad_tmp;// lhumod
    }


    if((prca->gop_cnt>0 || prca->frame_cnt>0) && prca->bu_cnt==0) {// && rca.mb_cnt==0))
        prca->frame_mad = prca->TotalFrameMAD/prca->FrameSizeInMbs;// lhumod, calculate the average MB's mad value of previous encoded frame.
        my_rc_update_pict( view_idx, prca->fbits_tmp );  // should put it to the frame-last
    }

    //// initial sequence (only once)
    if( prca->bu_cnt == 0 ) // first bu
    {
        if( prca->frame_cnt == 0 ) // first frame
        {
            prca->type = I_SLICE;
            if( prca->gop_cnt == 0 ) // first gop, sequence need set parameters , @jlliu
            {
                my_rc_params    ( view_idx );
                my_rc_init_seq  ( view_idx ); //// initial seq params
            }
            
            my_rc_init_gop_params   ( view_idx );
        }
        else {
            prca->type = P_SLICE;
        }
        
        my_rc_init_pict(view_idx, 1);
        prca->qp        = my_updateQP( view_idx );
        prca->slice_qp  = prca->qp;
    }

    
    // frame layer rate control //// BU-Level
    if (prca->basicunit < prca->FrameSizeInMbs)
    {
        // each I or B frame has only one QP
        //if(prca->type==I_SLICE)//g1 && rca.RCUpdateMode!=RC_MODE_1) || (rca.gop_cnt==0 && rca.frame_cnt==0)) //!(rca.number)
        if(prca->gop_cnt==0 && prca->frame_cnt==0) //lhuitune
        {
            prca->qp = prca->MyInitialQp;
        }
        //else if (prca->type == P_SLICE) //g1 || rca.RCUpdateMode == RC_MODE_1 )
        else if (prca->type == P_SLICE || prca->RCUpdateMode==RC_MODE_1 || (prca->type==I_SLICE && prca->RCUpdateMode==RC_MODE_3)) //lhuitune
        {
            // compute the quantization parameter for each basic unit of P frame
            if(prca->bu_cnt!=0) // && rca.mb_cnt==0)
            {
              my_updateRCModel( view_idx );
              prca->qp = my_updateQP( view_idx );
            }
        }
    }

    prca->qp = my_iClip3(prca->RCMinQP, prca->RCMaxQP, prca->qp); // -rca.bitdepth_luma_qp_scale

    my_rc_update_frame_stats( view_idx ); // computer frame parameters
    if( prca->bu_cnt==(prca->TotalNumberofBasicUnit-1) ) {
        if (prca->changeToIFrame==1 || prca->insertOneIFrame==1) {
            if (prca->type==P_SLICE)  my_criteria_decide_changeToIFrame((prca->ymseh_tmp>>16)&0xff,(prca->ymseh_tmp>>8)&0xff,(prca->ymseh_tmp>>5)&0x7,view_idx); // lhu, 2017/03/24
            else                      my_decide_backtoNormalGOP(view_idx); // lhu, 2017/03/24
        } else { // lhu, 2017/04/10
            prca->gop_change_NotResetRC=0; prca->IFduration=0;
        }
        prca->frm_ymse[1]  = prca->frm_ymse[0]; // lhu, 2017/03/27
        // renew the QPLastPFrame after intra_period updated, lhu, 2017/03/28
        if ( (prca->type==P_SLICE) && (prca->frame_cnt>=(prca->intra_period-1)) ) {
            prca->QPLastPFrame = (prca->TotalFrameQP+(prca->TotalNumberofBasicUnit>>1))/prca->TotalNumberofBasicUnit;
        }
    }

    if(prca->basicunit < prca->FrameSizeInMbs) // bu-level counter
    {
        if(prca->bu_cnt==(prca->TotalNumberofBasicUnit-1))
        {
            prca->bu_cnt=0;
            if(prca->frame_cnt>=(prca->intra_period-1)) // change "==" to ">=", lhu, 2017/03/09
            {
                prca->frame_cnt=0;
                //if(prca->gop_cnt<=1000)
                prca->gop_cnt++;
            }
            else
                prca->frame_cnt++;
        }
        else
            prca->bu_cnt++;
    }
    else // frame-level counter
    {
        if(prca->frame_cnt==(prca->intra_period-1))
        {
            prca->frame_cnt=0;
            //if(prca->gop_cnt<=1000)
            prca->gop_cnt++;
        }
        else
            prca->frame_cnt++;
    }

#ifndef ARMCM7_RC //#############
    my_hold( );
#endif //#############


    return prca->qp;
}
