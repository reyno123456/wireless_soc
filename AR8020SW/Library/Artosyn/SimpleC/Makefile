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
LIB = libsimplec.a

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
SRC_C   := Src/memory.c \
           Src/printf.c \
           Src/string.c \
           Src/abs.c \
           Src/errno.c

INCDIRS :=  -IInc \
            -I$(TOP_DIR)/Library/Artosyn/Common/Inc

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
	@cp -f Inc/memory.h $(OUTPUT_HEADER_STAGING_DIR)

clean:
	@echo "Cleaning..."
	@$(RM) $(LIB) $(OBJ_C) $(OBJ_CPP) .depend $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)/$(LIB) $(OUTPUT_HEADER_STAGING_DIR)/memory.h

.PHONY: all clean install
#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################

