#ifndef BOOT__ISR__H
#define BOOT__ISR__H

#include <stddef.h>
#include <stdint.h>

typedef enum
{
    UART_INTR0_VECTOR_NUM = 16,                 //16
}IRQ_type;

typedef void(*Irq_handler)(void);

int reg_IrqHandle(IRQ_type vct, Irq_handler hdl);
void INTR_NVIC_EnableIRQ(IRQ_type vct);
void INTR_NVIC_DisableIRQ(IRQ_type vct);
void INTR_NVIC_ClearPendingIRQ(IRQ_type vct);
void INTR_NVIC_SetIRQPriority(IRQ_type vct, uint32_t priority);

void SYSTICK_IRQHandler(void);

void IRQHandler_16(void);

#endif

