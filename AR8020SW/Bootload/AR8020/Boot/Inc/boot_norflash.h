#ifndef BOOT_NORFLASH_H
#define BOOT_NORFLASH_H

//--------------------------------------------------------------------
// memory address space define here
//--------------------------------------------------------------------
#define HP_SPI_BASE_ADDR    0x40C00000
#define FLASH_APB_BASE_ADDR 0x10000000

#define HP_SPI_CONFIG_REG       ( 112 )*4
//-------------------------------------------------------------------
// spansion instruction address mapping
//-------------------------------------------------------------------
#define CMD0                    (0*4)          //read SR1                      05h
#define CMD16                   (16*4)          //write enable                          06h
#define CMD20                   (20*4)          //sector erase(4KB)     20h
// Addr for cmd
#define CMD20_ADDR              (36*4)

#define BIT(n)    ((uint32_t)1 << (n))
#define TRUE  (1)
#define FALSE (0)

uint8_t QUAD_SPI_ReadByte(uint32_t flash_addr, uint8_t* value_ptr);
uint8_t QUAD_SPI_SetSpeed(void);
void QUAD_SPI_ReadBlockByByte(uint32_t flash_blk_st_addr, uint8_t* blk_val_table, uint32_t byte_size);
#endif

