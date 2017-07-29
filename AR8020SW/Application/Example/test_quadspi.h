#ifndef TEST_QUADSPI_H
#define TEST_QUADSPI_H

void command_setQuadSPISpeed(char* speed_str);
void command_testQuadSPISpeedData(void);
void command_initWinbondNorFlash(void);
void command_eraseWinbondNorFlash(char* type_str, char* start_addr_str);
void command_readWinbondNorFlash(char* start_addr_str, char* size_str);
void command_writeWinbondNorFlash(char* start_addr_str, char* size_str, char* val_str);
void command_testAllNorFlashOperations(char* start_addr_str, char* size_str, char* val_str);

#endif

