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
DEFS += -O1  -Wall 
LDFLAGS += -Bstatic 
LDFLAGS += -L$(OUTPUT_LIB_STAGING_DIR) -L$(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER) $(LD_LIBS)
             # source file(s), including c file(s)
SRC_S   :=  Src/boot_start.s 

SRC_C   :=  Src/boot_main.c \
			Src/boot_serial.c \
			Src/boot_regrw.c \
			Src/boot_norflash.c \
			Src/boot_string.c \
			Src/boot_core.c \
			Src/boot_md5.c

			

CFLAGS +=   -I./Inc 

# object file(s)
OBJ_C   := $(patsubst %.s,%.o,$(SRC_S)) $(patsubst %.c,%.o,$(SRC_C)) 


# executable file
TARGET = ar8020_boot.bin

###############################################################################

all: .depend $(TARGET) install

$(TARGET): $(OBJ_C)
	$(LD) $^ $(LDFLAGS) -TSrc/ar8020_boot.lds -o $(TARGET).elf 
	$(OBJCOPY) -O binary $(TARGET).elf $@
	$(OBJDUMP) -D -S $(TARGET).elf > $(TARGET).dis
	$(NM) $(TARGET).elf > $(TARGET).map

install:
	@cp $(TARGET) $(OUTPUT_LIB_STAGING_DIR)/ 
	@mkdir -p $(OUTPUT_LIB_STAGING_DIR)/cpu0 
	@mkdir -p $(OUTPUT_LIB_STAGING_DIR)/cpu1 
	@mkdir -p $(OUTPUT_LIB_STAGING_DIR)/cpu2

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



