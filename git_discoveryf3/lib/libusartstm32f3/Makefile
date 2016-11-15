#*******************************************************************************
#* File Name          : Makefile
#* Date First Issued  : 01/23/2015
#* Description        : Build library: libmiscstm32f3.a
#*******************************************************************************/ 
LIBNAME		= libusartstm32f3
###################################################

# Check for valid float argument
# NOTE that you have to run make clean after
# changing these as hardfloat and softfloat are not
# binary compatible

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

# Current directory
CURDIR=.

# Navigation to libraries in 'git_discoveryf3'
LIBF3D		= $(CURDIR)/..

# Directory path--includes for .h and .c files common to all routines
LIBCOMMONALL      = $(CURDIR)/../../../../common_all/trunk

# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS  = -std=gnu99 -g -Os -Wall
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -nostartfiles -mcpu=cortex-m4

# Let everybody know this is F3 and not F4 or F103
CFLAGS += -D STM32F3

CFLAGS += -D $(STM32TYPE)

ifeq ($(FLOAT_TYPE), hard)
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
else
CFLAGS += -msoft-float
endif
CFLAGS += -I$(LIBCOMMONALL)
CFLAGS += -I$(LIBF3D)
CFLAGS += -I$(LIBF3D)/libusartstm32f3
CFLAGS += -I$(LIBF3D)/libopencm3/cm3
CFLAGS += -I$(LIBF3D)/libopencm3/stm32
CFLAGS += -I$(LIBF3D)/libopencm3/stm32/f3/
CFLAGS += -I$(LIBF3D)/libdiscoveryf3
CFLAGS += -I$(LIBF3D)/libmiscf3

AFLAGS  = -mcpu=cortex-m4 -mthumb

ARFLAGS	= rcsv
#ARFLAGS		= rcs

OBJS =
OBJS	+= uart_pinsf3.o 
OBJS	+= usartx_setbaud.o 
OBJS	+= nvic_dma_mgrf3.o 
OBJS	+= bsp_uart.o 
OBJS	+= xprintf.o 



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

