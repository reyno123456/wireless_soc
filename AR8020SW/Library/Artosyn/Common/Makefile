# top file directory
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
LIB = libcommon.a

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
SRC_C   := Src/sys_event.c \
           Src/debuglog.c \
           Src/md5.c \
           Src/log10.c \
           Src/driver_buffer.c \
           Src/driver_mutex.c 

INCDIRS :=  -IInc \
            -I$(TOP_DIR)/Library/Artosyn/SimpleC/Inc \
            -I$(TOP_DIR)/Driver/Chip/AR8020/SysCtl/Inc \
            -I$(TOP_DIR)/Configure/Include \
            -I$(TOP_DIR)/Driver/Chip/AR8020/UART/Inc \
            -I$(TOP_DIR)/Driver/Chip/AR8020/BB/Inc \
            -I$(TOP_DIR)/Driver/Chip/AR8020/SPI/Inc \
            -I$(TOP_DIR)/Driver/HAL/Inc \
            -I$(OUTPUT_HEADER_STAGING_DIR)

# object file(s)
OBJ_C   := $(patsubst %.c,%.o,$(SRC_C))

all: .depend $(LIB) install

$(LIB):  $(OBJ_C)
	@echo "Generating static library: $(CPU_NUMBER) " $(notdir $@)
	@$(AR) $(ARFLAGS) $@ $^

install:
	@mkdir -p $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)
	@mkdir -p $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f $(LIB) $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)
	@cp -f Inc/debuglog.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/sys_event.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/md5.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/local_irq.h $(OUTPUT_HEADER_STAGING_DIR)

clean:
	@echo "Cleaning..."
	@$(RM) $(LIB) $(OBJ_C) $(OBJ_CPP) .depend $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)/$(LIB) $(OUTPUT_HEADER_STAGING_DIR)/debuglog.h $(OUTPUT_HEADER_STAGING_DIR)/sys_event.h $(OUTPUT_HEADER_STAGING_DIR)/local_irq.h $(OUTPUT_HEADER_STAGING_DIR)/md5.h

.PHONY: all clean install
#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################

