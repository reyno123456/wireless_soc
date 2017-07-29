#ifndef _TEST_HAL_SPI_FLASH_
#define _TEST_HAL_SPI_FLASH_



//#define W25Q64_ID   0xEF16 
#define W25Q128_ID  0xEF17 

#define MAX_TIEM_MS     (5)


/*====================================================*/
/*      WinBond SPI FLash Instruction Definition      */
/*====================================================*/
#define W25Q_WriteEnable        0x06
#define W25Q_WriteDisable       0x04
#define W25Q_ReadStatusReg      0x05
#define W25Q_WriteStatusReg     0x01
#define W25Q_ReadData           0x03
#define W25Q_PageProgram        0x02
#define W25Q_Sector4kErase      0x20        //4KB
#define W25Q_Block32kErase      0x52        //32KB
#define W25Q_Block64kErase      0xD8        //64KB
#define W25Q_ChipErase          0xC7
#define W25Q_PowerDown          0xB9
#define W25Q_ReleasePowerDown   0xAB
#define W25Q_DeviceID           0xAB
#define W25Q_ManufactDeviceID   0x90

/*====================================================*/
/*      WinBond SPI FLash  Functions  Declaration     */
/*====================================================*/
void command_WbFlashID(char *SPI_BASE_ADDR);
void command_TestWbBlockErase(char *SPI_BASE_ADDR, char *cmd, char *addr);
void command_TestWbFlash(char *SPI_BASE_ADDR);
void command_TestWbFlashWrite(char *spi_base, char *addr, char *len);
void command_TestWbFlashRead(char *spi_base, char *addr, char *len);
void command_TestSetWbFlashClk(char *clk);
void command_TestWbFlashWrite_loop(char *spi_base, char *addr, char *len);


#endif
