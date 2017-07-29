#include <string.h>
#include <stdlib.h>
#include "command.h"
#include "debuglog.h"
#include "interrupt.h"
#include "serial.h"
#include "cmsis_os.h"
#include "bb_ctrl.h"
#include "test_bb.h"
#include "hal_ret_type.h"
#include "hal_nvic.h"
#include "memory_config.h"

extern void BB_uart10_spi_sel(uint32_t sel_dat);

static void command_set_loglevel(char* cpu, char* loglevel);


unsigned int command_str2uint(char *str)
{
    return strtoul(str, NULL, 0); 
}

void command_run(char *cmdArray[], uint32_t cmdNum)
{
    extern int BB_add_cmds(uint8_t type, uint32_t param0, uint32_t param1, uint32_t param2);
    
    if (memcmp(cmdArray[0], "BB_uart10_spi_sel", strlen("BB_uart10_spi_sel")) == 0)
    {
        BB_uart10_spi_sel( strtoul(cmdArray[1], NULL, 0) );
    }
    else if(memcmp(cmdArray[0], "BB_add_cmds", strlen("BB_add_cmds")) == 0)
    {
        BB_add_cmds(strtoul(cmdArray[1], NULL, 0),  //type
                    strtoul(cmdArray[2], NULL, 0),  //param0
                    strtoul(cmdArray[3], NULL, 0),  //param1
                    strtoul(cmdArray[4], NULL, 0)   //param2
                    );
    }
    else if ((memcmp(cmdArray[0], "set_loglevel", strlen("set_loglevel")) == 0))
    {
        command_set_loglevel(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "help", strlen("help")) == 0) 
    {
        dlog_error("Please use commands like:");
        dlog_error("BB_uart10_spi_sel <value>");
        dlog_error("BB_add_cmds <type> <param0> <param1> <param2>");
        dlog_error("set_loglevel <cpuid> <loglevel>");
    }
}

static void command_set_loglevel(char* cpu, char* loglevel)
{
    uint8_t level = command_str2uint(loglevel);
    if (memcmp(cpu, "cpu2", strlen("cpu2")) == 0)
    {
        dlog_set_output_level(level);
    }

    return;
}


