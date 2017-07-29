///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <Utility.h>
//   @author Max.Kao@ite.com.tw
//   @date   2014/09/11
//   @fileversion: ITE_MHLRX_SAMPLE_V1.01
//******************************************/

#ifndef _Utility_h_
#define _Utility_h_
#include "it_typedef.h"


#ifdef MEGAWIN82516
	#define CLOCK  11059200L
	#define MachineCycle    		12
#else
	#ifdef _IT6602_BOARD_
	#define CLOCK 				27000000L
	#else
	#define CLOCK 				24000000L
	#endif

	#define MachineCycle    		6
#endif



#ifdef MYSON_8957
#define ENABLE_I2C_PROTOCAL_CMD
#define ENABLE_ITE_I2C_DEBUG_MODE
#define BAUD_RATE      		115200L	//  115200L	//     19200L	//  57600L
#else
#define BAUD_RATE      		57600L	//57600L	//  115200L	//     19200L	//  57600L
#endif


#define MICROSECONDS_TIMER(microseconds) (((CLOCK /1000000)* (microseconds))/MachineCycle)
#define COUNTER1ms			MICROSECONDS_TIMER(1000)		 	// 1000 us = 1 ms
#define COUNTER10ms		MICROSECONDS_TIMER(10000)		 	// 10000 us = 10 ms
#define Tick100us			MICROSECONDS_TIMER(100)		//Timer 0 100us(IR sampling time)
#define Tick1ms				(65536 - COUNTER1ms)
#define Tick10ms				(65536 - COUNTER10ms)
#define IR_COUNTER_VALUE	(65536 - Tick100us)
#define BAUD_SETTING            (65536L - (CLOCK / (32L * BAUD_RATE)))




#define CAT_HDMI_PORTA 0
#define CAT_HDMI_PORTB 1
#define SIZEOF_CSCMTX 18
#define SIZEOF_CSCGAIN 6
#define SIZEOF_CSCOFFSET 3



extern BYTE Ena_SP_FUNCTION;


void HoldSystem();
void DelayUS(USHORT us);

//void Dump_CAT6613Reg();
void init_printf(void);
//void Key_handler();
//void init_timer();
USHORT getloopTicCount();

int TimeOutCheck(USHORT timer, USHORT x);
USHORT GetCurrentVirtualTime();

void initialTimer1(void);

#endif
