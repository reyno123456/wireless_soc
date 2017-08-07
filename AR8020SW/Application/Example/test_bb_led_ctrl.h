#ifndef __TEST_BB_LED_CTRL_H__
#define __TEST_BB_LED_CTRL_H__

#ifdef __cplusplus
extern "C"
{
#endif

void BB_ledGpioInit(void);

void BB_ledLock(void);

void BB_ledUnlock(void);

void BB_EventHandler(void *p);


#ifdef __cplusplus
}
#endif

#endif
