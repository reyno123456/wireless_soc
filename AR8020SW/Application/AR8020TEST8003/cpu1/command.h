
#ifndef __COMMAND__H
#define __COMMAND__H
#include <stdint.h>

extern uint32_t g_sendUSBFlag;
void command_run(char *cmdArray[], uint32_t cmdNum);

#endif