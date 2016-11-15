#/******************************************************************************
#* File Name          : Makefile
#* Date First Issued  : 10/16/2013
#* Description        : Build common_all library for f103 (soft float)
#*******************************************************************************/ 
LIBNAME		= libcommon_allf103

###################################################

# Check for valid float argument
# NOTE that you have to run make clean after
# changing these as hardfloat and softfloat are not
# binary compatible
FLOAT_TYPE = soft

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
LIBF4D	  = $(CURDIR)/../../../svn_discoveryf4/sw_discoveryf4/trunk
LIBF103PD = $(CURDIR)/../../../svn_pod/sw_stm32/trunk
LIBF103SE = $(CURDIR)/../../../svn_sensor/sw_f103/trunk

# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS  = -std=gnu99 -g -Os -Wall -Wextra
#CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -nostartfiles -mcpu=cortex-m3
#CFLAGS	+= -Iinclude -fno-common -Wstrict-prototypes -fno-builtin-putc
CFLAGS	+= -Iinclude -fno-common -mcpu=cortex-m3 -mthumb -Wstrict-prototypes -fno-builtin-putc

ifeq ($(FLOAT_TYPE), hard)
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
else
CFLAGS += -msoft-float
endif

# Let everybody know this is F3 and not F4 or F103
CFLAGS += -D STM32F103

CFLAGS += -I$(LIBCOMMONALL)
CFLAGS += -I$(LIBF103PD)/devices
CFLAGS += -I$(LIBF103PD)/lib/libopenstm32
CFLAGS += -I$(LIBF103PD)/lib/libmiscstm32
CFLAGS += -I$(LIBF103PD)/lib/libusartstm32
CFLAGS += -I$(LIBF103PD)/lib
CFLAGS += -I$(LIBF103SE)/lib/libsupportstm32
CFLAGS += -I$(LIBF103SE)/lib
CFLAGS += -I$(LIBF4D)
CFLAGS += -I$(LIBF4D)/lib
CFLAGS += -I$(LIBF4D)/lib/libusartstm32f4

# Assembler options
AFLAGS  = -mcpu=cortex-m3 -mthumb

ARFLAGS	= rcsv
#ARFLAGS		= rcs

# Routines to be compiled into the library
OBJS = 
OBJS	+= PC_gateway_comm.o
OBJS	+= USB_PC_gateway.o
OBJS	+= pay_type_cnvt.o
OBJS	+= pay_flt_cnv.o
OBJS	+= can_driver.o
OBJS	+= can_driver_port.o
OBJS	+= can_driver_filter.o
OBJS	+= can_gps_phasing.o
OBJS	+= can_msg_reset.o
OBJS	+= register_clear.o
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

