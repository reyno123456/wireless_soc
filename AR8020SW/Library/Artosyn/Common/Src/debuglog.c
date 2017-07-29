#include <stdio.h>
#include <string.h>
#include "memory_config.h"
#include "debuglog.h"
#include "sys_event.h"
#include "serial.h"
#include "reg_map.h"
#include "cpu_info.h"
#include "interrupt.h"
#include "hal_uart.h"

volatile uint8_t sd_mountStatus = 0;
static uint8_t g_log_level = LOG_LEVEL_INFO;

static uint8_t s_u8_dlogServerCpuId = 0xFF;

#define DEBUG_LOG_OUTPUT_BUF_HEAD_0      ((char*)(((STRU_DebugLogOutputBuffer*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_0)->buf))
#define DEBUG_LOG_OUTPUT_BUF_TAIL_0      ((char*)(SRAM_DEBUG_LOG_OUTPUT_BUFFER_END_ADDR_0))
#define DEBUG_LOG_OUTPUT_BUF_HEAD_1      ((char*)(((STRU_DebugLogOutputBuffer*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_1)->buf))
#define DEBUG_LOG_OUTPUT_BUF_TAIL_1      ((char*)(SRAM_DEBUG_LOG_OUTPUT_BUFFER_END_ADDR_1))
#define DEBUG_LOG_OUTPUT_BUF_HEAD_2      ((char*)(((STRU_DebugLogOutputBuffer*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_2)->buf))
#define DEBUG_LOG_OUTPUT_BUF_TAIL_2      ((char*)(SRAM_DEBUG_LOG_OUTPUT_BUFFER_END_ADDR_2))

#define DEBUG_LOG_OUTPUT_BUF_WR_POS_0    (((STRU_DebugLogOutputBuffer*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_0)->header.output_buf_wr_pos)
#define DEBUG_LOG_OUTPUT_BUF_RD_POS_0    (s_debug_log_output_buf_rd_pos_0)
#define DEBUG_LOG_OUTPUT_BUF_WR_POS_1    (((STRU_DebugLogOutputBuffer*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_1)->header.output_buf_wr_pos)
#define DEBUG_LOG_OUTPUT_BUF_RD_POS_1    (s_debug_log_output_buf_rd_pos_1)
#define DEBUG_LOG_OUTPUT_BUF_WR_POS_2    (((STRU_DebugLogOutputBuffer*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_2)->header.output_buf_wr_pos)
#define DEBUG_LOG_OUTPUT_BUF_RD_POS_2    (s_debug_log_output_buf_rd_pos_2)

#define DEBUG_LOG_INPUT_BUF_HEAD         ((char*)(((STRU_DebugLogInputBuffer*)SRAM_DEBUG_LOG_INPUT_BUFFER_ST_ADDR)->buf))
#define DEBUG_LOG_INPUT_BUF_TAIL         ((char*)(SRAM_DEBUG_LOG_INPUT_BUFFER_ST_ADDR + SRAM_DEBUG_LOG_INPUT_BUFFER_SIZE - 1))

#define DEBUG_LOG_INPUT_BUF_WR_POS       (((STRU_DebugLogInputBuffer*)SRAM_DEBUG_LOG_INPUT_BUFFER_ST_ADDR)->header.input_buf_wr_pos)
#define DEBUG_LOG_INPUT_BUF_RD_POS_0     (s_debug_log_input_buf_rd_pos_0)
#define DEBUG_LOG_INPUT_BUF_RD_POS_1     (s_debug_log_input_buf_rd_pos_1)
#define DEBUG_LOG_INPUT_BUF_RD_POS_2     (s_debug_log_input_buf_rd_pos_2)

#define CHECK_DEBUG_BUF_INIT_STATUS()    do { if (DLOG_CheckDebugBufInitStatus() == 0 ) return 0; } while(0)

#define SRAM_DEBUG_BUF_INIT_FLAG         0x30A5A503

#define DEBUG_LOG_END                    (0x1f)

#define DLOG_INPUT_MAX_CMD_PAR           (10)

typedef struct
{
    volatile uint32_t input_buf_wr_pos;
    volatile uint32_t input_buf_init_flag;
} STRU_DebugLogInputBufferHeader;

typedef struct
{
    volatile STRU_DebugLogInputBufferHeader header;
    volatile char buf[1];
} STRU_DebugLogInputBuffer;

typedef struct
{
    volatile uint32_t output_buf_wr_pos;
    volatile uint32_t output_buf_init_flag;
} STRU_DebugLogOutputBufferHeader;

typedef struct
{
    volatile STRU_DebugLogOutputBufferHeader header;
    volatile char buf[1];
} STRU_DebugLogOutputBuffer;

static char *s_debug_log_output_buf_rd_pos_0 = DEBUG_LOG_OUTPUT_BUF_HEAD_0;
static char *s_debug_log_output_buf_rd_pos_1 = DEBUG_LOG_OUTPUT_BUF_HEAD_1;
static char *s_debug_log_output_buf_rd_pos_2 = DEBUG_LOG_OUTPUT_BUF_HEAD_2;

static char *s_debug_log_input_buf_rd_pos_0 = DEBUG_LOG_INPUT_BUF_HEAD;
static char *s_debug_log_input_buf_rd_pos_1 = DEBUG_LOG_INPUT_BUF_HEAD;
static char *s_debug_log_input_buf_rd_pos_2 = DEBUG_LOG_INPUT_BUF_HEAD;

static unsigned char s_u8_commandPos;
static unsigned char s_u8_commandLine[128];

static FUNC_CommandRun s_func_commandRun = NULL;
static FUNC_LogSave s_sd_log_func_commandRun = NULL;

__attribute__((weak)) ENUM_CPU_ID CPUINFO_GetLocalCpuId(void) 
{
    return *((ENUM_CPU_ID*)CPU_ID_INFO_ADDRESS);
}

__attribute__((weak)) void uart_putc(unsigned char index, char c)
{
}

__attribute__((weak)) void uart_puts(unsigned char index, const char *s)
{
}

__attribute__((weak)) void uart_putFifo(unsigned char index)
{
}

__attribute__((weak)) int32_t Uart_WaitTillIdle(unsigned char index, uint32_t timeOut)
{
}

__attribute__((weak)) HAL_RET_T HAL_UART_Init(ENUM_HAL_UART_COMPONENT e_uartComponent, 
                        ENUM_HAL_UART_BAUDR e_uartBaudr, 
                        HAL_UART_RxHandle pfun_rxFun)
{
}

static unsigned char DLOG_CheckDebugBufInitStatus(void)
{
    // Check input SRAM buffer init flag 
    
    if (((volatile STRU_DebugLogInputBufferHeader*)SRAM_DEBUG_LOG_INPUT_BUFFER_ST_ADDR)->input_buf_init_flag != SRAM_DEBUG_BUF_INIT_FLAG) 
    {
        return 0; 
    }

    // Check output SRAM buffer init flag 
    
    unsigned int out_buf_addr;
    switch(CPUINFO_GetLocalCpuId())
    {
    case ENUM_CPU0_ID:
        out_buf_addr = SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_0;
        break;
    case ENUM_CPU1_ID:
        out_buf_addr = SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_1;
        break;
    case ENUM_CPU2_ID:
        out_buf_addr = SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_2;
        break;
    }
    
    if (((STRU_DebugLogOutputBufferHeader*)out_buf_addr)->output_buf_init_flag != SRAM_DEBUG_BUF_INIT_FLAG) 
    {
        return 0; 
    }

    return 1;
}

static unsigned int DLOG_Input(char* buf, unsigned int byte_num)
{
    CHECK_DEBUG_BUF_INIT_STATUS();

    if (CPUINFO_GetLocalCpuId() != s_u8_dlogServerCpuId)
    {
        return 0;
    }

    unsigned int iByte = 0;

    char* target = (char*)DEBUG_LOG_INPUT_BUF_WR_POS;

    // Copy to log buffer
    while (iByte < byte_num)
    {
        *target = buf[iByte];

        if (target >= DEBUG_LOG_INPUT_BUF_TAIL)
        {
            target = DEBUG_LOG_INPUT_BUF_HEAD;
        }
        else
        {
            target++;
        }

        iByte++;
    }

    DEBUG_LOG_INPUT_BUF_WR_POS = (uint32_t)target;
    
    return iByte;
}

uint32_t DLOG_GetChar(uint8_t *u8_uartRxBuf, uint8_t u8_uartRxLen)
{
    uint8_t i = 0;
    char c = '\r';
    char u8_commandTemp[64] = {0};
    uint16_t u8_commandTempPos = 0;
    while (u8_uartRxLen)
    {
        c = *(u8_uartRxBuf + i);

        if ((s_u8_commandPos < (sizeof(s_u8_commandLine) - 1)) &&
            (u8_commandTempPos < (sizeof(u8_commandTemp) - 1)))
        {
            /* receive "enter" key */
            if (c == '\r')
            {
                s_u8_commandLine[s_u8_commandPos++] = c;
                u8_commandTemp[u8_commandTempPos++] = '\n';
                /* if s_u8_commandLine is not empty, go to parse command */
                if (s_u8_commandPos > 0)
                {
                    DLOG_Input(s_u8_commandLine, s_u8_commandPos);
                    s_u8_commandPos = 0;
                    memset(s_u8_commandLine, 0, sizeof(s_u8_commandLine));
                }
            }
            /* receive "backspace" key */
            else if (c == '\b')
            {
                if (s_u8_commandPos > 1)
                {
                    s_u8_commandLine[--s_u8_commandPos] = '\0';
                }
                u8_commandTemp[u8_commandTempPos++] = '\b';
                u8_commandTemp[u8_commandTempPos++] = ' ';
                u8_commandTemp[u8_commandTempPos++] = '\b';
            }
            /* receive normal data */
            else if ((c >= 32) && (c <= 126))
            {
                s_u8_commandLine[s_u8_commandPos++] = c;
                u8_commandTemp[u8_commandTempPos++] = c;
            }
        }
        else
        {
             s_u8_commandPos = 0;
             memset(s_u8_commandLine, 0, sizeof(s_u8_commandLine));
             printf("!!! Too long input string one time, skip the operation !!!\n\n\n");
        }


        i++;
        u8_uartRxLen--;  
    }

    if (u8_commandTempPos != 0)
    {
        if (u8_commandTemp[u8_commandTempPos-1] != '\n')
        {
            u8_commandTemp[u8_commandTempPos] = DEBUG_LOG_END;
            u8_commandTemp[u8_commandTempPos+1] = '\n';
        }
        
        printf("%s",u8_commandTemp);
    }
}

static void DLOG_InputCommandInit(void)
{
    s_u8_commandPos = 0;
    memset(s_u8_commandLine, 0, sizeof(s_u8_commandLine));
    
    HAL_UART_Init(DEBUG_LOG_UART_PORT, HAL_UART_BAUDR_115200, DLOG_GetChar);
}

unsigned int DLOG_InputParse(char *buf, unsigned int byte_max)
{
    unsigned char u8_dataValid = 0;
    unsigned int iByte = 0;
    char **p_src;

    CHECK_DEBUG_BUF_INIT_STATUS();

    ENUM_CPU_ID e_cpuID = CPUINFO_GetLocalCpuId();

    switch (e_cpuID)
    {
    case ENUM_CPU0_ID:
        p_src = &DEBUG_LOG_INPUT_BUF_RD_POS_0;
        break;
    case ENUM_CPU1_ID:
        p_src = &DEBUG_LOG_INPUT_BUF_RD_POS_1;
        break;
    case ENUM_CPU2_ID:
        p_src = &DEBUG_LOG_INPUT_BUF_RD_POS_2;
        break;
    default:
        return 0;
    }

    char *src = *p_src;

    while (src != (char *)DEBUG_LOG_INPUT_BUF_WR_POS)
    {
        *buf++ = *src;

        if ((*src == '\n') || (*src == '\r'))
        {
            u8_dataValid = 1;
        }

        if (src >= DEBUG_LOG_INPUT_BUF_TAIL)
        {
            src = DEBUG_LOG_INPUT_BUF_HEAD;
        }
        else
        {
            src++;
        }
        iByte++;

        if ((iByte >= byte_max) || (u8_dataValid == 1))
        {
            break;
        }
    }

    *p_src = src;
    
    if (u8_dataValid == 1)
    {
        return iByte;
    }
    else
    {
        return 0;
    }
}

static void DLOG_InputCommandParse(char *cmd)
{
    unsigned char cmdIndex;
    char *tempCommand[DLOG_INPUT_MAX_CMD_PAR];

    cmdIndex = 0;
    memset(tempCommand, 0, sizeof(tempCommand));

    while (cmdIndex < DLOG_INPUT_MAX_CMD_PAR)
    {
        /* skip the sapce */
        while ((*cmd == ' ') || (*cmd == '\t'))
        {
            ++cmd;
        }

        /* end of the cmdline */
        if (*cmd == '\0')
        {
            break;
        }

        tempCommand[cmdIndex++] = cmd;

        /* find the end of string */
        while (*cmd && (*cmd != ' ') && (*cmd != '\t'))
        {
            ++cmd;
        }

        /* no more command */
        if (*cmd == '\0')
        {
            break;
        }

        /* current cmd is end */
        *cmd++ = '\0';
    }

    if (s_func_commandRun != NULL)
    {
        s_func_commandRun(tempCommand, cmdIndex);
    }
}

void DLOG_Process(void* p)
{
    char commandBuf[50];
    memset(commandBuf, 0, sizeof(commandBuf));

    while(DLOG_InputParse(commandBuf, sizeof(commandBuf)))
    {
        DLOG_InputCommandParse(commandBuf);
        memset(commandBuf, 0, sizeof(commandBuf));
    }

    while(DLOG_Output(3000))
    {
    }
}



void DLOG_Init(FUNC_CommandRun func_command, 
                  FUNC_LogSave func_log_sd,
                  ENUM_DLOG_PROCESSOR e_dlogProcessor)
{
    s_func_commandRun = func_command;
    s_sd_log_func_commandRun = func_log_sd;

    if (e_dlogProcessor == DLOG_SERVER_PROCESSOR)
    {
        s_u8_dlogServerCpuId = CPUINFO_GetLocalCpuId();
        
        ((STRU_DebugLogInputBufferHeader*)SRAM_DEBUG_LOG_INPUT_BUFFER_ST_ADDR)->input_buf_wr_pos = (uint32_t)DEBUG_LOG_INPUT_BUF_HEAD;
        ((STRU_DebugLogInputBufferHeader*)SRAM_DEBUG_LOG_INPUT_BUFFER_ST_ADDR)->input_buf_init_flag = SRAM_DEBUG_BUF_INIT_FLAG;

        ((STRU_DebugLogOutputBufferHeader*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_0)->output_buf_wr_pos = (uint32_t)DEBUG_LOG_OUTPUT_BUF_HEAD_0;
        ((STRU_DebugLogOutputBufferHeader*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_0)->output_buf_init_flag = SRAM_DEBUG_BUF_INIT_FLAG;
        
        ((STRU_DebugLogOutputBufferHeader*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_1)->output_buf_wr_pos = (uint32_t)DEBUG_LOG_OUTPUT_BUF_HEAD_1;
        ((STRU_DebugLogOutputBufferHeader*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_1)->output_buf_init_flag = SRAM_DEBUG_BUF_INIT_FLAG;
        
        ((STRU_DebugLogOutputBufferHeader*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_2)->output_buf_wr_pos = (uint32_t)DEBUG_LOG_OUTPUT_BUF_HEAD_2;
        ((STRU_DebugLogOutputBufferHeader*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_2)->output_buf_init_flag = SRAM_DEBUG_BUF_INIT_FLAG;

        DLOG_InputCommandInit();
    }
    else
    {
        while (DLOG_CheckDebugBufInitStatus() == 0 ) ;
    }

#ifdef USE_SYS_EVENT_TRIGGER_DEBUG_LOG_PROCESS
    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_IDLE, DLOG_Process);
#endif
}

#ifdef DEBUG_LOG_USE_SRAM_OUTPUT_BUFFER

static unsigned int DLOG_StrCpyToDebugOutputLogBuf(const char *src, unsigned int length)
{
    char* dst;
    char* head;
    char* tail;

    CHECK_DEBUG_BUF_INIT_STATUS();

    switch(CPUINFO_GetLocalCpuId())
    {
    case ENUM_CPU0_ID:
        dst = (char*)DEBUG_LOG_OUTPUT_BUF_WR_POS_0;
        head = DEBUG_LOG_OUTPUT_BUF_HEAD_0;
        tail = DEBUG_LOG_OUTPUT_BUF_TAIL_0;
        break;
    case ENUM_CPU1_ID:
        dst = (char*)DEBUG_LOG_OUTPUT_BUF_WR_POS_1;
        head = DEBUG_LOG_OUTPUT_BUF_HEAD_1;
        tail = DEBUG_LOG_OUTPUT_BUF_TAIL_1;
        break;
    case ENUM_CPU2_ID:
        dst = (char*)DEBUG_LOG_OUTPUT_BUF_WR_POS_2;
        head = DEBUG_LOG_OUTPUT_BUF_HEAD_2;
        tail = DEBUG_LOG_OUTPUT_BUF_TAIL_2;
        break;
    default:
        return 0;
    }

    while ((*src) && (length--))
    {
        *dst = *src;
        if (dst >= tail)
        {
            dst = head;
        }
        else
        {
            dst++;
        }
        src++;
    }

    switch(CPUINFO_GetLocalCpuId())
    {
    case ENUM_CPU0_ID:
        DEBUG_LOG_OUTPUT_BUF_WR_POS_0 = (uint32_t)dst;
        break;
    case ENUM_CPU1_ID:
        DEBUG_LOG_OUTPUT_BUF_WR_POS_1 = (uint32_t)dst;
        break;
    case ENUM_CPU2_ID:
        DEBUG_LOG_OUTPUT_BUF_WR_POS_2 = (uint32_t)dst;
        break;
    default:
        return 0;
    }

    return 1;
}

unsigned int DLOG_Output(unsigned int byte_num)
{
    unsigned int iByte = 0;

    char **p_src;
    char* src;
    char* tmpsrc;
    char* write_pos;
    char* head;
    char* tail;
    
    unsigned char output_index;
    unsigned char enter_detected;

    char tmp_buf[256];
    unsigned int tmp_buf_index;
    uint32_t tmp_len;

    CHECK_DEBUG_BUF_INIT_STATUS();

    if (CPUINFO_GetLocalCpuId() != s_u8_dlogServerCpuId)
    {
        return 0;
    }

    // Print output buffer 0,1,2 to serial 
    output_index = 0;
    while (output_index <= 2) 
    {
        switch (output_index)
        {
        case 0:
            p_src  = &DEBUG_LOG_OUTPUT_BUF_RD_POS_0;
            write_pos = (char*)DEBUG_LOG_OUTPUT_BUF_WR_POS_0;
            head = DEBUG_LOG_OUTPUT_BUF_HEAD_0;
            tail = DEBUG_LOG_OUTPUT_BUF_TAIL_0;
            break;
        case 1:
            p_src  = &DEBUG_LOG_OUTPUT_BUF_RD_POS_1;
            write_pos = (char*)DEBUG_LOG_OUTPUT_BUF_WR_POS_1;
            head = DEBUG_LOG_OUTPUT_BUF_HEAD_1;
            tail = DEBUG_LOG_OUTPUT_BUF_TAIL_1;
            break;
        case 2:
            p_src  = &DEBUG_LOG_OUTPUT_BUF_RD_POS_2;
            write_pos = (char*)DEBUG_LOG_OUTPUT_BUF_WR_POS_2;
            head = DEBUG_LOG_OUTPUT_BUF_HEAD_2;
            tail = DEBUG_LOG_OUTPUT_BUF_TAIL_2;
            break;
        default:
            return 0;
        }

        src = *p_src;
        tmp_buf_index = 0;
        memset(tmp_buf, 0, sizeof(tmp_buf));
        enter_detected = 0;

        while (src != write_pos)
        {
            if (tmp_buf_index < sizeof(tmp_buf))
            {
                tmp_buf[tmp_buf_index++] = *src;
            }

            if ((*src == '\n') || (*src == DEBUG_LOG_END) )
            {
                enter_detected = 1;
                *tmpsrc = *src;
            }

            if (src >= tail)
            {
                src = head;
            }
            else
            {
                src++;
            }

            if (enter_detected == 1)
            {
                
                if ((*tmpsrc != DEBUG_LOG_END))
                {
                    uart_puts(DEBUG_LOG_UART_PORT, tmp_buf);
                    Uart_WaitTillIdle(DEBUG_LOG_UART_PORT, UART_DEFAULT_TIMEOUTMS);                    

                    if (s_sd_log_func_commandRun != NULL)
                    {
                        tmp_len = strlen(tmp_buf);
                        s_sd_log_func_commandRun(tmp_buf, tmp_len);
                    }

                    uart_putc(DEBUG_LOG_UART_PORT, '\r');
                    Uart_WaitTillIdle(DEBUG_LOG_UART_PORT, UART_DEFAULT_TIMEOUTMS);
                    iByte += tmp_buf_index;
                }
                else
                {   
                    tmp_buf[tmp_buf_index-1]='\0';
                    uart_puts(DEBUG_LOG_UART_PORT, tmp_buf);
                    Uart_WaitTillIdle(DEBUG_LOG_UART_PORT, UART_DEFAULT_TIMEOUTMS);
                    iByte += (tmp_buf_index-1);
                }
                               
                *p_src = src;

#ifdef DEBUG_LOG_OUTPUT_CPU_AFTER_CPU
                // Output cpu0, cpu1, cpu2.
                tmp_buf_index = 0;
                memset(tmp_buf, 0, sizeof(tmp_buf));
                enter_detected = 0;
#else
                // Output line by line
                break;
#endif
            }
        }

        if (iByte >= byte_num)
        {
            break;
        }

        output_index++;
    }

    return iByte;
}

// Output string when use libsimplec.a 
int puts(const char * s)
{
    unsigned int len = 0;

    while (('\n' != s[len]) && (DEBUG_LOG_END != s[len]) && (0 != s[len]))
    {
        len++;
    }
    
    // Print to buffer
    DLOG_StrCpyToDebugOutputLogBuf(s, len+1);

    return 0;
}

// Output data when use libc.a
int _write(int file, char *ptr, int len)
{
    int len2;
    
    if ((file == 0) || (file == 1) || (file ==2))
    {
        if ((len > 2) && ('\n' == ptr[len-1]) && (DEBUG_LOG_END == ptr[len-2]))
        {
            len2 = len -1;
        }
        else
        {
            len2 = len;
        }
        // Print to buffer
        DLOG_StrCpyToDebugOutputLogBuf(ptr, len2);
    }

    return len;
}

#else

int puts(const char * s)
{
    // Print to serial
    char c;

    CHECK_DEBUG_BUF_INIT_STATUS();

    if (CPUINFO_GetLocalCpuId() == s_u8_dlogServerCpuId)
    {
        while (*s)
        {
            c = *s++;

            if (c == '\n')
            {
                uart_putc(DEBUG_LOG_UART_PORT, '\r');
                Uart_WaitTillIdle(DEBUG_LOG_UART_PORT, UART_DEFAULT_TIMEOUTMS);
            }
            
            uart_putc(DEBUG_LOG_UART_PORT, c);
            Uart_WaitTillIdle(DEBUG_LOG_UART_PORT, UART_DEFAULT_TIMEOUTMS);
        }
    }
    
    return 0;
}

unsigned int DLOG_Output(unsigned int byte_num)
{
    return 0;
}

#endif

int dlog_set_output_level(unsigned char level)
{
    if (level <= LOG_LEVEL_INFO)
    {
        g_log_level = level;
        return 0;
    }

    return -1;
}

unsigned int dlog_get_output_level(void)
{
    return g_log_level;
}
