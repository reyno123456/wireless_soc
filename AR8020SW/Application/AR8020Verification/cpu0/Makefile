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

CPU_NUMBER = cpu0
CFLAGS   += $(CPU0_COMPILE_FLAGS)
CXXFLAGS += $(CPU0_COMPILE_FLAGS)

# executable file
TARGET = ar8020_cpu0.bin

# source file(s), including *.s *.c *.cpp file(s)
SRC_S   := start.s

ifeq ($(APPLICATION_SCENARIO), sky)
SRC_C   := main_sky.c \
           command.c \
           upgrade.c \
           ../../Example/test_i2c_adv7611.c \
           ../../Example/test_bb.c \
           ../../Example/test_hal_spi_flash.c \
           ../../Example/test_float.c \
           ../../Example/testhal_gpio.c \
           ../../Example/testhal_timer.c \
           ../../Example/testhal_pwm.c \
           ../../Example/testhal_softpwm.c \
           ../../Example/test_hal_i2c_24c256.c \
           ../../Example/test_mp3.c \
           ../../Example/testhal_dma.c \
           ../../Example/test_sd.c \
           ../../Example/test_freertos.c \
           ../../Example/test_hal_camera.c \
           ../../Example/test_hal_can.c \
           ../../Example/test_hal_i2c.c \
           ../../Example/test_hal_mipi.c \
           ../../Example/test_hal_nv.c \
           ../../Example/test_hal_uart.c \
           ../../Example/test_hal_spi.c \
           ../../Example/test_localirq.c \
           ../../Example/test_quadspi.c \
           ../../Example/test_usbh.c \
           ../../Example/test_os_mail.c \
           ../../Example/test_os_mutex.c \
           ../../Example/test_os_semaphore.c \
           ../../Example/test_os_signal.c \
           ../../Example/test_os_message.c \
           ../../Example/test_os_timer.c \
           ../../Example/test_bb_led_ctrl.c \
           ../../Example/test_net.c



SRC_CXX :=
endif

ifeq ($(APPLICATION_SCENARIO), ground)
SRC_C   := main_ground.c \
           command.c \
           upgrade.c \
           ../../Example/test_freertos.c \
           ../../Example/test_hal_can.c \
           ../../Example/test_hal_camera.c \
           ../../Example/test_i2c_adv7611.c \
           ../../Example/test_usbh.c \
           ../../Example/test_bb.c \
           ../../Example/test_hal_spi_flash.c \
           ../../Example/test_quadspi.c \
           ../../Example/test_float.c \
           ../../Example/testhal_gpio.c \
           ../../Example/testhal_timer.c \
           ../../Example/testhal_pwm.c \
           ../../Example/testhal_softpwm.c \
           ../../Example/test_hal_i2c_24c256.c \
           ../../Example/test_hal_uart.c \
           ../../Example/test_hal_spi.c \
           ../../Example/test_hal_mipi.c \
           ../../Example/test_mp3.c \
           ../../Example/test_hal_nv.c \
           ../../Example/test_hal_i2c.c \
           ../../Example/test_localirq.c \
           ../../Example/testhal_dma.c \
           ../../Example/test_os_mail.c \
           ../../Example/test_os_mutex.c \
           ../../Example/test_os_semaphore.c \
           ../../Example/test_os_signal.c \
           ../../Example/test_os_message.c \
           ../../Example/test_os_timer.c \
           ../../Example/test_bb_led_ctrl.c

SRC_CXX :=
endif

# compile flags
CFLAGS   += -I$(OUTPUT_HEADER_STAGING_DIR) -I$(TOP_DIR)/Configure/Include -I../share -I../../Example \
            -I$(TOP_DIR)/Library/ThirdParty/LwIP/lwip/src/include/netif \
            -I$(TOP_DIR)/Library/ThirdParty/LwIP/lwip/src/include \
            -I$(TOP_DIR)/Library/ThirdParty/LwIP/lwip/src/include/ipv4 \
            -I$(TOP_DIR)/Library/ThirdParty/LwIP/lwip/src/netif/ppp \
            -I$(TOP_DIR)/Library/ThirdParty/LwIP/porting/lwip \
            -I$(TOP_DIR)/Library/ThirdParty/LwIP/porting 
CXXFLAGS += -I$(OUTPUT_HEADER_STAGING_DIR) -I$(TOP_DIR)/Configure/Include -I../share -I../../Example

# libs
LD_LIBS := --start-group
LD_LIBS += -lfreertos -lutility -lfatfs -lhal -lboard -lAR8020drv -lmp3enc -lm -lcommon -lc_nano -lnosys -lgcc -llwip
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

ifeq ($(SRC_CXX), )
$(TARGET): $(OBJ_S) $(OBJ_C) $(OBJ_CXX)
else
$(TARGET): $(OBJ_S) $(OBJ_C) $(OBJ_CXX) $(CRTBEGIN_OBJ) $(CRTEND_OBJ)
endif
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



