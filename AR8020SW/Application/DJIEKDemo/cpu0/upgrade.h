#ifndef UPGRADE_H
#define UPGRADE_H

#define RDWR_SECTOR_SIZE            (1024*4)

#define IMAGE_HAER_OFSET            0x22
#define DATEY_OFFSET                0x21
#define DATEm_OFFSET                0x2F
#define DATEd_OFFSET                0x2E
#define DATEH_OFFSET                0x2D
#define DATEM_OFFSET                0x2C
#define DATES_OFFSET                0x2B
#define LOCALADDR_OFFSET            0x2A
#define VERSION_MAJOR_OFFSET        0x26
#define VERSION_MINOR_OFFSET        0x25
#define SIZE4_OFFSET                0x24
#define SIZE3_OFFSET                0x23
#define SIZE2_OFFSET                0x22
#define SIZE1_OFFSET                0x21

typedef struct
{
    unsigned char present_boot;
    unsigned char success_boot;
    unsigned int  bootloadaddress;
    unsigned int  apploadaddress;
}Boot_Info;

#define GET_WORD_BOOT_INOF(any_addr) ((((uint32_t)(*any_addr)) << 24) | \
                                         (((uint32_t)(*(any_addr+1))) << 16) | \
                                         (((uint32_t)(*(any_addr+2))) << 8) | \
                                         ((uint32_t)((*(any_addr+3)))))

#define GET_WORD_FROM_ANY_ADDR(any_addr) ((uint32_t)(*any_addr) | \
                                         (((uint32_t)(*(any_addr+1))) << 8) | \
                                         (((uint32_t)(*(any_addr+2))) << 16) | \
                                         ((uint32_t)((*(any_addr+3))) << 24))

void UPGRADE_Upgrade(void const *argument);

#endif 
