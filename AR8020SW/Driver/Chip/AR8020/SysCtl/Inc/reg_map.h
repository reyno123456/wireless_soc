#ifndef REG_MAP_H
#define REG_MAP_H

#define   __I     volatile const       /*!< Defines 'read only' permissions                 */
#define   __O     volatile             /*!< Defines 'write only' permissions                */
#define   __IO    volatile             /*!< Defines 'read / write' permissions              */

/*==========================================================*/
/*     ARM_M7 Interrupt Vector Enable  Declaration          */
/*==========================================================*/
#define NVIC_ISER_BASE 0xE000E100
typedef struct
{
   unsigned int ISER0; // 0x00
   unsigned int ISER1; // 0x04
   unsigned int ISER2; // 0x08
   unsigned int ISER3; // 0x0c
   unsigned int ISER4; // 0x10
   unsigned int ISER5; // 0x14
   unsigned int ISER6; // 0x18
   unsigned int ISER7; // 0x1c
} NVIC_ISER_type;
#define NVIC_ISER ((NVIC_ISER_type *) NVIC_ISER_BASE)

#define NVIC_CTRL_BASE 0xE000E100
typedef struct
{
  __IO uint32_t ISER[8];                 /*!< Offset: 0x000 (R/W)  Interrupt Set Enable Register           */
       uint32_t RESERVED0[24];
  __IO uint32_t ICER[8];                 /*!< Offset: 0x080 (R/W)  Interrupt Clear Enable Register         */
       uint32_t RSERVED1[24];
  __IO uint32_t ISPR[8];                 /*!< Offset: 0x100 (R/W)  Interrupt Set Pending Register          */
       uint32_t RESERVED2[24];
  __IO uint32_t ICPR[8];                 /*!< Offset: 0x180 (R/W)  Interrupt Clear Pending Register        */
       uint32_t RESERVED3[24];
  __IO uint32_t IABR[8];                 /*!< Offset: 0x200 (R/W)  Interrupt Active bit Register           */
       uint32_t RESERVED4[56];
  __IO uint8_t  IP[240];                 /*!< Offset: 0x300 (R/W)  Interrupt Priority Register (8Bit wide) */
       uint32_t RESERVED5[644];
  __O  uint32_t STIR;                    /*!< Offset: 0xE00 ( /W)  Software Trigger Interrupt Register     */
}  NVIC_CTRL_Type;
#define NVIC_CTRL ((NVIC_CTRL_Type *) NVIC_CTRL_BASE)

#define __NVIC_CTRL_PRIO_BITS          5       /*!< CM7 uses 4 Bits for the Priority Levels in stm32      */

#define SCB_CTRL_BASE 0xE000ED00 
typedef struct
{
  __I  uint32_t CPUID;                   /*!< Offset: 0x000 (R/ )  CPUID Base Register                                   */
  __IO uint32_t ICSR;                    /*!< Offset: 0x004 (R/W)  Interrupt Control and State Register                  */
  __IO uint32_t VTOR;                    /*!< Offset: 0x008 (R/W)  Vector Table Offset Register                          */
  __IO uint32_t AIRCR;                   /*!< Offset: 0x00C (R/W)  Application Interrupt and Reset Control Register      */
  __IO uint32_t SCR;                     /*!< Offset: 0x010 (R/W)  System Control Register                               */
  __IO uint32_t CCR;                     /*!< Offset: 0x014 (R/W)  Configuration Control Register                        */
  __IO uint8_t  SHPR[12];                /*!< Offset: 0x018 (R/W)  System Handlers Priority Registers (4-7, 8-11, 12-15) */
  __IO uint32_t SHCSR;                   /*!< Offset: 0x024 (R/W)  System Handler Control and State Register             */
  __IO uint32_t CFSR;                    /*!< Offset: 0x028 (R/W)  Configurable Fault Status Register                    */
  __IO uint32_t HFSR;                    /*!< Offset: 0x02C (R/W)  HardFault Status Register                             */
  __IO uint32_t DFSR;                    /*!< Offset: 0x030 (R/W)  Debug Fault Status Register                           */
  __IO uint32_t MMFAR;                   /*!< Offset: 0x034 (R/W)  MemManage Fault Address Register                      */
  __IO uint32_t BFAR;                    /*!< Offset: 0x038 (R/W)  BusFault Address Register                             */
  __IO uint32_t AFSR;                    /*!< Offset: 0x03C (R/W)  Auxiliary Fault Status Register                       */
  __I  uint32_t ID_PFR[2];               /*!< Offset: 0x040 (R/ )  Processor Feature Register                            */
  __I  uint32_t ID_DFR;                  /*!< Offset: 0x048 (R/ )  Debug Feature Register                                */
  __I  uint32_t ID_AFR;                  /*!< Offset: 0x04C (R/ )  Auxiliary Feature Register                            */
  __I  uint32_t ID_MFR[4];               /*!< Offset: 0x050 (R/ )  Memory Model Feature Register                         */
  __I  uint32_t ID_ISAR[5];              /*!< Offset: 0x060 (R/ )  Instruction Set Attributes Register                   */
       uint32_t RESERVED0[1];
  __I  uint32_t CLIDR;                   /*!< Offset: 0x078 (R/ )  Cache Level ID register                               */
  __I  uint32_t CTR;                     /*!< Offset: 0x07C (R/ )  Cache Type register                                   */
  __I  uint32_t CCSIDR;                  /*!< Offset: 0x080 (R/ )  Cache Size ID Register                                */
  __IO uint32_t CSSELR;                  /*!< Offset: 0x084 (R/W)  Cache Size Selection Register                         */
  __IO uint32_t CPACR;                   /*!< Offset: 0x088 (R/W)  Coprocessor Access Control Register                   */
       uint32_t RESERVED3[93];
  __O  uint32_t STIR;                    /*!< Offset: 0x200 ( /W)  Software Triggered Interrupt Register                 */
       uint32_t RESERVED4[15];
  __I  uint32_t MVFR0;                   /*!< Offset: 0x240 (R/ )  Media and VFP Feature Register 0                      */
  __I  uint32_t MVFR1;                   /*!< Offset: 0x244 (R/ )  Media and VFP Feature Register 1                      */
  __I  uint32_t MVFR2;                   /*!< Offset: 0x248 (R/ )  Media and VFP Feature Register 1                      */
       uint32_t RESERVED5[1];
  __O  uint32_t ICIALLU;                 /*!< Offset: 0x250 ( /W)  I-Cache Invalidate All to PoU                         */
       uint32_t RESERVED6[1];
  __O  uint32_t ICIMVAU;                 /*!< Offset: 0x258 ( /W)  I-Cache Invalidate by MVA to PoU                      */
  __O  uint32_t DCIMVAC;                 /*!< Offset: 0x25C ( /W)  D-Cache Invalidate by MVA to PoC                      */
  __O  uint32_t DCISW;                   /*!< Offset: 0x260 ( /W)  D-Cache Invalidate by Set-way                         */
  __O  uint32_t DCCMVAU;                 /*!< Offset: 0x264 ( /W)  D-Cache Clean by MVA to PoU                           */
  __O  uint32_t DCCMVAC;                 /*!< Offset: 0x268 ( /W)  D-Cache Clean by MVA to PoC                           */
  __O  uint32_t DCCSW;                   /*!< Offset: 0x26C ( /W)  D-Cache Clean by Set-way                              */
  __O  uint32_t DCCIMVAC;                /*!< Offset: 0x270 ( /W)  D-Cache Clean and Invalidate by MVA to PoC            */
  __O  uint32_t DCCISW;                  /*!< Offset: 0x274 ( /W)  D-Cache Clean and Invalidate by Set-way               */
       uint32_t RESERVED7[6];
  __IO uint32_t ITCMCR;                  /*!< Offset: 0x290 (R/W)  Instruction Tightly-Coupled Memory Control Register   */
  __IO uint32_t DTCMCR;                  /*!< Offset: 0x294 (R/W)  Data Tightly-Coupled Memory Control Registers         */
  __IO uint32_t AHBPCR;                  /*!< Offset: 0x298 (R/W)  AHBP Control Register                                 */
  __IO uint32_t CACR;                    /*!< Offset: 0x29C (R/W)  L1 Cache Control Register                             */
  __IO uint32_t AHBSCR;                  /*!< Offset: 0x2A0 (R/W)  AHB Slave Control Register                            */
       uint32_t RESERVED8[1];
  __IO uint32_t ABFSR;                   /*!< Offset: 0x2A8 (R/W)  Auxiliary Bus Fault Status Register                   */
} SCB_CTRL_Type;
#define SCB_CTRL ((SCB_CTRL_Type *) SCB_CTRL_BASE)

/* SCB Application Interrupt and Reset Control Register Definitions */
#define SCB_AIRCR_VECTKEY_Pos              16                                             /*!< SCB AIRCR: VECTKEY Position */
#define SCB_AIRCR_VECTKEY_Msk              (0xFFFFUL << SCB_AIRCR_VECTKEY_Pos)            /*!< SCB AIRCR: VECTKEY Mask */

#define SCB_AIRCR_VECTKEYSTAT_Pos          16                                             /*!< SCB AIRCR: VECTKEYSTAT Position */
#define SCB_AIRCR_VECTKEYSTAT_Msk          (0xFFFFUL << SCB_AIRCR_VECTKEYSTAT_Pos)        /*!< SCB AIRCR: VECTKEYSTAT Mask */

#define SCB_AIRCR_PRIGROUP_Pos              8                                             /*!< SCB AIRCR: PRIGROUP Position */
#define SCB_AIRCR_PRIGROUP_Msk             (7UL << SCB_AIRCR_PRIGROUP_Pos)                /*!< SCB AIRCR: PRIGROUP Mask */

//Video SoC Global Reg1 ADDR
#define VSOC_GLOBAL1_BASE               0x40B00000
//Global_Reg1 Register Mapping
typedef struct _GPIO_CFG{
    unsigned int SFR_PAD_CTRL0;             // Address: 0x40b0007c
    unsigned int SFR_PAD_CTRL1;
    unsigned int SFR_PAD_CTRL2;
    unsigned int SFR_PAD_CTRL3;
    unsigned int SFR_PAD_CTRL4;
    unsigned int SFR_PAD_CTRL5;
    unsigned int SFR_PAD_CTRL6;
    unsigned int SFR_PAD_CTRL7;
    unsigned int SFR_PAD_CTRL8;
    unsigned int SFR_DEBUG0;
    unsigned int reserved[9];
    unsigned int SFR_SDRAM_CLK_SEL;
} GPIO_CFG, *GPIO_CFG_PTR;

/** \brief  Structure type to access the System Timer (SysTick).
 */
typedef struct
{
  __IO uint32_t CTRL;                    /*!< Offset: 0x000 (R/W)  SysTick Control and Status Register */
  __IO uint32_t LOAD;                    /*!< Offset: 0x004 (R/W)  SysTick Reload Value Register       */
  __IO uint32_t VAL;                     /*!< Offset: 0x008 (R/W)  SysTick Current Value Register      */
  __I  uint32_t CALIB;                   /*!< Offset: 0x00C (R/ )  SysTick Calibration Register        */
} SysTick_Type;

#define SCS_BASE            (0xE000E000UL)                            /*!< System Control Space Base Address  */
#define SysTick_BASE        (SCS_BASE +  0x0010UL)                    /*!< SysTick Base Address               */
#define SysTick             ((SysTick_Type   *)     SysTick_BASE  )   /*!< SysTick configuration struct       */

/* SysTick Control / Status Register Definitions */
#define SysTick_CTRL_COUNTFLAG_Pos         16                                             /*!< SysTick CTRL: COUNTFLAG Position */
#define SysTick_CTRL_COUNTFLAG_Msk         (1UL << SysTick_CTRL_COUNTFLAG_Pos)            /*!< SysTick CTRL: COUNTFLAG Mask */

#define SysTick_CTRL_CLKSOURCE_Pos          2                                             /*!< SysTick CTRL: CLKSOURCE Position */
#define SysTick_CTRL_CLKSOURCE_Msk         (1UL << SysTick_CTRL_CLKSOURCE_Pos)            /*!< SysTick CTRL: CLKSOURCE Mask */

#define SysTick_CTRL_TICKINT_Pos            1                                             /*!< SysTick CTRL: TICKINT Position */
#define SysTick_CTRL_TICKINT_Msk           (1UL << SysTick_CTRL_TICKINT_Pos)              /*!< SysTick CTRL: TICKINT Mask */

#define SysTick_CTRL_ENABLE_Pos             0                                             /*!< SysTick CTRL: ENABLE Position */
#define SysTick_CTRL_ENABLE_Msk            (1UL /*<< SysTick_CTRL_ENABLE_Pos*/)           /*!< SysTick CTRL: ENABLE Mask */

/* SysTick Reload Register Definitions */
#define SysTick_LOAD_RELOAD_Pos             0                                             /*!< SysTick LOAD: RELOAD Position */
#define SysTick_LOAD_RELOAD_Msk            (0xFFFFFFUL /*<< SysTick_LOAD_RELOAD_Pos*/)    /*!< SysTick LOAD: RELOAD Mask */

/* SysTick Current Register Definitions */
#define SysTick_VAL_CURRENT_Pos             0                                             /*!< SysTick VAL: CURRENT Position */
#define SysTick_VAL_CURRENT_Msk            (0xFFFFFFUL /*<< SysTick_VAL_CURRENT_Pos*/)    /*!< SysTick VAL: CURRENT Mask */

/* SysTick Calibration Register Definitions */
#define SysTick_CALIB_NOREF_Pos            31                                             /*!< SysTick CALIB: NOREF Position */
#define SysTick_CALIB_NOREF_Msk            (1UL << SysTick_CALIB_NOREF_Pos)               /*!< SysTick CALIB: NOREF Mask */

#define SysTick_CALIB_SKEW_Pos             30                                             /*!< SysTick CALIB: SKEW Position */
#define SysTick_CALIB_SKEW_Msk             (1UL << SysTick_CALIB_SKEW_Pos)                /*!< SysTick CALIB: SKEW Mask */

#define SysTick_CALIB_TENMS_Pos             0                                             /*!< SysTick CALIB: TENMS Position */
#define SysTick_CALIB_TENMS_Msk            (0xFFFFFFUL /*<< SysTick_CALIB_TENMS_Pos*/)    /*!< SysTick CALIB: TENMS Mask */

/*@} end of group CMSIS_SysTick */

/** \ingroup  CMSIS_core_register
    \defgroup CMSIS_MPU     Memory Protection Unit (MPU)
    \brief      Type definitions for the Memory Protection Unit (MPU)
  @{
 */

/** \brief  Structure type to access the Memory Protection Unit (MPU).
 */
typedef struct
{
  __I  uint32_t TYPE;                    /*!< Offset: 0x000 (R/ )  MPU Type Register                              */
  __IO uint32_t CTRL;                    /*!< Offset: 0x004 (R/W)  MPU Control Register                           */
  __IO uint32_t RNR;                     /*!< Offset: 0x008 (R/W)  MPU Region RNRber Register                     */
  __IO uint32_t RBAR;                    /*!< Offset: 0x00C (R/W)  MPU Region Base Address Register               */
  __IO uint32_t RASR;                    /*!< Offset: 0x010 (R/W)  MPU Region Attribute and Size Register         */
  __IO uint32_t RBAR_A1;                 /*!< Offset: 0x014 (R/W)  MPU Alias 1 Region Base Address Register       */
  __IO uint32_t RASR_A1;                 /*!< Offset: 0x018 (R/W)  MPU Alias 1 Region Attribute and Size Register */
  __IO uint32_t RBAR_A2;                 /*!< Offset: 0x01C (R/W)  MPU Alias 2 Region Base Address Register       */
  __IO uint32_t RASR_A2;                 /*!< Offset: 0x020 (R/W)  MPU Alias 2 Region Attribute and Size Register */
  __IO uint32_t RBAR_A3;                 /*!< Offset: 0x024 (R/W)  MPU Alias 3 Region Base Address Register       */
  __IO uint32_t RASR_A3;                 /*!< Offset: 0x028 (R/W)  MPU Alias 3 Region Attribute and Size Register */
} MPU_Type;

/* MPU Type Register */
#define MPU_TYPE_IREGION_Pos               16                                             /*!< MPU TYPE: IREGION Position */
#define MPU_TYPE_IREGION_Msk               (0xFFUL << MPU_TYPE_IREGION_Pos)               /*!< MPU TYPE: IREGION Mask */

#define MPU_TYPE_DREGION_Pos                8                                             /*!< MPU TYPE: DREGION Position */
#define MPU_TYPE_DREGION_Msk               (0xFFUL << MPU_TYPE_DREGION_Pos)               /*!< MPU TYPE: DREGION Mask */

#define MPU_TYPE_SEPARATE_Pos               0                                             /*!< MPU TYPE: SEPARATE Position */
#define MPU_TYPE_SEPARATE_Msk              (1UL /*<< MPU_TYPE_SEPARATE_Pos*/)             /*!< MPU TYPE: SEPARATE Mask */

/* MPU Control Register */
#define MPU_CTRL_PRIVDEFENA_Pos             2                                             /*!< MPU CTRL: PRIVDEFENA Position */
#define MPU_CTRL_PRIVDEFENA_Msk            (1UL << MPU_CTRL_PRIVDEFENA_Pos)               /*!< MPU CTRL: PRIVDEFENA Mask */

#define MPU_CTRL_HFNMIENA_Pos               1                                             /*!< MPU CTRL: HFNMIENA Position */
#define MPU_CTRL_HFNMIENA_Msk              (1UL << MPU_CTRL_HFNMIENA_Pos)                 /*!< MPU CTRL: HFNMIENA Mask */

#define MPU_CTRL_ENABLE_Pos                 0                                             /*!< MPU CTRL: ENABLE Position */
#define MPU_CTRL_ENABLE_Msk                (1UL /*<< MPU_CTRL_ENABLE_Pos*/)               /*!< MPU CTRL: ENABLE Mask */

/* MPU Region Number Register */
#define MPU_RNR_REGION_Pos                  0                                             /*!< MPU RNR: REGION Position */
#define MPU_RNR_REGION_Msk                 (0xFFUL /*<< MPU_RNR_REGION_Pos*/)             /*!< MPU RNR: REGION Mask */

/* MPU Region Base Address Register */
#define MPU_RBAR_ADDR_Pos                   5                                             /*!< MPU RBAR: ADDR Position */
#define MPU_RBAR_ADDR_Msk                  (0x7FFFFFFUL << MPU_RBAR_ADDR_Pos)             /*!< MPU RBAR: ADDR Mask */

#define MPU_RBAR_VALID_Pos                  4                                             /*!< MPU RBAR: VALID Position */
#define MPU_RBAR_VALID_Msk                 (1UL << MPU_RBAR_VALID_Pos)                    /*!< MPU RBAR: VALID Mask */

#define MPU_RBAR_REGION_Pos                 0                                             /*!< MPU RBAR: REGION Position */
#define MPU_RBAR_REGION_Msk                (0xFUL /*<< MPU_RBAR_REGION_Pos*/)             /*!< MPU RBAR: REGION Mask */

/* MPU Region Attribute and Size Register */
#define MPU_RASR_ATTRS_Pos                 16                                             /*!< MPU RASR: MPU Region Attribute field Position */
#define MPU_RASR_ATTRS_Msk                 (0xFFFFUL << MPU_RASR_ATTRS_Pos)               /*!< MPU RASR: MPU Region Attribute field Mask */

#define MPU_RASR_XN_Pos                    28                                             /*!< MPU RASR: ATTRS.XN Position */
#define MPU_RASR_XN_Msk                    (1UL << MPU_RASR_XN_Pos)                       /*!< MPU RASR: ATTRS.XN Mask */

#define MPU_RASR_AP_Pos                    24                                             /*!< MPU RASR: ATTRS.AP Position */
#define MPU_RASR_AP_Msk                    (0x7UL << MPU_RASR_AP_Pos)                     /*!< MPU RASR: ATTRS.AP Mask */

#define MPU_RASR_TEX_Pos                   19                                             /*!< MPU RASR: ATTRS.TEX Position */
#define MPU_RASR_TEX_Msk                   (0x7UL << MPU_RASR_TEX_Pos)                    /*!< MPU RASR: ATTRS.TEX Mask */

#define MPU_RASR_S_Pos                     18                                             /*!< MPU RASR: ATTRS.S Position */
#define MPU_RASR_S_Msk                     (1UL << MPU_RASR_S_Pos)                        /*!< MPU RASR: ATTRS.S Mask */

#define MPU_RASR_C_Pos                     17                                             /*!< MPU RASR: ATTRS.C Position */
#define MPU_RASR_C_Msk                     (1UL << MPU_RASR_C_Pos)                        /*!< MPU RASR: ATTRS.C Mask */

#define MPU_RASR_B_Pos                     16                                             /*!< MPU RASR: ATTRS.B Position */
#define MPU_RASR_B_Msk                     (1UL << MPU_RASR_B_Pos)                        /*!< MPU RASR: ATTRS.B Mask */

#define MPU_RASR_SRD_Pos                    8                                             /*!< MPU RASR: Sub-Region Disable Position */
#define MPU_RASR_SRD_Msk                   (0xFFUL << MPU_RASR_SRD_Pos)                   /*!< MPU RASR: Sub-Region Disable Mask */

#define MPU_RASR_SIZE_Pos                   1                                             /*!< MPU RASR: Region Size Field Position */
#define MPU_RASR_SIZE_Msk                  (0x1FUL << MPU_RASR_SIZE_Pos)                  /*!< MPU RASR: Region Size Field Mask */

#define MPU_RASR_ENABLE_Pos                 0                                             /*!< MPU RASR: Region enable bit Position */
#define MPU_RASR_ENABLE_Msk                (1UL /*<< MPU_RASR_ENABLE_Pos*/)               /*!< MPU RASR: Region enable bit Disable Mask */

/*@} end of group CMSIS_MPU */

#define SCS_BASE          (0xE000E000UL)                            /*!< System Control Space Base Address  */
#define MPU_BASE          (SCS_BASE +  0x0D90UL)                    /*!< Memory Protection Unit             */
#define MPU               ((volatile MPU_Type       *)     MPU_BASE      )   /*!< Memory Protection Unit             */

#endif

