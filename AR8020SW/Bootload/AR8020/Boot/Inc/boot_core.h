#ifndef BOOT_CORE_H
#define BOOT_CORE_H

#define RDWR_SECTOR_SIZE           (1024*4)

#define APPLICATION_IMAGE_START    0x10020000
#define APP_ADDR_OFFSET            (0x20000)

#define ITCM0_START                0x00000000
#define ITCM1_START                0x44100000
#define ITCM2_START                0xB0000000

#define MCU1_CPU_WAIT              0x40B000CC  /* ENABLE CPU1 */
#define MCU2_CPU_WAIT              0xA0030088  /* ENABLE CPU2 */
#define MCU0_VECTOR_TABLE_REG      0xE000ED08

#define GET_WORD_FROM_ANY_ADDR(any_addr) ((uint32_t)(*any_addr) | \
                                         (((uint32_t)(*(any_addr+1))) << 8) | \
                                         (((uint32_t)(*(any_addr+2))) << 16) | \
                                         ((uint32_t)((*(any_addr+3))) << 24))

#define INFO_BASE                  0x1000
#define BOOT_ADDR0                 0x10002000
#define BOOT_ADDR1                 0x10011000
#define BOOT_SIZE                  (1024*60)

#define IMAGE_HAER_OFFSET           0x22

#define DATEY_OFFSET               0x21
#define DATEm_OFFSET               0x1F
#define DATEd_OFFSET               0x1E
#define DATEH_OFFSET               0x1D
#define DATEM_OFFSET               0x1C
#define DATES_OFFSET               0x1B
#define LOCALADDR_OFFSET           0x1A
#define VERSION_MAJOR_OFFSET       0x16
#define VERSION_MINOR_OFFSET       0x15
#define SIZE4_OFFSET               0x14
#define SIZE3_OFFSET               0x13
#define SIZE2_OFFSET               0x12
#define SIZE1_OFFSET               0x11


typedef struct
{
    unsigned char present_boot;
    unsigned char success_boot;
    unsigned int  bootloadaddress;
    unsigned int  apploadaddress;
}Boot_Info;

void BOOT_BootApp(void);
void BOOT_CopyFromNorToITCM(unsigned int address);
void BOOT_StartBoot(uint8_t index,unsigned int address);
void BOOT_PrintInfo(uint32_t u32_addr);
uint8_t BOOT_CheckUpgradeCode(uint32_t u32_BaseAddr);
#endif