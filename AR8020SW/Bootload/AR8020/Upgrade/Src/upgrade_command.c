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
        dlog_error("%s Command not found. Please use the commands like:",cmdArray[0]);
        //dlog_error("sd_upgradeapp");
        //dlog_error("sd_boot");
        dlog_error("uart_upgradeapp");
        dlog_error("uart_boot");
    }
}

