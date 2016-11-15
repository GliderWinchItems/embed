# Makefile - Cortex-M3, STM32F103
# 06-02-2015

# Name of the .c file with 'main' as well as output file name
NAME		= pod6

# Prefix the name for the particular toolchain
PREFIX		= arm-none-eabi
# The path for the commands to find the compiler, linker, et al.
TOOLDIR 	= $(HOME)/CodeSourcery/Sourcery_G++_Lite/bin
CC		= $(TOOLDIR)/$(PREFIX)-gcc
LD		= $(TOOLDIR)/$(PREFIX)-ld
OBJCOPY		= $(TOOLDIR)/$(PREFIX)-objcopy
OBJDUMP		= $(TOOLDIR)/$(PREFIX)-objdump

# Current directory
SRCDIR=.

CFLAGS	 = -std=gnu99 -g -Os -Wall -Wextra
CFLAGS	+= -mlittle-endian -mthumb -mthumb-interwork -nostartfiles
CFLAGS	+= -mfp16-format=alternative
CFLAGS	+= -fno-common -mcpu=cortex-m3 -Wstrict-prototypes -fno-builtin-putc

ifeq ($(FLOAT_TYPE), hard)
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
else
CFLAGS += -msoft-float
endif

# Library directory path prefix--svn_pod library
LIBPODLIB	= $(SRCDIR)/../../../../svn_pod/sw_stm32/trunk/lib
LIBPODDEV	= $(SRCDIR)/../../../../svn_pod/sw_stm32/trunk/devices

# Library directory path--svn_pod library 
LIBMISCSTM32	= $(LIBPODLIB)/libmiscstm32
LIBOPENSTM32	= $(LIBPODLIB)/libopenstm32
LIBUSARTSTM32	= $(LIBPODLIB)/libusartstm32
LIBSUPPORTSTM32 = $(LIBPODLIB)/libsupportstm32

# Library directory path prefix--svn_sensor, f103 library
LIBSENSORF103	= $(SRCDIR)/../../../sw_f103/trunk/lib

# Library directory path--svn_sensor, f103 library
LIBDEVICEF103SE	= $(LIBSENSORF103)/libdevicestm32
LIBSENSORMISC	= $(LIBSENSORF103)/libsensormisc

# Library directory path--local common includes
LIBCOMMONALL    = $(SRCDIR)/../../../common_all/trunk

# Library of common to all processors
LIBSVNCOMMON	= $(SRCDIR)/../../../../svn_common/trunk

LIBF4D	  = $(CURDIR)/../../../../sw_discoveryf4/trunk


# Library directory path--compile and stuff
LGCC		= $(HOME)/CodeSourcery/Sourcery_G++_Lite/lib/gcc/arm-none-eabi/4.5.2/thumb2
LOTHER		= $(HOME)/CodeSourcery/Sourcery_G++_Lite/arm-none-eabi/lib/thumb2

# Compiler directories for .h files
CFLAGS	+= -I$(LIBDEVICEF103SE) 
CFLAGS	+= -I$(LIBPODLIB) 
CFLAGS	+= -I$(LIBPODDEV) 
CFLAGS	+= -I$(LIBOPENSTM32) 
CFLAGS	+= -I$(LIBMISCSTM32) 
CFLAGS	+= -I$(LIBUSARTSTM32) 
CFLAGS	+= -I$(LIBSUPPORTSTM32) 
CFLAGS	+= -I$(LIBSENSORMISC) 
CFLAGS	+= -I$(LIBCOMMONALL) 
CFLAGS	+= -I$(LIBSVNCOMMON)
CFLAGS	+= -I$(LIBSVNCOMMON)/f103
CFLAGS	+= -I$(LIBF4D)/lib

# Assembler 
AFLAGS  = -mcpu=cortex-m3 -mthumb

# Linker script which has the definition of memory layout and "sections"
LDSCRIPT	= $(NAME).ld
#LDSCRIPT	= $(LIBCOMMONALL)/ldrapp.ld

# Libraries (.a), script (.ld), (.map) for the linker
LDFLAGS  = -L$(LOTHER) -L$(LGCC) -lgcc -lgcov -lc -lm -Map=$(NAME).map -T$(LDSCRIPT) -nostartfiles 
LDFLAGS	+= -L$(LIBDEVICEF103SE) 
LDFLAGS	+= -L$(LIBPODLIB) 
LDFLAGS	+= -L$(LIBPODDEV) 
LDFLAGS	+= -L$(LIBMISCSTM32) 
LDFLAGS	+= -L$(LIBUSARTSTM32) 
LDFLAGS	+= -L$(LIBSUPPORTSTM32) 
LDFLAGS	+= -L$(LIBSENSORMISC) 
LDFLAGS	+= -L$(LIBCOMMONALL) 
LDFLAGS	+= -L$(LIBSVNCOMMON)/f103

# List of files to be compiled
OBJS	 = vector.o $(NAME).o 
OBJS	+= newlib_support.o 
OBJS	+= tick_pod6.o 
OBJS	+= adcsensor_pod.o 
#OBJS	+= can_driver.o
OBJS	+= cmd_n_F103.o






# Paths for openocd and automatic flashing
OPENOCD_BASE	= /usr/local
OPENOCD		= $(OPENOCD_BASE)/bin/openocd
OPENOCD_SCRIPTS	= $(OPENOCD_BASE)/share/openocd/scripts
OPENOCD_FLASHER	= $(OPENOCD_SCRIPTS)/interface/rlink.cfg

OPENOCD_BOARD	= $(OPENOCD_SCRIPTS)/board/olimex_stm32_h103.cfg

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q := @
NULL := 2>/dev/null
endif

all: images



images: $(NAME)
	@printf "  OBJCOPY $(NAME).bin\n"
	$(Q)$(OBJCOPY) -Obinary $(NAME) $(NAME).bin
	@printf "  OBJCOPY $(NAME).hex\n"
	$(Q)$(OBJCOPY) -Oihex $(NAME) $(NAME).hex
	@printf "  OBJCOPY $(NAME).srec\n"
	$(Q)$(OBJCOPY) -Osrec $(NAME) $(NAME).srec
	@printf "  OBJDUMP $(NAME).list\n"
	$(Q)$(OBJDUMP) -S $(NAME) > $(NAME).list

.c.o:
#	$(CC)	$(CFLAGS) -S -o $@ $<
	$(CC)	$(CFLAGS) -c -o $@ $<
.s.o:
	$(AS)	$(AFLAGS) -a=$*.lst	-o $@ $<


$(NAME): $(OBJS) $(LDSCRIPT)
	@printf "  LD      $(subst $(shell pwd)/,,$(@)) $(LFLAGS)\n"
	$(Q)$(LD) $(LDFLAGS) -o $(NAME) $(OBJS)  -lm -lgcc -lc -lgcc -lcommon_all -lmiscstm32 -lusartstm32 -ldevicesstm32 -lsensormisc -lsupportstm32 -lusartstm32 -lmiscstm32  -lsupportstm32 -ldevicestm32 -ldevicesstm32  -lc -lm -lgcc -lcommon_all

clean:
	@printf "  CLEAN   $(subst $(shell pwd)/,,$(OBJS))\n"
	$(Q)rm -f *.o $(LIBOPENSTM32)/*.o
	@printf "  CLEAN   $(NAME):"
	$(Q)rm -f $(NAME)
	@printf " .bin"
#	$(Q)rm -f $(NAME).bin
#	@printf " .hex"
	$(Q)rm -f $(NAME).hex
	@printf " .srec"
	$(Q)rm -f $(NAME).srec
	@printf " .list"
	$(Q)rm -f $(NAME).list
	@printf " .map\n "
	$(Q)rm -f $(NAME).map


flash: images
	@printf "  FLASH   $(NAME).srec\n"
	@# IMPORTANT: Don't use "resume", only "reset" will work correctly!
	$(Q)$(OPENOCD) -s $(OPENOCD_SCRIPTS) \
		       -f $(OPENOCD_FLASHER) \
		       -f $(OPENOCD_BOARD) \
		       -c "init" -c "reset halt" \
		       -c "flash write_image erase $(NAME).srec" \
		       -c "reset" \
		       -c "shutdown" $(NULL)

.PHONY: images clean

