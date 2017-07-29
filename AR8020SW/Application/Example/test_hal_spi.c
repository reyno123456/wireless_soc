#include "test_hal_spi.h"
#include "hal_gpio.h"

extern unsigned long strtoul(const char *cp, char **endp, unsigned int base);

void command_TestHalSpiInit(unsigned char *ch, unsigned char *br, unsigned char *polarity, unsigned char *phase)
{
    STRU_HAL_SPI_INIT st_spiInitInfo;
    unsigned int u32_ch = strtoul(ch, NULL, 0);
    unsigned int u32_br = strtoul(br, NULL, 0);
    unsigned int u32_polarity = strtoul(polarity, NULL, 0);
    unsigned int u32_phase = strtoul(phase, NULL, 0);
    
    st_spiInitInfo.u16_halSpiBaudr = (uint16_t)(u32_br);
    st_spiInitInfo.e_halSpiPolarity = (ENUM_HAL_SPI_POLARITY)(u32_polarity);
    st_spiInitInfo.e_halSpiPhase = (ENUM_HAL_SPI_PHASE)(u32_phase);

    HAL_SPI_MasterInit((ENUM_HAL_SPI_COMPONENT)(u32_ch), 
                        &st_spiInitInfo);

    if (5 == u32_ch)
    {
        HAL_GPIO_SetMode(HAL_GPIO_NUM50,HAL_GPIO_PIN_MODE1);
        HAL_GPIO_SetMode(HAL_GPIO_NUM51,HAL_GPIO_PIN_MODE1);
        HAL_GPIO_SetMode(HAL_GPIO_NUM52,HAL_GPIO_PIN_MODE1);
        HAL_GPIO_SetMode(HAL_GPIO_NUM53,HAL_GPIO_PIN_MODE1);
    }
}

void command_TestHalSpiTx(unsigned char *ch,  unsigned char *addr, unsigned char *wdata)
{
    unsigned char u8_txData[3] = {0x0E,0x00,0x00};
    unsigned int u32_ch = strtoul(ch, NULL, 0);
    unsigned int u32_addr = strtoul(addr, NULL, 0);
    unsigned int u32_wdata = strtoul(wdata, NULL, 0);

    u8_txData[1] = (uint8_t)u32_addr;
    u8_txData[2] = (uint8_t)u32_wdata;
    dlog_info("u8_txData:0x%x 0x%x 0x%x",u8_txData[0],u8_txData[1],u8_txData[2]);

    HAL_SPI_MasterWriteRead((ENUM_HAL_SPI_COMPONENT)(u32_ch), 
                             u8_txData,
                             3,
                             NULL,
                             0,
                             2);
}

void command_TestHalSpiRx(unsigned char *ch, unsigned char *addr)
{
    unsigned char u8_txData[3] = {0x0F,0x00,0x00};
    unsigned char u8_rxData[3] = {0x00,0x00,0x00};
    unsigned int u32_ch = strtoul(ch, NULL, 0);
    unsigned int u32_addr = strtoul(addr, NULL, 0);

    u8_txData[1] = (uint8_t)u32_addr;

    HAL_SPI_MasterWriteRead((ENUM_HAL_SPI_COMPONENT)(u32_ch), 
                             u8_txData,
                             3,
                             u8_rxData,
                             3,
                             2);
   
    dlog_info("u8_rxData:0x%x 0x%x 0x%x",u8_rxData[0],u8_rxData[1],u8_rxData[2]);
 
}
