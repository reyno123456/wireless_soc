#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "debuglog.h"
#include "hal.h"
#include "lwip/tcpip.h"

#define configIP_ADDR0		172
#define configIP_ADDR1		16
#define configIP_ADDR2		162
#define configIP_ADDR3		15

#define configNET_MASK0		255
#define configNET_MASK1		255
#define configNET_MASK2		255
#define configNET_MASK3		0

#define configGW_ADDR0	        192
#define configGW_ADDR1	        168
#define configGW_ADDR2	        0
#define configGW_ADDR3	        1

void test_net(void)
{
    struct ip_addr  xIpAddr, xNetMast, xGateway;

    tcpip_init( NULL, NULL );

    IP4_ADDR( &xIpAddr, configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3 );
    IP4_ADDR( &xNetMast, configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3 );
    IP4_ADDR( &xGateway, configGW_ADDR0, configGW_ADDR1, configGW_ADDR2, configGW_ADDR3 );

}
