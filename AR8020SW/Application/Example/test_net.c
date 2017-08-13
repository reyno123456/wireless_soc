#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "debuglog.h"
#include "hal.h"
#include "lwip/tcpip.h"

void test_net(void)
{
    tcpip_init( NULL, NULL );
}
