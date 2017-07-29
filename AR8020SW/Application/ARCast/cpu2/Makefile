#top file directory
TOP_DIR ?= ../../..

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

CPU_NUMBER = cpu2
CFLAGS   += $(CPU2_COMPILE_FLAGS)
CXXFLAGS += $(CPU2_COMPILE_FLAGS)

# executable file
TARGET = ar8020_cpu2.bin

# source file(s), including *.s *.c *.cpp file(s)
SRC_S   := start.s

ifeq ($(APPLICATION_SCENARIO), sky)
    SRC_C   := main_sky.c command.c
    SRC_CXX :=
endif

ifeq ($(APPLICATION_SCENARIO), ground)
    SRC_C   := main_ground.c command.c
    SRC_CXX :=
endif

# compile flags
CFLAGS   += -I$(OUTPUT_HEADER_STAGING_DIR) -I$(TOP_DIR)/Configure/Include -I../share
CXXFLAGS += -I$(OUTPUT_HEADER_STAGING_DIR) -I$(TOP_DIR)/Configure/Include -I../share

# libs
LD_LIBS := --start-group
LD_LIBS += -lhal -lboard -lAR8020drv -lm -lcommon -lsimplec -lgcc
ifeq ($(SRC_CXX), )
LD_LIBS += --end-group
else
LD_LIBS += -lstdc++ --end-group
endif

# link flags
LDFLAGS += -Bstatic
LDFLAGS += -L$(OUTPUT_LIB_STAGING_DIR) -L$(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER) $(LD_LIBS)

###############################################################################

# object file(s)
OBJ_S   := $(patsubst %.s,%.ao,$(SRC_S))
OBJ_C   := $(patsubst %.c,%.o,$(SRC_C))
OBJ_CXX := $(patsubst %.cpp,%.cpp.o,$(SRC_CXX))

all: .depend $(TARGET)

$(TARGET): $(OBJ_S) $(OBJ_C) $(OBJ_CXX)
	$(LD) $^ $(LDFLAGS) -Tar8020.lds -o $(TARGET).elf 
	$(OBJCOPY) -O binary $(TARGET).elf $@
	$(OBJDUMP) -D $(TARGET).elf > $(TARGET).dis
	$(NM) $(TARGET).elf > $(TARGET).map
	@cp $(TARGET) $(OUTPUT_LIB_STAGING_DIR)

clean:
	rm -f  $(OBJ_S) $(OBJ_C) $(OBJ_CXX) .depend 
	rm -f $(TARGET) $(TARGET).elf  $(TARGET).bin  $(TARGET).dis  $(TARGET).mif  $(TARGET).map $(OUTPUT_DIR)/$(TARGET).mif
	rm -f $(OUTPUT_LIB_STAGING_DIR)/$(TARGET)

.PHONY: all clean $(TARGET)

#########################################################################

include $(BUILD_DIR)/rules.mk

ifneq "$(MAKECMDGOALS)" "clean"
sinclude .depend
endif 

#########################################################################



