#ifndef __TEST_BB_
#define __TEST_BB_

#ifdef __cplusplus
extern "C"
{
#endif


void command_test_BB_uart(char *index_str);

void BB_grdEventHandler(void *p);

void BB_skyEventHandler(void *p);


#ifdef __cplusplus
}
#endif

#endif
