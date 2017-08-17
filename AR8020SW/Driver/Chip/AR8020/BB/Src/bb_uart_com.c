#include <stdint.h>
#include <string.h>
#include "bb_uart_com.h"
#include "debuglog.h"
#include "serial.h"
#include "interrupt.h"
#include "memory_config.h"
#include "lock.h"
#include "bb_ctrl_internal.h"
#include "cpu_info.h"


static uint8_t header[] = {0xFF, 0x5A, 0xA5};
static STRU_BBUartComSession g_BBUARTComSessionArray[BB_UART_COM_SESSION_MAX] = {0};
static uint8_t g_BBUARTComSession0RxBuffer[128] = {0};
static STRU_BBUartComTxQueue *g_pstBBUartComTxQueue[BB_UART_SESSION_PRIORITY_MAX];
static uint8_t g_BBUartDataBuf[BBCOM_UART_RX_BUF_SIZE + 1];
static STRU_BBUartComTxFIFO  g_BBUartTxFIFO;


static void BB_UARTComLockAccquire(void)
{
    lock_type * lock_p = (lock_type*)SRAM_MODULE_LOCK_BB_UART_MUTEX_FLAG;
    Lock(lock_p);
}

static void BB_UARTComLockRelease(void)
{
    lock_type * lock_p = (lock_type*)SRAM_MODULE_LOCK_BB_UART_MUTEX_FLAG;
    UnLock(lock_p);
}

uint8_t get_session_eventid(uint8_t id, uint32_t *pu32_rcv_event)
{
    uint8_t ret = 0;
    uint32_t u32_rcv_event;

    if ( id == 0 )
    {
        u32_rcv_event = SYS_EVENT_ID_UART_DATA_RCV_SESSION0;
        ret = 1;
    }
    else if ( id == 1 )
    {
        u32_rcv_event = SYS_EVENT_ID_UART_DATA_RCV_SESSION1;
        ret = 1;
    }
    else if ( id == 2)
    {
        u32_rcv_event = SYS_EVENT_ID_UART_DATA_RCV_SESSION2;
        ret = 1;
    }
    else if ( id == 3)
    {
        u32_rcv_event = SYS_EVENT_ID_UART_DATA_RCV_SESSION3;
        ret = 1;
    }
    else if( id == 4)
    {
        u32_rcv_event = SYS_EVENT_ID_UART_DATA_RCV_SESSION4;
        ret = 1;        
    }
    
    if (ret && pu32_rcv_event )
    {
        *pu32_rcv_event = u32_rcv_event;
    }

    return ret;
}


static void BB_UARTComWriteSessionRxBuffer(ENUM_BBUARTCOMSESSIONID session_id, uint8_t* data_buf, uint32_t data_size)
{
    if (g_BBUARTComSessionArray[session_id].rx_buf->header.in_use == 1)
    {
        uint32_t wr_pos = g_BBUARTComSessionArray[session_id].rx_buf->header.rx_buf_wr_pos;
        uint32_t rd_pos = g_BBUARTComSessionArray[session_id].rx_buf->header.rx_buf_rd_pos;
        uint32_t tail_pos = g_BBUARTComSessionArray[session_id].data_max_size;

        uint32_t cnt = 0;
        do
        {
            g_BBUARTComSessionArray[session_id].rx_buf->data[wr_pos] = data_buf[cnt];
            wr_pos++;
            cnt++;

            if (wr_pos >= tail_pos)
            {
                wr_pos = 0;
            }
        } while ((wr_pos != rd_pos) && (cnt < data_size));

        if (wr_pos == rd_pos)
        {
            dlog_warning("FIFO is full!");
        }

        g_BBUARTComSessionArray[session_id].rx_buf->header.rx_buf_wr_pos = wr_pos;

        //notify the session
        {
            uint32_t u32_rcv_event;
            if ( get_session_eventid(session_id, &u32_rcv_event) )
            {
                SYS_EVENT_Notify_From_ISR(u32_rcv_event, NULL);                
            }
        }
    }
}


uint8_t BB_UARTComFindHeader(uint8_t u8_data,
                             uint16_t *u16_dataLen,
                             STRU_BBUartComSessionDataInfo *session_info)
{
    static uint8_t          rx_state = BB_UART_COM_RX_HEADER;
    static uint8_t          header_buf[BBCOM_UART_SESSION_DATA_HEADER_SIZE];
    static uint8_t          header_buf_index = 0;
    static uint8_t          data_sequence_num[BB_UART_SESSION_PRIORITY_MAX] = {0, 0};
    static uint8_t          data_length_index = 0;
    static uint16_t         data_length = 0;
    uint8_t                 check_sum = 0;
    uint8_t                 i;
    STRU_BBUartComSessionDataInfo           un_session_data_info;
    static ENUM_BBUARTCOMSESSIONID          session_id = 0;
    static ENUM_BB_UART_SESSION_PRIORITY    session_priority;
    static ENUM_BB_UART_SESSION_DATA_TYPE   session_data_type;
    uint8_t                 ret_value = 0;

    switch (rx_state)
    {
    case BB_UART_COM_RX_HEADER:
        if (u8_data == header[0])    // Reset flag
        {
            header_buf_index = 0;
            header_buf[header_buf_index++] = u8_data;
        }
        else if (header_buf_index < sizeof(header))    // Get header
        {
            header_buf[header_buf_index++] = u8_data;

            if ((header_buf_index == sizeof(header)) && (memcmp((void *)header, (void *)header_buf, sizeof(header)) == 0))
            {
                rx_state = BB_UART_COM_RX_SESSION_INFO;
            }
        }

        break;

    case BB_UART_COM_RX_SESSION_INFO:
        header_buf[header_buf_index++] = u8_data;
        un_session_data_info.u8_info   = u8_data;

        session_id = un_session_data_info.b.sessionNum;
        session_priority = un_session_data_info.b.priority;
        session_data_type = un_session_data_info.b.dataType;

        rx_state = BB_UART_COM_RX_SEQUENCE_NUM;

        break;

    case BB_UART_COM_RX_SEQUENCE_NUM:
        header_buf[header_buf_index] = u8_data;
    
        /* continuity check */
        if (header_buf[header_buf_index] != (uint8_t)(data_sequence_num[session_priority] + 1))
        {
            dlog_error("sequence not continuous, last: %d, this is: %d", 
                        data_sequence_num[session_priority],
                        header_buf[header_buf_index]);
        }
    
        data_sequence_num[session_priority] = header_buf[header_buf_index];
    
        header_buf_index++;
    
        rx_state = BB_UART_COM_RX_DATALENGTH;
    
        break;

    case BB_UART_COM_RX_DATALENGTH:
        header_buf[header_buf_index++] = u8_data;

        data_length_index++;
        if (data_length_index >= sizeof(uint16_t))
        {
            data_length_index = 0;

            data_length = (header_buf[header_buf_index-1] << 8) + header_buf[header_buf_index-2];

            if (data_length <= BBCOM_UART_RX_BUF_SIZE)
            {
                rx_state = BB_UART_COM_RX_HEADER_CHECKSUM;
            }
            else
            {
                header_buf_index = 0;
                rx_state = BB_UART_COM_RX_HEADER;

                dlog_error("BBCom RX data length is too long > %d !", BBCOM_UART_RX_BUF_SIZE);
            }
        }

        break;

    case BB_UART_COM_RX_HEADER_CHECKSUM:

        check_sum = 0;

        for (i = 0; i < header_buf_index; i++)
        {
            check_sum += header_buf[i];
        }

        if (check_sum == u8_data)
        {
            *u16_dataLen   = data_length;
            session_info->b.dataType    = session_data_type;
            session_info->b.sessionNum  = session_id;
            session_info->b.priority    = session_priority;

            ret_value = 1;
        }

        rx_state = BB_UART_COM_RX_HEADER;
        header_buf_index = 0;

        break;

    default:
        rx_state = BB_UART_COM_RX_HEADER;
        header_buf_index = 0;

        break;

    }

    return ret_value;
}


uint32_t BB_UARTComPacketDataAnalyze(uint8_t *u8_uartRxBuf, uint8_t u8_uartRxLen)
{
    uint16_t            i = 0;
    uint16_t            j = 0;
    uint8_t             chData = '\0';
    static uint8_t      find_header = 0;
    static uint8_t      receiving_data = 0;
    static uint16_t     data_buf_index = 0;
    static uint16_t     data_length = 0;
    uint8_t             check_sum = 0;
    STRU_BBUartComSessionDataInfo           un_session_data_info;
    static ENUM_BBUARTCOMSESSIONID          session_id;
    static ENUM_BB_UART_SESSION_PRIORITY    session_priority;
    static ENUM_BB_UART_SESSION_DATA_TYPE   session_data_type;

    while (u8_uartRxLen)
    {
        chData = *(u8_uartRxBuf + i);

        i++;
        u8_uartRxLen--;

        find_header = BB_UARTComFindHeader(chData, &data_length, &un_session_data_info);

        if (1 == find_header)
        {
            if (data_length > BBCOM_UART_RX_BUF_SIZE)
            {
                dlog_error("data length should not exceed: %d", BBCOM_UART_RX_BUF_SIZE);

                continue;
            }

            if (0 == receiving_data)
            {
                session_id          = un_session_data_info.b.sessionNum;
                session_priority    = un_session_data_info.b.priority;
                session_data_type   = un_session_data_info.b.dataType;

                /* begin to receive data */
                receiving_data  = 1;

                data_buf_index  = 0;
            }
            else
            {
                /* exception */
                dlog_error("find header while receiving user data");

                g_BBUartDataBuf[data_buf_index++] = chData;

                if (session_data_type == BB_UART_SESSION_DATA_RT)
                {
                    /* process data */
                    BB_UARTComWriteSessionRxBuffer(session_id, g_BBUartDataBuf, (data_buf_index - BBCOM_UART_SESSION_DATA_HEADER_SIZE));

                    receiving_data = 0;
                }
                else
                {
                    /* abandon data */
                }

                session_id          = un_session_data_info.b.sessionNum;
                session_priority    = un_session_data_info.b.priority;
                session_data_type   = un_session_data_info.b.dataType;

                data_buf_index  = 0;
            }
        }
        else
        {
            if (1 == receiving_data)
            {
                /* go on receiving data */
                g_BBUartDataBuf[data_buf_index++] = chData;

                /* user data all received */
                if (data_buf_index == (data_length + 1))
                {
                    if (session_data_type == BB_UART_SESSION_DATA_RT)
                    {
                        /* process data */
                        BB_UARTComWriteSessionRxBuffer(session_id, g_BBUartDataBuf, data_length);

                        #if 0
                        dlog_info("data_length: %d, session_id: %d, %d, %d",
                                    data_length,
                                    session_id,
                                    g_BBUartDataBuf[0],
                                    g_BBUartDataBuf[data_length - 1]);
                        #endif
                    }
                    else
                    {
                        check_sum = 0;
                        /* check sum */
                        for (j = 0; j < data_length; j++)
                        {
                            check_sum += g_BBUartDataBuf[j];
                        }

                        if (check_sum == g_BBUartDataBuf[data_length])
                        {
                            BB_UARTComWriteSessionRxBuffer(session_id, g_BBUartDataBuf, data_length);

                            #if 0
                            dlog_info("data_length: %d, session_id: %d, %d, %d",
                                        data_length,
                                        session_id,
                                        g_BBUartDataBuf[0],
                                        g_BBUartDataBuf[data_length - 1]);
                            #endif
                        }
                        else
                        {
                            dlog_error("user data checksum fail");
                        }
                    }

                    data_buf_index = 0;
                    receiving_data = 0;
                }
            }
        }
    }
}
#if 0
static void BB_UARTComUART10IRQHandler(uint32_t u32_vectorNum)
{
    char                 c;
    unsigned int         isrType;
		
    volatile uart_type   *uart_regs = (uart_type *)UART10_BASE;
    isrType    = uart_regs->IIR_FCR;


    /* receive data irq, try to get the data */
    if (UART_IIR_RECEIVEDATA == (isrType & 0xf))
    {
        
        int i = 14;
        while (i--)
        {
            c = uart_regs->RBR_THR_DLL;
            BB_UARTComPacketDataAnalyze(&c,1);
        }

    }

    if (UART_IIR_DATATIMEOUT == (isrType & 0xf))
    {        
        c = uart_regs->RBR_THR_DLL;
        BB_UARTComPacketDataAnalyze(&c,1);           

    }

    // TX empty interrupt.
    if (UART_IIR_THR_EMPTY == (isrType & 0xf))
    {       
        uart_putFifo(10);        
    }

}
#endif
void BB_UARTComInit(SYS_Event_Handler session0RcvDataHandler)
{
    *((lock_type*)(SRAM_MODULE_LOCK_BB_UART_MUTEX_FLAG)) = UNLOCK_STATE;
    *((volatile uint32_t*)(SRAM_MODULE_LOCK_BB_UART_INIT_FLAG)) = 0;

    uart_init(BBCOM_UART_INDEX, BBCOM_UART_BAUDRATE);
    reg_IrqHandle(VIDEO_UART10_INTR_VECTOR_NUM, UART_IntrSrvc, NULL);
    UART_RegisterUserRxHandler(BBCOM_UART_INDEX, BB_UARTComPacketDataAnalyze);
    INTR_NVIC_SetIRQPriority(VIDEO_UART10_INTR_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_VIDEO_UART10,0));
    INTR_NVIC_EnableIRQ(VIDEO_UART10_INTR_VECTOR_NUM);

    // Session 0 is registered by default
    g_BBUARTComSessionArray[0].rx_buf = (STRU_BBUartComSessionRxBuffer*)g_BBUARTComSession0RxBuffer;
    g_BBUARTComSessionArray[0].rx_buf->header.in_use = 1;
    g_BBUARTComSessionArray[0].rx_buf->header.rx_buf_wr_pos = 0;
    g_BBUARTComSessionArray[0].rx_buf->header.rx_buf_rd_pos = 0;
    g_BBUARTComSessionArray[0].rx_buf->header.cpu_id = ENUM_CPU2_ID;
    g_BBUARTComSessionArray[0].data_max_size = sizeof(g_BBUARTComSession0RxBuffer) - sizeof(STRU_BBUartComSessionRxBufferHeader);
    if ( session0RcvDataHandler )
    {
        SYS_EVENT_RegisterHandler(SYS_EVENT_ID_UART_DATA_RCV_SESSION0, session0RcvDataHandler);
    }

    // Sessions 1-4 are registered dynamicly
    g_BBUARTComSessionArray[1].rx_buf = (STRU_BBUartComSessionRxBuffer*)SRAM_BB_UART_COM_SESSION_1_SHARE_MEMORY_ST_ADDR;
    g_BBUARTComSessionArray[1].rx_buf->header.in_use = 0;
    g_BBUARTComSessionArray[1].rx_buf->header.rx_buf_wr_pos = 0;
    g_BBUARTComSessionArray[1].rx_buf->header.rx_buf_rd_pos = 0;
    g_BBUARTComSessionArray[1].rx_buf->header.cpu_id = ENUM_CPU0_ID;
    g_BBUARTComSessionArray[1].data_max_size = SRAM_BB_UART_COM_SESSION_1_SHARE_MEMORY_SIZE - sizeof(STRU_BBUartComSessionRxBufferHeader);

    g_BBUARTComSessionArray[2].rx_buf = (STRU_BBUartComSessionRxBuffer*)SRAM_BB_UART_COM_SESSION_2_SHARE_MEMORY_ST_ADDR;
    g_BBUARTComSessionArray[2].rx_buf->header.in_use = 0;
    g_BBUARTComSessionArray[2].rx_buf->header.rx_buf_wr_pos = 0;
    g_BBUARTComSessionArray[2].rx_buf->header.rx_buf_rd_pos = 0;
    g_BBUARTComSessionArray[2].rx_buf->header.cpu_id = ENUM_CPU0_ID;
    g_BBUARTComSessionArray[2].data_max_size = SRAM_BB_UART_COM_SESSION_2_SHARE_MEMORY_SIZE - sizeof(STRU_BBUartComSessionRxBufferHeader);

    g_BBUARTComSessionArray[3].rx_buf = (STRU_BBUartComSessionRxBuffer*)SRAM_BB_UART_COM_SESSION_3_SHARE_MEMORY_ST_ADDR;
    g_BBUARTComSessionArray[3].rx_buf->header.in_use = 0;
    g_BBUARTComSessionArray[3].rx_buf->header.rx_buf_wr_pos = 0;
    g_BBUARTComSessionArray[3].rx_buf->header.rx_buf_rd_pos = 0;
    g_BBUARTComSessionArray[3].rx_buf->header.cpu_id = ENUM_CPU0_ID;
    g_BBUARTComSessionArray[3].data_max_size = SRAM_BB_UART_COM_SESSION_3_SHARE_MEMORY_SIZE - sizeof(STRU_BBUartComSessionRxBufferHeader);

    g_BBUARTComSessionArray[4].rx_buf = (STRU_BBUartComSessionRxBuffer*)SRAM_BB_UART_COM_SESSION_4_SHARE_MEMORY_ST_ADDR;
    g_BBUARTComSessionArray[4].rx_buf->header.in_use = 0;
    g_BBUARTComSessionArray[4].rx_buf->header.rx_buf_wr_pos = 0;
    g_BBUARTComSessionArray[4].rx_buf->header.rx_buf_rd_pos = 0;
    g_BBUARTComSessionArray[4].rx_buf->header.cpu_id = ENUM_CPU0_ID;
    g_BBUARTComSessionArray[4].data_max_size = SRAM_BB_UART_COM_SESSION_4_SHARE_MEMORY_SIZE - sizeof(STRU_BBUartComSessionRxBufferHeader);

    g_pstBBUartComTxQueue[BB_UART_SESSION_PRIORITY_HIGH]
                = (STRU_BBUartComTxQueue *)SRAM_BB_UART_COM_TX_HIGH_PRIO_SHARE_MEMORY_ST_ADDR;
    g_pstBBUartComTxQueue[BB_UART_SESSION_PRIORITY_HIGH]->tx_queue_header.tx_buf_rd_pos
                = 0;
    g_pstBBUartComTxQueue[BB_UART_SESSION_PRIORITY_HIGH]->tx_queue_header.tx_buf_wr_pos
                = 0;
    g_pstBBUartComTxQueue[BB_UART_SESSION_PRIORITY_HIGH]->tx_queue_header.tx_buff_max_size
                = SRAM_BB_UART_COM_TX_HIGH_PRIO_SHARE_MEMORY_SIZE - sizeof(STRU_BB_UARTComTxQueueHeader);

    g_pstBBUartComTxQueue[BB_UART_SESSION_PRIORITY_LOW]
                = (STRU_BBUartComTxQueue *)SRAM_BB_UART_COM_TX_LOW_PRIO_SHARE_MEMORY_ST_ADDR;
    g_pstBBUartComTxQueue[BB_UART_SESSION_PRIORITY_LOW]->tx_queue_header.tx_buf_rd_pos
                = 0;
    g_pstBBUartComTxQueue[BB_UART_SESSION_PRIORITY_LOW]->tx_queue_header.tx_buf_wr_pos
                = 0;
    g_pstBBUartComTxQueue[BB_UART_SESSION_PRIORITY_LOW]->tx_queue_header.tx_buff_max_size
                = SRAM_BB_UART_COM_TX_LOW_PRIO_SHARE_MEMORY_SIZE - sizeof(STRU_BB_UARTComTxQueueHeader);

    g_BBUartTxFIFO.tx_fifo_rd_pos   = 0;
    g_BBUartTxFIFO.tx_fifo_wr_pos   = 0;

    *((volatile uint32_t*)(SRAM_MODULE_LOCK_BB_UART_INIT_FLAG)) = 0x10A5A501;
}

void BB_UARTComRemoteSessionInit(void)
{
    // Wait for the init finish
    while (*((volatile uint32_t*)(SRAM_MODULE_LOCK_BB_UART_INIT_FLAG)) != 0x10A5A501) { }
    
    g_BBUARTComSessionArray[0].rx_buf = (STRU_BBUartComSessionRxBuffer*)g_BBUARTComSession0RxBuffer;
    g_BBUARTComSessionArray[0].rx_buf->header.in_use = 1;
    g_BBUARTComSessionArray[1].rx_buf = (STRU_BBUartComSessionRxBuffer*)SRAM_BB_UART_COM_SESSION_1_SHARE_MEMORY_ST_ADDR;
    g_BBUARTComSessionArray[1].data_max_size = SRAM_BB_UART_COM_SESSION_1_SHARE_MEMORY_SIZE - sizeof(STRU_BBUartComSessionRxBufferHeader);
    g_BBUARTComSessionArray[2].rx_buf = (STRU_BBUartComSessionRxBuffer*)SRAM_BB_UART_COM_SESSION_2_SHARE_MEMORY_ST_ADDR;
    g_BBUARTComSessionArray[2].data_max_size = SRAM_BB_UART_COM_SESSION_2_SHARE_MEMORY_SIZE - sizeof(STRU_BBUartComSessionRxBufferHeader);
    g_BBUARTComSessionArray[3].rx_buf = (STRU_BBUartComSessionRxBuffer*)SRAM_BB_UART_COM_SESSION_3_SHARE_MEMORY_ST_ADDR;
    g_BBUARTComSessionArray[3].data_max_size = SRAM_BB_UART_COM_SESSION_3_SHARE_MEMORY_SIZE - sizeof(STRU_BBUartComSessionRxBufferHeader);
    g_BBUARTComSessionArray[4].rx_buf = (STRU_BBUartComSessionRxBuffer*)SRAM_BB_UART_COM_SESSION_4_SHARE_MEMORY_ST_ADDR;
    g_BBUARTComSessionArray[4].data_max_size = SRAM_BB_UART_COM_SESSION_4_SHARE_MEMORY_SIZE - sizeof(STRU_BBUartComSessionRxBufferHeader);

    g_pstBBUartComTxQueue[BB_UART_SESSION_PRIORITY_HIGH]
            = (STRU_BBUartComTxQueue *)SRAM_BB_UART_COM_TX_HIGH_PRIO_SHARE_MEMORY_ST_ADDR;
    g_pstBBUartComTxQueue[BB_UART_SESSION_PRIORITY_LOW]
            = (STRU_BBUartComTxQueue *)SRAM_BB_UART_COM_TX_LOW_PRIO_SHARE_MEMORY_ST_ADDR;

}

uint8_t BB_UARTComRegisterSession(ENUM_BBUARTCOMSESSIONID session_id,
                                 ENUM_BB_UART_SESSION_PRIORITY session_priority,
                                 ENUM_BB_UART_SESSION_DATA_TYPE session_dataType)
{
    if ((session_id != BB_UART_COM_SESSION_0) && (session_id < BB_UART_COM_SESSION_MAX))
    {
        if (g_BBUARTComSessionArray[session_id].rx_buf->header.in_use == 0)
        {
            g_BBUARTComSessionArray[session_id].rx_buf->header.in_use = 1;
            g_BBUARTComSessionArray[session_id].rx_buf->header.rx_buf_wr_pos = 0;
            g_BBUARTComSessionArray[session_id].rx_buf->header.rx_buf_rd_pos = 0;
            g_BBUARTComSessionArray[session_id].rx_buf->header.cpu_id = CPUINFO_GetLocalCpuId();

            g_BBUARTComSessionArray[session_id].e_sessionDataType = session_dataType;
            g_BBUARTComSessionArray[session_id].e_sessionPriority = session_priority;

            return 1;
        }
        else
        {
            dlog_error("Session %d occupied", session_id);
            return 0;
        }
    }
}

void BB_UARTComUnRegisterSession(ENUM_BBUARTCOMSESSIONID session_id)
{
    if ((session_id != BB_UART_COM_SESSION_0) && (session_id < BB_UART_COM_SESSION_MAX))
    {
        g_BBUARTComSessionArray[session_id].rx_buf->header.in_use = 0;
        g_BBUARTComSessionArray[session_id].rx_buf->header.rx_buf_wr_pos = 0;
        g_BBUARTComSessionArray[session_id].rx_buf->header.rx_buf_rd_pos = 0;
    }
}


uint8_t BB_UARTComSendMsg(ENUM_BBUARTCOMSESSIONID session_id,
                            uint8_t* data_buf,
                            uint16_t length)
{
    uint8_t                         u8_checkSum;
    ENUM_BB_UART_SESSION_PRIORITY   e_sessionPriority;
    ENUM_BB_UART_SESSION_DATA_TYPE  e_sessionDataType;
    uint8_t                         headerBuff[BBCOM_UART_SESSION_DATA_HEADER_SIZE];
    uint8_t                         headerCount = 0;
    uint32_t                        u32_wrPos;
    uint16_t                        i;
    uint16_t                        j;
    static uint8_t                  s_u8SeqNum[BB_UART_SESSION_PRIORITY_MAX] = {0, 0};
    STRU_BBUartComSessionDataInfo   st_sessionDataInfo;

    if (data_buf == NULL)
    {
        return 0;
    }

    if (CPUINFO_GetLocalCpuId() != g_BBUARTComSessionArray[session_id].rx_buf->header.cpu_id)
    {
        dlog_error("cpu id not match: session id: %d, local cpu: %d, session cpu: %d",
                    session_id,
                    CPUINFO_GetLocalCpuId(),
                    g_BBUARTComSessionArray[session_id].rx_buf->header.cpu_id);

        return 0;
    }

    if ((session_id >= BB_UART_COM_SESSION_MAX)||
        (g_BBUARTComSessionArray[session_id].rx_buf->header.in_use == 0))
    {
        dlog_error("not in use: %d", session_id);

        return 0;
    }

    e_sessionPriority       = g_BBUARTComSessionArray[session_id].e_sessionPriority;

    if (BB_UARTComGetTXQueueFreeLength(e_sessionPriority) < ((length + BBCOM_UART_SESSION_DATA_HEADER_SIZE) + 1))
    {
        dlog_error("buffer not enough");

        BB_UARTComLockRelease();

        return 0;
    }

    /* header */
    for (i = 0; i < sizeof(header); i++)
    {
        headerBuff[headerCount] = header[i];

        headerCount++;
    }

    e_sessionDataType           = g_BBUARTComSessionArray[session_id].e_sessionDataType;

    /* session data type */
    /* session id */
    /* session priority */
    st_sessionDataInfo.b.dataType   = e_sessionDataType;
    st_sessionDataInfo.b.sessionNum = session_id;
    st_sessionDataInfo.b.priority   = e_sessionPriority;

    headerBuff[headerCount]     = st_sessionDataInfo.u8_info;
    headerCount++;

    /* seqNum */
    s_u8SeqNum[e_sessionPriority]++;
    headerBuff[headerCount]     = s_u8SeqNum[e_sessionPriority];
    headerCount++;

    /* data length */
    headerBuff[headerCount]     = (uint8_t)length;
    headerCount++;
    headerBuff[headerCount]     = (uint8_t)(length >> 8);
    headerCount++;

    u8_checkSum = 0;

    /* header checksum */
    for (i = 0; i < headerCount; i++)
    {
        u8_checkSum += headerBuff[i];
    }

    /* header checksum */
    headerBuff[headerCount] = u8_checkSum;

    headerCount++;

    BB_UARTComLockAccquire();

    u32_wrPos               = g_pstBBUartComTxQueue[e_sessionPriority]->tx_queue_header.tx_buf_wr_pos;

    /* insert header into tx queue */
    for (i = 0; i < headerCount; i++)
    {
        g_pstBBUartComTxQueue[e_sessionPriority]->tx_data[u32_wrPos] = headerBuff[i];

        u32_wrPos++;

        if (u32_wrPos >= g_pstBBUartComTxQueue[e_sessionPriority]->tx_queue_header.tx_buff_max_size)
        {
            u32_wrPos = 0;
        }
    }

    u8_checkSum     = 0;

    /* insert user data into tx queue */
    for (i = 0; i < length; i++)
    {
        g_pstBBUartComTxQueue[e_sessionPriority]->tx_data[u32_wrPos] = data_buf[i];

        u8_checkSum += data_buf[i];

        u32_wrPos++;

        if (u32_wrPos >= g_pstBBUartComTxQueue[e_sessionPriority]->tx_queue_header.tx_buff_max_size)
        {
            u32_wrPos = 0;
        }
    }

    g_pstBBUartComTxQueue[e_sessionPriority]->tx_data[u32_wrPos] = u8_checkSum;

    u32_wrPos++;

    if (u32_wrPos >= g_pstBBUartComTxQueue[e_sessionPriority]->tx_queue_header.tx_buff_max_size)
    {
        u32_wrPos = 0;
    }

    /* update write postion */
    g_pstBBUartComTxQueue[e_sessionPriority]->tx_queue_header.tx_buf_wr_pos = u32_wrPos;

    BB_UARTComLockRelease();

    return 1;
}


uint32_t BB_UARTComReceiveMsg(ENUM_BBUARTCOMSESSIONID session_id, uint8_t* data_buf, uint32_t length_max)
{
    if (g_BBUARTComSessionArray[session_id].rx_buf->header.in_use == 1)
    {
        uint32_t wr_pos = g_BBUARTComSessionArray[session_id].rx_buf->header.rx_buf_wr_pos;
        uint32_t rd_pos = g_BBUARTComSessionArray[session_id].rx_buf->header.rx_buf_rd_pos;
        uint32_t tail_pos = g_BBUARTComSessionArray[session_id].data_max_size;

        uint32_t cnt = 0;
        while (rd_pos != wr_pos)
        {
            data_buf[cnt] = g_BBUARTComSessionArray[session_id].rx_buf->data[rd_pos];
            rd_pos++;
            cnt++;

            if (rd_pos >= tail_pos)
            {
                rd_pos = 0;
            }

            if (cnt == length_max)
            {
                break;
            }
        }

        g_BBUARTComSessionArray[session_id].rx_buf->header.rx_buf_rd_pos = rd_pos;

        return cnt;
    }
}


uint16_t BB_UARTComGetMsgFromTXQueue(ENUM_BB_UART_SESSION_PRIORITY session_priority)
{
    STRU_BBUartComTxQueue  *pst_txQueue;
    uint32_t                read_pos;
    uint32_t                write_pos;
    uint32_t                max_size;
    uint32_t                current_length;
    uint16_t                read_size;
    uint32_t                i = 0;
    uint16_t                user_data_length;
    uint16_t                total_length;
    uint16_t                tx_fifo_free_length;

    pst_txQueue     = g_pstBBUartComTxQueue[session_priority];

    read_pos        = pst_txQueue->tx_queue_header.tx_buf_rd_pos;
    write_pos       = pst_txQueue->tx_queue_header.tx_buf_wr_pos;
    max_size        = pst_txQueue->tx_queue_header.tx_buff_max_size;

    current_length = BB_UARTComGetTxQueueCurrentLength(session_priority);

    if (current_length <= BBCOM_UART_SESSION_DATA_HEADER_SIZE)
    {
        return 0;
    }

    for (i = 0; i < (current_length-8); i++)
    {
        if ((pst_txQueue->tx_data[(read_pos+i)%max_size] == header[0])&&
            (pst_txQueue->tx_data[(read_pos+(i+1))%max_size] == header[1])&&
            (pst_txQueue->tx_data[(read_pos+(i+2))%max_size] == header[2]))
        {
            break;
        }
    }

    if (i >= 1)
    {
        dlog_error("have invalid data");
    }

    read_pos += i;

    if (read_pos >= max_size)
    {
        read_pos -= max_size;
    }

    if (i >= (current_length-8))
    {
        read_size = 0;
        dlog_error("no header found");
    }
    else
    {
        user_data_length   = (pst_txQueue->tx_data[(read_pos+6)%max_size]);
        user_data_length <<= 8;
        user_data_length  += (pst_txQueue->tx_data[(read_pos+5)%max_size]);

        /* header + userdata + userdataChechSum */
        total_length       = ((user_data_length + BBCOM_UART_SESSION_DATA_HEADER_SIZE) + 1);

        /* get free length of tx fifo */
        if (g_BBUartTxFIFO.tx_fifo_rd_pos <= g_BBUartTxFIFO.tx_fifo_wr_pos)
        {
            tx_fifo_free_length = (g_BBUartTxFIFO.tx_fifo_rd_pos + BBCOM_UART_TX_FIFO_SIZE) - g_BBUartTxFIFO.tx_fifo_wr_pos;
        }
        else
        {
            tx_fifo_free_length = g_BBUartTxFIFO.tx_fifo_rd_pos - g_BBUartTxFIFO.tx_fifo_wr_pos;
        }

        /* get tx fifo free length */
        if (tx_fifo_free_length > total_length)
        {
            /* insert msg into tx fifo */
            for (i = 0; i < total_length; i++)
            {
                g_BBUartTxFIFO.tx_fifo_data[g_BBUartTxFIFO.tx_fifo_wr_pos] = pst_txQueue->tx_data[read_pos];

                read_pos++;
                if (read_pos >= max_size)
                {
                    read_pos = 0;
                }

                g_BBUartTxFIFO.tx_fifo_wr_pos++;
                if (g_BBUartTxFIFO.tx_fifo_wr_pos >= BBCOM_UART_TX_FIFO_SIZE)
                {
                    g_BBUartTxFIFO.tx_fifo_wr_pos = 0;
                }
            }

            read_size = i;
        }
        else
        {
            read_size = 0;
        }
    }

    pst_txQueue->tx_queue_header.tx_buf_rd_pos = read_pos;

    return read_size;
}


void BB_UARTComFlushTXQueue(ENUM_BB_UART_SESSION_PRIORITY session_priority)
{
    STRU_BBUartComTxQueue  *pst_txQueue;

    pst_txQueue     = g_pstBBUartComTxQueue[session_priority];

    BB_UARTComLockAccquire();

    pst_txQueue->tx_queue_header.tx_buf_rd_pos = 0;
    pst_txQueue->tx_queue_header.tx_buf_wr_pos = 0;

    BB_UARTComLockRelease();
}


uint32_t BB_UARTComGetTXQueueFreeLength(ENUM_BB_UART_SESSION_PRIORITY e_sessionPriority)
{
    uint32_t                max_size;
    uint32_t                wr_pos;
    uint32_t                rd_pos;
    STRU_BBUartComTxQueue  *pst_txQueue;
    uint32_t                free_length;

    if (e_sessionPriority > BB_UART_SESSION_PRIORITY_MAX)
    {
        dlog_error("error of invalid priority");

        return 0;
    }

    pst_txQueue     = g_pstBBUartComTxQueue[e_sessionPriority];

    max_size        = pst_txQueue->tx_queue_header.tx_buff_max_size;
    wr_pos          = pst_txQueue->tx_queue_header.tx_buf_wr_pos;
    rd_pos          = pst_txQueue->tx_queue_header.tx_buf_rd_pos;

    if (wr_pos >= rd_pos)
    {
        free_length = ((max_size - wr_pos) + rd_pos);
    }
    else
    {
        free_length = rd_pos - wr_pos;
    }

    return free_length;
}


uint32_t BB_UARTComGetTxQueueCurrentLength(ENUM_BB_UART_SESSION_PRIORITY e_sessionPriority)
{
    uint32_t                max_size;
    uint32_t                wr_pos;
    uint32_t                rd_pos;
    STRU_BBUartComTxQueue  *pst_txQueue;
    uint32_t                current_length;

    if (e_sessionPriority > BB_UART_SESSION_PRIORITY_MAX)
    {
        dlog_error("error of invalid priority");

        return 0;
    }

    pst_txQueue     = g_pstBBUartComTxQueue[e_sessionPriority];

    max_size        = pst_txQueue->tx_queue_header.tx_buff_max_size;
    wr_pos          = pst_txQueue->tx_queue_header.tx_buf_wr_pos;
    rd_pos          = pst_txQueue->tx_queue_header.tx_buf_rd_pos;

    if (wr_pos >= rd_pos)
    {
        current_length = wr_pos - rd_pos;
    }
    else
    {
        current_length = (max_size - rd_pos) + wr_pos;
    }

    return current_length;
}


void BB_UARTComCycleMsgProcess(void)
{
    uint8_t         i;
    uint8_t         j;
    uint8_t         max_msg_count = 10;
    uint16_t        msg_length;

    /* get high priority message first, every 14ms, get 10 messages ultimately */
    for (i = max_msg_count; i > 0; i--)
    {
        msg_length = BB_UARTComGetMsgFromTXQueue(BB_UART_SESSION_PRIORITY_HIGH);

        if (0 == msg_length)
        {
            break;
        }
    }

    /* then get low priority message */
    for (j = 0; j < i; j++)
    {
        msg_length = BB_UARTComGetMsgFromTXQueue(BB_UART_SESSION_PRIORITY_LOW);

        if (0 == msg_length)
        {
            break;
        }
    }

    return;
}


void BB_UARTComCycleSendMsg(void)
{
    uint8_t             send_buff[BB_UART_TX_FIFO_QPSK_SIZE];
    uint16_t            send_size = 0;
    uint16_t            i;
    uint16_t            read_pos;
    uint16_t            write_pos;
    uint32_t            max_tx_size;
    uint16_t            size_align;

    if (0 != uart_checkoutFifoStatus(BBCOM_UART_INDEX))
    {
        dlog_error("bb uart fifo busy");
        return;
    }

    read_pos        = g_BBUartTxFIFO.tx_fifo_rd_pos;
    write_pos       = g_BBUartTxFIFO.tx_fifo_wr_pos;

    /* in sky, no need to consider RC rate */
    if (context.en_bbmode == BB_SKY_MODE)
    {
        max_tx_size = BB_UART_TX_FIFO_QPSK_SIZE;
    }
    else
    {
        max_tx_size = BB_UART_GetTxFifoMaxSize(BB_GRD_MODE);
    }

    for (i = 0; i < max_tx_size; i++)
    {
        if ((read_pos == write_pos))
        {
            break;
        }

        send_buff[i] = g_BBUartTxFIFO.tx_fifo_data[read_pos++];

        if (read_pos >= BBCOM_UART_TX_FIFO_SIZE)
        {
            read_pos = 0;
        }

        send_size++;
    }

    g_BBUartTxFIFO.tx_fifo_rd_pos   = read_pos;

    /* 16 byte aligned */
    if ((send_size > 0)&&
        ((send_size % 16) != 0))
    {
        size_align = send_size;
        size_align = 16 - (size_align % 16);
    
        for (i = 0; i < size_align; i++)
        {
            send_buff[send_size++] = 0;
        }
    }

    if (send_size != 0)
    {
        uart_putdata(BBCOM_UART_INDEX,  send_buff, send_size);
    }

    return;
}


/* only for sky */
uint16_t BB_UARTComGetBBFIFOGap(void)
{
    uint8_t         read_fifo_gap_enable;
    uint8_t         fifo_gap_high;
    uint8_t         fifo_gap_low;
    uint16_t        fifo_gap;

    read_fifo_gap_enable    = BB_ReadReg(PAGE0, 0x0C);
    read_fifo_gap_enable   |= 0x40;
    BB_WriteReg(PAGE0, 0x0C, read_fifo_gap_enable);

    fifo_gap_high           = BB_ReadReg(PAGE0, 0xF6);

    fifo_gap_low            = BB_ReadReg(PAGE0, 0xF7);

    fifo_gap                = (fifo_gap_high << 8) + fifo_gap_low;

    fifo_gap                = 2048 - fifo_gap;

    return fifo_gap;
}


uint32_t BB_UART_GetTxFifoMaxSize(ENUM_BB_MODE e_bb_mode)
{
    uint32_t      rc_rate = 0;

    rc_rate = BB_GetRcRate(e_bb_mode);

    if (rc_rate == 1)
    {
        return BB_UART_TX_FIFO_BPSK_SIZE;
    }
    else if (rc_rate == 2)
    {
        return BB_UART_TX_FIFO_QPSK_SIZE;
    }
    else
    {
        return BB_UART_TX_FIFO_BPSK_SIZE;
    }
}


#if 0
void BB_UARTComTestSend(void)
{
    uint8_t   buff[100];
    uint16_t  tx_size;
    uint16_t  tx_temp_size;
    uint16_t  i;

    tx_size = BB_UARTComGetMsgFromTXQueue(BB_UART_SESSION_PRIORITY_HIGH, buff);

    // Pad "0" when not aligned by 16 bytes
    tx_temp_size = tx_size;
    tx_temp_size = 16 - (tx_temp_size % 16);
    for (i = 0; i < tx_temp_size; i++)
    {
        buff[tx_size++] = 0;
    }

    dlog_info("tx_size: %d", tx_size);

    Uart_WaitTillIdle(BBCOM_UART_INDEX, tx_size);
    uart_putdata(BBCOM_UART_INDEX,  buff, tx_size);

    return;
}
#endif

