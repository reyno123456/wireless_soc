#top file directory
TOP_DIR ?= ../..

# build file directory
BUILD_DIR ?= $(TOP_DIR)/Build

# binary file director
OUTPUT_DIR ?= $(TOP_DIR)/Output
OUTPUT_HEADER_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Inc
OUTPUT_LIB_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Lib

# utility file director
UTILITY_DIR ?= $(TOP_DIR)/Utility

# current directory
CURDIR ?= $(shell pwd)

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

APPLICATION_SCENARIO ?= sky

###############################################################################

sky: APPLICATION_SCENARIO := sky
sky: all

ground: APPLICATION_SCENARIO := ground
ground: all

all: .depend $(TARGET) install

$(TARGET): $(TARGET_CPU0) $(TARGET_CPU1) $(TARGET_CPU2)
	@echo $(APPLICATION_SCENARIO) > .APPLICATION_SCENARIO 

$(TARGET_CPU0):
	@$(MAKE) -C cpu0 all APPLICATION_SCENARIO=$(APPLICATION_SCENARIO)

$(TARGET_CPU1):
	@$(MAKE) -C cpu1 all

$(TARGET_CPU2):
	@$(MAKE) -C cpu2 all APPLICATION_SCENARIO=$(APPLICATION_SCENARIO)

install:
	@echo "Creating the flash image..."
	@cp ./cpu0/$(TARGET_CPU0).bin  .
	@cp ./cpu1/$(TARGET_CPU1).bin  .
	@cp ./cpu2/$(TARGET_CPU2).bin  .
	$(UTILITY_DIR)/joint2flash.sh -i $(OUTPUT_LIB_STAGING_DIR)/ar8020_boot.bin $(OUTPUT_LIB_STAGING_DIR)/ar8020_upgrade.bin $(TARGET_CPU0).bin $(TARGET_CPU1).bin $(TARGET_CPU2).bin -o $(TARGET_FLASH_BOOT) $(TARGET_FLASH_APP) $(TARGET_FLASH) 
	@echo "Creat the flash image done."
	@cp *.bin ../../Output
clean:
	@$(MAKE) -C cpu0 clean APPLICATION_SCENARIO=`cat .APPLICATION_SCENARIO`
	@$(MAKE) -C cpu1 clean
	@$(MAKE) -C cpu2 clean APPLICATION_SCENARIO=`cat .APPLICATION_SCENARIO`
	@rm -f $(TARGET_CPU0).bin $(TARGET_CPU1).bin $(TARGET_CPU2).bin 
	@rm -f $(TARGET_FLASH)
	@rm -f $(TARGET_FLASH_BOOT)
	@rm -f $(TARGET_FLASH_APP)
	@rm -f ../../Output/$(TARGET_FLASH_BOOT)
	@rm -f ../../Output/$(TARGET_FLASH_APP)
	@rm -f ../../Output/$(TARGET_FLASH)


.PHONY: all clean $(TARGET) $(TARGET_CPU0) $(TARGET_CPU1) $(TARGET_CPU2)

#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################



