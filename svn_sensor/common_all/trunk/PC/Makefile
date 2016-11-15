#/******************** (C) COPYRIGHT 2012 **************************************
#* File Name          : Makefile
#* Hackor             : 
#* Date First Issued  : 10/06/2013
#* Description        : Build library common to PC and STM32--STM32
#*******************************************************************************/ 
LIBNAME		= libcommon_all



# Define path to source files.
VPATH=../

# Current directory
THISDIR=.

# Library directory path--common includes
LIBCOMMONALL = $(VPATH)

# Library directory path prefix--svn_pod library
LIBPODLIB	= $(THISDIR)/../../../../svn_pod/sw_stm32/trunk/lib
LIBPODDEV	= $(THISDIR)/../../../../svn_pod/sw_stm32/trunk/devices
# Library directory path--svn_pod library 
LIBOPENSTM32	= $(LIBPODLIB)/libopenstm32

# Library directory path prefix--svn_sensor, f103 library
LIBSENSORF103	= $(THISDIR)/../../../sw_f103/trunk/lib
# Library directory path--svn_sensor, f103 library
LIBSENSORMISC	= $(LIBSENSORF103)/libsensormisc


# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS		= -Wall \
-I$(LIBCOMMONALL) \
-I$(LIBOPENSTM32) \
-I$(LIBSENSORMISC) \



OBJS = \


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

