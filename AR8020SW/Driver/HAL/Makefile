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
# static library name
LIB = libhal.a

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
SRC_C := Src/hal.c \
         Src/hal_i2c.c \
         Src/hal_sys_ctl.c \
         Src/hal_hdmi_rx.c \
         Src/hal_gpio.c \
         Src/hal_timer.c \
         Src/hal_pwm.c \
         Src/hal_softpwm.c \
         Src/hal_sram.c \
         Src/hal_h264.c \
         Src/hal_usb_host.c \
         Src/hal_usb_device.c \
         Src/hal_usb_otg.c \
         Src/hal_uart.c \
         Src/hal_spi.c \
         Src/hal_bb_common.c \
         Src/hal_bb_ground.c \
         Src/hal_bb_sky.c \
         Src/hal_bb_debug.c \
         Src/hal_nvic.c \
         Src/hal_sd.c \
         Src/hal_dma.c \
         Src/hal_adc.c \
         Src/hal_nv.c \
         Src/hal_can.c \
         Src/hal_camera.c \
         Src/hal_mipi.c \
         Src/hal_dvp.c \
         Src/hal_encodemp3.c\
         Src/hal_softi2s.c \
         Src/hal_softi2s_left44.c \
         Src/hal_softi2s_right44.c \
         Src/hal_softi2s_left48.c \
         Src/hal_softi2s_right48.c \
         Src/hal_norflash.c \
         Src/hal_pmu.c \
         Src/hal_rtc.c

INCDIRS := -IInc \
           -I$(TOP_DIR)/Configure/Include \
           -I$(TOP_DIR)/Library/Artosyn/Common/Inc \
           -I$(TOP_DIR)/Library/ThirdParty/FatFs/Port \
           -I$(TOP_DIR)/Library/ThirdParty/FatFs/Inc \
		   -I$(TOP_DIR)/Library/ThirdParty/MP3Encoder/shine-3.1.0 \
           -I$(TOP_DIR)/Driver/Chip/AR8020/I2C/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/SysCtl/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/BB/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/SRAM/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/SD/Inc \
           -I$(TOP_DIR)/Driver/Board/Common/ADV7611/Inc \
           -I$(TOP_DIR)/Driver/Board/Common/IT66021/Inc \
           -I$(TOP_DIR)/Driver/Board/ARCast/Inc/ \
           -I$(TOP_DIR)/Driver/Board/Common/OV5640/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/H264Encoder/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/USB/USB_Device/Core/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/USB/USB_Device/Class/HID/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/USB/USB_LL_Driver/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/USB/USB_Host/Core/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/USB/USB_Host/Class/MSC/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/USB/USB_Host/Class/UVC/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/SPI/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/UART/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/CAN/Inc \
           -I$(TOP_DIR)/Driver/Chip/AR8020/DMA/Inc \
           -I$(TOP_DIR)/Driver/Board/Common/NORFlash/Inc \
           -I$(TOP_DIR)/Driver/Board \
           -I$(TOP_DIR)/Driver/Chip/AR8020/MIPI/Inc \
           -I$(TOP_DIR)/Driver/HAL/Inc \
           -I$(TOP_DIR)/Kernel/FreeRTOS/CMSIS_RTOS \
           -I$(TOP_DIR)/Kernel/FreeRTOS/include \
           -I$(TOP_DIR)/Kernel/FreeRTOS/portable/GCC/ARM_CM7/r0p1 \
           -I$(OUTPUT_HEADER_STAGING_DIR)


# object file(s)
OBJ_C     := $(patsubst %.c,%.o,$(SRC_C))

all: .depend $(LIB) install

$(LIB): $(OBJ_C)
	@echo "Generating static library: $(CPU_NUMBER) " $(notdir $@)
	$(AR) $(ARFLAGS) $@ $(OBJ_C)

install:
	@echo "Installing static library: $(CPU_NUMBER) $(LIB)"
	@mkdir -p $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)
	@mkdir -p $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f $(LIB) $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)
	@cp -f Inc/hal.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_i2c.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_sys_ctl.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_hdmi_rx.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_sram.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_gpio.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_timer.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_pwm.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_softpwm.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_h264.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_timer.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_ret_type.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_usb_host.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_usb_device.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_usb_otg.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_uart.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_spi.h $(OUTPUT_HEADER_STAGING_DIR) 
	@cp -f Inc/hal_bb.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_nvic.h $(OUTPUT_HEADER_STAGING_DIR) 
	@cp -f Inc/hal_sd.h  $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_dma.h  $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_adc.h  $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_nv.h  $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_can.h $(OUTPUT_HEADER_STAGING_DIR) 
	@cp -f Inc/hal_camera.h $(OUTPUT_HEADER_STAGING_DIR) 
	@cp -f Inc/hal_mipi.h $(OUTPUT_HEADER_STAGING_DIR) 
	@cp -f Inc/hal_dvp.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_softi2s.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_encodemp3.h $(OUTPUT_HEADER_STAGING_DIR) 
	@cp -f Inc/hal_norflash.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_pmu.h $(OUTPUT_HEADER_STAGING_DIR)
	@cp -f Inc/hal_rtc.h $(OUTPUT_HEADER_STAGING_DIR)
clean:
	@echo "Cleaning..."
	@$(RM) $(LIB) $(OBJ_C) .depend $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)/$(LIB)
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_i2c.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_sys_ctl.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_hdmi_rx.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_sram.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_uart.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_spi.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_bb.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_sd.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_dma.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_adc.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_nv.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_usb_host.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_usb_device.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_usb_otg.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_can.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_camera.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_mipi.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_dvp.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_softi2s.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_encodemp3.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_pmu.h
	@$(RM) $(OUTPUT_HEADER_STAGING_DIR)/hal_rtc.h

.PHONY: all clean install
#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################

