/**
  * @file    sd_core.c
  * @author  Minzha & min.wu
  * @version V1.0.0
  * @date    7-7-2016
  * @brief   source file of sd core.
  *          This file contains:
  *          Initialization/de-initialization functions
  *            I/O operation functions
  *            Peripheral Control functions
  *            Peripheral State functions
  */
#include <stddef.h>
#include <stdint.h>
#include "reg_rw.h"
#include "sd_core.h"
#include "stm32f746xx.h"
#include "FreeRTOSConfig.h"
#include "memory_config.h"
#include "debuglog.h"
#include "systicks.h"

#define SD_TIME_OUT 1000
/**
  * @brief  Get SDMMC Power state.
  * @param  SDMMCx: Pointer to SDMMC register base
  * @retval Power status of the controller. The returned value can be one of the
  *         following values:
  *            - 0x00: Power OFF
  *            - 0x01: Power ON
  */
uint32_t Core_SDMMC_GetPowerState(HOST_REG *SDMMCx)
{
  return (SDMMCx->PWREN & SDMMC_PWREN_0); //for card one
}

/**
  * @brief  Configure the SDMMC command path according to the specified parameters in
  *         SDMMC_CmdInit structure and send the command
  * @param  SDMMCx: Pointer to SDMMC register base
  * @param  Command: pointer to a SDMMC_CmdInitTypeDef structure that contains
  *         the configuration information for the SDMMC command
  * @retval HAL status
  */
EMU_SD_RTN Core_SDMMC_SendCommand(HOST_REG *SDMMCx, SDMMC_CmdInitTypeDef *Command)
{
  uint32_t tmpreg = 0;
  Core_SDMMC_SetCMDARG(SDMMCx, Command->Argument);
  tmpreg |= (uint32_t)(Command->CmdIndex | Command->Attribute);
  Core_SDMMC_SetCMD(SDMMCx, tmpreg);
  return SD_OK;
}

EMU_SD_RTN Core_SDMMC_WaiteCmdDone(HOST_REG *SDMMCx)
{
  uint32_t get_val, cmd_done;
  
  uint32_t start;
  start = SysTicks_GetTickCount();

  do {
    get_val = Core_SDMMC_GetRINTSTS(SDMMCx);
    cmd_done = (get_val & SDMMC_RINTSTS_CMD_DONE); 
    if((SysTicks_GetDiff(start, SysTicks_GetTickCount())) > SD_TIME_OUT)
    {
        dlog_error("time out");
        return SD_DATA_TIMEOUT;
    }
  } while (!cmd_done);
  return SD_OK;
}

EMU_SD_RTN Core_SDMMC_WaiteDataOver(HOST_REG *SDMMCx)
{
  uint32_t get_val, data_over;
  uint32_t start;
  start = SysTicks_GetTickCount();

  do {
    get_val = Core_SDMMC_GetRINTSTS(SDMMCx);
    data_over = (get_val & SDMMC_RINTSTS_DATA_OVER);
    if((SysTicks_GetDiff(start, SysTicks_GetTickCount())) > SD_TIME_OUT)
    {
        dlog_error("time out");
        return SD_DATA_TIMEOUT;
    }
  } while (!data_over);
  return SD_OK;
}

EMU_SD_RTN Core_SDMMC_WaiteCardBusy(HOST_REG *SDMMCx)
{
  uint32_t get_val, card_busy;
  uint32_t start;
  start = SysTicks_GetTickCount();

  do {
    get_val = Core_SDMMC_GetSTATUS(SDMMCx);
    card_busy = (get_val & SDMMC_STATUS_DATA_BUSY); 
    if((SysTicks_GetDiff(start, SysTicks_GetTickCount())) > SD_TIME_OUT)
    {
        dlog_error("time out");
        return SD_DATA_TIMEOUT;
    }
  } while (card_busy);
  return SD_OK;
}

EMU_SD_RTN Core_SDMMC_WaiteCmdStart(HOST_REG *SDMMCx)
{
  uint32_t get_val, cmd_start;
  uint32_t start;
  start = SysTicks_GetTickCount();

  do {
    get_val = Core_SDMMC_GetCMD(SDMMCx); 
    cmd_start = (get_val & SDMMC_CMD_START_CMD);
    if((SysTicks_GetDiff(start, SysTicks_GetTickCount())) > SD_TIME_OUT)
    {
        dlog_error("time out");
        return SD_DATA_TIMEOUT;
    }
  } while (cmd_start);
  return SD_OK;
}

EMU_SD_RTN Core_SDMMC_WaiteVoltSwitchInt(HOST_REG *SDMMCx) 
{
  uint32_t get_val, volt_switch_int;
  uint32_t start;

  start = SysTicks_GetTickCount();
  do {
    get_val = Core_SDMMC_GetRINTSTS(SDMMCx);
    volt_switch_int = (get_val & SDMMC_RINTSTS_HTO);
    if((SysTicks_GetDiff(start, SysTicks_GetTickCount())) > SD_TIME_OUT)
    {
        dlog_error("time out");
        return SD_DATA_TIMEOUT;
    }
  } while (!volt_switch_int);
  return SD_OK;
}


/**
  * @brief  Return the response received from the card for the last command
  * @param  SDMMCx: Pointer to SDMMC register base
  * @param  Response: Specifies the SDMMC response register.
  *          This parameter can be one of the following values:
  *            @arg SDMMC_RESP0: Response Register 0
  *            @arg SDMMC_RESP1: Response Register 1
  *            @arg SDMMC_RESP2: Response Register 2
  *            @arg SDMMC_RESP3: Response Register 3
  * @retval The Corresponding response register value
  */
uint32_t Core_SDMMC_GetResponse(HOST_REG *SDMMCx, uint32_t Response)
{
  __IO uint32_t tmp = 0;
  switch (Response) {
  case SDMMC_RESP0:
    tmp = Core_SDMMC_GetRESP0(SDMMCx);
    break;
  case SDMMC_RESP1:
    tmp = Core_SDMMC_GetRESP1(SDMMCx);
    break;
  case SDMMC_RESP2:
    tmp = Core_SDMMC_GetRESP2(SDMMCx);
    break;
  case SDMMC_RESP3:
    tmp = Core_SDMMC_GetRESP3(SDMMCx);
    break;
  }
  return tmp;
}

