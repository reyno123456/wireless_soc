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
LIB = liblwip.a

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
SRC_C   := lwip/src/api/tcpip.c \
           lwip/src/api/api_lib.c \
           lwip/src/api/netbuf.c \
           lwip/src/api/api_msg.c \
           lwip/src/api/err.c \
           lwip/src/api/netdb.c \
           lwip/src/api/netifapi.c \
           lwip/src/api/sockets.c \
           lwip/src/netif/etharp.c \
           lwip/src/netif/ethernetif.c \
           lwip/src/netif/slipif.c \
           lwip/src/netif/ppp/ipcp.c \
           lwip/src/netif/ppp/randm.c \
           lwip/src/netif/ppp/fsm.c \
           lwip/src/netif/ppp/chpms.c \
           lwip/src/netif/ppp/lcp.c \
           lwip/src/netif/ppp/md5.c \
           lwip/src/netif/ppp/chap.c \
           lwip/src/netif/ppp/pap.c \
           lwip/src/netif/ppp/ppp_oe.c \
           lwip/src/netif/ppp/vj.c \
           lwip/src/netif/ppp/auth.c \
           lwip/src/netif/ppp/magic.c \
           lwip/src/netif/ppp/ppp.c \
           lwip/src/netif/loopif.c \
           lwip/src/core/tcp.c \
           lwip/src/core/netif.c \
           lwip/src/core/raw.c \
           lwip/src/core/tcp_in.c \
           lwip/src/core/sys.c \
           lwip/src/core/memp.c \
           lwip/src/core/snmp/msg_out.c \
           lwip/src/core/snmp/mib_structs.c \
           lwip/src/core/snmp/msg_in.c \
           lwip/src/core/snmp/asn1_enc.c \
           lwip/src/core/snmp/asn1_dec.c \
           lwip/src/core/snmp/mib2.c \
           lwip/src/core/tcp_out.c \
           lwip/src/core/pbuf.c \
           lwip/src/core/stats.c \
           lwip/src/core/dns.c \
           lwip/src/core/dhcp.c \
           lwip/src/core/mem.c \
           lwip/src/core/ipv4/inet.c \
           lwip/src/core/ipv4/autoip.c \
           lwip/src/core/ipv4/ip_addr.c \
           lwip/src/core/ipv4/ip.c \
           lwip/src/core/ipv4/igmp.c \
           lwip/src/core/ipv4/inet_chksum.c \
           lwip/src/core/ipv4/ip_frag.c \
           lwip/src/core/ipv4/icmp.c \
           lwip/src/core/init.c \
           lwip/src/core/udp.c \
           porting/etharp.c \
#           porting/ethernetif.c \
           porting/sys_arch.c

INCDIRS :=  -I$(TOP_DIR)/Library/ThirdParty/LwIP/lwip/test/unit/udp \
            -I$(TOP_DIR)/Library/ThirdParty/LwIP/lwip/test/unit/tcp \
            -I$(TOP_DIR)/Library/ThirdParty/LwIP/lwip/src/include/netif \
            -I$(TOP_DIR)/Library/ThirdParty/LwIP/lwip/src/include \
            -I$(TOP_DIR)/Library/ThirdParty/LwIP/lwip/src/include \
            -I$(TOP_DIR)/Library/ThirdParty/LwIP/lwip/src/include/ipv4 \
            -I$(TOP_DIR)/Library/ThirdParty/LwIP/lwip/src/netif/ppp \
            -I$(TOP_DIR)/Library/ThirdParty/LwIP/porting/lwip \
            -I$(TOP_DIR)/Library/ThirdParty/LwIP/porting \
            -I$(TOP_DIR)/Configure/Include \
            -I$(TOP_DIR)/Kernel/FreeRTOS/include \
            -I$(TOP_DIR)/Driver/HAL/Inc \
            -I$(TOP_DIR)/Library/Artosyn/Common/Inc \
            -I$(TOP_DIR)/Kernel/FreeRTOS/portable/GCC/ARM_CM7/r0p1 \
            -I$(OUTPUT_HEADER_STAGING_DIR)

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
#	@cp -f Inc/debuglog.h $(OUTPUT_HEADER_STAGING_DIR)
#	@cp -f Inc/sys_event.h $(OUTPUT_HEADER_STAGING_DIR)
#	@cp -f Inc/md5.h $(OUTPUT_HEADER_STAGING_DIR)
#	@cp -f Inc/local_irq.h $(OUTPUT_HEADER_STAGING_DIR)
#	@cp -f Inc/cfg_parser.h $(OUTPUT_HEADER_STAGING_DIR)

clean:
	@echo "Cleaning..."
	@$(RM) $(LIB) $(OBJ_C) $(OBJ_CPP) .depend $(OUTPUT_LIB_STAGING_DIR)/$(CPU_NUMBER)/$(LIB) $(OUTPUT_HEADER_STAGING_DIR)/debuglog.h $(OUTPUT_HEADER_STAGING_DIR)/sys_event.h $(OUTPUT_HEADER_STAGING_DIR)/local_irq.h $(OUTPUT_HEADER_STAGING_DIR)/md5.h

.PHONY: all clean install
#########################################################################

include $(BUILD_DIR)/rules.mk

sinclude .depend

#########################################################################

