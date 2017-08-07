#ifndef __TEST_HAL_NV_H__
#define __TEST_HAL_NV_H__

#ifdef __cplusplus
extern "C"
{
#endif


void command_TestNvSkyAutoSearhRcId(void);

void command_TestNvResetBbRcId(void);

void command_TestNvSetBbRcId(uint8_t *id1, uint8_t *id2, uint8_t *id3, uint8_t *id4, uint8_t *id5);


#ifdef __cplusplus
}
#endif 

#endif
