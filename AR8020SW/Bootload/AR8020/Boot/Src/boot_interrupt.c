#include "boot_interrupt.h"
#include "boot_regmap.h"

#define MAX_IRQ_VECTROS     (99)

static Irq_handler handlers[MAX_IRQ_VECTROS] = {
    0
};

int reg_IrqHandle(IRQ_type vct, Irq_handler hdl)
{
    if(vct < MAX_IRQ_VECTROS)
    {
        handlers[vct] = hdl;
        return 0;
    }

    return 1;
}

static inline void run_irq_hdl(IRQ_type vct)
{
    if(handlers[vct] != 0)
    {
        (handlers[vct])();
    }
}

void INTR_NVIC_EnableIRQ(IRQ_type vct)
{
    int32_t IRQn = vct - UART_INTR0_VECTOR_NUM; 
    NVIC_CTRL->ISER[(((uint32_t)(int32_t)IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)(int32_t)IRQn) & 0x1FUL));
}

void INTR_NVIC_DisableIRQ(IRQ_type vct)
{
    int32_t IRQn = vct - UART_INTR0_VECTOR_NUM;
    NVIC_CTRL->ICER[(((uint32_t)(int32_t)IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)(int32_t)IRQn) & 0x1FUL));
}


void INTR_NVIC_ClearPendingIRQ(IRQ_type vct)
{
    int32_t IRQn = vct - UART_INTR0_VECTOR_NUM;
    NVIC_CTRL->ICPR[(((uint32_t)(int32_t)IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)(int32_t)IRQn) & 0x1FUL));
}


void INTR_NVIC_SetIRQPriority(IRQ_type vct, uint32_t priority)
{
    int32_t IRQn = vct - UART_INTR0_VECTOR_NUM;
    if((int32_t)IRQn < 0)
    {
        SCB_CTRL->SHPR[(((uint32_t)(int32_t)IRQn) & 0xFUL)-4UL] = (uint8_t)((priority << (8 - __NVIC_CTRL_PRIO_BITS)) & (uint32_t)0xFFUL);
    }
    else
    {
        NVIC_CTRL->IP[((uint32_t)(int32_t)IRQn)] = (uint8_t)((priority << (8 - __NVIC_CTRL_PRIO_BITS)) & (uint32_t)0xFFUL);
    }
}

__attribute__((weak)) void SysTicks_IncTickCount(void)
{
}

__attribute__((weak)) void osSystickHandler(void)
{
}

void SYSTICK_IRQHandler(void)
{
    SysTicks_IncTickCount();
    osSystickHandler();
}

// To replace the weak function in start.s
void default_isr(void)
{
}

// To replace the weak function in start.s
void hardfault_isr(void)
{
}

void IRQHandler_16(void)  { run_irq_hdl(16);  }
