# Makefile - Cortex-M3, STM32F103
# Build of Stripped and Hacked ..svn_sensor_sensor/tension/trunk 

# Name of the .c file with 'main' as well as output file name
NAME	= tension

# Prefix the name for the particular toolchain
PREFIX	= arm-none-eabi-

CC	=$(PREFIX)gcc
LD	=$(PREFIX)ld
OBJCOPY	=$(PREFIX)objcopy
SIZE	=$(PREFIX)size
OBJDUMP	=$(PREFIX)objdump


# Current directory
SRCDIR=.

CFLAGS	 = -std=gnu99 -g -Os -Wall -Wextra
CFLAGS	+= -mlittle-endian -mthumb -mthumb-interwork -nostartfiles
#CFLAGS	+= -mfp16-format=alternative
CFLAGS	+= -fno-common -mcpu=cortex-m3 -Wstrict-prototypes -save-temps

override FLOAT_TYPE = soft

CFLAGS	+= -msoft-float

CFLAGS	+= -I$(SRCDIR)
CFLAGS	+= -I$(SRCDIR)/libopenstm32

# Library directory path--compile and stuff
LGCC	= /opt/launchpad/gcc-arm-none-eabi-4_9-2015q2/lib/gcc/arm-none-eabi/4.9.3/armv7-m
LOTHER	= /opt/launchpad/gcc-arm-none-eabi-4_9-2015q2/arm-none-eabi/lib/armv7-m

# Assembler 
AFLAGS  = -mcpu=cortex-m3 -mthumb

# Linker script which has the definition of memory layout and "sections"
LDSCRIPT	= $(NAME).ld

# Libraries (.a), script (.ld), (.map) for the linker
LDFLAGS  =  -Map=$(NAME).map -T$(LDSCRIPT) -nostartfiles
LDFLAGS += -L$(LGCC)
LDFLAGS += -L$(LOTHER)
LDFLAGS	+= -L$(SRCDIR)
LDFLAGS	+= -L$(SRCDIR)/libusartstm32

# List of files to be compiled
OBJS	 = vector.o $(NAME).o 
OBJS	+= newlib_support.o
OBJS	+= PODpinconfig.o
OBJS	+= clockspecifysetup.o
OBJS	+= busfreq.o
OBJS	+= DTW_counter.o
OBJS	+= startup_deh.o

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
	$(Q)$(LD) $(LDFLAGS) -o $(NAME) $(OBJS)  -lusartstm32  -lc -lgcc

clean:
	@printf "  CLEAN   $(subst $(shell pwd)/,,$(OBJS))\n"
	$(Q)rm -f *.o $(LIBOPENSTM32)/*.o
	@printf "  CLEAN   $(NAME):"
	$(Q)rm -f $(NAME)
	@printf " .bin"
	$(Q)rm -f $(NAME).bin
#	@printf " .hex"
#	$(Q)rm -f $(NAME).hex
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

