#ifndef _SD_COMMON_H
#define _SD_COMMON_H

typedef struct
{
  uint32_t  CTRL         ;
  uint32_t  PWREN        ;
  uint32_t  CLKDIV       ;
  uint32_t  CLKSRC       ;
  uint32_t  CLKENA       ;
  uint32_t  TMOUT        ;
  uint32_t  CTYPE        ;
  uint32_t  BLKSIZ       ;
  uint32_t  BYCTNT       ;
  uint32_t  INTMASK      ;
  uint32_t  CMDARG       ;
  uint32_t  CMD          ;
  uint32_t  RESP0        ;
  uint32_t  RESP1        ;
  uint32_t  RESP2        ;
  uint32_t  RESP3        ;
  uint32_t  MINTSTS      ;
  uint32_t  RINTSTS      ;
  uint32_t  STATUS       ;
  uint32_t  FIFOTH       ;
  uint32_t  CDETECT      ;
  uint32_t  WRTPRT       ;
  uint32_t  GPIO         ;
  uint32_t  TCBCNT       ;
  uint32_t  TBBCNT       ;
  uint32_t  DEBNCE       ;
  uint32_t  USRID        ;
  uint32_t  VERID        ;
  uint32_t  HCON         ;
  uint32_t  UHSREG       ;
  uint32_t  RST_N        ;
  uint32_t  RESERVED     ;
  uint32_t  BMOD         ;
  uint32_t  PLDMND       ;
  uint32_t  DBADDR       ;
  uint32_t  IDSTS        ;
  uint32_t  IDINTEN      ;
  uint32_t  DSCADDR      ;
  uint32_t  BUFADDR      ;
  uint32_t  CardThrCtl   ;
  uint32_t  Back_end_powe;
  uint32_t  UHS_REG_EXT  ;
  uint32_t  EMMC_DDR_REG ;
  uint32_t  ENABLE_SHIFT ;
}HOST_REG;

typedef enum
{
	/**
	  * @brief  SD specific error defines
	  */
    SD_FAIL                            = (-1),
	SD_OK                              = (0),
	SD_CMD_CRC_FAIL                    = (1),   /*!< Command response received (but CRC check failed)              */
	SD_DATA_CRC_FAIL                   = (2),   /*!< Data block sent/received (CRC check failed)                   */
	SD_CMD_RSP_TIMEOUT                 = (3),   /*!< Command response timeout                                      */
	SD_DATA_TIMEOUT                    = (4),   /*!< Data timeout                                                  */
	SD_TX_UNDERRUN                     = (5),   /*!< Transmit FIFO underrun                                        */
	SD_RX_OVERRUN                      = (6),   /*!< Receive FIFO overrun                                          */
	SD_START_BIT_ERR                   = (7),   /*!< Start bit not detected on all data signals in wide bus mode   */
	SD_OUT_OF_RANGE                    = (8),   /*!< Command's argument was out of range.                          */
	SD_ADDRESS_ERROR                   = (9),   /*!< Misaligned address                                            */
	SD_BLOCK_LEN_ERROR                 = (10),  /*!< Transferred block length is not allowed for the card or the number of transferred bytes does not match the block length */
	SD_ERASE_SEQ_ERROR                 = (11),  /*!< An error in the sequence of erase command occurs.            */
	SD_ERASE_PARAM                     = (12),  /*!< An invalid selection for erase groups                        */
	SD_WP_VIOLATION                    = (13),  /*!< Attempt to program a write protect block                     */
	SD_CARD_IS_LOCKED                  = (34),  /*!< When set, signals that the card is locked by the host        */
	SD_LOCK_UNLOCK_FAILED              = (14),  /*!< Sequence or password error has been detected in unlock command or if there was an attempt to access a locked card */
	SD_COM_CRC_ERROR                   = (15),  /*!< CRC check of the previous command failed                     */
	SD_ILLEGAL_COMMAND                 = (16),  /*!< Command is not legal for the card state                      */
	SD_CARD_ECC_FAILED                 = (17),  /*!< Card internal ECC was applied but failed to correct the data */
	SD_CC_ERROR                        = (18),  /*!< Internal card controller error                               */
	SD_GENERAL_UNKNOWN_ERROR           = (19),  /*!< General or unknown error                                     */
	SD_STREAM_READ_UNDERRUN            = (20),  /*!< The card could not sustain data transfer in stream read operation. */
	SD_STREAM_WRITE_OVERRUN            = (21),  /*!< The card could not sustain data programming in stream mode   */
	SD_CSD_OVERWRITE                   = (22),  /*!< CSD overwrite error                                      */
	SD_WP_ERASE_SKIP                   = (23),  /*!< Only partial address space was erased                        */
	SD_CARD_ECC_DISABLED               = (24),  /*!< Command has been executed without using internal ECC         */
	SD_ERASE_RESET                     = (25),  /*!< Erase sequence was cleared before executing because an out of erase sequence command was received */
	SD_AKE_SEQ_ERROR                   = (26),  /*!< Error in sequence of authentication.                         */
	SD_INVALID_VOLTRANGE               = (27),
	SD_ADDR_OUT_OF_RANGE               = (28),
	SD_SWITCH_ERROR                    = (29),
	SD_SDMMC_DISABLED                  = (30),
	SD_SDMMC_FUNCTION_BUSY             = (31),
	SD_SDMMC_FUNCTION_FAILED           = (32),
	SD_SDMMC_UNKNOWN_FUNCTION          = (33),
	SD_INTERNAL_ERROR                  = (34),
	SD_NOT_CONFIGURED                  = (35),
	SD_REQUEST_PENDING                 = (36),
	SD_REQUEST_NOT_APPLICABLE          = (37),
	SD_INVALID_PARAMETER               = (38),
	SD_UNSUPPORTED_FEATURE             = (39),
	SD_UNSUPPORTED_HW                  = (40),
	SD_ERROR                           = (41),
	SD_NOTCARD                         = (42), /* sdcard is not detected */
	SD_UNSUPPORTED_VOLTAGE             = (43)
} EMU_SD_RTN;

#endif
