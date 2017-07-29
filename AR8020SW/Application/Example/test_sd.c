#include "debuglog.h"
#include "interrupt.h"
#include "test_sd.h"
#include "cpu_info.h"
#include "cmsis_os.h"
#include "hal.h"
#include "systicks.h"
#include <string.h>
#include <stdlib.h>
#include "../../../Driver/Chip/AR8020/SD/Inc/sd_host.h"

FIL outFile, inFile;
FATFS SDFatFs;  /* File system object for SD card logical drive */
// FIL MyFile, MyFileIn;     /* File object */
char *SDPath = "SD:/"; 

extern Diskio_drvTypeDef  SD_Driver;
extern unsigned int command_str2uint(char *str);

static FRESULT open_append (
    FIL* fp,            /* [OUT] File object to create */
    const TCHAR* path    /* [IN]  File name to be opened */
)
{
    FRESULT fr;

    /* Opens an existing file. If not exist, creates a new file. */
    fr = f_open(fp, path, FA_WRITE | FA_OPEN_ALWAYS);
    if (fr == FR_OK) {
        /* Seek to end of the file to append data */
        fr = f_lseek(fp, f_size(fp));
        dlog_info("%d, f_size(fp) = %d", __LINE__, f_size(fp));
        if (fr != FR_OK)
            f_close(fp);
    }
    return fr;
}

void TestWR()
{
	uint32_t info;

	ENUM_HAL_SD_CTRL cmd = HAL_SD_GET_SECTOR_COUNT;
	if (HAL_SD_Ioctl(cmd, &info) != HAL_OK)
	{
		dlog_info("ioctl failed\n");
	}

	cmd = HAL_SD_GET_SECTOR_SIZE;
	if (HAL_SD_Ioctl(cmd, &info) != HAL_OK)
	{
		dlog_info("ioctl failed\n");
	}

	cmd = HAL_SD_GET_CSD_VERSION;
	if (HAL_SD_Ioctl(cmd, &info) != HAL_OK)
	{
		dlog_info("ioctl failed\n");
	}

	cmd = HAL_SD_GET_MANUID;
	if (HAL_SD_Ioctl(cmd, &info) != HAL_OK)
	{
		dlog_info("ioctl failed\n");
	}

	cmd = HAL_SD_GET_TRAN_SPEED;
	if (HAL_SD_Ioctl(cmd, &info) != HAL_OK)
	{
		dlog_info("ioctl failed\n");
	}

	cmd = HAL_SD_GET_CARD_STATUS;
	if (HAL_SD_Ioctl(cmd, &info) != HAL_OK)
	{
		dlog_info("ioctl failed\n");
	}

	int i = 0;
	uint32_t sect = 0;
	while(i < 10)
	{
		HAL_SD_Write(sect, 0x81000000+i*0x1400, 10);
		HAL_SD_Read(0x81100000, sect, 2);
		dlog_info("%d",i);
		i++;
		sect += 1;
	}
/* 	HAL_SD_Deinit(); */
}

void TestFatFs()
{
	FRESULT res;
	uint32_t byteswritten, bytesread;
	static char *name;

	name = malloc(128);
	memset(name, 0, 128);
	memcpy(name, "test.txt", strlen("test.txt"));
    FIL MyFile, MyFileIn;
	uint32_t u32_start; 

	uint32_t rw_size = 0x1000000;
    
	if (FR_OK != f_chdrive("SD:"))
	{
		dlog_info("%d chdrive to SD:\\ fail", __LINE__);
    }
    
    res = f_open(&MyFile, (const TCHAR*)(name), FA_OPEN_ALWAYS | FA_WRITE | FA_OPEN_APPEND);
    if (res != FR_OK) 
    {
    	dlog_info("f_open error, res = %d", res);
    }
    else
    {
/*     	dlog_info("%d f_open success!", __LINE__); */
    	u32_start = SysTicks_GetTickCount();
        if (FR_OK != f_chdrive("SD:"))
        {
            dlog_info("%d chdrive to SD:\\ fail", __LINE__);
        }
    	res = f_write(&MyFile, (const void*)(0x81000000), 
    				rw_size, (void *)&byteswritten);    	

        dlog_info("write %d byte, used %d ms, speed = %d kB/S", 
                    rw_size, 
                    SysTicks_GetTickCount() - u32_start, 
                    rw_size/(SysTicks_GetTickCount() - u32_start));

    	if ((byteswritten == 0) || (res != FR_OK))
    	{
    		dlog_info("f_write error!");
    	}
    	else
    	{
/*     		dlog_info("f_write success!"); */
    		if ( FR_OK == f_close(&MyFile))
    		{
        		dlog_info("f_close success");
    		}
    		
            if (FR_OK != f_chdrive("SD:"))
            {
                dlog_info("%d chdrive to SD:\\ fail", __LINE__);
            }
/*     		if (f_open(&MyFile, (const TCHAR*)name, FA_READ) != FR_OK) */
            if(FR_OK != f_open(&MyFile, (const TCHAR*)(name), FA_OPEN_EXISTING | FA_READ))
    		{
    			dlog_info("f_open error!");
    		}
    		else
    		{
    			bytesread = rw_size;
    			u32_start = SysTicks_GetTickCount();
                if (FR_OK != f_chdrive("SD:"))
                {
                    dlog_info("%d chdrive to SD:\\ fail", __LINE__);
                }
    			res = f_read(&MyFile, (void*)(0x81000000), 
    						bytesread, (UINT*)&bytesread);
                dlog_info("read %d byte, used %d ms, speed = %d kB/S", 
                            rw_size, 
                            SysTicks_GetTickCount() - u32_start, 
                            rw_size/(SysTicks_GetTickCount() - u32_start));

    			if ((bytesread == 0) || (res != FR_OK))
    			{
    				dlog_info("f_read error!");
    			}
    			else
    			{
/*     				dlog_info("f_read success!"); */
    				f_close(&MyFile);

    				if ((bytesread != byteswritten))
    				{
    					dlog_error("f_close error!");
    				}
    				else
    				{
/*     					dlog_info("f_close success!"); */
    				}
    			}
    		}
    	}
	}

	free(name);
}

void TestFatFs1()
{
	FRESULT res;                                          /* FatFs function common result code */
	uint32_t byteswritten, bytesread;                     /* File write/read counts */

    int read, write;
    uint8_t *pcm_buffer = malloc(2048);
    memset(pcm_buffer, 0, 2048);

	/*##-1- Link the micro SD disk I/O driver ##################################*/
	if (FATFS_LinkDriver(&SD_Driver, SDPath) == 0)
	{
		dlog_info("Link success!");
		/*##-2- Register the file system object to the FatFs module ##############*/
		if ((res = f_mount(&SDFatFs, (TCHAR const*)SDPath, 1)) != FR_OK)
		{
			/* FatFs Initialization Error */
			dlog_info("f_mount = %d", res);
			dlog_info("f_mount error!");
		}
		else
		{
			dlog_info("f_mount success!");
			// res = f_mkfs((TCHAR const*)SDPath, 0, 0);
			// dlog_info("f_mkfs = %d\n", res);
			/*##-4- Create and Open a new text file object with write access #####*/
/* 			if (f_open(&outFile, "a.mp3", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) */
            if (f_open(&outFile, (const TCHAR*)"a.mp3", FA_WRITE | FA_OPEN_APPEND) != FR_OK)
            // if (open_append(&outFile, "a.mp3"))
			{
				dlog_info("f_open out error!");
			}
			else
			{
                dlog_info("f_open a.mp3 success!");
/*                 f_printf(&outFile, "%s %s\n", __TIME__, __DATE__); */
			}

			if (f_open(&inFile, (const TCHAR*)"a.wav", FA_READ) != FR_OK)
			{
				dlog_info("f_open in error!");
			}
            else
            {
                dlog_info("f_open a.wav success!");                
            }
			
			dlog_info("f_open success!");
			/*##-5- Write data to the text file ################################*/
			
			//do
			//{
				res = f_read(&inFile, pcm_buffer, 10, &read);
                dlog_info("%d res = %d", __LINE__, res);
                
                dlog_info("pcm_buffer = %s", pcm_buffer);

                if ((read == 0) || (res != FR_OK))
                {
                    //break;
                }
#if 0
				if ((read == 0) || (res != FR_OK))
				{
					dlog_info("f_read error!");
				}
				else
				{
					dlog_info("read %d",read);
					res = f_write(&outFile, pcm_buffer, read, (void *)&write);
				}
#endif
                res = f_write(&outFile, pcm_buffer, read, (void *)&write);
                dlog_info("%d res = %d", __LINE__, res);
			//} while(read > 0);

			res = f_close(&inFile);
            dlog_info("%d res = %d", __LINE__, res);
			res = f_close(&outFile);
            dlog_info("%d res = %d", __LINE__, res);

			dlog_info("complete!");		
		}
	}

	/*##-11- Unlink the micro SD disk I/O driver ###############################*/
	free(pcm_buffer);
	res = FATFS_UnLinkDriver(SDPath);
}

void TestFatFs2()
{
	FRESULT res;                                          /* FatFs function common result code */
	uint32_t byteswritten, bytesread = 1;                     /* File write/read counts */
	uint8_t wtext[] = "This is STM32 working with FatFs"; /* File write buffer */
	uint8_t rtext[2048];                                   /* File read buffer */
	FIL fileIn;
	/*##-1- Link the micro SD disk I/O driver ##################################*/
	if (FATFS_LinkDriver(&SD_Driver, SDPath) == 0)
	{
		dlog_info("Link success!\n");
		/*##-2- Register the file system object to the FatFs module ##############*/
		if ((res = f_mount(&SDFatFs, (TCHAR const*)SDPath, 1)) != FR_OK)
		{
			/* FatFs Initialization Error */
			dlog_info("f_mount = %d\n", res);
			dlog_info("f_mount error!\n");
		}
		else
		{
			dlog_info("f_mount success!\n");
			// res = f_mkfs((TCHAR const*)SDPath, 0, 0);
			// dlog_info("f_mkfs = %d\n", res);
			/*##-4- Create and Open a new text file object with write access #####*/
			// if (f_open(&MyFile, "STM32.TXT", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
			// {
			// 	/* 'STM32.TXT' file Open for write Error */
			// 	dlog_info("f_open error!\n");
			// }
			if (f_open(&fileIn, (const TCHAR*)("a.wav"), FA_READ) != FR_OK)
			{
				/* 'STM32.TXT' file Open for write Error */
				dlog_info("f_open error!\n");
			}
			else
			{
				dlog_info("f_open success!\n");
				/*##-5- Write data to the text file ################################*/
				while(bytesread)
				{
				res = f_read(&fileIn, rtext, 1024, (void *)&bytesread);
				dlog_info("read %d \n", bytesread);
				}


			}
		
		}
	}

	/*##-11- Unlink the micro SD disk I/O driver ###############################*/
	res = FATFS_UnLinkDriver(SDPath);
}


void command_initSdcard()
{
    // HAL_SD_InitIRQ();
	int res;
	if ((res = HAL_SD_Init()) != 0)
	{
		return;
	}

}

void command_sd_release()
{
    HAL_SD_Deinit();
}

void OS_TestRawWR_Handler(void const * argument)
{
	uint32_t sect;
	uint32_t u32_start; 
	uint32_t u32_end; 
	uint32_t u32_diff; 
	uint32_t totol_sects;

    uint32_t NUM_OF_BLOCK = 64;
    uint32_t TIMES = 10000;
    uint32_t START_BLOCK = 1000;
    
	uint32_t sect_multi;
	totol_sects = 30541 * 1024;
	
#if 0
	if (HAL_OK == HAL_SD_Erase(0, totol_sects) )
	{
		dlog_info("erase %d sects, used %d ms", totol_sects, SysTicks_GetTickCount() - u32_start);
	}
	else
	{
		dlog_info("error");
	}
#endif

    dlog_info("in testing, please wait for several seconds...");
	
    u32_start = SysTicks_GetTickCount();
    for (sect_multi = 0; sect_multi < TIMES; sect_multi++)
    {
        if ( HAL_OK == HAL_SD_Write(START_BLOCK + sect_multi * NUM_OF_BLOCK, 
                                    0x21006000, 
                                    NUM_OF_BLOCK))
        {       
/*          dlog_info("times = %d", sect_multi); */
        }
        else
        {
            dlog_error("error");
        }
    }
    
    u32_diff = SysTicks_GetTickCount() - u32_start;
    dlog_info("write %d sects, used %d ms, speed = %d kB/S", 
                NUM_OF_BLOCK * TIMES, 
                u32_diff,
                NUM_OF_BLOCK * TIMES * 512/u32_diff);

    u32_start = SysTicks_GetTickCount();
	for (sect_multi = 0; sect_multi < TIMES; sect_multi++)
	{
		if ( HAL_OK == HAL_SD_Read(0x21006000, 
									START_BLOCK + sect_multi * NUM_OF_BLOCK, 
									NUM_OF_BLOCK))
/*
		if ( HAL_OK == HAL_SD_Read(0x81000000 - DTCM_CPU0_DMA_ADDR_OFFSET, 
									sect_multi * NUM_OF_BLOCK, 
									NUM_OF_BLOCK))
*/
		{		
/* 			dlog_info("times = %d", sect_multi); */
		}
		else
		{
			dlog_error("error");
		}
	}
	
	u32_diff = SysTicks_GetTickCount() - u32_start;
    dlog_info("read %d sects, used %d ms, speed = %d kB/S", 
                NUM_OF_BLOCK * TIMES, 
                u32_diff,
                NUM_OF_BLOCK * TIMES * 512/u32_diff);
    
#if 0
	for (sect_multi = 0; sect_multi < 10; sect_multi++)
	{
		u32_start = SysTicks_GetTickCount();
/*
		if ( HAL_OK == HAL_SD_Write(sect_multi * NUM_OF_BLOCK, 
		                            0x81000000 - DTCM_CPU0_DMA_ADDR_OFFSET, 
		                            NUM_OF_BLOCK) )
*/
        if ( HAL_OK == HAL_SD_Write(0, 0x21000000, NUM_OF_BLOCK) )              // sram
		{
			dlog_info("write %d sects, sect_multi = %d, used %d ms, speed = %d kB/S", 
			            NUM_OF_BLOCK, 
			            sect_multi, 
						SysTicks_GetTickCount() - u32_start, 
						NUM_OF_BLOCK*512/(SysTicks_GetTickCount() - u32_start));
		}
		else
		{
			dlog_info("error");
		}
	}

	for (sect_multi = 0; sect_multi < 10; sect_multi++)
	{
		u32_start = SysTicks_GetTickCount();
		if ( HAL_OK == HAL_SD_Read(0x21000000, 
									sect_multi * NUM_OF_BLOCK, 
									NUM_OF_BLOCK))
/*
		if ( HAL_OK == HAL_SD_Read(0x81000000 - DTCM_CPU0_DMA_ADDR_OFFSET, 
									sect_multi * NUM_OF_BLOCK, 
									NUM_OF_BLOCK))
*/
		{
			dlog_info("read %d sects, sect_multi = %d, used %d ms, speed = %d kB/S", 
			            NUM_OF_BLOCK, 
			            sect_multi, 
						SysTicks_GetTickCount() - u32_start,
						NUM_OF_BLOCK*512/(SysTicks_GetTickCount() - u32_start));			
		}
		else
		{
			dlog_info("error");
		}
	}
#endif

	dlog_info("task finished");
	
	for (;;)
	{
		HAL_Delay(1500);
	}
}

void OS_TestSD_Erase_Handler(void const * argument)
{
	uint32_t totol_sects = 30541 * 1024;
	uint32_t u32_start = SysTicks_GetTickCount();
	
	if (HAL_OK == HAL_SD_Erase(0, totol_sects) )
	{
		dlog_info("erase %d sects, used %d ms", totol_sects, SysTicks_GetTickCount() - u32_start);
	}
	else
	{
		dlog_info("error");
	}
	
        for (;;)
	{
		HAL_Delay(1500);
	}
}


/*
void OS_TestRawWR()
{	
	osThreadDef(TestRawWR_Handler, OS_TestRawWR_Handler, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(TestRawWR_Handler), NULL);
}
*/

/*
void OS_TestSD_Erase()
{
	osThreadDef(TestSD_Erase, OS_TestSD_Erase_Handler, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(TestSD_Erase), NULL);
}
*/

void Test_hal_read()
{
    if (HAL_OK == HAL_SD_Read(0x81000000 - DTCM_CPU0_DMA_ADDR_OFFSET, 0, 13))
    {
        dlog_info("read done");
    }
}

extern SD_HandleTypeDef sdhandle;
void TestFatFs_with_usb()
{
	FRESULT res;                                          /* FatFs function common result code */
	uint32_t byteswritten, bytesread;                     /* File write/read counts */
	uint8_t wtext[] = "This is STM32 working with FatFs"; /* File write buffer */
	uint8_t rtext[100];                                   /* File read buffer */
	static char name[] = "myfile1";

	uint32_t u32_start; 

	dlog_info("%d, sdhandle.inited = %d", __LINE__, sdhandle.inited);
    
	if (FATFS_LinkDriver(&SD_Driver, SDPath) != 0)
	{
		dlog_info("Link error!");
		return;
	}
	
	dlog_info("Link success!");

	if ((res = f_mount(&SDFatFs, (TCHAR const*)SDPath, 1)) != FR_OK)
	{
		dlog_info("f_mount = %d", res);
		dlog_info("f_mount error!");
	}
	else
	{
		dlog_info("%d f_mount success!", __LINE__);
		dlog_output(1000);
#if 0
		name[6] += 1;
		if (f_open(&MyFile, name, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) 
		{
			dlog_info("f_open error!");
		}
		else
		{
			dlog_info("%d f_open success!", __LINE__);
	
			u32_start = SysTicks_GetTickCount();
			res = f_write(&MyFile, (const void*)(0x81000000 - DTCM_CPU0_DMA_ADDR_OFFSET), 
						0x1000000, (void *)&byteswritten);
			dlog_info("%d, write %d ms", __LINE__, SysTicks_GetTickCount() - u32_start);
			
			if ((byteswritten == 0) || (res != FR_OK))
			{
				dlog_info("f_write error!");
			}
			else
			{
				dlog_info("f_write success!");
				f_close(&MyFile);

				if (f_open(&MyFile, name, FA_READ) != FR_OK)
				{
					dlog_info("f_open error!");
				}
				else
				{
					bytesread = 0x1000000;
					u32_start = SysTicks_GetTickCount();
					res = f_read(&MyFile, (void*)(0x81000000 - DTCM_CPU0_DMA_ADDR_OFFSET), 
								bytesread, (UINT*)&bytesread);
					dlog_info("%d, read %d ms", __LINE__, SysTicks_GetTickCount() - u32_start);

					if ((bytesread == 0) || (res != FR_OK))
					{
						dlog_info("f_read error!");
					}
					else
					{
						dlog_info("f_read success!");
						f_close(&MyFile);

						if ((bytesread != byteswritten))
						{
							dlog_info("f_close error!");
						}
						else
						{
							dlog_info("f_close success!");
						}
					}
				}
			}
		}
#endif
	
	}

    
	dlog_info("%d, sdhandle.inited = %d", __LINE__, sdhandle.inited);

	res = FATFS_UnLinkDriver(SDPath);
}

void TestRawWR_SDRAM()
{
    uint32_t sect;
	uint32_t u32_start; 
	uint32_t u32_end; 
	uint32_t u32_diff; 
	uint32_t totol_sects;

    uint32_t NUM_OF_BLOCK = 30000;
    uint32_t TIMES = 10;
    uint32_t START_BLOCK = 1000;
    
	uint32_t sect_multi;
	totol_sects = 30541 * 1024;
	
    
	for (sect_multi = 0; sect_multi < TIMES; sect_multi++)
	{
		u32_start = SysTicks_GetTickCount();
		if ( HAL_OK == HAL_SD_Write(START_BLOCK + sect_multi * NUM_OF_BLOCK, 
		                            0x81000000, 
		                            NUM_OF_BLOCK) )
		{
		    u32_diff = SysTicks_GetTickCount() - u32_start;
			dlog_info("write %d sects, sect_multi = %d, used %d ms, speed = %d kB/S", 
			            NUM_OF_BLOCK, 
			            sect_multi, 
						u32_diff, 
						NUM_OF_BLOCK*512/u32_diff);
		}
		else
		{
			dlog_info("error");
		}
	}

	for (sect_multi = 0; sect_multi < TIMES; sect_multi++)
	{
		u32_start = SysTicks_GetTickCount();
		if ( HAL_OK == HAL_SD_Read(0x81000000, 
									START_BLOCK + sect_multi * NUM_OF_BLOCK, 
									NUM_OF_BLOCK))
		{
		    u32_diff = SysTicks_GetTickCount() - u32_start;
			dlog_info("read %d sects, sect_multi = %d, used %d ms, speed = %d kB/S", 
			            NUM_OF_BLOCK, 
			            sect_multi, 
						u32_diff,
						NUM_OF_BLOCK*512/u32_diff);			
		}
		else
		{
			dlog_info("error");
		}
	}

	dlog_info("task finished");	
}

static void test_multi_read()
{
	uint32_t sect;
	uint32_t u32_start; 
	uint32_t u32_end; 
	uint32_t u32_diff; 
	uint32_t totol_sects;

    uint32_t NUM_OF_BLOCK = 64;
    uint32_t TIMES = 100;
    uint32_t START_BLOCK = 1000;
    
	uint32_t sect_multi;
	totol_sects = 30541 * 1024;

	if ( HAL_OK == HAL_SD_Read_test(0x21006000, 
							START_BLOCK + NUM_OF_BLOCK, 
							NUM_OF_BLOCK))
	{
        dlog_info("done");
	}						
}

static void test_multi_write()
{
	uint32_t sect;
	uint32_t u32_start; 
	uint32_t u32_end; 
	uint32_t u32_diff; 
	uint32_t totol_sects;

    uint32_t NUM_OF_BLOCK = 64;
    uint32_t TIMES = 100;
    uint32_t START_BLOCK = 1000;
    
	uint32_t sect_multi;
	totol_sects = 30541 * 1024;

    if ( HAL_OK == HAL_SD_Write_test(START_BLOCK, 0x21006000, NUM_OF_BLOCK))
	{
        dlog_info("done");
	}						
}


void command_SdcardFatFs(char *argc)
{	
	uint8_t choise;
	choise = command_str2uint(argc);
    uint32_t i;
    
	switch (choise)
	{
		case 0:
			TestWR();
		break;

		case 1:
		    for (i = 0; i < 10;i++)
		    {
			    dlog_info("time = %d", i);
			    TestFatFs();
			}
		break;

		case 2:
			TestFatFs1();
		break;

		case 3:
			TestFatFs2();
		break;

		case 4:
/* 			OS_TestRawWR(); */
		break;

		case 5:
/* 			OS_TestSD_Erase(); */
		break;

		case 6:
		    Test_hal_read();
		break;

		case 7:
		    TestFatFs_with_usb();
		break;

		case 8:
		    TestRawWR_SDRAM();
		break;

		case 9:
		    test_multi_read();
		break;

        case 10:
            test_multi_write();
        break;

		default: break;
	}
}

