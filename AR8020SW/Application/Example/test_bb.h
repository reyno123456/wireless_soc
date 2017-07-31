#ifndef __TEST_BB_
#define __TEST_BB_


void command_test_BB_uart(char *index_str);

void BB_ledGpioInit(void);

void BB_ledLock(void);

void BB_ledUnlock(void);

void BB_grdEventHandler(void *p);

void BB_skyEventHandler(void *p);

#endif
