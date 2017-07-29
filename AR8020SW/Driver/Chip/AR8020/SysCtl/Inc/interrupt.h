#ifndef __ISR__H
#define __ISR__H

#include <stddef.h>
#include <stdint.h>

typedef enum
{
    SYSTICK_VECTOR_NUM = 15,                    //15
    UART_INTR0_VECTOR_NUM = 16,                 //16
    UART_INTR1_VECTOR_NUM,                      //17
    UART_INTR2_VECTOR_NUM,                      //18
    UART_INTR3_VECTOR_NUM,                      //19
    UART_INTR4_VECTOR_NUM,                      //20
    UART_INTR5_VECTOR_NUM,                      //21
    UART_INTR6_VECTOR_NUM,                      //22
    UART_INTR7_VECTOR_NUM,                      //23
    UART_INTR8_VECTOR_NUM,                      //24
    TIMER_INTR00_VECTOR_NUM,                    //25
    TIMER_INTR01_VECTOR_NUM,                    //26
    TIMER_INTR02_VECTOR_NUM,                    //27
    TIMER_INTR03_VECTOR_NUM,                    //28
    TIMER_INTR04_VECTOR_NUM,                    //29
    TIMER_INTR05_VECTOR_NUM,                    //30
    TIMER_INTR06_VECTOR_NUM,                    //31
    TIMER_INTR07_VECTOR_NUM,                    //32
    TIMER_INTR10_VECTOR_NUM,                    //33
    TIMER_INTR11_VECTOR_NUM,                    //34
    TIMER_INTR12_VECTOR_NUM,                    //35
    TIMER_INTR13_VECTOR_NUM,                    //36
    TIMER_INTR14_VECTOR_NUM,                    //37
    TIMER_INTR15_VECTOR_NUM,                    //38
    TIMER_INTR16_VECTOR_NUM,                    //39
    TIMER_INTR17_VECTOR_NUM,                    //40
    TIMER_INTR20_VECTOR_NUM,                    //41
    TIMER_INTR21_VECTOR_NUM,                    //42
    TIMER_INTR22_VECTOR_NUM,                    //43
    TIMER_INTR23_VECTOR_NUM,                    //44
    TIMER_INTR24_VECTOR_NUM,                    //45
    TIMER_INTR25_VECTOR_NUM,                    //46
    TIMER_INTR26_VECTOR_NUM,                    //47
    TIMER_INTR27_VECTOR_NUM,                    //48
    SSI_INTR_N0_VECTOR_NUM,                     //49
    SSI_INTR_N1_VECTOR_NUM,                     //50
    SSI_INTR_N2_VECTOR_NUM,                     //51
    SSI_INTR_N3_VECTOR_NUM,                     //52
    SSI_INTR_N4_VECTOR_NUM,                     //53
    SSI_INTR_N5_VECTOR_NUM,                     //54
    SSI_INTR_N6_VECTOR_NUM,                     //55
    I2C_INTR0_VECTOR_NUM,                       //56
    I2C_INTR1_VECTOR_NUM,                       //57
    I2C_INTR2_VECTOR_NUM,                       //58
    I2C_INTR3_VECTOR_NUM,                       //59
    CAN_IRQ0_VECTOR_NUM,                        //60
    CAN_IRQ1_VECTOR_NUM,                        //61
    CAN_IRQ2_VECTOR_NUM,                        //62
    CAN_IRQ3_VECTOR_NUM,                        //63
    WDT_INTR0_VECTOR_NUM,                       //64
    WDT_INTR1_VECTOR_NUM,                       //65
    GPIO_INTR_N0_VECTOR_NUM,                    //66
    GPIO_INTR_N1_VECTOR_NUM,                    //67
    GPIO_INTR_N2_VECTOR_NUM,                    //68
    GPIO_INTR_N3_VECTOR_NUM,                    //69
    I2C_SLV_INTR_VECTOR_NUM,                    //70
    RTC_INTR_VECTOR_NUM,                        //71
    OTG_INTR0_VECTOR_NUM,                       //72
    OTG_INTR1_VECTOR_NUM,                       //73
    SD_INTR_VECTOR_NUM,                         //74
    DMA_INTR_N_VECTOR_NUM,                      //75
    VIDEO_UART9_INTR_VECTOR_NUM,                //76
    VIDEO_ARMCM7_IRQ_VECTOR_NUM,                //77
    VIDEO_UART10_INTR_VECTOR_NUM,               //78
    VIDEO_I2C_INTR_VIDEO_VECTOR_NUM,            //79
    VIDEO_SSI_INTR_VIDEO_VECTOR_NUM,            //80
    VIDEO_WDT_INTR2_VECTOR_NUM,                 //81
    VIDEO_SPI_INTR_BB_VECTOR_NUM,               //82
    VIDEO_IMGWR_FD_CH1_INTR_VECTOR_NUM,         //83
    VIDEO_IMGWR_FD_CH0_INTR_VECTOR_NUM,         //84
    VIDEO_GLOBAL2_INTR_RES_VSOC0_VECTOR_NUM,    //85
    VIDEO_GLOBAL2_INTR_RES_VSOC1_VECTOR_NUM,    //86
    BB_SRAM_READY_IRQ_0_VECTOR_NUM,             //87
    BB_SRAM_READY_IRQ_1_VECTOR_NUM,             //88
    BB_TX_ENABLE_VECTOR_NUM,                    //89
    BB_RX_ENABLE_VECTOR_NUM,                    //90
    CM7_1_FPU_IRQ0 = 91,                        //91
    CM7_1_FPU_IRQ1,                             //92
    CM7_1_FPU_IRQ2,                             //93
    CM7_1_FPU_IRQ3,                             //94
    CM7_1_FPU_IRQ4,                             //95
    CM7_1_FPU_IRQ5,                             //96
    CM7_1_CTIIRQ0,                              //97
    CM7_1_CTIIRQ1,                              //98
    CM7_2_FPU_IRQ0 = 91,                        //91
    CM7_2_FPU_IRQ1,                             //92
    CM7_2_FPU_IRQ2,                             //93
    CM7_2_FPU_IRQ3,                             //94
    CM7_2_FPU_IRQ4,                             //95
    CM7_2_FPU_IRQ5,                             //96
    CM7_2_CTIIRQ0,                              //97
    CM7_2_CTIIRQ1,                              //98
    CM7_3_FPU_IRQ0 = 91,                        //91
    CM7_3_FPU_IRQ1,                             //92
    CM7_3_FPU_IRQ2,                             //93
    CM7_3_FPU_IRQ3,                             //94
    CM7_3_FPU_IRQ4,                             //95
    CM7_3_FPU_IRQ5,                             //96
    CM7_3_CTIIRQ0,                              //97
    CM7_3_CTIIRQ1,                              //98
}IRQ_type;
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bits for pre-emption priority
                                                                 5 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bits for pre-emption priority
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority
                                                                 1 bits for subpriority */
#define NVIC_PRIORITYGROUP_5         ((uint32_t)0x00000002) /*!< 5 bits for pre-emption priority
                                                                 0 bits for subpriority */

#define INTR_NVIC_PRIORITY_VIDEO_VSOC0          (8)
#define INTR_NVIC_PRIORITY_VIDEO_VSOC1          (8)
#define INTR_NVIC_PRIORITY_UART_DEFAULT         (5)
#define INTR_NVIC_PRIORITY_SPI_DEFAULT          (5)
#define INTR_NVIC_PRIORITY_I2C_DEFAULT          (5)

//cpu0 interrupt priority use 5-10,
#define INTR_NVIC_PRIORITY_UART0                (5)
#define INTR_NVIC_PRIORITY_OTG_INITR0           (8)
#define INTR_NVIC_PRIORITY_OTG_INITR1           (8)
#define INTR_NVIC_PRIORITY_SRAM0                (7)
#define INTR_NVIC_PRIORITY_SRAM1                (7)
#define INTR_NVIC_PRIORITY_SD                   (6)
#define INTR_NVIC_PRIORITY_HDMI_GPIO            (6)
#define INTR_NVIC_PRIORITY_RTC                  (5)
#define INTR_NVIC_PRIORITY_GLOBAL2_INTR_VSOC0   (8)
#define INTR_NVIC_PRIORITY_GLOBAL2_INTR_VSOC1   (8)

//cpu1 interrupt priority use 5-10,
#define INTR_NVIC_PRIORITY_UART1                (5)
//cpu2 interrupt priority use 5-10,
#define INTR_NVIC_PRIORITY_VIDEO_UART10         (5)
#define INTR_NVIC_PRIORITY_UART2                (5)
#define INTR_NVIC_PRIORITY_TIMER00              (6)
#define INTR_NVIC_PRIORITY_TIMER01              (6)
#define INTR_NVIC_PRIORITY_BB_TX                (7)
#define INTR_NVIC_PRIORITY_BB_RX                (7)
#define INTR_NVIC_PRIORITY_VIDEO_ARMCM7         (5)

typedef void(*Irq_handler)(uint32_t);

int reg_IrqHandle(IRQ_type vct, Irq_handler hdl, Irq_handler postHdl);
int rmv_IrqHandle(IRQ_type vct);

void INTR_NVIC_EnableIRQ(IRQ_type vct);
void INTR_NVIC_DisableIRQ(IRQ_type vct);
uint32_t INTR_NVIC_GetPendingIRQ(IRQ_type vct);
void INTR_NVIC_SetPendingIRQ(IRQ_type vct);
void INTR_NVIC_ClearPendingIRQ(IRQ_type vct);
uint32_t INTR_NVIC_GetActiveIRQ(IRQ_type vct);
void INTR_NVIC_SetIRQPriority(IRQ_type vct, uint32_t priority);
uint32_t INTR_NVIC_GetIRQPriority(IRQ_type vct);
void INTR_NVIC_SetPriorityGrouping(uint32_t PriorityGroup);
uint32_t INTR_NVIC_GetPriorityGrouping(void);
uint32_t INTR_NVIC_EncodePriority (uint32_t PriorityGroup, uint32_t PreemptPriority, uint32_t SubPriority);
void INTR_NVIC_DecodePriority (uint32_t Priority, uint32_t PriorityGroup, uint32_t* pPreemptPriority, uint32_t* pSubPriority);

void SYSTICK_IRQHandler(void);

void IRQHandler_16(void);
void IRQHandler_17(void);
void IRQHandler_18(void);
void IRQHandler_19(void);
void IRQHandler_20(void);
void IRQHandler_21(void);
void IRQHandler_22(void);
void IRQHandler_23(void);
void IRQHandler_24(void);
void IRQHandler_25(void);
void IRQHandler_26(void);
void IRQHandler_27(void);
void IRQHandler_28(void);
void IRQHandler_29(void);
void IRQHandler_30(void);
void IRQHandler_31(void);
void IRQHandler_32(void);
void IRQHandler_33(void);
void IRQHandler_34(void);
void IRQHandler_35(void);
void IRQHandler_36(void);
void IRQHandler_37(void);
void IRQHandler_38(void);
void IRQHandler_39(void);
void IRQHandler_40(void);
void IRQHandler_41(void);
void IRQHandler_42(void);
void IRQHandler_43(void);
void IRQHandler_44(void);
void IRQHandler_45(void);
void IRQHandler_46(void);
void IRQHandler_47(void);
void IRQHandler_48(void);
void IRQHandler_49(void);
void IRQHandler_50(void);
void IRQHandler_51(void);
void IRQHandler_52(void);
void IRQHandler_53(void);
void IRQHandler_54(void);
void IRQHandler_55(void);
void IRQHandler_56(void);
void IRQHandler_57(void);
void IRQHandler_58(void);
void IRQHandler_59(void);
void IRQHandler_60(void);
void IRQHandler_61(void);
void IRQHandler_62(void);
void IRQHandler_63(void);
void IRQHandler_64(void);
void IRQHandler_65(void);
void IRQHandler_66(void);
void IRQHandler_67(void);
void IRQHandler_68(void);
void IRQHandler_69(void);
void IRQHandler_70(void);
void IRQHandler_71(void);
void IRQHandler_72(void);
void IRQHandler_73(void);
void IRQHandler_74(void);
void IRQHandler_75(void);
void IRQHandler_76(void);
void IRQHandler_77(void);
void IRQHandler_78(void);
void IRQHandler_79(void);
void IRQHandler_80(void);
void IRQHandler_81(void);
void IRQHandler_82(void);
void IRQHandler_83(void);
void IRQHandler_84(void);
void IRQHandler_85(void);
void IRQHandler_86(void);
void IRQHandler_87(void);
void IRQHandler_88(void);
void IRQHandler_89(void);
void IRQHandler_90(void);
void IRQHandler_91(void);
void IRQHandler_92(void);
void IRQHandler_93(void);
void IRQHandler_94(void);
void IRQHandler_95(void);
void IRQHandler_96(void);
void IRQHandler_97(void);
void IRQHandler_98(void);

#endif

