#/******************** (C) COPYRIGHT 2010 **************************************
#* File Name          : Makefile
#* Hackor             : deh
#* Date First Issued  : 06/04/2011 deh
#* Description        : Build library: libudevicesstm32.a
#*******************************************************************************/ 
LIBNAME		= libdevicesstm32

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
LIBDIR		= $(SRCDIR)/../lib
LIBDEV		= $(SRCDIR)/../devices
GPIODIR		= $(SRCDIR)/../gpiotest

# Library directory path
LIBDIR		= $(SRCDIR)/../lib/

# Library directory path
LIBDEVICESSTM32	= $(LIBDEV)
LIBMISCSTM32	= $(LIBDIR)/libmiscstm32
LIBOPENSTM32	= $(LIBDIR)/libopenstm32
LIBUSARTSTM32	= $(LIBDIR)/libusartstm32
LIBSUPPORTSTM32	= $(LIBDIR)/libsupportstm32
LGCC		= $(HOME)/CodeSourcery/Sourcery_G++_Lite/lib/gcc/arm-none-eabi/4.5.2/thumb2
LOTHER		= $(HOME)/CodeSourcery/Sourcery_G++_Lite/arm-none-eabi/lib/thumb2



# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS		= -Os -g -Wall -Wextra  -I$(LIBDEV) -I$(LIBDIR) -Iinclude -fno-common \
		  -mcpu=cortex-m3 -mthumb -Wstrict-prototypes \
 -I$(LIBOPENSTM32) \
 -I$(LIBMISCSTM32) \
 -I$(LIBSUPPORTSTM32) 

AFLAGS  = -mcpu=cortex-m3 -mthumb

ARFLAGS	= rcsv
#ARFLAGS		= rcs

OBJS = \
32KHz.o \
spi1ad7799.o \
ad7799_comm.o \
adcpod.o \
gps_1pps.o \
hwcrc.o \
pinconfig.o \
pinconfig_all.o \
PODpinconfig.o \
pwrctl.o \
sdcard.o \
sdcard_ll.o \
spi2sdcard.o \
Tim2_pod.o \
Tim3_pod.o \
 \





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

