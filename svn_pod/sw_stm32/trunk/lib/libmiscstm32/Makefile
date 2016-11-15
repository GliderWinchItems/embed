#/******************** (C) COPYRIGHT 2010 **************************************
#* File Name          : Makefile
#* Hackor             : deh
#* Date First Issued  : 10/20/2010 deh
#* Description        : Build library: libmiscstm32.a
#*******************************************************************************/ 
LIBNAME		= libmiscstm32

# Prefix the name for the particular toolchain
# Prefix the name for the particular toolchain
PREFIX		= arm-none-eabi-

CC	=$(PREFIX)gcc
LD	=$(PREFIX)ld
OBJCOPY	=$(PREFIX)objcopy
SIZE	=$(PREFIX)size
OBJDUMP	=$(PREFIX)objdump

# Current directory
SRCDIR=.
# Library directory path prefix--svn_pod library
LIBPODDIR	= $(SRCDIR)/../../../../../svn_discoveryf4/sw_discoveryf4/trunk/lib
LIBPODDEV	= $(SRCDIR)/../../../../../svn_discoveryf4/sw_discoveryf4/trunk/devices

# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS		= -Os -g -Wall -Wextra -Iinclude -fno-common \
		  -mcpu=cortex-m3 -mthumb -Wstrict-prototypes
CFLAGS	+= -I$(LIBPODDIR) 
CFLAGS	+= -I$(LIBPODDEV) 


AFLAGS  = -mcpu=cortex-m3 -mthumb

ARFLAGS	= rcsv
#ARFLAGS		= rcs

OBJS = \
busfreq.o \
clocksetup.o \
clockspecifysetup.o \
DTW_counter.o \
printf.o \
startup_deh.o \
systick1.o \
systick_delay.o \
SYSTICK_24bitdiff.o \
SYSTICK_getcount32.o \
SYSTICK_getcount64.o \
SYSTICK_getcountdiv256.o \
SYSTICK_systickduration.o \




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

