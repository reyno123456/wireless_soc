#include <stdint.h>
#include <string.h>

#include "upgrade_command.h"
#include "debuglog.h"
#include "upgrade_sd.h"
#include "upgrade_uart.h"

void UPGRADE_CommandRun(char *cmdArray[], uint32_t cmdNum)
{
    /*
    if (memcmp(cmdArray[0], "sd_upgradeapp", strlen("sd_upgradeapp")) == 0)
    {
        UPGRADE_UpdataFromSDToNor();
    }
    else if (memcmp(cmdArray[0], "sd_boot", strlen("sd_boot")) == 0)
    {
        UPGRADE_BootFromSD();
    }
	*/

	if (memcmp(cmdArray[0], "uart_upgradeapp", strlen("uart_upgradeapp")) == 0)
    {
        UPGRADE_APPFromUart();
    }
    else if (memcmp(cmdArray[0], "uart_boot", strlen("uart_boot")) == 0)
    {
        UPGRADE_BootloadFromUart();
    }
    else
    {
        dlog_critical("%s Command not found. Please use the commands like:",cmdArray[0]);
        //dlog_critical("sd_upgradeapp");
        //dlog_critical("sd_boot");
        dlog_critical("uart_upgradeapp");
        dlog_critical("uart_boot");
    }
}

