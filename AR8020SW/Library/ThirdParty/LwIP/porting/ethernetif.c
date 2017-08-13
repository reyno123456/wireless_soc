/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/opt.h"

#if 1 /* ready */

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
#include "netif/ppp_oe.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

// #include "common.h"
/* Standard library includes. */
#include <string.h>

#define configMAC_ADDR0	0x00
#define configMAC_ADDR1	0xCF
#define configMAC_ADDR2	0x52
#define configMAC_ADDR3	0x35
#define configMAC_ADDR4	0x00
#define configMAC_ADDR5	0x01

/*
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
*/

#define configNUM_ENET_RX_BUFFERS	8
#define configNUM_ENET_TX_BUFFERS       8
#define configENET_RX_BUFFER_SIZE	1520
#define configENET_TX_BUFFER_SIZE	1520
#define configUSE_PROMISCUOUS_MODE	0
#define configUSE_MII_MODE              0/*FSL: using RMII mode*/
#define configETHERNET_INPUT_TASK_STACK_SIZE ( 256 )
#define configETHERNET_INPUT_TASK_PRIORITY ( configMAX_PRIORITIES - 1 )

#define NBUF_LITTLE_ENDIAN

#define ENHANCED_BD

#define TX_BD_R			0x0080
#define TX_BD_TO1		0x0040
#define TX_BD_W			0x0020
#define TX_BD_TO2		0x0010
#define TX_BD_L			0x0008
#define TX_BD_TC		0x0004
#define TX_BD_ABC		0x0002

// ----------------------------------------------------------------------
// TX Enhanced BD Bit Definitions
// ----------------------------------------------------------------------
#define TX_BD_INT       0x00000040 
#define TX_BD_TS        0x00000020 
#define TX_BD_PINS      0x00000010 
#define TX_BD_IINS      0x00000008 
#define TX_BD_TXE       0x00800000 
#define TX_BD_UE        0x00200000 
#define TX_BD_EE        0x00100000
#define TX_BD_FE        0x00080000 
#define TX_BD_LCE       0x00040000 
#define TX_BD_OE        0x00020000 
#define TX_BD_TSE       0x00010000 

#define TX_BD_BDU       0x00000080    

// ----------------------------------------------------------------------
// RX Buffer Descriptor Bit Definitions
// ----------------------------------------------------------------------

// Offset 0 flags - status: Big Endian
#define RX_BD_E			0x0080
#define RX_BD_R01		0x0040
#define RX_BD_W			0x0020
#define RX_BD_R02		0x0010
#define RX_BD_L			0x0008
#define RX_BD_M			0x0001
#define RX_BD_BC		0x8000
#define RX_BD_MC		0x4000
#define RX_BD_LG		0x2000
#define RX_BD_NO		0x1000
#define RX_BD_CR		0x0400
#define RX_BD_OV		0x0200
#define RX_BD_TR		0x0100

// ----------------------------------------------------------------------
// RX Enhanced BD Bit Definitions
// ----------------------------------------------------------------------
#define RX_BD_ME               0x00000080    
#define RX_BD_PE               0x00000004    
#define RX_BD_CE               0x00000002    
#define RX_BD_UC               0x00000001
    
#define RX_BD_INT              0x00008000    

#define RX_BD_ICE              0x20000000    
#define RX_BD_PCR              0x10000000    
#define RX_BD_VLAN             0x04000000    
#define RX_BD_IPV6             0x02000000    
#define RX_BD_FRAG             0x01000000    

#define RX_BD_BDU              0x00000080    

typedef struct
{
	uint16_t status;	            /* control and status */
	uint16_t length;	            /* transfer length */
	uint8_t  *data;	                /* buffer address */
	uint32_t ebd_status;
	uint16_t length_proto_type;
	uint16_t payload_checksum;
	uint32_t bdu;
	uint32_t timestamp;
	uint32_t reserverd_word1;
	uint32_t reserverd_word2;
} NBUF;

/* Demo includes. */
/* #include "eth_phy.h" */
/* #include "enet.h" */
/* #include "mii.h" */

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

/* PHY hardware specifics. */
#define PHY_STATUS								( 0x1F )
#define PHY_DUPLEX_STATUS							( 4<<2 )
#define PHY_SPEED_STATUS							( 1<<2 )

/* Delay to wait for a DMA buffer to become available if one is not already
available. */
#define netifBUFFER_WAIT_ATTEMPTS					        10
#define netifBUFFER_WAIT_DELAY						        (10 / portTICK_RATE_MS)

/* Delay between polling the PHY to see if a link has been established. */
#define netifLINK_DELAY								( 500 / portTICK_RATE_MS )

/* Delay between looking for incoming packets.  In ideal world this would be
infinite. */
#define netifBLOCK_TIME_WAITING_FOR_INPUT			                netifLINK_DELAY

/* Very short delay to use when waiting for the Tx to finish with a buffer if
we run out of Rx buffers. */
#define enetMINIMAL_DELAY							( 2 / portTICK_RATE_MS )

/*FSL: arrays to be used*/
static unsigned char xENETTxDescriptors_unaligned[16];
static unsigned char xENETRxDescriptors_unaligned[16];
static unsigned char ucENETTxBuffers[16];
static unsigned char ucENETRxBuffers[16];

/* The DMA descriptors.  This is a char array to allow us to align it correctly. */
static NBUF *xENETTxDescriptors;
static NBUF *xENETRxDescriptors;

/* The DMA buffers.  These are char arrays to allow them to be alligned correctly. */
static unsigned portBASE_TYPE uxNextRxBuffer = 0, uxNextTxBuffer = 0;

/* Semaphore used by the ENET interrupt handler to wake the handler task. */
static xSemaphoreHandle xENETSemaphore;

extern int periph_clk_khz;

xTaskHandle xEthIntTask;

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};

/* Standard lwIP netif handlers. */
static void prvInitialiseENETBuffers( void );
static void low_level_init( struct netif *netif );
static err_t low_level_output(struct netif *netif, struct pbuf *p);
static struct pbuf *low_level_input(struct netif *netif);

/* Forward declarations. */
static void  ethernetif_input(/*FSL:struct netif *netif*/void *pParams);

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
low_level_init(struct netif *netif)
{
/*unsigned portLONG*/ int usData;
const unsigned portCHAR ucMACAddress[6] = 
{
  configMAC_ADDR0, configMAC_ADDR1,configMAC_ADDR2,configMAC_ADDR3,configMAC_ADDR4,configMAC_ADDR5
};

//FSL:struct ethernetif *ethernetif = netif->state;
  
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* set MAC hardware address */
  netif->hwaddr[0] = configMAC_ADDR0;
  netif->hwaddr[1] = configMAC_ADDR1;
  netif->hwaddr[2] = configMAC_ADDR2;
  netif->hwaddr[3] = configMAC_ADDR3;
  netif->hwaddr[4] = configMAC_ADDR4;
  netif->hwaddr[5] = configMAC_ADDR5;

  /* maximum transfer unit */
  netif->mtu = configENET_TX_BUFFER_SIZE;
  
  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
 
  /* Do whatever else is needed to initialize interface. */  
  
  /* Enable the ENET clock. */
  //SIM_SCGC2 |= SIM_SCGC2_ENET_MASK;

  /*FSL: allow concurrent access to MPU controller. Example: ENET uDMA to SRAM, otherwise bus error*/
  //MPU_CESR = 0;         
        
  prvInitialiseENETBuffers();
  vSemaphoreCreateBinary( xENETSemaphore );
  
  /* Set the Reset bit and clear the Enable bit */
  //ENET_ECR = ENET_ECR_RESET_MASK;

  /* Wait at least 8 clock cycles */
    
  /*FSL: start MII interface*/
  //mii_init(0, periph_clk_khz/1000/*MHz*/);       
        
  //enet_interrupt_routine
  //set_irq_priority (76, 6);
  //enable_irq(76);//ENET xmit interrupt
  //enet_interrupt_routine
  //set_irq_priority (77, 6);
  //enable_irq(77);//ENET rx interrupt
  //enet_interrupt_routine
  //set_irq_priority (78, 6);
  //enable_irq(78);//ENET error and misc interrupts       
        
  /*
   * Make sure the external interface signals are enabled
   */
  //PORTB_PCR0  = PORT_PCR_MUX(4);//GPIO;//RMII0_MDIO/MII0_MDIO
  //PORTB_PCR1  = PORT_PCR_MUX(4);//GPIO;//RMII0_MDC/MII0_MDC    

  /* Create the task that handles the MAC ENET. */
  xTaskCreate( ethernetif_input, ( signed char * ) "ETH_INT", configETHERNET_INPUT_TASK_STACK_SIZE, (void *)netif, configETHERNET_INPUT_TASK_PRIORITY, &xEthIntTask );  
  
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
//FSL:  struct ethernetif *ethernetif = netif->state;
  u16_t l = 0;
  struct pbuf *q;
  unsigned char *pcTxData = NULL;
  portBASE_TYPE i;
  
  //initiate transfer();
  
#if ETH_PAD_SIZE
  pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

  /* Get a DMA buffer into which we can write the data to send. */
  for( i = 0; i < netifBUFFER_WAIT_ATTEMPTS; i++ )
  {
    if( xENETTxDescriptors[ uxNextTxBuffer ].status & TX_BD_R )
    {
      /* Wait for the buffer to become available. */
      vTaskDelay( netifBUFFER_WAIT_DELAY );
    }
    else
    {
      #ifdef NBUF_LITTLE_ENDIAN
        pcTxData = (unsigned char *)((uint32_t)xENETTxDescriptors[ uxNextTxBuffer ].data);
      #else
        pcTxData = xENETTxDescriptors[ uxNextTxBuffer ].data;
      #endif
      break;
    }
  }

  if( pcTxData == NULL ) 
  {
    /* For break point only. */
    portNOP();

    return ERR_BUF;
  }
  else
  {
    for(q = p; q != NULL; q = q->next) 
    {
      /* Send the data from the pbuf to the interface, one pbuf at a
         time. The size of the data in each pbuf is kept in the ->len
         variable. */
      memcpy( &pcTxData[l], (u8_t*)q->payload, q->len );
      l += q->len;
    }    
  }
  
  //signal that packet should be sent();
        
  /* Setup the buffer descriptor for transmission */
  #ifdef NBUF_LITTLE_ENDIAN
  xENETTxDescriptors[ uxNextTxBuffer ].length = l;//nbuf->length + ETH_HDR_LEN;
  #else
  xENETTxDescriptors[ uxNextTxBuffer ].length = l;//nbuf->length + ETH_HDR_LEN;
  #endif
  xENETTxDescriptors[ uxNextTxBuffer ].status |= (TX_BD_R | TX_BD_L);
                
  #ifdef ENHANCED_BD
  xENETTxDescriptors[ uxNextTxBuffer ].bdu = 0x00000000;
  xENETTxDescriptors[ uxNextTxBuffer ].ebd_status = TX_BD_INT | TX_BD_TS;// | TX_BD_IINS | TX_BD_PINS;
  #endif
                
  /* Continue the Tx DMA task (in case it was waiting for a new TxBD) */
  //ENET_TDAR = ENET_TDAR_TDAR_MASK;

  uxNextTxBuffer++;
  if( uxNextTxBuffer >= configNUM_ENET_TX_BUFFERS )
  {
    uxNextTxBuffer = 0;
  }

#if ETH_PAD_SIZE
  pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
  
  //LINK_STATS_INC(link.xmit);

  return ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *
low_level_input(struct netif *netif)
{
//FSL:  struct ethernetif *ethernetif = netif->state;
  u32_t l = 0;
  struct pbuf *p, *q;
  u16_t len;
  #ifdef NBUF_LITTLE_ENDIAN
  u8_t *data_temp;
  #endif

  ( void ) netif;
  
  l = 0;
  p = NULL;
  
  /* Obtain the size of the packet and put it into the "len"
     variable. */
  #ifdef NBUF_LITTLE_ENDIAN
  len = (xENETRxDescriptors[ uxNextRxBuffer ].length);
  #else
  len = xENETRxDescriptors[ uxNextRxBuffer ].length;
  #endif

  if( ( len != 0 ) && ( ( xENETRxDescriptors[ uxNextRxBuffer ].status & RX_BD_E ) == 0 ) )
  {  
  
     #if ETH_PAD_SIZE
     len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
     #endif

     /* We allocate a pbuf chain of pbufs from the pool. */
     p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
  
     if (p != NULL) 
     {
        #if ETH_PAD_SIZE
        pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
        #endif

        /* We iterate over the pbuf chain until we have read the entire
         * packet into the pbuf. */
        for(q = p; q != NULL; q = q->next) 
        {
           /* Read enough bytes to fill this pbuf in the chain. The
            * available data in the pbuf is given by the q->len
            * variable.
            * This does not necessarily have to be a memcpy, you can also preallocate
            * pbufs for a DMA-enabled MAC and after receiving truncate it to the
            * actually received size. In this case, ensure the tot_len member of the
            * pbuf is the sum of the chained pbuf len members.
            */
            #ifdef NBUF_LITTLE_ENDIAN
            data_temp = (u8_t *)((u32_t)xENETRxDescriptors[ uxNextRxBuffer ].data);
            memcpy((u8_t*)q->payload, &( data_temp[l] ), q->len);
            #else
            memcpy((u8_t*)q->payload, &( xENETRxDescriptors[ uxNextRxBuffer ].data[l] ), q->len);
            #endif
            l = l + q->len;
                
            #ifdef ENHANCED_BD
	    //FSL: not implemented at stack level
            //rx_packet->ebd_status = RxNBUF[index_rxbd].ebd_status;
	    #ifdef NBUF_LITTLE_ENDIAN	
            //rx_packet->timestamp = __REV(RxNBUF[index_rxbd].timestamp);
            //rx_packet->length_proto_type = __REVSH(RxNBUF[index_rxbd].length_proto_type);
	    //rx_packet->payload_checksum = __REVSH(RxNBUF[index_rxbd].payload_checksum);
            #else
            //rx_packet->timestamp = RxNBUF[index_rxbd].timestamp;
	    //rx_packet->length_proto_type = RxNBUF[index_rxbd].length_proto_type;
	    //rx_packet->payload_checksum = RxNBUF[index_rxbd].payload_checksum;
            #endif
            #endif
        }

        #if ETH_PAD_SIZE
        pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
        LINK_STATS_INC(link.recv);
        #endif
     }
     else
     {
        //drop packet();
        //LINK_STATS_INC(link.memerr);
        //LINK_STATS_INC(link.drop);     
     }
    
     //acknowledge that packet has been read();
     /* Free the descriptor. */
     xENETRxDescriptors[ uxNextRxBuffer ].status |= RX_BD_E;
     // ENET_RDAR = ENET_RDAR_RDAR_MASK;
    
     uxNextRxBuffer++;
     if( uxNextRxBuffer >= configNUM_ENET_RX_BUFFERS )
     {
        uxNextRxBuffer = 0;
     }
  } 
  
  return p;  
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
static void
ethernetif_input(/*FSL:struct netif *netif*/void *pParams)
{
//FSL:  struct ethernetif *ethernetif;
  struct netif *netif;
  struct eth_hdr *ethhdr;
  struct pbuf *p;

//FSL:  ethernetif = netif->state;
  netif = (struct netif*) pParams;

  for( ;; )
  {  
    do
    {
      /* move received packet into a new pbuf */
      p = low_level_input(netif);
      /* no packet could be read, silently ignore this */
      if (p == NULL)
      {
	/* No packet could be read.  Wait a for an interrupt to tell us
	   there is more data available. */
	xSemaphoreTake( xENETSemaphore, netifBLOCK_TIME_WAITING_FOR_INPUT );        
      }
    }while( p == NULL );  
    /* points to packet payload, which starts with an Ethernet header */
    ethhdr = p->payload;

        if (netif->input(p, netif)!=ERR_OK)
         { LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
           pbuf_free(p);
           p = NULL;
         }

    }
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t
ethernetif_init(struct netif *netif)
{
  struct ethernetif *ethernetif;

  LWIP_ASSERT("netif != NULL", (netif != NULL));
    
  ethernetif = mem_malloc(sizeof(struct ethernetif));
  if (ethernetif == NULL) {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
    return ERR_MEM;
  }

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

  netif->state = ethernetif;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;
  
  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
  
  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}

/*-----------------------------------------------------------*/

static void prvInitialiseENETBuffers( void )
{
unsigned portBASE_TYPE ux;
unsigned char *pcBufPointer;

  pcBufPointer = &( xENETTxDescriptors_unaligned[ 0 ] );
  while( ( ( unsigned long ) pcBufPointer & 0x0fUL ) != 0 )
  {
    pcBufPointer++;
  }
  
  xENETTxDescriptors = ( NBUF * ) pcBufPointer;
  
  pcBufPointer = &( xENETRxDescriptors_unaligned[ 0 ] );
  while( ( ( unsigned long ) pcBufPointer & 0x0fUL ) != 0 )
  {
    pcBufPointer++;
  }
  
  xENETRxDescriptors = ( NBUF * ) pcBufPointer;

  /* Setup the buffers and descriptors. */
  pcBufPointer = &( ucENETTxBuffers[ 0 ] );
  while( ( ( unsigned long ) pcBufPointer & 0x0fUL ) != 0 )
  {
    pcBufPointer++;
  }

  for( ux = 0; ux < configNUM_ENET_TX_BUFFERS; ux++ )
  {
    xENETTxDescriptors[ ux ].status = TX_BD_TC;
    #ifdef NBUF_LITTLE_ENDIAN
    xENETTxDescriptors[ ux ].data = (uint8_t *)((uint32_t)pcBufPointer);
    #else
    xENETTxDescriptors[ ux ].data = pcBufPointer;
    #endif
    pcBufPointer += configENET_TX_BUFFER_SIZE;
    xENETTxDescriptors[ ux ].length = 0;
    #ifdef ENHANCED_BD
    xENETTxDescriptors[ ux ].ebd_status = TX_BD_IINS | TX_BD_PINS;
    #endif
  }

  pcBufPointer = &( ucENETRxBuffers[ 0 ] );
  while( ( ( unsigned long ) pcBufPointer & 0x0fUL ) != 0 )
  {
    pcBufPointer++;
  }
  
  for( ux = 0; ux < configNUM_ENET_RX_BUFFERS; ux++ )
  {
      xENETRxDescriptors[ ux ].status = RX_BD_E;
      xENETRxDescriptors[ ux ].length = 0;
      #ifdef NBUF_LITTLE_ENDIAN
      xENETRxDescriptors[ ux ].data = (uint8_t *)((uint32_t)pcBufPointer);
      #else
      xENETRxDescriptors[ ux ].data = pcBufPointer;
      #endif
      pcBufPointer += configENET_RX_BUFFER_SIZE;
      #ifdef ENHANCED_BD
      xENETRxDescriptors[ ux ].bdu = 0x00000000;
      xENETRxDescriptors[ ux ].ebd_status = RX_BD_INT;
      #endif    
  }

  /* Set the wrap bit in the last descriptors to form a ring. */
  xENETTxDescriptors[ configNUM_ENET_TX_BUFFERS - 1 ].status |= TX_BD_W;
  xENETRxDescriptors[ configNUM_ENET_RX_BUFFERS - 1 ].status |= RX_BD_W;

  uxNextRxBuffer = 0;
  uxNextTxBuffer = 0;
}
/*-----------------------------------------------------------*/

void vENETISRHandler( void )
{  
portBASE_TYPE xHighPriorityTaskWoken = pdFALSE;

    /* A packet has been received.  Wake the handler task. */
    xSemaphoreGiveFromISR( xENETSemaphore, &xHighPriorityTaskWoken );
    portEND_SWITCHING_ISR( xHighPriorityTaskWoken );
}

#endif /* 0 */
