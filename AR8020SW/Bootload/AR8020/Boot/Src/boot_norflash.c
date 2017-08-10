#include <stdint.h>
#include <stdio.h>
#include "boot_norflash.h"
#include "boot_regrw.h"


uint8_t QUAD_SPI_ReadByte(uint32_t flash_addr, uint8_t* value_ptr)
{

    volatile uint8_t* p = (uint8_t*)(FLASH_APB_BASE_ADDR + flash_addr);
    
    *value_ptr = *p;
            
    return 0;
}

uint8_t QUAD_SPI_SetSpeed(void)
{
    uint32_t mask_val = BIT(2);
    uint32_t mask_bit = BIT(2) | BIT(3) | BIT(7);

    Reg_Write32_Mask(HP_SPI_BASE_ADDR + HP_SPI_CONFIG_REG, mask_val, mask_bit);

    return 0;
}

void QUAD_SPI_ReadBlockByByte(uint32_t flash_blk_st_addr, uint8_t* blk_val_table, uint32_t byte_size)
{
    uint32_t i;
    volatile uint8_t* write_addr = (uint8_t*)(FLASH_APB_BASE_ADDR + flash_blk_st_addr);

    if (blk_val_table == NULL)
    {
        return ;
    }

    for (i = 0; i < byte_size; i++)
    {
        blk_val_table[i] = *write_addr;
        write_addr++;
    }

}
