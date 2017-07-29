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
LIB = libmp3enc.a
CFLAGS += -O3
# source file(s), including c file(s)
SRC_C   := ./shine-3.1.0/bitstream.c \
           ./shine-3.1.0/formatbits.c \
           ./shine-3.1.0/huffman.c \
           ./shine-3.1.0/l3bitstream.c \
           ./shine-3.1.0/l3loop.c \
           ./shine-3.1.0/l3mdct.c \
           ./shine-3.1.0/l3subband.c \
           ./shine-3.1.0/layer3.c \
           ./shine-3.1.0/reservoir.c \
           ./shine-3.1.0/tables.c

INCDIRS :=  -I./shine-3.1.0

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
	@cp -f ./shine-3.1.0/layer3.h $(OUTPUT_HEADER_STAGING_DIR)/

clean:
	@echo "Cleaning..."
	@$(RM) $(LIB) $(OBJ_C) $(OBJ_CPP) .depend $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)/$(LIB) $(OUTPUT_HEADER_STAGING_DIR)/layer3.h

.PHONY: all clean install
#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################

