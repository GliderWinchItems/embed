#/******************** (C) COPYRIGHT 2012 **************************************
#* File Name          : Makefile
#* Hackor             : deh
#* Date First Issued  : 11/15/2012 deh
#* Description        : Build library--sensor related routines: libsensormisc.a
#*******************************************************************************/ 
LIBNAME		= libsensormisc

# Prefix the name for the particular toolchain
PREFIX		= arm-none-eabi
# The path for the commands to find the compiler, linker, et al.
TOOLDIR 	= $(HOME)/CodeSourcery/Sourcery_G++_Lite/bin
CC		= $(TOOLDIR)/$(PREFIX)-gcc
LD		= $(TOOLDIR)/$(PREFIX)-ld
OBJCOPY		= $(TOOLDIR)/$(PREFIX)-objcopy
OBJDUMP		= $(TOOLDIR)/$(PREFIX)-objdump
AR		= $(TOOLDIR)/$(PREFIX)-ar

# Current directory
SRCDIR=.
# Library directory path prefix--svn_pod library
LIBPODDIR	= $(SRCDIR)/../../../../../svn_pod/sw_stm32/trunk/lib
LIBPODDEV	= $(SRCDIR)/../../../../../svn_pod/sw_stm32/trunk/devices

# Library directory path prefix--svn_sensor, f103 library
LIBSENSORF103	= $(SRCDIR)/..

# Library directory path--svn_pod library
DEVICESSTM32	= $(LIBPODDEV)
LIBMISCSTM32	= $(LIBPODDIR)/libmiscstm32
LIBOPENSTM32	= $(LIBPODDIR)/libopenstm32
LIBUSARTSTM32	= $(LIBPODDIR)/libusartstm32
LIBSUPPORTSTM32 = $(LIBPODDIR)/libsupportstm32

# Library directory path--svn_sensor library
LIBDEVICEF103SE	= $(LIBSENSORF103)/libdevicestm32
LIBDSENSORMISC	= $(LIBSENSORF103)/libsensormisc

# Library directory path--common includes
LIBCOMMONALL    = $(SRCDIR)/../../../../common_all/trunk
LIBSVNCOMMON	= $(SRCDIR)/../../../../../svn_common/trunk

# Library directory path--compile and stuff
LGCC		= $(HOME)/CodeSourcery/Sourcery_G++_Lite/lib/gcc/arm-none-eabi/4.5.2/thumb2
LOTHER		= $(HOME)/CodeSourcery/Sourcery_G++_Lite/arm-none-eabi/lib/thumb2


# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS		= -Os -g -Wall -Wextra\
-I$(LIBDEVICEF103SE) \
-I$(LIBPODDIR) \
-I$(LIBOPENSTM32) \
-I$(DEVICESSTM32)  \
-I$(LIBMISCSTM32) \
-I$(LIBUSARTSTM32) \
-I$(LIBSUPPORTSTM32)  \
-I$(LIBDSENSORMISC) \
-I$(LIBCOMMONALL) \
-I$(LIBSVNCOMMON) \
-Iinclude -fno-common -mcpu=cortex-m3 -mthumb -Wstrict-prototypes -fno-builtin-putc

AFLAGS  = -mcpu=cortex-m3 -mthumb

ARFLAGS	= rcsv
#ARFLAGS		= rcs

OBJS = 
OBJS	+= CANascii.o 
OBJS	+= can_hub.o 
OBJS	+= crc-32.o  
OBJS	+= eeprom_can.o 
OBJS	+= CAN_gateway.o 
OBJS	+= can_nxp_setRS.o 
# OBJS	+= canwinch_ldrproto.o 
OBJS	+= canwinch_setup_F103_pod.o
OBJS	+= canwinch_setup_F103_Olimex.o
OBJS	+= canmsg_send_pay_type.o
OBJS	+= packet_compression.o 
OBJS	+= packet_mgr.o 
OBJS	+= panic_leds.o 
OBJS	+= panic_leds_Olimex.o 
OBJS	+= panic_leds_pod.o
OBJS	+= pay_flt_cnv.o 
OBJS	+= poly_compute_dbl.o
OBJS	+= poly_compute_flt.o  
OBJS	+= rw_eeprom.o 
OBJS	+= sensor_threshold.o 
# OBJS	+= temp_calc.o 
OBJS	+= temp_calc_param.o 
OBJS	+= USART1_PC_gateway.o 
OBJS	+= iir_filter_l.o


# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q := @
endif

all: $(LIBNAME).a

$(LIBNAME).a: $(OBJS)
	@printf "  AR      $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	@printf "  CC      $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<

clean:
	@printf "  CLEAN   $(subst $(shell pwd)/,,$(OBJS))\n"
	$(Q)rm -f *.o
	@printf "  CLEAN   $(LIBNAME).a\n"
	$(Q)rm -f $(LIBNAME).a

.PHONY: clean

