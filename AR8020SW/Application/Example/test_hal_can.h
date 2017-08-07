#ifndef __TEST_HAL_CAN_H__
#define __TEST_HAL_CAN_H__

#ifdef __cplusplus
extern "C"
{
#endif


void command_TestCanInit(unsigned char *ch, unsigned char *br, unsigned char *acode,unsigned char *amask, unsigned char *format);

void command_TestCanSetInt(unsigned char *ch, unsigned char *flag);

void command_TestCanTx(unsigned char *ch, unsigned char *id, unsigned char *len,unsigned char *format, unsigned char *type);

void command_TestCanRx(void);


#ifdef __cplusplus
}
#endif 

#endif
