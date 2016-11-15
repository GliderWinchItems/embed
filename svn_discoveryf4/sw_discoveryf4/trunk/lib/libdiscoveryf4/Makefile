#******************************************************************************
#* File Name          : Makefile
#* Date First Issued  : 02/26/2012
#* Description        : Build library for STM32 Discovery board hardware: libdiscoveryf4.a
#*******************************************************************************/ 
LIBNAME		= libdiscoveryf4

###################################################

# Check for valid float argument
# NOTE that you have to run make clean after
# changing these as hardfloat and softfloat are not
# binary compatible
#FLOAT_TYPE = soft

ifneq ($(FLOAT_TYPE), hard)
ifneq ($(FLOAT_TYPE), soft)
override FLOAT_TYPE = soft
endif
endif

###################################################
# Prefix the name for the particular toolchain
PREFIX		= arm-none-eabi-

CC	=$(PREFIX)gcc
LD	=$(PREFIX)ld
OBJCOPY	=$(PREFIX)objcopy
SIZE	=$(PREFIX)size
OBJDUMP	=$(PREFIX)objdump

#CFLAGS		= -Os -g -Wall -Wextra -fno-common -mcpu=cortex-m4 -mthumb -Wstrict-prototypes
CFLAGS  = -std=gnu99 -g -Os -Wall
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -nostartfiles -mcpu=cortex-m4

ifeq ($(FLOAT_TYPE), hard)
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
else
CFLAGS += -msoft-float
endif

# Current directory
CURDIR=.

# Navigation to libraries in 'svn_discoveryf4'
LIBF4D		= $(CURDIR)/..

# Directory path--includes for .h and .c files common to all routines
LIBCOMMONALL      = $(CURDIR)/../../../../common_all/trunk

# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS += -I$(LIBCOMMONALL)
CFLAGS += -I$(LIBF4D)
CFLAGS += -I$(LIBF4D)/libusartstm32f4
CFLAGS += -I$(LIBF4D)/libopencm3/cm3
CFLAGS += -I$(LIBF4D)/libopencm3/stm32
CFLAGS += -I$(LIBF4D)/libopencm3/stm32/f4/

AFLAGS  = -mcpu=cortex-m4 -mthumb

ARFLAGS	= rcsv
#ARFLAGS		= rcs

#OBJS	+= default_irq_handler.o
OBJS	+= DISCpinconfig.o
#OBJS	+= canwinch_ldr.o
OBJS	+= can_cnvt.o
OBJS	+= panic_leds.o
OBJS	+= CAN_gateway.o
OBJS	+= adc_mc.o
OBJS	+= adc_internal.o
OBJS	+= cic_filter_l_N2_M3.o
OBJS	+= countdowntimer.o
OBJS	+= f4dreset.o
OBJS	+= lis302.o 
OBJS	+= spi1rwB.o
OBJS	+= spi2rw.o
OBJS	+= spi2rwB.o
OBJS	+= switchdebounce.o
OBJS	+= switchdebounce2.o



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

