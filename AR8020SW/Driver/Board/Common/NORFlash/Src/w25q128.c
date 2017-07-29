#include <stddef.h>
#include <stdint.h>
#include "quad_spi_ctrl.h"
#include "w25q128.h"
#include "debuglog.h"
#include "mpu.h"

void W25Q128_Init(void)
{
    MPU_QuadspiProtectDisable();
    //05h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_0, 0x001c14, 0x700);
    //35h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_1, 0x001cd4, 0x700);
    //15h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_2, 0x001c54, 0x700);
    //04h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_3, 0x609c10, 0x0);
    //01h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_4, 0x609c04, 0x700);
    //31h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_5, 0x609cc4, 0x700);
    //11h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_6, 0x609c44, 0x700);
    //C7h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_9, 0x409f1c, 0x0);
    //06h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_16, 0x609c18, 0x0);
    //50h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_17, 0x609d40, 0x0);
    //20h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_20, 0x609c80, 0x17);
    //D8h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_21, 0x609f60, 0x17);
    //4Bh
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_7, 0x601D2C, 0xf83f00);

    MPU_QuadspiProtectEnable();
}

