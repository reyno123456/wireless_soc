#include <stdint.h>
#include "sram.h"
#include "debuglog.h"
#include "usbd_def.h"
#include "usbd_hid.h"
#include "reg_rw.h"
#include "systicks.h"
#include "dma.h"
#include "cpu_info.h"

volatile uint32_t               sramReady0;
volatile uint32_t               sramReady1;
extern USBD_HandleTypeDef       USBD_Device[USBD_PORT_NUM];
volatile uint8_t                g_u8DataPathReverse = 0;
uint32_t                        g_TickRecord[2];
STRU_CHANNEL_PORT_CONFIG        g_stChannelPortConfig[SRAM_CHANNEL_NUM];
#ifdef ARCAST
uint8_t                         g_mp3DecodeBuff[SRAM_MP3_DECODE_BUFF_SIZE];
uint32_t                        g_mp3DecodeBuffRdPos = 0;
uint32_t                        g_mp3DecodeBuffWrPos = 0;
#endif

void SRAM_Ready0IRQHandler(uint32_t u32_vectorNum)
{
    uint8_t                *buff;
    uint32_t                dataLen;
    uint8_t                 u8_usbPortId;
    USBD_HandleTypeDef     *pdev;
    uint8_t                 u8_endPoint;

    u8_usbPortId            = g_stChannelPortConfig[0].u8_usbPort;
    pdev                    = &USBD_Device[u8_usbPortId];

    if (pdev->u8_videoDisplay == 0)
    {
        SRAM_Ready0Confirm();

        return;
    }

    buff                    = (uint8_t *)SRAM_BUFF_0_ADDRESS;

    dataLen                 = SRAM_DATA_VALID_LEN_0;
    dataLen                 = (dataLen << 2);

    u8_endPoint             = g_stChannelPortConfig[0].u8_usbEp;

    #ifdef ARCAST
    SRAM_InsertMp3Buffer(dataLen, buff);

    SRAM_Ready0Confirm();

    return;
    #endif

    if (USBD_OK != USBD_HID_SendReport(pdev, buff, dataLen, u8_endPoint))
    {
        dlog_error("HID0 Send Error!\n");

        SRAM_Ready0Confirm();
    }
    else
    {
        if (pdev->u8_connState != 2)
        {
            SRAM_Ready0Confirm();
        }
        else
        {
            g_TickRecord[0] = SysTicks_GetTickCount();
            sramReady0 = 1;
        }
    }
}


void SRAM_Ready1IRQHandler(uint32_t u32_vectorNum)
{
    uint8_t                *buff;
    uint32_t                dataLen;
    uint8_t                 u8_usbPortId;
    USBD_HandleTypeDef     *pdev;
    uint8_t                 u8_endPoint;

    u8_usbPortId            = g_stChannelPortConfig[1].u8_usbPort;
    pdev                    = &USBD_Device[u8_usbPortId];

    if (pdev->u8_videoDisplay == 0)
    {
        SRAM_Ready1Confirm();

        return;
    }

    buff                    = (uint8_t *)SRAM_BUFF_1_ADDRESS;

    dataLen                 = SRAM_DATA_VALID_LEN_1;
    dataLen                 = (dataLen << 2);

    u8_endPoint             = g_stChannelPortConfig[1].u8_usbEp;

    if (USBD_OK != USBD_HID_SendReport(pdev, buff, dataLen, u8_endPoint))
    {
        dlog_error("HID1 Send Error!\n");

        SRAM_Ready1Confirm();
    }
    else
    {
        if (pdev->u8_connState != 2)
        {
            SRAM_Ready1Confirm();
        }
        else
        {
            g_TickRecord[1] = SysTicks_GetTickCount();
            sramReady1 = 1;
        }
    }
}


void SRAM_Ready0Confirm(void)
{
    /* confirm to Baseband that the SRAM data has been processed, ready to receive new data */
    Reg_Write32(DMA_READY_0, 1);

    sramReady0 = 0;
}


void SRAM_Ready1Confirm(void)
{
    /* confirm to Baseband that the SRAM data has been processed, ready to receive new data */
    Reg_Write32(DMA_READY_1, 1);

    sramReady1 = 0;
}


void SRAM_GROUND_ReceiveVideoConfig(void)
{
    //uint8_t      temp;

    /* Base Band bypass data, directly write to SRAM */
    #if 0
    temp    = BB_SPI_ReadByte(PAGE1, 0x8d);
    temp   |= 0x40;
    BB_SPI_WriteByte(PAGE1, 0x8d, temp);

    BB_SPIWriteByte(PAGE2, 0x56, 0x06);
    #endif
    /* Threshold of usb_fifo in BaseBand */

    /* Set the start address of sram for bb bypass channel 0*/
    Reg_Write32(SRAM_WR_ADDR_OFFSET_0, SRAM_BB_BYPASS_OFFSET_0);

    /* Set the max num of SRAM_READY interrupt trigger signal */
    Reg_Write32(SRAM_WR_MAX_LEN_0, SRAM_DMA_READY_LEN);

    /* Set the start address of sram for bb bypass channel 1*/
    Reg_Write32(SRAM_WR_ADDR_OFFSET_1, SRAM_BB_BYPASS_OFFSET_1);

    /* Set the max num of SRAM_READY interrupt trigger signal */
    Reg_Write32(SRAM_WR_MAX_LEN_1, SRAM_DMA_READY_LEN);

}


void SRAM_SKY_EnableBypassVideoConfig(uint32_t channel)
{
    if (0 == channel)
    {
        Reg_Write32(SRAM_VIEW0_ENABLE_ADDR, SRAM_VIEW0_ENABLE);
    }
    else
    {
        Reg_Write32(SRAM_VIEW1_ENABLE_ADDR, SRAM_VIEW1_ENABLE);
    }

    Reg_Write32(SRAM_SKY_MASTER_ID_ADDR, SRAM_SKY_MASTER_ID_VALUE);

    Reg_Write32(SRAM_SKY_MASTER_ID_MASK_ADDR, SRAM_SKY_MASTER_ID_MASK_VALUE);
}


void SRAM_SKY_DisableBypassVideoConfig(uint32_t channel)
{
    uint32_t        regValue;

    if (0 == channel)
    {
        regValue    = Reg_Read32(SRAM_VIEW0_ENABLE_ADDR);
        regValue   &= ~SRAM_VIEW0_ENABLE;

        Reg_Write32(SRAM_VIEW0_ENABLE_ADDR, regValue);
    }
    else
    {
        regValue    = Reg_Read32(SRAM_VIEW1_ENABLE_ADDR);
        regValue   &= ~SRAM_VIEW1_ENABLE;

        Reg_Write32(SRAM_VIEW1_ENABLE_ADDR, regValue);
    }
}


void SRAM_CheckTimeout(void)
{
    if (sramReady0)
    {
        if ((SysTicks_GetDiff(g_TickRecord[0], SysTicks_GetTickCount())) >= SRAM_TIMEOUT_THRESHOLD)
        {
            dlog_error("channel 0 timeout");

            SRAM_Ready0Confirm();
        }
    }

    if (sramReady1)
    {
        if ((SysTicks_GetDiff(g_TickRecord[1], SysTicks_GetTickCount())) >= SRAM_TIMEOUT_THRESHOLD)
        {
            dlog_error("channel 1 timeout");

            SRAM_Ready1Confirm();
        }
    }
}


#ifdef ARCAST

uint32_t SRAM_GetMp3BufferLength(void)
{
    if (g_mp3DecodeBuffWrPos >= g_mp3DecodeBuffRdPos)
    {
        return g_mp3DecodeBuffWrPos - g_mp3DecodeBuffRdPos;
    }
    else
    {
        return ((SRAM_MP3_DECODE_BUFF_SIZE + g_mp3DecodeBuffWrPos) - g_mp3DecodeBuffRdPos);
    }
}


void SRAM_InsertMp3Buffer(uint32_t dataLen, uint8_t *data)
{
    uint32_t         src;
    uint32_t         dest;
    uint32_t         dataLenTemp;

    src     = (uint32_t)data;
    dest    = (uint32_t)(g_mp3DecodeBuff) + g_mp3DecodeBuffWrPos;

    if ((SRAM_MP3_DECODE_BUFF_SIZE - SRAM_GetMp3BufferLength()) < dataLen)
    {
        return;
    }

    if ((g_mp3DecodeBuffWrPos + dataLen) <= SRAM_MP3_DECODE_BUFF_SIZE)
    {
        memcpy((void *)dest, (void *)src, dataLen);

        g_mp3DecodeBuffWrPos    += dataLen;

        if (g_mp3DecodeBuffWrPos >= SRAM_MP3_DECODE_BUFF_SIZE)
        {
            g_mp3DecodeBuffWrPos -= SRAM_MP3_DECODE_BUFF_SIZE;
        }

    }
    else
    {
        dataLenTemp     = (SRAM_MP3_DECODE_BUFF_SIZE - g_mp3DecodeBuffWrPos);

        memcpy((void *)dest, (void *)src, dataLenTemp);

        src             = (uint32_t)data + dataLenTemp;
        dest            = (uint32_t)g_mp3DecodeBuff;
        dataLenTemp     = dataLen - dataLenTemp;

        memcpy((void *)dest, (void *)src, dataLenTemp);

        g_mp3DecodeBuffWrPos    = dataLenTemp;


        if (g_mp3DecodeBuffWrPos >= SRAM_MP3_DECODE_BUFF_SIZE)
        {
            g_mp3DecodeBuffWrPos -= SRAM_MP3_DECODE_BUFF_SIZE;
        }
    }

    return;
}


uint32_t SRAM_GetMp3Data(uint32_t dataLen, uint8_t *dataBuff)
{
    uint32_t        i;
    uint32_t        read_size = 0;

    //dlog_info("read  pos: %d", g_mp3DecodeBuffRdPos);
    /* ensure 4 bytes align */
    dataLen   -= (dataLen & 0x3);

    for (i = 0; i < dataLen; i+=4)
    {

        dataBuff[i]       = g_mp3DecodeBuff[g_mp3DecodeBuffRdPos+3];
        dataBuff[i+1]     = g_mp3DecodeBuff[g_mp3DecodeBuffRdPos+2];
        dataBuff[i+2]     = g_mp3DecodeBuff[g_mp3DecodeBuffRdPos+1];
        dataBuff[i+3]     = g_mp3DecodeBuff[g_mp3DecodeBuffRdPos];

        g_mp3DecodeBuffRdPos+=4;
		read_size+=4;
        if (g_mp3DecodeBuffRdPos >= SRAM_MP3_DECODE_BUFF_SIZE)
        {
            g_mp3DecodeBuffRdPos = 0;
        }
    }
    return read_size;
}


#endif



