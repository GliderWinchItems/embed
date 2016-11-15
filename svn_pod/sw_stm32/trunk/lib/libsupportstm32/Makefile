#/******************** (C) COPYRIGHT 2010 **************************************
#* File Name          : Makefile
#* Hackor             : deh
#* Date First Issued  : 08/29/2011 deh
#* Description        : Build library: libsupportstm32.a
#*******************************************************************************/ 
LIBNAME		= libsupportstm32

# Prefix the name for the particular toolchain
PREFIX		= arm-none-eabi
# The path for the commands to find the compiler, linker, et al.
TOOLDIR 	= $(HOME)/CodeSourcery/Sourcery_G++_Lite/bin
CC		= $(TOOLDIR)/$(PREFIX)-gcc
LD		= $(TOOLDIR)/$(PREFIX)-ld
OBJCOPY		= $(TOOLDIR)/$(PREFIX)-objcopy
OBJDUMP		= $(TOOLDIR)/$(PREFIX)-objdump
AR		= $(TOOLDIR)/$(PREFIX)-ar

# Current directory
SRCDIR=.
# Library directory path
LIBDIR		= $(SRCDIR)/../../lib/
LIBDEV		= $(SRCDIR)/../../devices
LIBPOD		= $(SRCDIR)/../../../../sw_pod/trunk/pod_v1

# Library directory path
LIBDEVICESSTM32	= $(LIBDEV)
LIBMISCSTM32	= $(LIBDIR)/libmiscstm32
LIBOPENSTM32	= $(LIBDIR)/libopenstm32
LIBUSARTSTM32	= $(LIBDIR)/libusartstm32
LIBSUPPORTSTM32 = $(LIBDIR)/libsupportstm32
LGCC		= $(HOME)/CodeSourcery/Sourcery_G++_Lite/lib/gcc/arm-none-eabi/4.5.2/thumb2
LOTHER		= $(HOME)/CodeSourcery/Sourcery_G++_Lite/arm-none-eabi/lib/thumb2


# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS		= -Os -g -Wall -Wextra\
 -I$(LIBDIR) \
-I$(LIBOPENSTM32) \
-I$(LIBDEVICESSTM32)  \
-I$(LIBMISCSTM32) \
-I$(LIBUSARTSTM32) \
-I$(LIBSUPPORTSTM32)  \
-I$(LIBPOD) \
-Iinclude \
-fno-common -mcpu=cortex-m3 -mthumb -Wstrict-prototypes

AFLAGS  = -mcpu=cortex-m3 -mthumb

ARFLAGS	= rcsv
#ARFLAGS		= rcs

OBJS = \
1_pps_cal_filter.o \
ad7799_filter.o \
adcppm.o \
adcppm_cal.o \
adcppm_cal_1.o \
adcthermtmp.o \
adctherm_cal.o \
calibration.o \
calibration_init.o \
cic_filter_L64_D32.o \
cic_filter_l_N2_M3.o \
cic_filter_ll_N2_M3.o \
gps_time_convert.o \
gps_time_convert1.o \
PPMtable.o \
rtctimers.o \
sdcard_cid.o \
sdcard_cid_print.o \
sdcard_csd.o \
sdcard_csd_print.o \
sdlog.o \





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

