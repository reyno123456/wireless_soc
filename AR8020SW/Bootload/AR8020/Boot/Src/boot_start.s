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
  /* disable write flash command */
  ldr  r0, =0x40C00000
  movs  r1,  #0
  ldr  r1, [r0, #0x1DC]

/* copy the data to DTCM0 */
  ldr r0, =_data_start
  ldr r1, =_data_end
  ldr r2, =DTCM0
  loop_mov:
	  cmp r0,r1
	  beq loop_end
	  ldr r3, [r0]
      str r3, [r2]
      add r0, #4
      add r2, #4
      b loop_mov
loop_end:

/* clear bss */
     ldr r0, =_bss_start
     ldr r1, =_bss_end
     mov r2, #0

loop_bss:
     cmp r0, r1
     beq loop_bss_end
     str r2, [r0]
     add r0, #4
     b loop_bss
loop_bss_end:

/* branc0 to main */
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
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  # .word     default_isr
  
/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler. 
* As they are weak aliases, any function with the same name will override 
* this definition.
* 
*******************************************************************************/
   .weak      default_isr
   .thumb_set default_isr,Default_Handler


