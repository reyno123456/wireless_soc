# top file directory
TOP_DIR ?= ..

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
	@$(MAKE) -C $(BOOT)/Boot all
	@$(MAKE) -C $(BOOT)/Upgrade all

clean:
	@$(MAKE) -C $(BOOT)/Boot clean
	@$(MAKE) -C $(BOOT)/Upgrade clean

.PHONY: all clean

#########################################################################

