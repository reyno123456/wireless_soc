
include $(BUILD_DIR)/define.mk

APPLICATION_DIR ?= $(TOP_DIR)/Application/DJIEKDemo

DEBUG ?= n

ifeq ($(DEBUG), y)
CPU0_COMPILE_FLAGS = -g
CPU1_COMPILE_FLAGS = -g
CPU2_COMPILE_FLAGS = -O1 -g
DEBREL = Debug
else
CPU0_COMPILE_FLAGS = -O2 -s
CPU1_COMPILE_FLAGS = -O2 -s
CPU2_COMPILE_FLAGS = -O1 -s
DEBREL = Release
endif

export CPU0_COMPILE_FLAGS
export CPU1_COMPILE_FLAGS
export CPU2_COMPILE_FLAGS

###############################################################################

export CHIP = AR8020
export BOOT = AR8020
export BOARD = EKDemo

export USB_DEV_CLASS_HID_ENABLE = 1

FUNCTION_DEFS += -DSTM32F746xx -DUSE_USB_HS -DUSE_HAL_DRIVER -DUSE_WINBOND_SPI_NOR_FLASH -DUSE_ADV7611_EDID_CONFIG_BIN -DBBRF_2T4R

###############################################################################
