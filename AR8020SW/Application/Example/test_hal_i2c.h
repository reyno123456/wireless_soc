#ifndef __TEST_HAL_I2C__
#define __TEST_HAL_I2C__


void command_TestHalI2cInit(uint8_t *ch, uint8_t *i2c_addr, uint8_t *speed);
void command_TestHalI2cWrite(uint8_t *ch, uint8_t *subAddr, uint8_t *subAddrLen, uint8_t *data, uint8_t *dataLen);
void command_TestHalI2cRead(uint8_t *ch, uint8_t *subAddr, uint8_t *subAddrLen, uint8_t *dataLen);

#endif
