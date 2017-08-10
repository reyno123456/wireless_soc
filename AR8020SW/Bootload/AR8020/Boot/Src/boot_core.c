#include <stdint.h>
#include <string.h>
#include "boot_core.h"
#include "boot_serial.h"
#include "boot_md5.h"
/****************************************
boot App
*****************************************/

char  BOOT_HexToASCII(unsigned char  data_hex)
{ 
    char  ASCII_Data;
    ASCII_Data=data_hex & 0x0F;
    if(ASCII_Data<10) 
        ASCII_Data=ASCII_Data+0x30; //‘0--9’
    else  
        ASCII_Data=ASCII_Data+0x37; //‘A--F’
    return ASCII_Data;
}

void BOOT_HexGroupToString(unsigned int addr, unsigned int HexLength)
{
    
    uint8_t i=0;
    char OutStrBuffer[3];
    for(i=0;i<HexLength;i++)
    {
        
        OutStrBuffer[0]=BOOT_HexToASCII(((*((uint8_t*)(addr+i)))>>4)&0x0F);
        OutStrBuffer[1]=BOOT_HexToASCII((*((uint8_t*)(addr+i)))&0x0F);
        OutStrBuffer[2]='\0';
        uart_puts(0,OutStrBuffer);
    }    
}

void BOOT_PrintInfo(uint32_t u32_addr)
{
    uart_puts(0,"DATE:");    
    BOOT_HexGroupToString(u32_addr-DATEY_OFFSET, 2);
    uart_puts(0," ");
    BOOT_HexGroupToString(u32_addr-DATEm_OFFSET, 2);
    uart_puts(0," ");
    BOOT_HexGroupToString(u32_addr-DATEH_OFFSET, 1);
    uart_puts(0,":");
    BOOT_HexGroupToString(u32_addr-DATEM_OFFSET, 1);
    uart_puts(0,":");
    BOOT_HexGroupToString(u32_addr-DATES_OFFSET, 1);
    uart_puts(0,"\r\nVer:");
    BOOT_HexGroupToString(u32_addr-VERSION_MAJOR_OFFSET, 1);
    uart_puts(0,".");
    BOOT_HexGroupToString(u32_addr-VERSION_MINOR_OFFSET, 1);
    uart_puts(0,"\r\n");
    
}

void BOOT_BootApp(void)
{

    *((volatile uint32_t*)MCU1_CPU_WAIT) = 0;

    *((volatile uint32_t*)MCU2_CPU_WAIT) = 0;
    *((uint32_t*)MCU0_VECTOR_TABLE_REG) = 0;
    (*((void(*)())((*((uint32_t*)(ITCM0_START+4))))))();
}

void BOOT_StartBoot(uint8_t index, unsigned int address)
{
    uint8_t i=0;
    uint32_t u32_bootAddress=address;
    
    if((0 != index) && (1 != index))
    {
        uart_puts(0,"info err\r\n");
        for (i=1;i<6;i++)
        {
            if(((*((uint8_t*)(BOOT_ADDR0+i))) > (*((uint8_t*)(BOOT_ADDR1+i)))))
            {
                if((0xff !=(*((uint8_t*)(BOOT_ADDR0+i)))))
                {
                    u32_bootAddress=BOOT_ADDR0;
                    break;                    
                }
            }
            else if(((*((uint8_t*)(BOOT_ADDR0+i))) < (*((uint8_t*)(BOOT_ADDR1+i)))))
            {
                if((0xff !=(*((uint8_t*)(BOOT_ADDR1+i)))))
                {
                    u32_bootAddress=BOOT_ADDR1;
                    break; 
                }
                
            }
        }
        if(6 == i)
        {
            u32_bootAddress=BOOT_ADDR0;
        }
    }
         
    if(0 != BOOT_CheckUpgradeCode(u32_bootAddress))
    {
        uart_puts(0,"code err\r\n");
        
        if(BOOT_ADDR0 == u32_bootAddress)
        {
            u32_bootAddress=BOOT_ADDR1;
        }
        else
        {
            u32_bootAddress=BOOT_ADDR0;
        }

        if(0 != BOOT_CheckUpgradeCode(u32_bootAddress))
        {           
            uart_puts(0,"backup code err\r\n");
            return;
        }
        
        uart_puts(0,"boot previous BOOT\r\n");
    }    


    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wnonnull"    
    memcpy((void*)ITCM0_START, (void*)(u32_bootAddress+IMAGE_HAER_OFFSET), BOOT_SIZE);         
    #pragma GCC diagnostic pop
    *((volatile uint32_t*)(MCU0_VECTOR_TABLE_REG)) = ITCM0_START;
    (*((void(*)())((*((uint32_t*)(ITCM0_START+4))))))();
    
    
}

void BOOT_CopyFromNorToITCM(unsigned int address)
{
    
    uint8_t* cpu0_app_size_addr = (uint8_t*)address;
    uint32_t cpu0_app_size = GET_WORD_FROM_ANY_ADDR(cpu0_app_size_addr);
    uint32_t cpu0_app_start_addr = address + 4;

    uint8_t* cpu1_app_size_addr = (uint8_t*)(cpu0_app_start_addr + cpu0_app_size);
    uint32_t cpu1_app_size = GET_WORD_FROM_ANY_ADDR(cpu1_app_size_addr);
    uint32_t cpu1_app_start_addr = cpu0_app_start_addr + cpu0_app_size + 4;

    uint8_t* cpu2_app_size_addr = (uint8_t*)(cpu1_app_start_addr + cpu1_app_size);
    uint32_t cpu2_app_size = GET_WORD_FROM_ANY_ADDR(cpu2_app_size_addr);
    uint32_t cpu2_app_start_addr = cpu1_app_start_addr + cpu1_app_size + 4;

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wnonnull"
    memcpy((void*)ITCM0_START, (void*)cpu0_app_start_addr, cpu0_app_size);
    #pragma GCC diagnostic pop
    memcpy((void*)ITCM1_START, (void*)cpu1_app_start_addr, cpu1_app_size);
    memcpy((void*)ITCM2_START, (void*)cpu2_app_start_addr, cpu2_app_size);
}

uint8_t BOOT_CheckUpgradeCode(uint32_t u32_BaseAddr)
{
    uint8_t i=0;

    uint8_t md5_value[16];
    uint8_t u8_Amd5Sum[16];

    uint8_t *p8_data = (uint8_t *)(u32_BaseAddr+34);
    uint8_t* p8_tmpAddr = (uint8_t*)(u32_BaseAddr+14);
    uint32_t g_u32ImageSize = GET_WORD_FROM_ANY_ADDR(p8_tmpAddr);
    uint32_t u32_RecCountTmp=g_u32ImageSize-34;
    p8_tmpAddr = (uint8_t*)(u32_BaseAddr+18);
    for(i=0;i<16;i++)
    {
        u8_Amd5Sum[i]=*(p8_tmpAddr+i);
    } 
    

    MD5_CTX md5;
    MD5Init(&md5);

    for(i=0;i<((u32_RecCountTmp)/RDWR_SECTOR_SIZE);i++)
    {
        MD5Update(&md5, (p8_data+RDWR_SECTOR_SIZE*i), RDWR_SECTOR_SIZE);
    }
    if(0 != ((u32_RecCountTmp)%RDWR_SECTOR_SIZE))
    {
        MD5Update(&md5, (p8_data+RDWR_SECTOR_SIZE*i), (u32_RecCountTmp%RDWR_SECTOR_SIZE));
    }

    MD5Final(&md5, md5_value);

    for(i=0;i<16;i++)
    {
        if(md5_value[i] != u8_Amd5Sum[i])
        {           
            return 1;
            
        }
    }
    return 0;   
}
/*
uint8_t BOOT_Check(void)
{
    uint8_t i=0;

    uint8_t md5_value[16];

    MD5_CTX md5;
    MD5Init(&md5);
    uint8_t *p8_data = (uint8_t *)(0);
    for(i=0;i<((u32_RecCountTmp)/RDWR_SECTOR_SIZE);i++)
    {
        MD5Update(&md5, (p8_data+RDWR_SECTOR_SIZE*i), RDWR_SECTOR_SIZE);
    }
    if(0 != ((u32_RecCountTmp)%RDWR_SECTOR_SIZE))
    {
        MD5Update(&md5, (p8_data+RDWR_SECTOR_SIZE*i), (u32_RecCountTmp%RDWR_SECTOR_SIZE));
    }

    MD5Final(&md5, md5_value);
    BOOT_HexGroupToString((uint32_t)(&md5_value),16);
    return 0;
}
*/
