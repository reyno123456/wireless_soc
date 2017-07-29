# top file directory
TOP_DIR ?= ../..

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

all:
	@$(MAKE) -C $(CHIP) CPU_NUMBER=$(CPU_NUMBER)

clean:
	@$(MAKE) -C $(CHIP) CPU_NUMBER=$(CPU_NUMBER) clean

.PHONY: all clean

#########################################################################

