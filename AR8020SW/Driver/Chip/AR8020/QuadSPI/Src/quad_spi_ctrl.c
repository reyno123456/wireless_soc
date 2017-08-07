#include <stdint.h>
#include "data_type.h"
#include "debuglog.h"
#include "quad_spi_ctrl.h"
#include "reg_rw.h"

void QUAD_SPI_Delay(unsigned int delay)
{
    volatile int i = delay;
    while(i > 0)
    {
        i--;
    }
}

uint8_t QUAD_SPI_EnableDefaultInstruct(void)
{
    Reg_Write32(HP_SPI_BASE_ADDR + HP_SPI_INIT_REG, BIT(0));
}

uint8_t QUAD_SPI_WriteEnable(void)
{
    uint8_t ret = TRUE;
    
    Reg_Write32(HP_SPI_BASE_ADDR + CMD16,   0x0);
    Reg_Write32(HP_SPI_BASE_ADDR + HP_SPI_WR_HW_REG,0x739c08);
    Reg_Write32(HP_SPI_BASE_ADDR + HP_SPI_WR_LW_REG,0xff17);
    Reg_Write32(HP_SPI_BASE_ADDR + HP_SPI_UPDATE_WR_REG,0);
            
    return ret;
}

uint8_t QUAD_SPI_WriteDisable(void)
{
    uint8_t ret = TRUE;
    
    Reg_Write32(HP_SPI_BASE_ADDR + HP_SPI_WR_HW_REG,0);
    Reg_Write32(HP_SPI_BASE_ADDR + HP_SPI_WR_LW_REG,0);
    Reg_Write32(HP_SPI_BASE_ADDR + HP_SPI_UPDATE_WR_REG,0);
            
    return ret;
}

uint8_t QUAD_SPI_CheckBusy(void)
{
    unsigned int    rd_tmp;
    unsigned char   busy_flag;

    do
    {
        QUAD_SPI_Delay(200);
        rd_tmp = Reg_Read32(HP_SPI_BASE_ADDR + CMD0);   //read SR1
        busy_flag = rd_tmp & 0x1;
    } while(busy_flag);
}

uint8_t QUAD_SPI_BlockErase(uint32_t flash_blk_st_addr)
{
    uint8_t ret = TRUE;

    Reg_Write32(HP_SPI_BASE_ADDR + CMD21_ADDR, flash_blk_st_addr);        //tx addr
    Reg_Write32(HP_SPI_BASE_ADDR + CMD21, 0x0);                           //activate erase
    
    return ret;
}

uint8_t QUAD_SPI_SectorErase(uint32_t flash_sect_st_addr)
{
    uint8_t ret = TRUE;

    Reg_Write32(HP_SPI_BASE_ADDR + CMD20_ADDR, flash_sect_st_addr);       //tx addr
    Reg_Write32(HP_SPI_BASE_ADDR + CMD20, 0x0);                           //activate erase
        
    return ret;
}

uint8_t QUAD_SPI_ChipErase(void)
{
    uint8_t ret = TRUE;

    Reg_Write32(HP_SPI_BASE_ADDR + CMD9, 0x0);                           //activate erase
       
    return ret;
}

uint8_t QUAD_SPI_WriteByte(uint32_t flash_addr, uint8_t value)
{
    uint8_t ret = TRUE;
    
    *((volatile uint8_t*)(FLASH_APB_BASE_ADDR + flash_addr)) = value;
            
    return ret;
}

uint8_t QUAD_SPI_WriteHalfWord(uint32_t flash_addr, uint16_t value)
{
    uint8_t ret = TRUE;

    *((volatile uint16_t*) (FLASH_APB_BASE_ADDR + flash_addr)) = value;  

    return ret;
}

uint8_t QUAD_SPI_WriteWord(uint32_t flash_addr, uint32_t value)
{
    uint8_t ret = TRUE;

    *((volatile uint32_t*) (FLASH_APB_BASE_ADDR + flash_addr)) = value;     
            
    return ret;
}

uint8_t QUAD_SPI_WriteBlockByByte(uint32_t flash_blk_st_addr, uint8_t* blk_val_table, uint32_t byte_size)
{
    uint32_t i;
    uint8_t ret = TRUE;
    volatile uint8_t* write_addr = (uint8_t*)(FLASH_APB_BASE_ADDR + flash_blk_st_addr);

    if (blk_val_table == NULL)
    {
        dlog_error("blk_val_table pointer is NULL\n");
        return FALSE;
    }

    for (i = 0; i < byte_size; i++)
    {
        *write_addr = blk_val_table[i];
        write_addr++;
    }
    
    return ret;
}

uint8_t QUAD_SPI_WriteBlockByHalfWord(uint32_t flash_blk_st_addr, uint16_t* blk_val_table, uint32_t halfword_size)
{
    uint32_t i;
    uint8_t ret = TRUE;
    volatile uint16_t* write_addr = (uint16_t*)(FLASH_APB_BASE_ADDR + flash_blk_st_addr);

    if (blk_val_table == NULL)
    {
        dlog_error("blk_val_table pointer is NULL\n");
        return FALSE;
    }

    for (i = 0; i < halfword_size; i++)
    {
        *write_addr = blk_val_table[i];
        write_addr++;
    }
    
    return ret;
}

uint8_t QUAD_SPI_WriteBlockByWord(uint32_t flash_blk_st_addr, uint32_t* blk_val_table, uint32_t word_size)
{
    uint32_t i;
    uint8_t ret = TRUE;
    volatile uint32_t* write_addr = (uint32_t*)(FLASH_APB_BASE_ADDR + flash_blk_st_addr);

    if (blk_val_table == NULL)
    {
        dlog_error("blk_val_table pointer is NULL\n");
        return FALSE;
    }

    for (i = 0; i < word_size; i++)
    {
        *write_addr = blk_val_table[i];
        write_addr++;
    }
    
    return ret;
}

uint8_t QUAD_SPI_ReadByte(uint32_t flash_addr, uint8_t* value_ptr)
{
    uint8_t ret = TRUE;

    volatile uint8_t* p = (uint8_t*)(FLASH_APB_BASE_ADDR + flash_addr);
    
    *value_ptr = *p;
            
    return ret;
}

uint8_t QUAD_SPI_ReadHalfWord(uint32_t flash_addr, uint16_t* value_ptr)
{
    uint8_t ret = TRUE;

    volatile uint16_t* p = (uint16_t*)(FLASH_APB_BASE_ADDR + flash_addr);
    
    *value_ptr = *p;

    return ret;
}

uint8_t QUAD_SPI_ReadWord(uint32_t flash_addr, uint32_t* value_ptr)
{
    uint8_t ret = TRUE;

    volatile uint32_t* p = (uint32_t*)(FLASH_APB_BASE_ADDR + flash_addr);
    
    *value_ptr = *p;
            
    return ret;
}

void QUAD_SPI_ReadBlockByByte(uint32_t flash_blk_st_addr, uint8_t* blk_val_table, uint32_t byte_size)
{
    uint32_t i;
    volatile uint8_t* write_addr = (uint8_t*)(FLASH_APB_BASE_ADDR + flash_blk_st_addr);

    if (blk_val_table == NULL)
    {
        dlog_error("blk_val_table pointer is NULL\n");
        return ;
    }

    for (i = 0; i < byte_size; i++)
    {
        blk_val_table[i] = *write_addr;
        write_addr++;
    }
    
}
uint8_t QUAD_SPI_SetSpeed(ENUM_QUAD_SPI_SPEED speed)
{
    uint8_t ret = TRUE;
   
    uint32_t mask_val = 0;
    uint32_t mask_bit = BIT(2) | BIT(3) | BIT(7);

    switch(speed)
    {
    case QUAD_SPI_SPEED_25M:
        mask_val = 0;
        break;
    case QUAD_SPI_SPEED_50M:
        mask_val = BIT(2);
        break;
    case QUAD_SPI_SPEED_100M:
        mask_val = BIT(3);
        break;
    case QUAD_SPI_SPEED_25M_ENCRYPT:
        mask_val = BIT(7);
        break;
    case QUAD_SPI_SPEED_50M_ENCRYPT:
        mask_val = BIT(2) | BIT(7);
        break;
    case QUAD_SPI_SPEED_100M_ENCRYPT:
        mask_val = BIT(3) | BIT(7);
        break;
    default:
        ret = FALSE;
        dlog_warning("The quad spi speed is not supported!\n");
        break;
     }

    Reg_Write32_Mask(HP_SPI_BASE_ADDR + HP_SPI_CONFIG_REG, mask_val, mask_bit);
    
    return ret;
}

uint8_t QUAD_SPI_UpdateInstruct(ENUM_QUAD_SPI_INSTR_ID instr_id, uint32_t cmd_h, uint32_t cmd_l)
{
    if (instr_id > QUAD_SPI_INSTR_UNKNOWN)
    {
        dlog_info("Quad SPI instruct ID to be updated is not right!");
        return FALSE;
    }
    
    uint32_t instr_h_offset = INSTR0_H + (instr_id * 2 * 4);
    uint32_t instr_l_offset = INSTR0_L + (instr_id * 2 * 4);
    Reg_Write32(HP_SPI_BASE_ADDR + instr_h_offset, cmd_h);
    Reg_Write32(HP_SPI_BASE_ADDR + instr_l_offset, cmd_l);

    return TRUE;
}

void QUAD_SPI_ReadProductID(uint8_t* data_buf)
{
    uint32_t i = 0;
    uint64_t u64_id = 0;
    u64_id = Reg_Read64(HP_SPI_BASE_ADDR + CMD7);
    for (i=0;i<8;i++)
    {
        data_buf[i] = (u64_id>>i*8)&0xff;
    }
}

