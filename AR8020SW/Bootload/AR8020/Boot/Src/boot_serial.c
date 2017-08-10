#include <stddef.h>
#include "boot_serial.h"

void uart_init(unsigned char index, unsigned int baud_rate)
{
    int devisor;
    volatile uart_type *uart_regs =(uart_type *)UART0_BASE;
    uart_regs->IIR_FCR = UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT | UART_FCR_TRIGGER_14;
    uart_regs->DLH_IER = 0x00000000;
    uart_regs->LCR = UART_LCR_WLEN8 & ~(UART_LCR_STOP | UART_LCR_PARITY);

    devisor = CLK_FREQ / (16 * baud_rate);
    uart_regs->LCR |= UART_LCR_DLAB;
    uart_regs->DLH_IER = (devisor >> 8) & 0x000000ff;
    uart_regs->RBR_THR_DLL = devisor & 0x000000ff;
    uart_regs->LCR &= ~UART_LCR_DLAB;
    uart_regs->DLH_IER = 0x1;
}

void uart_putc(unsigned char index, unsigned char c)
{
    volatile uart_type *uart_regs = (uart_type *)UART0_BASE;

    while ((uart_regs->LSR & UART_LSR_THRE) != UART_LSR_THRE);
    uart_regs->RBR_THR_DLL = c;

}

void uart_puts(unsigned char index, const char *s)
{
    while (*s)
    {
        uart_putc(index, *s++);
    }
}

char uart_getc(unsigned char index)
{
    volatile uart_type *uart_regs = (uart_type *)UART0_BASE;

    if ((uart_regs->LSR & UART_LSR_DATAREADY) != UART_LSR_DATAREADY)
    {
        return -1;
    }
    else
    {
        return uart_regs->RBR_THR_DLL;
    }
}


