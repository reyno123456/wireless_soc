# cross compile...
CROSS_COMPILE              = /opt/toolchain/gcc-arm-none-eabi-5_2-2015q4/bin/arm-none-eabi-
CROSS_COMPILE_LIB_GCC_PATH = /opt/toolchain/gcc-arm-none-eabi-5_2-2015q4/lib/gcc/arm-none-eabi/5.2.1/armv7e-m
CROSS_COMPILE_LIB_PATH     = /opt/toolchain/gcc-arm-none-eabi-5_2-2015q4/arm-none-eabi/lib/armv7e-m
CRTBEGIN_OBJ               = /opt/toolchain/gcc-arm-none-eabi-5_2-2015q4/lib/gcc/arm-none-eabi/5.2.1/armv7e-m/crtbegin.o
CRTEND_OBJ                 = /opt/toolchain/gcc-arm-none-eabi-5_2-2015q4/lib/gcc/arm-none-eabi/5.2.1/armv7e-m/crtend.o

CC      = $(CROSS_COMPILE)gcc
CXX     = $(CROSS_COMPILE)g++
AR      = $(CROSS_COMPILE)ar
LD      = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
NM      = $(CROSS_COMPILE)nm
AS      = $(CROSS_COMPILE)as

ARFLAGS = cr
RM = -rm -rf
MAKE = make

suffix = $(notdir $(CURDIR))

export CC
export CXX
export AS
export AR
export CFLAGS
export CXXFLAGS
export INCDIRS
export LDFLAGS
export ARFLAGS
export MAKE
export RM
export suffix

###############################################################################

CPU_DEFS = -mthumb -mcpu=cortex-m7 -mlittle-endian -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -c -Wa,-mimplicit-it=thumb 

LDFLAGS = -L$(CROSS_COMPILE_LIB_GCC_PATH) -L$(CROSS_COMPILE_LIB_PATH) $(LIBS)

CFLAGS = $(DEFS) $(CPU_DEFS) $(FUNCTION_DEFS) -std=c99 -Werror $(INCDIRS)

CXXFLAGS = $(DEFS) $(CPU_DEFS) $(FUNCTION_DEFS) -std=c++11 $(INCDIRS)

###############################################################################

