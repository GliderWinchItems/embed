#/******************************************************************************
#* File Name          : Makefile
#* Date First Issued  : 10/16/2013
#* Description        : Build library common to PC and STM32--for PC
#*******************************************************************************/ 
LIBNAME		= libcommon_all

# Current directory
THISDIR=.

# Define path to source files (one directory "up").
VPATH=../

# Current directory
CURDIR	=.

# Library directory path--compile and stuff
LGCC	= $(HOME)/CodeSourcery/Sourcery_G++_Lite/lib/gcc/arm-none-eabi/4.5.2/thumb2
LOTHER	= $(HOME)/CodeSourcery/Sourcery_G++_Lite/arm-none-eabi/lib/thumb2

# Library directory path--common includes
LIBCOMMONALL = $(VPATH)

# Navigation to libraries specific to the f4 and discoveryf4
LIBF4D	= $(CURDIR)/../../../sw_discoveryf4/trunk

# Compiler flags -Os = minimum space, -O3 = fastest
CFLAGS		= -Wall 
CFLAGS += -I$(LIBCOMMONALL)
CFLAGS += -I$(LIBF4D)/lib
CFLAGS += -I$(LIBF4D)/lib/libdiscoveryf4
CFLAGS += -I$(LIBF4D)/lib/libmiscstm32f4 
CFLAGS += -I$(LIBF4D)/lib/libusartstm32f4


# Routines to be compiled into the library
OBJS = 
OBJS	+= PC_gateway_comm.o
OBJS	+= USB_PC_gateway.o
OBJS	+= pay_flt_cnv.o
OBJS	+= pay_type_cnvt.o



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

