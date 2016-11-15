# Makefile - Cortex-M3, STM32F103
# 06/06/2013


#
# Note: This version descends into 'libusartstm32' and executes 'make' if
# started by the command 'make', but not when started with the command
# 'make flash'.  
#
# 'make clean' descends into 'libusartstm32'


# Name of the .c file with 'main' as well as output file name
NAME		= se1

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

# Library directory path--compile and stuff
LGCC		= $(HOME)/CodeSourcery/Sourcery_G++_Lite/lib/gcc/arm-none-eabi/4.5.2/thumb2
LOTHER		= $(HOME)/CodeSourcery/Sourcery_G++_Lite/arm-none-eabi/lib/thumb2

# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS   = -Os -g -Wall -Wextra
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
CFLAGS	+= -Iinclude -fno-common -mcpu=cortex-m3 -mthumb -Wstrict-prototypes -fno-builtin-putc

AFLAGS  = -mcpu=cortex-m3 -mthumb

# Linker script which has the definition of memory layout and "sections"
#LDSCRIPT	= $(NAME).ld
LDSCRIPT	= $(LIBCOMMONALL)/ldrapp.ld

# Libraries (.a), script (.ld), (.map) for the linker
LDFLAGS  = -L$(LOTHER) -L$(LGCC) -lgcc -lgcov -lc -lm -Map=$(NAME).map -T$(LDSCRIPT) -nostartfiles 
LDFLAGS	+= -L$(LIBDEVICEF103SE) 
LDFLAGS	+= -L$(LIBPODLIB) 
LDFLAGS	+= -L$(LIBPODDEV) 
LDFLAGS	+= -L$(LIBMISCSTM32) 
LDFLAGS	+= -L$(LIBUSARTSTM32) 
LDFLAGS	+= -L$(LIBSUPPORTSTM32) 
LDFLAGS	+= -L$(LIBSENSORMISC) 
#LDFLAGS	+= -L$(LIBCOMMONALL)
LDFLAGS	+= -L$(LIBSVNCOMMON)/f103

# List of files to be compiled
OBJS	 =  vector.o $(NAME).o 
OBJS	+= newlib_support.o 
OBJS	+= adcsensor_eng.o 
OBJS	+= rpmsensor.o
OBJS	+= temp_calc.o


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
	$(Q)$(LD) $(LDFLAGS) -o $(NAME) $(OBJS)  -lgcc  -lc -lgcc -lmiscstm32  -lcommon_all -lusartstm32 -lsensormisc -ldevicestm32 -lsensormisc -lsupportstm32 -ldevicesstm32  -lm -lc -lgcc -lcommon_all -lgcc
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

