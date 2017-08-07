#ifndef BB_UARTCOM_H
#define BB_UARTCOM_H

#include "sys_event.h"
#include "bb_types.h"

#define BBCOM_UART_INDEX                    10
#define BBCOM_UART_BAUDRATE                 256000
#define BBCOM_UART_RX_BUF_SIZE              1024
#define BBCOM_UART_TX_FIFO_SIZE             1024
#define BB_UART_TX_FIFO_BPSK_SIZE           32
#define BB_UART_TX_FIFO_QPSK_SIZE           208


#define BBCOM_UART_SESSION_DATA_HEADER_SIZE 8


typedef enum
{
    BB_UART_COM_RX_HEADER = 0,
    BB_UART_COM_RX_SESSION_INFO,
    BB_UART_COM_RX_SEQUENCE_NUM,
    BB_UART_COM_RX_DATALENGTH,
    BB_UART_COM_RX_HEADER_CHECKSUM,
    BB_UART_COM_RX_DATABUFFER,
    BB_UART_COM_RX_CHECKSUM,
} ENUM_BBUartComRxState;


typedef struct
{
    volatile uint8_t    u8_magic[3];
    volatile uint8_t    u8_dataType;
    volatile uint8_t    u8_sessionNum;
    volatile uint16_t   u16_dataLen;
    volatile uint8_t    u8_headerCheckSum;
    volatile uint8_t    u8_userDataCheckSum;
    volatile uint8_t    u8_userData[1];
} STRU_BBUartComSessionData;


typedef union
{
    volatile uint8_t    u8_info;

    struct
    {
        volatile uint8_t    dataType:2;
        volatile uint8_t    sessionNum:3;
        volatile uint8_t    priority:1;
        volatile uint8_t    reserved:2;
    } b;
} STRU_BBUartComSessionDataInfo;


typedef struct
{
    volatile uint32_t in_use;
    volatile uint32_t rx_buf_wr_pos;
    volatile uint32_t rx_buf_rd_pos;
} STRU_BBUartComSessionRxBufferHeader;

typedef struct
{
    STRU_BBUartComSessionRxBufferHeader header;
    volatile uint8_t data[1];
} STRU_BBUartComSessionRxBuffer;

typedef struct
{
    STRU_BBUartComSessionRxBuffer* rx_buf;
    uint32_t data_max_size;
    ENUM_BB_UART_SESSION_PRIORITY  e_sessionPriority;
    ENUM_BB_UART_SESSION_DATA_TYPE  e_sessionDataType;
} STRU_BBUartComSession;


typedef struct
{
    volatile uint32_t   tx_buff_max_size;
    volatile uint32_t   tx_buf_wr_pos;
    volatile uint32_t   tx_buf_rd_pos;
    uint32_t            reserved;
} STRU_BB_UARTComTxQueueHeader;

typedef struct
{
    STRU_BB_UARTComTxQueueHeader    tx_queue_header;
    volatile uint8_t                tx_data[1];
} STRU_BBUartComTxQueue;


typedef struct
{
    uint8_t          tx_fifo_data[BBCOM_UART_TX_FIFO_SIZE];
    uint16_t         tx_fifo_rd_pos;
    uint16_t         tx_fifo_wr_pos;
} STRU_BBUartComTxFIFO;


void BB_UARTComInit(SYS_Event_Handler session0RcvDataHandler);

void BB_UARTComRemoteSessionInit(void);
uint8_t BB_UARTComRegisterSession(ENUM_BBUARTCOMSESSIONID session_id,
                                 ENUM_BB_UART_SESSION_PRIORITY session_priority,
                                 ENUM_BB_UART_SESSION_DATA_TYPE session_dataType);
void BB_UARTComUnRegisterSession(ENUM_BBUARTCOMSESSIONID session_id);
uint8_t BB_UARTComSendMsg(ENUM_BBUARTCOMSESSIONID session_id, uint8_t* data_buf, uint16_t length);
uint32_t BB_UARTComReceiveMsg(ENUM_BBUARTCOMSESSIONID session_id, uint8_t* data_buf, uint32_t length_max);
uint8_t get_session_eventid(uint8_t id, uint32_t *pu32_rcv_event);
uint32_t BB_UARTComGetTXQueueFreeLength(ENUM_BB_UART_SESSION_PRIORITY e_sessionPriority);
uint16_t BB_UARTComGetMsgFromTXQueue(ENUM_BB_UART_SESSION_PRIORITY session_priority);
void BB_UARTComFlushTXQueue(ENUM_BB_UART_SESSION_PRIORITY session_priority);
uint32_t BB_UARTComGetTxQueueCurrentLength(ENUM_BB_UART_SESSION_PRIORITY e_sessionPriority);
void BB_UARTComCycleMsgProcess(void);
void BB_UARTComCycleSendMsg(void);
uint16_t BB_UARTComGetBBFIFOGap(void);
uint32_t BB_UART_GetTxFifoMaxSize(ENUM_BB_MODE e_bb_mode);

#if 0
void BB_UARTComTestSend(void);
#endif

#endif
