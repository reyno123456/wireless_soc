/**
  ******************************************************************************
  * @file      start.s
  * @author    Min Zhao
  * @Version    V1.1.0
  * @Date       14-September-2016
  ******************************************************************************
  */
#include "debuglog.h"
#include "interrupt.h"

.equ  DTCM0, 0x20000000
.equ  Local_CPUID, 0x0

.syntax unified
.cpu cortex-m7
.fpu softvfp
.thumb

.global  vectors
.global  Default_Handler

/* defined in linker script */
/* start address for the .data/rodata/bss section. */
.word  _data_start
/* end address for the .data/rodata/bss section. */
.word  _data_end

.word  _bss_start
.word  _bss_end

/**
 * @brief  This is the code that gets called when the processor first
 *          starts execution following a reset event. Only the absolutely
 *          necessary set is performed, after which the application
 *          supplied main() routine is called. 
 * @param  None
 * @retval : None
*/
  .section  .start.Reset_Handler
  .weak  Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:  
  ldr  sp,  =_estack             /* set stack pointer */

/* copy the BOOT data to DTCM0 */
  movs r1, #0
  ldr  r4, =_data_start           /* The start addr of data image */
  b  LoopCopyData
CopyInitData:
  ldr  r3, =_data_start
  ldr  r3, [r3, r1]
  str  r3, [r0, r1]
  adds  r1, r1, #4
LoopCopyData:
  ldr  r0, =DTCM0                 /* The start addr of Dtcm0 */
  ldr  r3, =_data_end             /* The end addr of data image */
  adds  r2, r4, r1
  cmp  r2, r3
  bcc  CopyInitData

  /* clear the bss section */
  ldr r0, =0x0
  ldr r1, =_bss_start
  ldr r2, =_bss_end
LoopClearBss:
  str r0, [r1]
  add r1, r1, #4
  cmp r1, r2
  bcc LoopClearBss

/* branch to main */
  bl main

/**
 * @brief  This is the code that gets called when the processor receives an 
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 * @param  None     
 * @retval None       
*/
  .section .start.Default_Handler, "ax", %progbits
Default_Handler:
Infinite_Loop:
  b  Infinite_Loop
  .size  Default_Handler, .-Default_Handler

/******************************************************************************
*
* The minimal vector table for a Cortex M3. Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
* 
*******************************************************************************/
  .section  .isr_vectors, "a", %progbits
  .type   vectors, %object
  .size   vectors, .-vectors
   
  vectors:
  .word     _estack
  .word     Reset_Handler
  .word     default_isr
  .word     hardfault_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     SVC_Handler
  .word     default_isr
  .word     default_isr
  .word     PendSV_Handler
  .word     SYSTICK_IRQHandler
  .word     IRQHandler_16
  .word     IRQHandler_17
  .word     IRQHandler_18
  .word     IRQHandler_19
  .word     IRQHandler_20
  .word     IRQHandler_21
  .word     IRQHandler_22
  .word     IRQHandler_23
  .word     IRQHandler_24
  .word     IRQHandler_25
  .word     IRQHandler_26
  .word     IRQHandler_27
  .word     IRQHandler_28
  .word     IRQHandler_29
  .word     IRQHandler_30
  .word     IRQHandler_31
  .word     IRQHandler_32
  .word     IRQHandler_33
  .word     IRQHandler_34
  .word     IRQHandler_35
  .word     IRQHandler_36
  .word     IRQHandler_37
  .word     IRQHandler_38
  .word     IRQHandler_39
  .word     IRQHandler_40
  .word     IRQHandler_41
  .word     IRQHandler_42
  .word     IRQHandler_43
  .word     IRQHandler_44
  .word     IRQHandler_45
  .word     IRQHandler_46
  .word     IRQHandler_47
  .word     IRQHandler_48
  .word     IRQHandler_49
  .word     IRQHandler_50
  .word     IRQHandler_51
  .word     IRQHandler_52
  .word     IRQHandler_53
  .word     IRQHandler_54
  .word     IRQHandler_55
  .word     IRQHandler_56
  .word     IRQHandler_57
  .word     IRQHandler_58
  .word     IRQHandler_59
  .word     IRQHandler_60
  .word     IRQHandler_61
  .word     IRQHandler_62
  .word     IRQHandler_63
  .word     IRQHandler_64
  .word     IRQHandler_65
  .word     IRQHandler_66
  .word     IRQHandler_67
  .word     IRQHandler_68
  .word     IRQHandler_69
  .word     IRQHandler_70
  .word     IRQHandler_71
  .word     IRQHandler_72
  .word     IRQHandler_73
  .word     IRQHandler_74
  .word     IRQHandler_75
  .word     IRQHandler_76
  .word     IRQHandler_77
  .word     IRQHandler_78
  .word     IRQHandler_79
  .word     IRQHandler_80
  .word     IRQHandler_81
  .word     IRQHandler_82
  .word     IRQHandler_83
  .word     IRQHandler_84
  .word     IRQHandler_85
  .word     IRQHandler_86
  .word     IRQHandler_87
  .word     IRQHandler_88
  .word     IRQHandler_89
  .word     IRQHandler_90
  .word     IRQHandler_91
  .word     IRQHandler_92
  .word     IRQHandler_93
  .word     IRQHandler_94
  .word     IRQHandler_95
  .word     IRQHandler_96
  .word     IRQHandler_97
  .word     IRQHandler_98
  .word     Local_CPUID
  
/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler. 
* As they are weak aliases, any function with the same name will override 
* this definition.
* 
*******************************************************************************/
   .weak      default_isr
   .thumb_set default_isr,Default_Handler

   .weak      hardfault_isr
   .thumb_set hardfault_isr,Default_Handler

   .weak      SVC_Handler
   .thumb_set SVC_Handler,Default_Handler

   .weak      PendSV_Handler
   .thumb_set PendSV_Handler,Default_Handler

   .weak      SYSTICK_IRQHandler
   .thumb_set SYSTICK_IRQHandler,Default_Handler


