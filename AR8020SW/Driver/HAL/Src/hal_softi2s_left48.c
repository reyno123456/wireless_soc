/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_softi2s_leftinterrupt.c
Description: audio left interrupt
NOTE: this file don't use -O1 to complie
Author: Artosy Software Team
Version: 0.0.1
Date: 2017/02/21
History:
         0.0.1    2017/02/21    The initial version of hal_softi2s_leftinterrupt.c
*****************************************************************************/

#include <stdint.h>
#include "hal_softi2s.h"
#include "debuglog.h"
#include "pll_ctrl.h"
         
extern volatile uint16_t g_u16_audioDataArray[ADUIO_DATA_BUFF_LENGHT];
extern volatile uint32_t g_u32_audioDataConut;
extern volatile uint32_t g_u32_audioDataReady;
extern volatile uint8_t g_u8_audioDataOffset;
extern volatile uint32_t g_u32_audioDataAddr;
extern volatile uint32_t g_u32_audioLeftInterruptAddr;


#if (CPU0_CPU1_CORE_PLL_CLK == 200) 
void LeftAudio_48K(void) 
{      
    __asm volatile (
    "cpsid i \n" /* Errata workaround. */ 
    "stmdb sp!, {r4-r11} \n"       
    "ldr  r0, =g_u32_audioLeftInterruptAddr\n"
    "ldr  r1, [r0]\n" //clear interrupt
    "mov  r3, #0xff\n"
    "strb r3, [r1]\n" 

    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "mov r3, #0\n"
    "ldr  r0, =g_u32_audioDataAddr\n" //read audio date
    "ldr  r5, [r0]\n"
    "ldr  r0, =g_u8_audioDataOffset\n"
    "ldr  r6, [r0]\n"     

    //sample1
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;\n"
    "mov  r4, r4\n"
    "mov  r4, #15\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample2 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"      
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;\n"
    "mov  r4, r4\n"          
    "mov  r4, #14\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample3
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"      
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "mov  r4, r4\n" 
    "mov  r4, #13\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample4
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"      
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;\n"
    "mov  r4, r4\n"     
    "mov  r4, #12\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample5
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"      
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;\n"
    "mov  r4, r4\n"       
    "mov  r4, #11\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample6
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"      
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;\n"
    "mov  r4, r4\n"
    "mov  r4, #10\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample7  
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"      
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "mov  r4, r4\n"
    "mov  r4, #9\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample8
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"      
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;\n"
    "mov  r4, r4\n"
    "mov  r4, #8\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample9
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"      
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "mov  r4, r4\n"
    "mov  r4, #7\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample10
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"     
    "NOP;NOP;NOP;NOP;NOP;NOP;\n"   
    "mov  r4, r4\n"   
    "mov  r4, #6\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample11
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"      
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;\n"
    "mov  r4, r4\n"
    "mov  r4, #5\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample12
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"      
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;\n"      
    "mov  r4, r4\n"
    "mov  r4, #4\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample13
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"      
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;\n"    
    "mov  r4, r4\n"  
    "mov  r4, #3\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample14
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"      
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;\n"      
    "mov  r4, r4\n"
    "mov  r4, #2\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample15
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"      
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;\n"  
    "mov  r4, r4\n"    
    "mov  r4, #1\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample16
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"      
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;\n"  
    "mov  r4, r4\n"    
    "mov  r4, #0\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    "ldr  r5, =g_u32_audioDataConut\n"  
    "ldr  r4, [r5]\n"
    "lsl  r2, r4, #1\n"//base=RightArray offset=RightDataConut*2
    "ldr  r6, =g_u16_audioDataArray\n" 
    "add  r6, r2\n"      
    "str  r3, [r6]\n"
    "add  r4, #1\n"
    "cmp  r4, #0x900\n"
    "bne  CountGpioInterruptLeftZero48\n"
    "mov  r4, #0\n"
    "ldr  r0, =g_u32_audioDataReady\n"    
    "str  r4, [r0]\n"
    "CountGpioInterruptLeftZero48:\n"
    "str  r4, [r5]\n"
    "ldmia sp!, {r4-r11} \n"
    "cpsie i\n" /* Errata workaround. */
    );
    //dlog_info("%d %x \n",g_u32_audioDataReady,g_u32_audioDataConut);
}
#endif /*CPU0_CPU1_CORE_PLL_CLK*/

#if (CPU0_CPU1_CORE_PLL_CLK == 100)
void LeftAudio_48K(void) 
{      
    __asm volatile (
    "cpsid i \n" /* Errata workaround. */ 
    "stmdb sp!, {r4-r11} \n"       
    "ldr  r0, =g_u32_audioLeftInterruptAddr\n"
    "ldr  r1, [r0]\n" //clear interrupt
    "mov  r3, #0xff\n"
    "strb r3, [r1]\n" 

    "mov r3, #0\n"
    "ldr  r0, =g_u32_audioDataAddr\n" //read audio date
    "ldr  r5, [r0]\n"
    "ldr  r0, =g_u8_audioDataOffset\n"
    "ldr  r6, [r0]\n"     

    //sample1
    "NOP;NOP;NOP;NOP;\n"    
    "mov  r4, r4\n"
    "mov  r4, #15\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"
    
    //sample2 
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "mov  r4, r4\n"          
    "mov  r4, #14\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample3
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "mov  r4, r4\n" 
    "mov  r4, #13\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample4     
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n" 
    "NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "mov  r4, r4\n"     
    "mov  r4, #12\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample5
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "mov  r4, r4\n"       
    "mov  r4, #11\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample6
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "mov  r4, r4\n"
    "mov  r4, #10\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample7  
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "mov  r4, r4\n"
    "mov  r4, #9\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample8
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "mov  r4, r4\n"
    "mov  r4, #8\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample9
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;\n"
    "mov  r4, r4\n"
    "mov  r4, #7\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample10
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"     
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"   
    "mov  r4, r4\n"   
    "mov  r4, #6\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample11
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "mov  r4, r4\n"
    "mov  r4, #5\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample12
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;\n"      
    "mov  r4, r4\n"
    "mov  r4, #4\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample13
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"    
    "mov  r4, r4\n"  
    "mov  r4, #3\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample14
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"      
    "mov  r4, r4\n"
    "mov  r4, #2\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample15
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "mov  r4, r4\n"    
    "mov  r4, #1\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    //sample16
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;\n"
    "NOP;NOP;\n"  
    "mov  r4, r4\n"    
    "mov  r4, #0\n"
    "ldr  r1, [r5]\n"
    "lsr  r1, r6\n"   //r2=r1>>4 gpio100
    "lsl  r1, r4\n" //r2=r2<<r4
    "orr  r3, r1\n"

    "ldr  r5, =g_u32_audioDataConut\n"  
    "ldr  r4, [r5]\n"
    "lsl  r2, r4, #1\n"//base=RightArray offset=RightDataConut*2
    "ldr  r6, =g_u16_audioDataArray\n" 
    "add  r6, r2\n"      
    "str  r3, [r6]\n"
    "add  r4, #1\n"
    "cmp  r4, #0x900\n"
    "bne  CountGpioInterruptLeftZero44\n"
    "mov  r4, #0\n"
    "ldr  r0, =g_u32_audioDataReady\n"    
    "str  r4, [r0]\n"
    "CountGpioInterruptLeftZero44:\n"
    "str  r4, [r5]\n"
    "ldmia sp!, {r4-r11} \n"
    "cpsie i\n" /* Errata workaround. */
    );
    //dlog_info("%d %x \n",g_u32_audioDataReady,g_u32_audioDataConut);
}
#endif /*CPU0_CPU1_CORE_PLL_CLK*/
