obj = chipdrv

# top file directory
TOP_DIR ?= ../../../

# build file directory
BUILD_DIR ?= $(TOP_DIR)/Build

# binary file director
OUTPUT_DIR ?= $(TOP_DIR)/Output
OUTPUT_HEADER_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Inc
OUTPUT_LIB_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Lib
OUTPUT_BIN_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Bin

# current directory
CURDIR ?= $(shell pwd)

include $(BUILD_DIR)/config.mk

###############################################################################
# static library name
LIB = lib$(CHIP)drv.a

CFG_BIN = $(BB_REG_CFG_BIN_FILE_NAME)

ifeq ($(CPU_NUMBER), cpu0)
CFLAGS += $(CPU0_COMPILE_FLAGS)
include ./src.cpu0
endif

ifeq ($(CPU_NUMBER), cpu1)
CFLAGS += $(CPU1_COMPILE_FLAGS)
include ./src.cpu1
endif

ifeq ($(CPU_NUMBER), cpu2)
CFLAGS += $(CPU2_COMPILE_FLAGS)
include ./src.cpu2
endif

# object file(s)
OBJ_C     := $(patsubst %.c,%.o,$(SRC_C))

all: $(obj).depend $(LIB) install

$(LIB): CFLAGS += -DCPU0_DRV 
$(LIB): $(OBJ_C)
	@echo "Generating static library: $(CPU_NUMBER) $(FUNCTION_DEFS)" $(notdir $@)
	$(AR) $(ARFLAGS) $@ $(OBJ_C)
	$(CC) $(FUNCTION_DEFS) -c BB/Src/bb_init_regs.c -o BB/Src/bb_init_regs_config.o
	$(OBJCOPY) -O binary BB/Src/bb_init_regs_config.o BB/Src/$(CFG_BIN)


install:
	@echo "Installing static library: $(LIB)"
	@mkdir -p $(OUTPUT_LIB_STAGING_DIR)
	@mkdir -p $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)
	@mkdir -p $(OUTPUT_HEADER_STAGING_DIR)
	@mkdir -p $(OUTPUT_BIN_STAGING_DIR)
	@cp -f $(LIB) $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)
	@cp -f UART/Inc/serial.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f CAN/Inc/can.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f I2C/Inc/i2c.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f SPI/Inc/spi.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f BB/Inc/bb_spi.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f BB/Inc/bb_uart_com.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f SysCtl/Inc/inter_core.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f SysCtl/Inc/pll_ctrl.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f SysCtl/Inc/systicks.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f SysCtl/Inc/fpu.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f SysCtl/Inc/gpio.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f SysCtl/Inc/cpu_info.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f SysCtl/Inc/interrupt.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f SysCtl/Inc/mpu.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f QuadSPI/Inc/quad_spi_ctrl.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f MIPI/Inc/mipi.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f H264Encoder/Inc/h264_encoder.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f SPI/Inc/spi.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f I2C/Inc/i2c.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f BB/Inc/bb_ctrl.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f BB/Inc/bb_types.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f BB/Inc/bb_spi_com.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f SysCtl/Inc/timer.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f BB/Src/$(CFG_BIN) $(OUTPUT_BIN_STAGING_DIR)/
	@cp -f SysCtl/Inc/stm32f746xx.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f SysCtl/Inc/core_cm7.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f SysCtl/Inc/core_cmInstr.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f SysCtl/Inc/core_cmFunc.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f SysCtl/Inc/core_cmSimd.h $(OUTPUT_HEADER_STAGING_DIR)

clean:
	@echo "Cleaning..."
	@$(RM) $(LIB) $(OBJ_C) $(obj).depend $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)/$(LIB)
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/serial.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/sd_*
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/stm32f746xx.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/core_cm7.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/core_cmInstr.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/core_cmFunc.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/core_cmSimd.h

.PHONY: all clean install
#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude $(obj).depend

#########################################################################

