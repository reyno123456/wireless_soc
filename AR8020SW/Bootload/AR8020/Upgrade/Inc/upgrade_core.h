#ifndef UPGRADE_CORE_H
#define UPGRADE_CORE_H

#include "ff.h"

#define RDWR_SECTOR_SIZE           (1024*4) //or (1024*32)
#define SDRAM_INIT_DONE (*(volatile uint32_t *)0xA0030024)
#define SFR_TRX_MODE_SEL (*(volatile uint32_t *)0x40B00068)
#define SDRAM_INIT_DONE  (*(volatile uint32_t *)0xA0030024)
#define RECEIVE_ADDR (0x81000000)

#define APPLICATION_IMAGE_START    0x10020000
#define APP_ADDR_OFFSET            (0x20000)
#define ITCM0_START                0x00000000
#define ITCM1_START                0x44100000
#define ITCM2_START                0xB0000000
#define FLASH_BASE_ADDR            0x10000000

#define BOOT_ADDR0                 0x10002000
#define BOOT_ADDR1                 0x10011000

#define GET_WORD_FROM_ANY_ADDR(any_addr) ((uint32_t)(*any_addr) | \
                                         (((uint32_t)(*(any_addr+1))) << 8) | \
                                         (((uint32_t)(*(any_addr+2))) << 16) | \
                                         ((uint32_t)((*(any_addr+3))) << 24))

#define GET_WORD_BOOT_INOF(any_addr) ((((uint32_t)(*any_addr)) << 24) | \
                                         (((uint32_t)(*(any_addr+1))) << 16) | \
                                         (((uint32_t)(*(any_addr+2))) << 8) | \
                                         ((uint32_t)((*(any_addr+3)))))

enum updata_error
{
    UPGRADE_SUCCESS = 0,
    UPGRADE_READFILE= -1,
    UPGRADE_OPENFILE= -2

};
typedef struct
{
    unsigned char present_boot;
    unsigned char success_boot;
    unsigned int  bootloadaddress;
    unsigned int  apploadaddress;
}Boot_Info;

void UPGRADE_CopyFromNorToITCM(void);
int8_t UPGRADE_CopyDataToNor(FIL *MyFile,uint32_t u32_addrOffset);
int8_t UPGRADE_CopyDataToITCM(FIL *MyFile,uint32_t u32_TCMADDR);

#endif
