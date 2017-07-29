
include $(BUILD_DIR)/define.mk

APPLICATION_DIR ?= $(TOP_DIR)/Application/ARCast

DEBUG ?= n

ifeq ($(DEBUG), y)
CPU0_COMPILE_FLAGS = -g
CPU1_COMPILE_FLAGS = -g
CPU2_COMPILE_FLAGS = -O1 -g
DEBREL = Debug
else
CPU0_COMPILE_FLAGS = -g
CPU1_COMPILE_FLAGS = -g
CPU2_COMPILE_FLAGS = -O1 -g
DEBREL = Release
endif

export CPU0_COMPILE_FLAGS
export CPU1_COMPILE_FLAGS
export CPU2_COMPILE_FLAGS

###############################################################################

export CHIP = AR8020
export BOOT = AR8020
export BOARD = ARCast

export USB_DEV_CLASS_HID_ENABLE = 1
export BB_REG_CFG_BIN_FILE_NAME = 001_cfg_bb_reg.bin
export HDMI_EDID_CFG_BIN_FILE_NAME = 002_cfg_it_66021_edid.bin
export CFG_BIN_FILE_NAME_LIST = $(BB_REG_CFG_BIN_FILE_NAME) $(HDMI_EDID_CFG_BIN_FILE_NAME)

FUNCTION_DEFS += -DSTM32F746xx -DUSE_USB_HS -DUSE_HAL_DRIVER -DUSE_WINBOND_SPI_NOR_FLASH -DUSE_BB_REG_CONFIG_BIN -DUSE_IT66021_EDID_CONFIG_BIN -DARCAST

###############################################################################
