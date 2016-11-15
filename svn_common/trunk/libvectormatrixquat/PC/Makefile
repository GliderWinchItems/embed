#/******************************************************************************
#* File Name          : Makefile
#* Date First Issued  : 02/02/2015
#* Description        : Build library common to PC and STM32--for PC
#*******************************************************************************/ 
LIBNAME		= libvmq

# Current directory
THISDIR=.

# Current directory
CURDIR	=.

# Define path to source files.
VPATH=../

# Navigation to libraries specific to the f4 and discoveryf4
LIBF103	= $(CURDIR)/../../../../sw_f103/trunk/lib

# Compiler flags -Os = minimum space, -O3 = fastest
CFLAGS		= -Wall 
CFLAGS += -I$(LIBF103)/lib
CFLAGS += -I$(LIBF103)/lib/libvectormatrixquat


# Routines to be compiled into the library
# ### OBJS is 'export'ed with object files in 'Makefile' in directory above.

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

