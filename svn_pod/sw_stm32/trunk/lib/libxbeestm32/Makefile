#/******************** (C) COPYRIGHT 2012 **************************************
#* File Name          : Makefile
#* Authority          : deh
#* Date First Issued  : 03/24/2012 deh
#* Description        : Build library: libxbeestm32.a
#*******************************************************************************/ 
LIBNAME		= libxbeestm32

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
# Library directory path
LIBDIR		= $(SRCDIR)/../../lib/
LIBDEV		= $(SRCDIR)/../../devices
LIBPOD		= $(SRCDIR)/../../../../sw_pod/trunk/pod_v1

# Library directory path
LIBDEVICESSTM32	= $(LIBDEV)
LIBMISCSTM32	= $(LIBDIR)/libmiscstm32
LIBOPENSTM32	= $(LIBDIR)/libopenstm32
LIBUSARTSTM32	= $(LIBDIR)/libusartstm32
LIBSUPPORTSTM32 = $(LIBDIR)/libsupportstm32
LGCC		= $(HOME)/CodeSourcery/Sourcery_G++_Lite/lib/gcc/arm-none-eabi/4.5.2/thumb2
LOTHER		= $(HOME)/CodeSourcery/Sourcery_G++_Lite/arm-none-eabi/lib/thumb2


# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS		= -Os -g -Wall -Wextra\
 -I$(LIBDIR) \
-I$(LIBOPENSTM32) \
-I$(LIBDEVICESSTM32)  \
-I$(LIBMISCSTM32) \
-I$(LIBUSARTSTM32) \
-I$(LIBSUPPORTSTM32)  \
-I$(LIBPOD) \
-Iinclude \
-fno-common -mcpu=cortex-m3 -mthumb -Wstrict-prototypes

AFLAGS  = -mcpu=cortex-m3 -mthumb

ARFLAGS	= rcsv
#ARFLAGS		= rcs

OBJS = \
xb_api_tx.o \
xb_api_rx.o \
xb_api_atcmd.o \
xb_at_cmd.o \




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

