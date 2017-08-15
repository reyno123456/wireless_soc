#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmsis_os.h"
#include "debuglog.h"
#include "hal.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "test_net_loop.h"
#include "netif/loopif.h"

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

static char *itoa(int val, char *buf, unsigned radix)
{
    char   *p;             
    char   *firstdig;      
    char   temp;           
    unsigned   digval;     
    p = buf;
    if(val <0)
    {
        *p++ = '-';
        val = (unsigned long)(-(long)val);
    }
    firstdig = p; 
    do{
        digval = (unsigned)(val % radix);
        val /= radix;
       
        if  (digval > 9)
            *p++ = (char)(digval - 10 + 'a'); 
        else
            *p++ = (char)(digval + '0');      
    }while(val > 0);
   
    *p-- = '\0';         
    do{
        temp = *p;
        *p = *firstdig;
        *firstdig = temp;
        --p;
        ++firstdig;        
    }while(firstdig < p);  
    return buf;
}

static void test_net_loop_server( void const * argument)
{
    int fd; 
    int sockfd; 
     
    struct sockaddr_in myaddr = {0};
    struct sockaddr_in clientaddr = {0};
 
    char buf[100]={0};
    memset(buf, 0, sizeof(buf));
    int len = sizeof(clientaddr); 
    myaddr.sin_family = AF_INET; 
    myaddr.sin_port = htons(2222); 
    myaddr.sin_addr.s_addr = inet_addr("127.0.0.1");//设定本机的回环地址 

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0) 
    { 
        dlog_error("socket"); 
        return;     
    }

    if(bind(sockfd, (struct sockaddr*)&myaddr, sizeof(struct sockaddr))!=0) 
    { 
        dlog_error("bind"); 
        return;     
    }

    while(1)
    {
        recvfrom(sockfd, buf, 100, 0, (struct sockaddr*)&clientaddr, (socklen_t*)&len); 
        dlog_info("recv:%s\n",buf); 
        sendto(sockfd, "OK!", 3, 0, (struct sockaddr*)&clientaddr, len);
        HAL_Delay(1000); 
    }
}

static void test_net_loop_client( void const * argument)
{
    int fd; 
    int sockfd; 
     
    struct sockaddr_in myaddr = {0};
    struct sockaddr_in clientaddr = {0}; 
     
    char buf[100]={0}; 
    memset(buf, 0, sizeof(buf));
    int len = sizeof(clientaddr); 
    myaddr.sin_family = AF_INET; 
    myaddr.sin_port = htons(2222); 
    myaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    char send_buf[100] = {0};
     
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0) 
    { 
        dlog_error("socket"); 
        return;     
    } 

    uint32_t i = 0;

    char buf_tmp[9] = {0};

    while(1)
    {
        //fprintf(send_buf, "send time:%s", itoa(send_buf, i));
        strcpy(send_buf, "send times:");
        itoa(i++, buf_tmp, 10);
        strcpy(&send_buf[strlen("send times:")], buf_tmp);  
        sendto(sockfd, send_buf, strlen(send_buf), 0, (struct sockaddr*)&myaddr, len); 
        recvfrom(sockfd, buf, 100, 0, (struct sockaddr*)&myaddr, (socklen_t*)&len); 
        dlog_info("recv:%s\n",buf);
        HAL_Delay(1000);
    }  
}

void test_net_loop(void)
{
	struct ip_addr  xIpAddr, xNetMast, xGateway;
    extern err_t ethernetif_init( struct netif *netif );

    tcpip_init( NULL, NULL );
    IP4_ADDR( &xIpAddr, 127, 0, 0, 1 );
    IP4_ADDR( &xNetMast, configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3 );
    IP4_ADDR( &xGateway, 127, 0, 0, 1 );
    netif_add( &ENET_if, &xIpAddr, &xNetMast, &xGateway, NULL, ethernetif_init, tcpip_input );

    netif_set_default( &ENET_if );
    netif_set_up( &ENET_if );

    loopif_init(&ENET_if);

	osThreadDef(Task_net_loop_server, test_net_loop_server, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(Task_net_loop_server), NULL);
	osThreadDef(Task_net_loop_client, test_net_loop_client, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(Task_net_loop_client), NULL);

    return;
}
