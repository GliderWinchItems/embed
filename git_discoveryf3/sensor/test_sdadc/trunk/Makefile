# Sources

# Name of the .c file with 'main' as well as output file name
NAME	= test

# Linker file
LDFILE	=stm32_flash.ld

# Current directory
CURDIR=.
SRCS =
#SRCS += startup_stm32f4xx.s
SRCS += vector.c
#SRCS += vector_chipset.c
#SRCS += vector_nvic.c
SRCS += $(NAME).c syscalls.c 
#SRCS += CAN_test_msgs.c
#SRCS += CAN_error_msgs.c
#SRCS += Default_Handler.c
SRCS += sdadc_discovery.c
SRCS += sdadc_filter.c
SRCS += sdadc_poly_correct.c
SRCS += sdadc_poly_correct_f.c
SRCS += cic_filter_l_N8_M3_f3.c
SRCS += sdadc_discovery_nodma.c
SRCS += sdadc_recalib_stddev.c

###################################################

# Check for valid float argument
# NOTE that you have to run make clean after
# changing these as hardfloat and softfloat are not
# binary compatible

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

CFLAGS  = -std=gnu99 -g -Os -Wall
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -nostartfiles -mcpu=cortex-m4
CFLAGS += -mfp16-format=alternative

# Let everybody know this is F3 and not F4 or F103
CFLAGS += -D STM32F3

CFLAGS += -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard

###################################################

# Directory path--includes for .h and .c files common to all routines
LIBCOMMONALL      = $(CURDIR)/../../../../common_all/trunk

# Navigation to libraries specific to the f4 and discoveryf4
LIBF3D		= $(CURDIR)/../../../lib

# Routes to USB 
#LIBUSB		= $(LIBF3D)/STM32F4_USB_CDC

# Library of common to all processors
LIBSVNCOMMON	= $(CURDIR)/../../../../svn_common/trunk

# Includes--ST USB
#CFLAGS += -I$(LIBUSB)/inc
#CFLAGS += -I$(LIBUSB)/inc/StdPeriph/inc
#CFLAGS += -I$(LIBUSB)/lib/StdPeriph/inc
#CFLAGS += -I$(LIBUSB)/lib/USB_OTG/inc
#CFLAGS += -I$(LIBUSB)/inc/USB_Device/Core/inc
#CFLAGS += -I$(LIBUSB)/inc/USB_Device/Class/cdc/inc
#CFLAGS += -I$(LIBUSB)/lib/Core/stm32
#CFLAGS += -I$(LIBUSB)/lib/Core/cmsis
#CFLAGS += -I$(LIBUSB)/lib/USB_Device/Class/cdc/inc
#CFLAGS += -I$(LIBUSB)/lib/USB_Device/Core/inc/

# Includes--our stuff
CFLAGS += -I$(LIBCOMMONALL)
CFLAGS += -I$(LIBF3D)
CFLAGS += -I$(LIBF3D)/libusartstm32f3
CFLAGS += -I$(LIBF3D)/libopencm3/cm3
CFLAGS += -I$(LIBF3D)/libopencm3/stm32
CFLAGS += -I$(LIBF3D)/libopencm3/stm32/f3/
CFLAGS += -I$(LIBF3D)/libdiscoveryf3
CFLAGS += -I$(LIBF3D)/libmiscf3
CFLAGS += -I$(LIBSVNCOMMON)

# Library paths--ST USB
#LIBPATHS  = -L$(LIBUSB)
#LIBPATHS += -L$(LIBUSB)/lib/StdPeriph
#LIBPATHS += -L$(LIBUSB)/lib/USB_Device/Core
#LIBPATHS += -L$(LIBUSB)/lib/USB_Device/Class/cdc
#LIBPATHS += -L$(LIBUSB)/lib/USB_OTG

# Library paths--our stuff
LIBPATHS += -L$(LIBF3D)/libmiscf3 
LIBPATHS += -L$(LIBCOMMONALL)/stm32
LIBPATHS += -L$(LIBF3D)/libusartstm32f3
LIBPATHS += -L$(LIBSVNCOMMON)/f3

# Libraries to link
#   USB
#LIBS 	 = -lm -lusbf4 -lstdperiph -lusbdevcdc -lusbcore -lusbdevcore  -lusbf4  -lstdperiph


#   our stuff
LIBS	+= -lm
LIBS	+= -lmiscf3 -lusartstm32f3
LIBS	+= -lcommon_allf3

# Linker script (.ld file)
LDFLAGS	 = -T$(LDFILE)

OBJS = $(SRCS:.c=.o)

# Paths for openocd and automatic flashing
OPENOCD_BASE	= /usr/local
OPENOCD		= $(OPENOCD_BASE)/bin/openocd
OPENOCD_SCRIPTS	= $(OPENOCD_BASE)/share/openocd/scripts
OPENOCD_FLASHER	= $(OPENOCD_SCRIPTS)/interface/stlink-v2.cfg

OPENOCD_BOARD	= $(OPENOCD_SCRIPTS)/board/stm32f3discovery.cfg

###################################################

.PHONY: proj

all: proj
	$(SIZE) $(NAME).elf

proj: 	$(NAME).elf

$(NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBPATHS) $(LIBS) $(LDFLAGS) -Wl,-Map=$(NAME).map
	$(OBJCOPY) -O ihex   $(NAME).elf $(NAME).hex
	$(OBJCOPY) -O binary $(NAME).elf $(NAME).bin
	$(OBJCOPY) -O srec   $(NAME).elf $(NAME).srec
	$(OBJDUMP) -S $(NAME).elf > $(NAME).list

$(NAME): $(OBJS) $(LDSCRIPT)
	$(LD) $(LDFLAGS) -o $(NAME) $(OBJS)

clean:

clean:
	rm -f *.o
	rm -f $(NAME).elf
	rm -f $(NAME).hex
	rm -f $(NAME).bin
#	$(MAKE) clean -C lib # Remove this line if you don't want to clean the libs as well
	
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

