#include "command.h"
#include "debuglog.h"
#include "interrupt.h"
#include "serial.h"
#include "stm32f746xx.h"
#include "debuglog.h"
#include <string.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "hal_sd.h"
#include "hal_ret_type.h"
#include "hal_nvic.h"

static void command_set_loglevel(char* cpu, char* loglevel);

void command_run(char *cmdArray[], uint32_t cmdNum)
{
    if ((memcmp(cmdArray[0], "set_loglevel", strlen("set_loglevel")) == 0))
    {
        command_set_loglevel(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "help", strlen("help")) == 0)
    {
        dlog_error("Please use the commands like:");
        dlog_error("set_loglevel <cpuid> <loglevel>");
    }
}

unsigned int command_str2uint(char *str)
{
    return strtoul(str, NULL, 0); 
}

static void command_set_loglevel(char* cpu, char* loglevel)
{
    uint8_t level = command_str2uint(loglevel);
    if (memcmp(cpu, "cpu1", strlen("cpu1")) == 0)
    {
        dlog_set_output_level(level);
    }

    return;
}

