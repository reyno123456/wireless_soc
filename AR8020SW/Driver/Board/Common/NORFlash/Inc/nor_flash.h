#ifndef NOR_FLASH_H
#define NOR_FLASH_H

uint8_t NOR_FLASH_Init(void);
uint8_t NOR_FLASH_EraseSector(uint32_t flash_start_addr);
uint8_t NOR_FLASH_EraseBlock(uint32_t flash_start_addr);
uint8_t NOR_FLASH_EraseChip(void);
void NOR_FLASH_WriteByteBuffer(uint32_t start_addr, uint8_t* data_buf, uint32_t size);
void NOR_FLASH_WriteHalfWordBuffer(uint32_t start_addr, uint16_t* data_buf, uint32_t size);
void NOR_FLASH_WriteWordBuffer(uint32_t start_addr, uint32_t* data_buf, uint32_t size);
void NOR_FLASH_ReadByteBuffer(uint32_t start_addr, uint8_t* data_buf, uint32_t size);
void NOR_FLASH_ReadProductID(uint8_t* data_buf);

#endif

