#include <stdint.h>
#include <string.h>
#include "boot_command.h"
#include "boot_serial.h"
#include "boot_interrupt.h"
#include "boot_systicks.h"

static uint8_t g_u32CommandPos;
static int8_t g_commandLine[50];
static uint8_t g_commandEnter = 0;
extern uint8_t g_u8BootMode;


static void BOOT_UartIrqHandler(void)
{
    char                  c;
    unsigned int          status;
    unsigned int          isrType;
    volatile uart_type   *uart_regs;
    uart_regs = (uart_type *)UART0_BASE;
    status     = uart_regs->LSR;
    isrType    = uart_regs->IIR_FCR;

    /* receive data irq, try to get the data */
    if (UART_IIR_RECEIVEDATA == (isrType & UART_IIR_RECEIVEDATA))
    {
        if ((status & UART_LSR_DATAREADY) == UART_LSR_DATAREADY)
        {
            c = uart_regs->RBR_THR_DLL;
            if(g_u32CommandPos >49)
            {
                g_u32CommandPos = 0;
            }            
            if (c == '\r')
            {
                uart_putc(0,'\r');
                uart_putc(0,'\n');
                /* if g_commandLine is not empty, go to parse command */
                if (g_u32CommandPos > 0)
                {
                    g_commandEnter = 1;
                }
            }
            else
            {
                uart_putc(0, c);
                if (g_u32CommandPos < sizeof(g_commandLine))
                {
                    g_commandLine[g_u32CommandPos++] = c;
                }
            }
            if ((c == 't') && (g_u8BootMode == 0))
            {
                 g_u8BootMode = 1;
                if (g_u32CommandPos >= 3)
                {
                     if ((g_commandLine[g_u32CommandPos-1] == 't') && (g_commandLine[g_u32CommandPos-2] == 't') && (g_commandLine[g_u32CommandPos-3] == 't'))
                     {
                         g_u8BootMode = 1;
                     }
                }
            }
        }
    }
}

void BOOT_CommandInit(uint8_t uart_num)
{
    g_u32CommandPos = 0;
 
    reg_IrqHandle(UART_INTR0_VECTOR_NUM, BOOT_UartIrqHandler);
    INTR_NVIC_SetIRQPriority(UART_INTR0_VECTOR_NUM, 1);
    INTR_NVIC_EnableIRQ(UART_INTR0_VECTOR_NUM);
}
