#top file directory
TOP_DIR ?= ../..

# build file directory
BUILD_DIR ?= $(TOP_DIR)/Build

# binary file director
OUTPUT_DIR ?= $(TOP_DIR)/Output
OUTPUTCFG_DIR ?= $(TOP_DIR)/Output/Staging/Config
OUTPUT_HEADER_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Inc
OUTPUT_LIB_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Lib

# utility file director
UTILITY_DIR ?= $(TOP_DIR)/Utility

# current directory
CURDIR ?= $(shell pwd)

include ./src.cfg
include $(BUILD_DIR)/config.mk

###############################################################################

# executable file
TARGET = ar8020_all
TARGET_CPU0 = ar8020_cpu0
TARGET_CPU1 = ar8020_cpu1
TARGET_CPU2 = ar8020_cpu2
TARGET_FLASH = ar8020.bin
TARGET_FLASH_BOOT = boot.bin
TARGET_FLASH_APP = app.bin
TARGET_CONFIG	 = cfgdata.bin

APPLICATION_SCENARIO ?= sky

###############################################################################

sky: APPLICATION_SCENARIO := sky
sky: all

ground: APPLICATION_SCENARIO := ground
ground: all

all: .depend $(TARGET) install

$(TARGET): $(TARGET_CPU0) $(TARGET_CPU1) $(TARGET_CPU2) $(TARGET_CONFIG)
	@echo $(APPLICATION_SCENARIO) > .APPLICATION_SCENARIO 

$(TARGET_CPU0):
	@$(MAKE) -C cpu0 all APPLICATION_SCENARIO=$(APPLICATION_SCENARIO)

$(TARGET_CPU1):
	@$(MAKE) -C cpu1 all APPLICATION_SCENARIO=$(APPLICATION_SCENARIO)

$(TARGET_CPU2):
	@$(MAKE) -C cpu2 all APPLICATION_SCENARIO=$(APPLICATION_SCENARIO)

CONFIG_BIN  :=  $(patsubst %.c,%.bin,$(CONFIG_SRC_C))

$(TARGET_CONFIG) : CFLAGS = -I$(OUTPUT_HEADER_STAGING_DIR)
$(TARGET_CONFIG) : $(CONFIG_BIN)


$(CONFIG_BIN) : $(CONFIG_SRC_C)
install:
	@echo "Creating the config bin file..."
	@mkdir -p $(OUTPUTCFG_DIR)
	@mv $(CONFIG_BIN) $(OUTPUTCFG_DIR)
	$(UTILITY_DIR)/jointbin.sh
	@echo "Creating the flash image..."
	$(UTILITY_DIR)/joint2flash.sh  
	@cp *.bin ../../Output/

clean:
	@$(MAKE) -C cpu0 clean APPLICATION_SCENARIO=`cat .APPLICATION_SCENARIO`
	@$(MAKE) -C cpu1 clean APPLICATION_SCENARIO=`cat .APPLICATION_SCENARIO`
	@$(MAKE) -C cpu2 clean APPLICATION_SCENARIO=`cat .APPLICATION_SCENARIO`
	@rm -f $(TARGET_FLASH)
	@rm -f $(TARGET_FLASH_BOOT)
	@rm -f $(TARGET_FLASH_APP)
	@rm -f ../../Output/$(TARGET_FLASH_BOOT)
	@rm -f ../../Output/$(TARGET_FLASH_APP)
	@rm -f ../../Output/$(TARGET_FLASH)
	@rm -f ../../Output/*.bin
	@rm -f $(CONFIG_BIN) $(CONFIG_BIN:.bin=.o)
	@rm -f $(OUTPUTCFG_DIR)/*.bin 

.PHONY: all clean $(TARGET) $(TARGET_CPU0) $(TARGET_CPU1) $(TARGET_CPU2)

#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################



