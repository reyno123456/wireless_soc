#ifndef TEST_I2C_ADV7611_H
#define TEST_I2C_ADV7611_H

void command_initADV7611(char *index_str);
void command_dumpADV7611Settings(char *index_str);
void command_readADV7611(char *slvAddr, char *regAddr);
void command_writeADV7611(char *slvAddr, char *regAddr, char *regVal);
void command_readADV7611VideoFormat(char *index_str, uint16_t* widthPtr, uint16_t* hightPtr, uint8_t* framteratePtr);
void command_Test24C256Write(char* i2c_num_str,char* i2c_value);
void command_Test24C256Read(char* i2c_num_str);

#endif
