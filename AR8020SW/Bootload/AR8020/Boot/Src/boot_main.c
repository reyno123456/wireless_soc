#include <string.h>
#include <stdint.h>
#include "boot_serial.h"
#include "boot_norflash.h"
#include "boot_core.h"

void delay_ms(uint32_t num)
{
    volatile uint32_t i;

    for (i = 0; i < (num * 1000); i++)
    {
        ;      
    }
}

int main(void)
{
    Boot_Info st_bootInfo;    
    uart_init(0,115200);
    uart_puts(0,"stag 1\r\n");
    uint32_t bootAddress = 0;
    QUAD_SPI_SetSpeed();
    
    delay_ms(500);
    memset(&st_bootInfo,0xff,sizeof(st_bootInfo));
    QUAD_SPI_ReadBlockByByte(INFO_BASE,(uint8_t *)(&st_bootInfo),sizeof(st_bootInfo));

    if('t' == uart_getc(0))
    {
        uart_puts(0,"boot BOOT\r\n");
        delay_ms(100);
        if('t' == uart_getc(0))
        {            
            if(0 == st_bootInfo.present_boot)
            {
                uart_puts(0,"BOOT0\r\n");
                bootAddress = BOOT_ADDR0;
            }
            else if(1 == st_bootInfo.present_boot)
            {
                uart_puts(0,"BOOT1\r\n");
                bootAddress = BOOT_ADDR1;
            }
            delay_ms(50);
            BOOT_PrintInfo(bootAddress+IMAGE_HAER_OFFSET);
            BOOT_StartBoot(st_bootInfo.present_boot,bootAddress);
        }
        
    }
    
    uart_puts(0,"boot app\r\n");
    BOOT_PrintInfo(0x10020000+IMAGE_HAER_OFFSET);
    BOOT_CopyFromNorToITCM(0x10020000+IMAGE_HAER_OFFSET);
    BOOT_BootApp();

} 

