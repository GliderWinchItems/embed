#/******************** (C) COPYRIGHT 2012 **************************************
#* File Name          : Makefile
#* Hackor             : deh
#* Date First Issued  : 11/10/2012 deh
#* Description        : Build library--f103 sensor related devices: libdevicestm32.a
#*******************************************************************************/ 
LIBNAME		= libdevicestm32
#                  libdevicesstm32 // POD lib name, NOTE 'ss'

# Prefix the name for the particular toolchain
PREFIX		= arm-none-eabi-

CC	=$(PREFIX)gcc
LD	=$(PREFIX)ld
OBJCOPY	=$(PREFIX)objcopy
SIZE	=$(PREFIX)size
OBJDUMP	=$(PREFIX)objdump
AR	= $(PREFIX)ar

# Current directory
SRCDIR=.

CFLAGS	 = -std=gnu99 -g -Os -Wall -Wextra
CFLAGS	+= -mlittle-endian -mthumb -mthumb-interwork -nostartfiles
CFLAGS	+= -mfp16-format=alternative
CFLAGS	+= -fno-common -mcpu=cortex-m3 -Wstrict-prototypes -fno-builtin-putc

###################################################
# float argument
override FLOAT_TYPE = soft
CFLAGS += -msoft-float
###################################################

# Library directory path prefix--svn_pod library
LIBPODDIR	= $(SRCDIR)/../../../../../svn_pod/sw_stm32/trunk/lib
LIBPODDEV	= $(SRCDIR)/../../../../../svn_pod/sw_stm32/trunk/devices

LIBSVNCOMMON	= $(SRCDIR)/../../../../../svn_common/trunk

# Library directory path prefix--svn_sensor, f103 library
LIBSENSORF103	= $(SRCDIR)/..

# Library directory path--svn_pod library
DEVICESSTM32	= $(LIBPODDEV)
LIBMISCSTM32	= $(LIBPODDIR)/libmiscstm32
LIBOPENSTM32	= $(LIBPODDIR)/libopenstm32
LIBUSARTSTM32	= $(LIBPODDIR)/libusartstm32
LIBSUPPORTSTM32 = $(LIBPODDIR)/libsupportstm32

# Library directory path--svn_sensor library
LIBDEVICEF103SE	= $(LIBSENSORF103)/libdevicestm32
LIBSENSORMISC	= $(LIBSENSORF103)/libsensormisc

# Library directory path--common includes
LIBCOMMONALL    = $(SRCDIR)/../../../../common_all/trunk


# Library directory path--compile and stuff
LGCC		= $(HOME)/CodeSourcery/Sourcery_G++_Lite/lib/gcc/arm-none-eabi/4.5.2/thumb2
LOTHER		= $(HOME)/CodeSourcery/Sourcery_G++_Lite/arm-none-eabi/lib/thumb2

# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS	+= -I$(LIBDEVICEF103SE) 
CFLAGS	+= -I$(LIBPODDIR) 
CFLAGS	+= -I$(LIBOPENSTM32) 
CFLAGS	+= -I$(DEVICESSTM32)  
CFLAGS	+= -I$(LIBMISCSTM32) 
CFLAGS	+= -I$(LIBUSARTSTM32) 
CFLAGS	+= -I$(LIBSUPPORTSTM32)  
CFLAGS	+= -I$(LIBSENSORMISC) 
CFLAGS	+= -I$(LIBCOMMONALL) 
CFLAGS	+= -I$(LIBSVNCOMMON) 

AFLAGS  = -mcpu=cortex-m3 -mthumb
ARFLAGS	= rcsv

OBJS = 
# OBJS 	+=  adcsensor_eng.o 
# OBJS	+= adcsensor_foto.o
# OBJS	+= adcsensor_foto_h.o 
# OBJS	+= adcsensor_pod.o 
# OBJS	+= adcsensor_tension.o
#OBJS	+= canwinch_pod_common.o 
#OBJS	+= canwinch_pod_common_systick2048.o 
# OBJS	+= canwinch_pod_common_systick2048_PRIORITYFIX.o
OBJS	+= canwinch_pod_common_systick2048_printerr.o
# OBJS	+= canwinch_ldr.o 
OBJS	+= dma17_fill.o 
OBJS	+= flash_write.o 
OBJS	+= flash_write_n.o 
# OBJS	+= rpmsensor.o 
OBJS	+= SENSORpinconfig.o 
#OBJS	+= spi1eeprom.o 
OBJS	+= tim3_su.o 
#OBJS	+= tim3_ten.o 





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

