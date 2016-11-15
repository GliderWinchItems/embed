#*******************************************************************************
#* File Name          : Makefile
#* Date First Issued  : 02/29/2012 deh
#* Description        : Build library: libmiscstm32f4.a
#*******************************************************************************/ 
LIBNAME		= libmiscstm32f4

###################################################
# Prefix the name for the particular toolchain
PREFIX		= arm-none-eabi-

CC	=$(PREFIX)gcc
LD	=$(PREFIX)ld
OBJCOPY	=$(PREFIX)objcopy
SIZE	=$(PREFIX)size
OBJDUMP	=$(PREFIX)objdump

CFLAGS  = -std=gnu99 -g -Os -Wall
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -nostartfiles -mcpu=cortex-m4
CFLAGS += -mfp16-format=alternative
CFLAGS += 

ifeq ($(FLOAT_TYPE), hard)
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
else
CFLAGS += -msoft-float
endif

###################################################
# Current directory
CURDIR=.

# Navigation to libraries in 'svn_discoveryf4'
LIBF4D		= $(CURDIR)/..

# Directory path--includes for .h and .c files common to all routines
LIBCOMMONALL      = $(CURDIR)/../../../../common_all/trunk

# Library of common to all processors
LIBSVNCOMMON	= $(CURDIR)/../../../../../svn_common/trunk

# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS  = -std=gnu99 -g -Os -Wall
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -nostartfiles -mcpu=cortex-m4

ifeq ($(FLOAT_TYPE), hard)
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
else
CFLAGS 	+= -msoft-float
endif
CFLAGS 	+= -I$(LIBCOMMONALL)
CFLAGS 	+= -I$(LIBF4D)
CFLAGS 	+= -I$(LIBF4D)/libusartstm32f4
CFLAGS 	+= -I$(LIBF4D)/libopencm3/cm3
CFLAGS 	+= -I$(LIBF4D)/libopencm3/stm32
CFLAGS 	+= -I$(LIBF4D)/libopencm3/stm32/f4
CFLAGS	+= -I$(LIBSVNCOMMON)
CFLAGS	+= -I$(LIBSVNCOMMON)/f4

AFLAGS  = -mcpu=cortex-m4 -mthumb

ARFLAGS	= rcsv
#ARFLAGS		= rcs

OBJS =
OBJS	+= busfreq.o
OBJS	+= clockspecifysetup.o
OBJS	+= canwinch_setup_F4_discovery.o
OBJS	+= newlib_support.o
OBJS	+= printf.o
OBJS	+= systick1.o
OBJS	+= SYSTICK_24bitdiff.o
OBJS	+= SYSTICK_getcount32.o
OBJS	+= SYSTICK_getcount64.o
OBJS	+= SYSTICK_getcountdiv256.o
OBJS	+= SYSTICK_systickduration.o
OBJS	+= reset.o 
#OBJS	+= USB_PC_gateway.o
OBJS	+= DTW_counter.o



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

