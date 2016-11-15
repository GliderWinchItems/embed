#/******************************************************************************
#* File Name          : Makefile
#* Date First Issued  : 01/21/2015
#* Description        : Build common_all library for f3 (hard float)
#*******************************************************************************/ 
LIBNAME		= libcommon_allf3

###################################################

# Check for valid float argument
# NOTE that you have to run make clean after
# changing these as hardfloat and softfloat are not
# binary compatible
FLOAT_TYPE = hard

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

# Navigation to libraries specific to the f3 and discoveryf3
LIBF3D	= $(CURDIR)/../../../git_discoveryf3/lib

# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS  = -std=gnu99 -g -Os -Wall
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -nostartfiles -mcpu=cortex-m4
# Let everybody know this is F3 and not F4 or F103
CFLAGS += -D STM32F3

ifeq ($(FLOAT_TYPE), hard)
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
else
CFLAGS += -msoft-float
endif
CFLAGS += -I$(LIBCOMMONALL)
CFLAGS += -I$(LIBF3D)
CFLAGS += -I$(LIBF3D)/libopencm3
CFLAGS += -I$(LIBF3D)/libopencm3/cm3
CFLAGS += -I$(LIBF3D)/libopencm3/stm32
CFLAGS += -I$(LIBF3D)/libusartstm32f3

# Assembler options
AFLAGS  = -mcpu=cortex-m3 -mthumb

ARFLAGS	= rcsv
#ARFLAGS		= rcs

# Routines to be compiled into the library
OBJS = 
OBJS	+= PC_gateway_comm.o
OBJS	+= USB_PC_gateway.o
OBJS	+= pay_flt_cnv.o
OBJS	+= pay_type_cnvt.o
OBJS	+= can_driver.o
OBJS	+= can_driver_port.o
OBJS	+= can_driver_filter.o
#OBJS	+= can_gps_phasing.o
OBJS	+= can_msg_reset.o
#OBJS	+= register_clear.o
OBJS	+= queue_dbl.o

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

