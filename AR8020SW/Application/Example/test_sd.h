#ifndef __TEST__SD__H
#define __TEST__SD__H

#ifdef __cplusplus
extern "C"
{
#endif


#include "ff.h"
#include "ff_gen_drv.h"
#include "hal_sd.h"

void TestWR(void);
void TestFatFs(void);
void TestFatFs2();
void TestFatFs1();
void TestSDIRQ(void);
void command_initSdcard();
void command_SdcardFatFs(char *argc);
void TestRawWR();
void OS_TestRawWR_Handler(void const * argument);
void OS_TestRawWR();
void OS_TestSD_Erase_Handler(void const * argument);
void OS_TestSD_Erase();
void Test_hal_read();
void TestFatFs_with_usb();
void command_sd_release();


#ifdef __cplusplus
}
#endif 

#endif
