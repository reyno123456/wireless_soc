# top file directory
TOP_DIR ?= ../../..

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
LIB = libboard.a

ifeq ($(CPU_NUMBER), cpu0)
    CFLAGS += $(CPU0_COMPILE_FLAGS)
endif

ifeq ($(CPU_NUMBER), cpu1)
    CFLAGS += $(CPU1_COMPILE_FLAGS)
endif

ifeq ($(CPU_NUMBER), cpu2)
    CFLAGS += $(CPU2_COMPILE_FLAGS)
endif

CFG_BIN = $(HDMI_EDID_CFG_BIN_FILE_NAME)

# source file(s), including c file(s)
SRC_C := ../Common/ADV7611/Src/adv_7611.c \
         ../Common/NORFlash/Src/nor_flash.c \
         ../Common/NORFlash/Src/w25q128.c \
         ../Common/OV5640/Src/ov5640.c \
         ../Common/RF8003/Src/rf_8003.c


INCDIRS := -I../ \
           -I../Common/ADV7611/Inc \
           -I../Common/NORFlash/Inc \
           -I../Common/OV5640/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/BB/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/I2C/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/SPI/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/QuadSPI/Inc \
           -I$(TOP_DIR)/Configure \
           -I$(TOP_DIR)/Configure/Include \
           -I$(TOP_DIR)/Library/Artosyn/Common/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/SysCtl/Inc

# object file(s)
OBJ_C     := $(patsubst %.c,%.o,$(SRC_C))

all: .depend $(LIB) install

$(LIB): $(OBJ_C)
	@echo "Generating static library: $(CPU_NUMBER) " $(notdir $@)
	$(AR) $(ARFLAGS) $@ $(OBJ_C)

install:
	@echo "Installing static library: $(LIB)"
	@mkdir -p $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)
	@mkdir -p $(OUTPUT_BIN_STAGING_DIR)
	@cp -f $(LIB) $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)
	@cp -f ../boardParameters.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f ../Common/ADV7611/Inc/adv_7611.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f ../Common/NORFlash/Inc/nor_flash.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f ../Common/OV5640/Inc/ov5640.h $(OUTPUT_HEADER_STAGING_DIR)

clean:
	@echo "Cleaning..."
	@$(RM) $(LIB) $(OBJ_C) .depend $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)/$(LIB)
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/adv_7611.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/nor_flash.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/ov5640.h

.PHONY: all clean install
#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################

