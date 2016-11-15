#*******************************************************************************
#* File Name          : Makefile
#* Date First Issued  : 01/26/2015 deh
#* Description        : Build library: libmiscf3.a
#*******************************************************************************/ 
LIBNAME		= libmiscf3

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
CFLAGS += -DSTM32F3

ifeq ($(FLOAT_TYPE), hard)
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
else
CFLAGS += -msoft-float
endif

###################################################
# Current directory
CURDIR=.

# Navigation to libraries in 'svn_discoveryf3'
LIBF3D		= $(CURDIR)/..

# Directory path--includes for .h and .c files common to all routines
LIBCOMMONALL      = $(CURDIR)/../../common_all_f3/trunk

# Library of common to all processors
LIBSVNCOMMON	= $(CURDIR)/../../../svn_common

# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS  = -std=gnu99 -g -Os -Wall
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -nostartfiles -mcpu=cortex-m4

# Let everybody know this is F3 and not F4 or F103
CFLAGS += -DSTM32F3

ifeq ($(FLOAT_TYPE), hard)
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
else
CFLAGS 	+= -msoft-float
endif
CFLAGS 	+= -I$(LIBCOMMONALL)
CFLAGS 	+= -I$(LIBF3D)
CFLAGS 	+= -I$(LIBF3D)/libopencm3/cm3
CFLAGS 	+= -I$(LIBF3D)/libopencm3/stm32
CFLAGS 	+= -I$(LIBF3D)/libopencm3/stm32/f3
CFLAGS	+= -I$(LIBSVNCOMMON)
CFLAGS	+= -I$(LIBSVNCOMMON)/f3

AFLAGS  = -mcpu=cortex-m4 -mthumb

ARFLAGS	= rcsv
#ARFLAGS		= rcs

OBJS =
OBJS	+= busfreq.o
OBJS	+= clockspecifysetup_F3.o
OBJS	+= canwinch_setup_F3.o
OBJS	+= f3Discovery_led_pinconfig.o
OBJS	+= f3bms_led_pinconfig.o
OBJS	+= f3DISCpinconfig.o
OBJS	+= newlib_support.o
OBJS	+= panic_ledsDf3.o
#OBJS	+= printf.o
OBJS	+= systick1.o
OBJS	+= SYSTICK_24bitdiff.o
OBJS	+= SYSTICK_getcount32.o
OBJS	+= SYSTICK_getcount64.o
OBJS	+= SYSTICK_getcountdiv256.o
OBJS	+= SYSTICK_systickduration.o
OBJS	+= reset.o 
#OBJS	+= USB_PC_gateway.o
OBJS	+= DTW_counter_F3.o
OBJS	+= mymalloc.o
OBJS	+= cic_filter_l_N2_M3_f3.o
OBJS	+= cic_filter_ll_N2_M3_f3.o




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

