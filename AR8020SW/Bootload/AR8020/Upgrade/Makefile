# top file directory
TOP_DIR ?= ../../..

# build file directory
BUILD_DIR ?= $(TOP_DIR)/Build

# binary file director
OUTPUT_DIR ?= $(TOP_DIR)/Output
OUTPUT_HEADER_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Inc
OUTPUT_LIB_STAGING_DIR ?= $(OUTPUT_DIR)/Staging/Lib
SRC_C_DIR ?= $(TOP_DIR)/Driver/Chip/AR8020

# utility file director
UTILITY_DIR ?= $(TOP_DIR)/Utility

# current directory
CURDIR ?= $(shell pwd)

include $(BUILD_DIR)/config.mk

###############################################################################
# libs
CPU_NUMBER := cpu0

CFLAGS += $(CPU0_COMPILE_FLAGS)

LD_LIBS := -lfatfs -lutility -lhal -lboard -lAR8020drv -lcommon -lsimplec

LDFLAGS += -Bstatic
LDFLAGS += -L$(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER) $(LD_LIBS)
                 
# source file(s), including c file(s)
SRC_S   :=  Src/upgrade_start.s 
SRC_C   :=  Src/upgrade_main.c \
            Src/upgrade_command.c \
            Src/upgrade_uart.c
#            Src/upgrade_core.c 

CFLAGS += -I./Inc \
          -I$(OUTPUT_HEADER_STAGING_DIR) \
          -I$(TOP_DIR)/Configure/Include 

# object file(s)
OBJ_C   := $(patsubst %.s,%.o,$(SRC_S)) $(patsubst %.c,%.o,$(SRC_C)) 

#$(patsubst %.s,%.o,$(SRC_C))

# executable file
TARGET = ar8020_upgrade.bin

###############################################################################

all: .depend $(TARGET) install

$(TARGET): $(OBJ_C)
	$(LD) $^ $(LDFLAGS) -TSrc/upgrade_ar8020.lds -o $(TARGET).elf 
	$(OBJCOPY) -O binary $(TARGET).elf $@
	$(OBJDUMP) -D -S $(TARGET).elf > $(TARGET).dis
	$(NM) $(TARGET).elf > $(TARGET).map

install:
	@cp $(TARGET) $(OUTPUT_LIB_STAGING_DIR)/ 
	

clean:
	rm -f $(OBJ_C) .depend 
	rm -f $(TARGET) $(TARGET).elf  $(TARGET).bin  $(TARGET).dis  $(TARGET).mif  $(TARGET).map $(OUTPUT_DIR)/$(TARGET).mif $(TARGET_FLASH)
	rm -f $(OUTPUT_LIB_STAGING_DIR)/$(TARGET)
	

.PHONY: all clean $(TARGET)

#########################################################################

include $(BUILD_DIR)/rules.mk

ifneq "$(MAKECMDGOALS)" "clean"
sinclude .depend
endif

#########################################################################



