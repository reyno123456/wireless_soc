#include <stddef.h>
#include <stdint.h>
#include "data_type.h"
#include "quad_spi_ctrl.h"
#include "w25q128.h"
#include "nor_flash.h"
#include "debuglog.h"
#include "mpu.h"

uint8_t NOR_FLASH_Init(void)
{
#ifdef USE_WINBOND_SPI_NOR_FLASH
    W25Q128_Init();    
#endif
}

uint8_t NOR_FLASH_EraseSector(uint32_t flash_start_addr)
{
    uint32_t sector_size = 0x1000;

#ifdef USE_WINBOND_SPI_NOR_FLASH
    sector_size = W25Q128_SECTOR_SIZE;
#endif
    
    if ((flash_start_addr % sector_size) != 0)
    {
        dlog_error("The w25q128 sector erase address is not sector aligned!");
        return FALSE;
    }

    MPU_QuadspiProtectDisable();
    QUAD_SPI_WriteEnable();
    QUAD_SPI_CheckBusy();
    QUAD_SPI_SectorErase(flash_start_addr);
    QUAD_SPI_CheckBusy();
    QUAD_SPI_WriteDisable();
    MPU_QuadspiProtectEnable();
    
    return TRUE;
}

uint8_t NOR_FLASH_EraseBlock(uint32_t flash_start_addr)
{
    uint32_t sector_size = 0x10000;

#ifdef USE_WINBOND_SPI_NOR_FLASH
    sector_size = W25Q128_BLOCK_SIZE;
#endif
    
    if ((flash_start_addr % sector_size) != 0)
    {
        dlog_error("The w25q128 block erase address is not block aligned!");
        return FALSE;
    }

    MPU_QuadspiProtectDisable();
    QUAD_SPI_WriteEnable();
    QUAD_SPI_CheckBusy();
    QUAD_SPI_BlockErase(flash_start_addr);
    QUAD_SPI_CheckBusy();
    QUAD_SPI_WriteDisable();
    MPU_QuadspiProtectEnable();

    return TRUE;
}

uint8_t NOR_FLASH_EraseChip(void)
{
    MPU_QuadspiProtectDisable();
    QUAD_SPI_WriteEnable();
    QUAD_SPI_CheckBusy();
    QUAD_SPI_ChipErase();
    QUAD_SPI_CheckBusy();
    QUAD_SPI_WriteDisable();
    MPU_QuadspiProtectEnable();

    return TRUE;
}

void NOR_FLASH_WriteByteBuffer(uint32_t start_addr, uint8_t* data_buf, uint32_t size)
{
    uint32_t i;

    MPU_QuadspiProtectDisable();
    QUAD_SPI_WriteEnable();
    QUAD_SPI_CheckBusy();

    QUAD_SPI_WriteBlockByByte(start_addr, data_buf, size);

    QUAD_SPI_CheckBusy();
    QUAD_SPI_WriteDisable();
    MPU_QuadspiProtectEnable();
}

void NOR_FLASH_WriteHalfWordBuffer(uint32_t start_addr, uint16_t* data_buf, uint32_t size)
{
    uint32_t i;

    MPU_QuadspiProtectDisable();
    QUAD_SPI_WriteEnable();
    QUAD_SPI_CheckBusy();

    QUAD_SPI_WriteBlockByHalfWord(start_addr, data_buf, size);

    QUAD_SPI_CheckBusy();
    QUAD_SPI_WriteDisable();
    MPU_QuadspiProtectEnable();
}

void NOR_FLASH_WriteWordBuffer(uint32_t start_addr, uint32_t* data_buf, uint32_t size)
{
    uint32_t i;

    MPU_QuadspiProtectDisable();
    QUAD_SPI_WriteEnable();
    QUAD_SPI_CheckBusy();

    QUAD_SPI_WriteBlockByWord(start_addr, data_buf, size);

    QUAD_SPI_CheckBusy();
    QUAD_SPI_WriteDisable();
    MPU_QuadspiProtectEnable();
}

void NOR_FLASH_ReadByteBuffer(uint32_t start_addr, uint8_t* data_buf, uint32_t size)
{

    QUAD_SPI_ReadBlockByByte(start_addr, data_buf, size);
}

void NOR_FLASH_ReadProductID(uint8_t* data_buf)
{
    QUAD_SPI_ReadProductID(data_buf);

}
