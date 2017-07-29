#ifndef __TEST_HAL_CAMERA_H__
#define	__TEST_HAL_CAMERA_H__

#include <stdlib.h>
#include <string.h>
#include "debuglog.h"
#include "hal_camera.h"

void command_TestHalCameraInit(unsigned char *rate, unsigned char *mode);

void command_TestCameraWrite(unsigned char *subAddr, unsigned char *value);

void command_TestCameraRead(unsigned char *subAddr);


#endif
