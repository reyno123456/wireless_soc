#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static struct netif ENET_if;

/* ------------------------ Defines --------------------------------------- */
/* The size of the buffer in which the dynamic WEB page is created. */
#define webMAX_PAGE_SIZE        ( 1024 ) /*FSL: buffer containing array*/

/* Standard GET response. */
#define webHTTP_OK  "HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n"

/* The port on which we listen. */
#define webHTTP_PORT            ( 80 )

/* Delay on close error. */
#define webSHORT_DELAY          ( 10 )

#define webHTML_START \
"<html><head></head><BODY onLoad=\"window.setTimeout(&quot;location.href='index.html'&quot;,1000)\"bgcolor=\"#CCCCff\">\r\n\r\nPage Hits = "

#define webHTML_END \
"\r\n" \
"</pre>\r\n" \
"</BODY>\r\n" \
"</html>"

static void vProcessConnection( struct netconn *pxNetCon )
{
    static portCHAR cDynamicPage[webMAX_PAGE_SIZE], cPageHits[11];
    struct netbuf  *pxRxBuffer;
    portCHAR       *pcRxString;
    unsigned portSHORT usLength;
    static unsigned portLONG ulPageHits = 0;

    /* We expect to immediately get data. */
    pxRxBuffer = netconn_recv( pxNetCon );

    if( pxRxBuffer != NULL )
    {
        /* Where is the data? */
        netbuf_data( pxRxBuffer, ( void * )&pcRxString, &usLength );

        /* Is this a GET?  We don't handle anything else. */
        if( !strncmp( pcRxString, "GET", 3 ) )
        {
            pcRxString = cDynamicPage;

            /* Update the hit count. */
            ulPageHits++;
            sprintf( cPageHits, "%d", (int)ulPageHits );

            /* Write out the HTTP OK header. */            
            netconn_write( pxNetCon, webHTTP_OK, ( u16_t ) strlen( webHTTP_OK ), NETCONN_COPY );

            /* Generate the dynamic page...
            
               ... First the page header. */
            strcpy( cDynamicPage, webHTML_START );
            /* ... Then the hit count... */
            strcat( cDynamicPage, cPageHits );
            
            strcat( cDynamicPage,
                    "<p><pre>Task          State  Priority  Stack      #<br\
>************************************************<br>" );
            /* ... Then the list of tasks and their status... */
            vTaskList( ( signed portCHAR * )cDynamicPage + strlen( cDynamicPage ) );            
            
            /* ... Finally the page footer. */
            strcat( cDynamicPage, webHTML_END );

            /* Write out the dynamically generated page. */
            netconn_write( pxNetCon, cDynamicPage, ( u16_t ) strlen( cDynamicPage ), NETCONN_COPY 
);
        }
        netbuf_delete( pxRxBuffer );
    }
    netconn_close( pxNetCon );
}

static void net_http_handler( void const * argument)
{
	struct ip_addr  xIpAddr, xNetMast, xGateway;
    struct netconn *pxHTTPListener, *pxNewConnection;
    extern err_t ethernetif_init( struct netif *netif );

    tcpip_init( NULL, NULL );

    IP4_ADDR( &xIpAddr, configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3 );
    IP4_ADDR( &xNetMast, configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3 );
    IP4_ADDR( &xGateway, configGW_ADDR0, configGW_ADDR1, configGW_ADDR2, configGW_ADDR3 );
    netif_add( &ENET_if, &xIpAddr, &xNetMast, &xGateway, NULL, ethernetif_init, tcpip_input );

    netif_set_default( &ENET_if );
    netif_set_up( &ENET_if );

    /* Create a new tcp connection handle */
    pxHTTPListener = netconn_new( NETCONN_TCP );
    netconn_bind( pxHTTPListener, NULL, webHTTP_PORT );
    netconn_listen( pxHTTPListener );

    for( ;; )
    {	
        /* Wait for connection. */
        pxNewConnection = netconn_accept( pxHTTPListener );

        if( pxNewConnection != NULL )
        {
            /* Service connection. */
            vProcessConnection( pxNewConnection );
            while( netconn_delete( pxNewConnection ) != ERR_OK )
            {
                vTaskDelay( webSHORT_DELAY );
            }
        }
    }
}

void test_net_http(void)
{
	osThreadDef(Task_net_http, net_http_handler, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(Task_net_http), NULL);
}

void command_test_net(char* arg1)
{
    uint8_t choise = strtoul(arg1, NULL, 0);
    
    dlog_info("input choise = %d", choise);

    switch(choise)
    {
        case 0:
            test_net_http();
        break;

        default:break;
    }

    return;
}
