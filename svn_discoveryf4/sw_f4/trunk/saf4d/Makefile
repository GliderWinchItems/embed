# Makefile - Cortex-M4, STM32F407
# 02/29/2012
# 20120414.0004 -- caw -- hacked
#
# Name of the .c file with 'main' as well as output file name
NAME		= saf4d

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

# Navigation to libraries in 'svn_pod'
PODLIB		=../../../../svn_pod/sw_stm32/trunk

# 'svn_pod' library directory paths
LIBDIR		= $(SRCDIR)/$(PODLIB)/lib
LIBDEV		= $(SRCDIR)/$(PODLIB)/devices

# Navigation to libraries specific to the f4 and discoveryf4
LIBF4D		= $(SRCDIR)/../../../sw_discoveryf4/trunk/lib



# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS		= -Os -g -Wall -Wextra  -I$(LIBDEV) -I$(LIBDIR) -I$(LIBF4D) -Iinclude -fno-common \
		  -mcpu=cortex-m4 -mthumb -Wstrict-prototypes -mfix-cortex-m3-ldrd -msoft-float 

AFLAGS  = -mcpu=cortex-m3 -mthumb

# Linker script which has the definition of memory layout and "sections"
LDSCRIPT	= saf4d.ld

# Library directory path
LIBUSARTSTM32	= $(LIBDIR)/libusartstm32

LIBTIMERSTM32	= $(SRCDIR)/libtimerstm32

LIBDISCOVERYF4	= $(LIBF4D)/libdiscoveryf4
LIBMISCSTM32F4	= $(LIBF4D)/libmiscstm32f4 
LIBOPENCM3F4	= $(LIBF4D)/libopencm3

LGCC		= $(HOME)/CodeSourcery/Sourcery_G++_Lite/lib/gcc/arm-none-eabi/4.5.2/thumb2
LOTHER		= $(HOME)/CodeSourcery/Sourcery_G++_Lite/arm-none-eabi/lib/thumb2

# Libraries (.a), script (.ld), (.map) for the linker
LDFLAGS         = -L$(LOTHER) -L$(LGCC) -lgcc -lgcov -lc -lm -Map=$(NAME).map -T$(LDSCRIPT) -nostartfiles -L$(LIBMISCSTM32F4) -L$(LIBUSARTSTM32) -L$(LIBDISCOVERYF4) -L$(LIBOPENCM3F4)

# List of files to be compiled
OBJS		=  vector.o $(NAME).o  startup_deh.o


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

.c.o:
	$(CC)	$(CFLAGS) -S -o $@ $<
	$(CC)	$(CFLAGS) -c -o $@ $<
.s.o:
	$(AS)	$(AFLAGS) -a=$*.lst	-o $@ $<


$(NAME): $(OBJS) $(LDSCRIPT)
	@printf "  LD      $(subst $(shell pwd)/,,$(@)) $(LFLAGS)\n"
	$(Q)$(LD) $(LDFLAGS) -o $(NAME) $(OBJS) -lm -lgcc -lc -lgcc -lusartstm32 -ldiscoveryf4 -lmiscstm32f4 


	@printf "  OBJCOPY $(NAME).bin\n"
	$(Q)$(OBJCOPY) -Obinary $(NAME) $(NAME).bin
	@printf "  OBJCOPY $(NAME).hex\n"
	$(Q)$(OBJCOPY) -Oihex $(NAME) $(NAME).hex
	@printf "  OBJCOPY $(NAME).srec\n"
	$(Q)$(OBJCOPY) -Osrec $(NAME) $(NAME).srec
	@printf "  OBJDUMP $(NAME).list\n"
	$(Q)$(OBJDUMP) -S $(NAME) > $(NAME).list

clean:
	@printf "  CLEAN   $(subst $(shell pwd)/,,$(OBJS))\n"
	$(Q)rm -f *.o
	@printf "  CLEAN   $(NAME):"
	$(Q)rm -f $(NAME)
	@printf " .bin"
	$(Q)rm -f $(NAME).bin
	@printf " .list"
	$(Q)rm -f $(NAME).list
	@printf " .map"
	$(Q)rm -f $(NAME).map
	@printf " .hex"
	$(Q)rm -f $(NAME).hex
	@printf " .srec\n"
	$(Q)rm -f $(NAME).srec

flash: images
	@printf "  FLASHING   $(NAME)\n"
	@# IMPORTANT: Don't use "resume", only "reset" will work correctly!
	st-flash write $(NAME).bin 0x08000000

.PHONY: images clean

