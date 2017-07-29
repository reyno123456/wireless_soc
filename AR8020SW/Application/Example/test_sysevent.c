#include <stdint.h>
#include <stdlib.h>
#include "sys_event.h"
#include "test_sysevent.h"
#include "debuglog.h"

static void test_IdleCallback(void * p);

static void test_IdleCallback(void * p)
{
    dlog_info("idle function ...");
}

void command_TestSysEventIdle(void)
{
    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_IDLE, test_IdleCallback);
}

void command_TestSysEventInterCore(char* ptr)
{
    STRU_SysEvent_BB_DATA_BUFFER_FULL_RATIO_Change p;
    p.BB_Data_Full_Ratio = strtoul(ptr, NULL, 0);
    SYS_EVENT_Notify(SYS_EVENT_ID_BB_DATA_BUFFER_FULL, (void*)&p);
}

