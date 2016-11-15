#/******************************************************************************
#* File Name          : Makefile
#* Date First Issued  : 10/16/2013
#* Description        : Build library common to PC and STM32--for stm32
#*******************************************************************************/ 
LIBNAME		= libcommon_all

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

# Define path to source files (one directory "up").
VPATH=../

# Current directory
CURDIR	=.


# Library directory path--common includes
LIBCOMMONALL = $(VPATH)

# Navigation to libraries specific to the f4 and discoveryf4
LIBF4D	= $(CURDIR)/../../../sw_discoveryf4/trunk

# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS  = -std=gnu99 -g -Os -Wall
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -nostartfiles -mcpu=cortex-m4

ifeq ($(FLOAT_TYPE), hard)
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
else
CFLAGS += -msoft-float
endif
CFLAGS += -I$(LIBCOMMONALL)
CFLAGS += -I$(LIBF4D)/lib
CFLAGS += -I$(LIBF4D)/lib/libdiscoveryf4
CFLAGS += -I$(LIBF4D)/lib/libmiscstm32f4 
CFLAGS += -I$(LIBF4D)/lib/libusartstm32f4

# Assembler options
AFLAGS  = -mcpu=cortex-m3 -mthumb

ARFLAGS	= rcsv
#ARFLAGS		= rcs

# Routines to be compiled into the library
OBJS = 
OBJS	+= PC_gateway_comm.o
OBJS	+= USB_PC_gateway.o


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

