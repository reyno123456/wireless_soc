TOP_DIR ?= ../../..

# build file directory
BUILD_DIR ?= $(TOP_DIR)/Build

# binary file director
OUTPUT_DIR ?= $(TOP_DIR)/Output
OUTPUT_HEADER_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Inc
OUTPUT_LIB_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Lib

# current directory
CURDIR ?= $(shell pwd)

include $(BUILD_DIR)/config.mk

###############################################################################
# static library name
LIB = libfatfs.a

ifeq ($(CPU_NUMBER), cpu0)
    CFLAGS += $(CPU0_COMPILE_FLAGS)
endif

ifeq ($(CPU_NUMBER), cpu1)
    CFLAGS += $(CPU1_COMPILE_FLAGS)
endif

ifeq ($(CPU_NUMBER), cpu2)
    CFLAGS += $(CPU2_COMPILE_FLAGS)
endif


# source file(s), including c file(s)
SRC_C   := Src/ff.c \
           Src/ff_gen_drv.c \
           Src/diskio.c \
           Src/option/syscall.c \
           Port/sd_diskio.c \
           Port/usbh_diskio.c \
           Src/option/ffunicode.c


INCDIRS := -IInc \
           -IPort \
           -I$(TOP_DIR)/Configure/Include \
           -I$(TOP_DIR)/Driver/Chip/AR8020/USB/USB_LL_Driver/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/USB/USB_Host/Class/MSC/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/USB/USB_Host/Core/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/UART/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/SD/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/SysCtl/Inc \
           -I$(TOP_DIR)/Driver/HAL/Inc/ \
           -I$(TOP_DIR)/Library/Artosyn/Common/Inc \
           -I$(TOP_DIR)/Kernel/FreeRTOS/CMSIS_RTOS \
           -I$(TOP_DIR)/Kernel/FreeRTOS/include \
           -I$(TOP_DIR)/Kernel/FreeRTOS/portable/GCC/ARM_CM7/r0p1 \
           -I$(TOP_DIR)/Driver/HAL/Inc

# object file(s)
OBJ_C   := $(patsubst %.c,%.o,$(SRC_C))

CFLAGS +=

all: .depend $(LIB) install

$(LIB):  $(OBJ_C)
	@echo "Generating static library:  $(CPU_NUMBER) " $(notdir $@)
	@$(AR) $(ARFLAGS) $@ $^

install:
	@mkdir -p $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)
	@mkdir -p $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f $(LIB) $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)
	@cp -f Inc/ff.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/ffconf.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/ff_gen_drv.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/diskio.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/integer.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Port/usbh_diskio.h $(OUTPUT_HEADER_STAGING_DIR)

clean:
	@echo "Cleaning..."
	@$(RM) $(LIB) $(OBJ_C) $(OBJ_CPP) .depend $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)/$(LIB)
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/ff.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/ffconf.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/ff_gen_drv.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/diskio.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/integer.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/usbh_diskio.h

.PHONY: all clean install
#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################

