/**
  ******************************************************************************
  * @file    stm32f746xx.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    25-June-2015
  * @brief   CMSIS STM32F746xx Device Peripheral Access Layer Header File.
  *
  *          This file contains:
  *           - Data structures and the address mapping for all peripherals
  *           - Peripheral's registers declarations and bits definition
  *           - Macros to access peripheral’s registers hardware
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/** @addtogroup CMSIS_Device
  * @{
  */

/** @addtogroup stm32f746xx
  * @{
  */
    
#ifndef __STM32F746xx_H
#define __STM32F746xx_H

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */
  

/**
  * @}
  */
typedef enum IRQn
{
    NonMaskableInt_IRQn         = -14,    /*!< 2 Non Maskable Interrupt */
    MemoryManagement_IRQn       = -12,    /*!< 4 Cortex-M7 Memory Management Interrupt */
    BusFault_IRQn               = -11,    /*!< 5 Cortex-M7 Bus Fault Interrupt */
    UsageFault_IRQn             = -10,    /*!< 6 Cortex-M7 Usage Fault Interrupt */
    SVCall_IRQn                 = -5,     /*!< 11 Cortex-M7 SV Call Interrupt */
    DebugMonitor_IRQn           = -4,     /*!< 12 Cortex-M7 Debug Monitor Interrupt */
    PendSV_IRQn                 = -2,     /*!< 14 Cortex-M7 Pend SV Interrupt */
    SysTick_IRQn                = -1,     /*!< 15 Cortex-M7 System Tick Interrupt */
    OTG0_HS_IRQn                = 56,
    OTG1_HS_IRQn                = 57,  
} IRQn_Type;


/**
 * @brief Configuration of the Cortex-M7 Processor and Core Peripherals 
 */
#define __CM7_REV                 0x0000   /*!< Cortex-M7 revision r0p1                       */
#define __MPU_PRESENT             1       /*!< CM7 provides an MPU                           */
#define __NVIC_PRIO_BITS          4       /*!< CM7 uses 4 Bits for the Priority Levels       */
#define __Vendor_SysTickConfig    0       /*!< Set to 1 if different SysTick Config is used  */
#define __FPU_PRESENT             1       /*!< FPU present                                   */
#define __ICACHE_PRESENT          1       /*!< CM7 instruction cache present                 */
#define __DCACHE_PRESENT          1       /*!< CM7 data cache present                        */
#include "core_cm7.h"                 /*!< Cortex-M7 processor and core peripherals      */
#include <stdint.h>




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __STM32F746xx_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
