# Sources

# Name of the .c file with 'main' as well as output file name
NAME	= gatef

# Linker file
LDFILE	=stm32_flash.ld

# Current directory
CURDIR=.

SRCS = $(NAME).c startup_stm32f4xx.s syscalls.c CAN_test_msgs.c


#SRCS	+= canmsg_send.c


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

TOOLDIR	=~/CodeSourcery/Sourcery_G++_Lite/bin
CC	=$(TOOLDIR)/$(PREFIX)gcc
LD	=$(TOOLDIR)/$(PREFIX)ld
OBJCOPY	=$(TOOLDIR)/$(PREFIX)objcopy
SIZE	=$(TOOLDIR)/$(PREFIX)size
OBJDUMP	=$(TOOLDIR)/$(PREFIX)objdump

CFLAGS  = -std=gnu99 -g -Os -Wall
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -nostartfiles -mcpu=cortex-m4

ifeq ($(FLOAT_TYPE), hard)
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
else
CFLAGS += -msoft-float
endif

###################################################

# Directory path--includes for .h and .c files common to all routines
LIBCOMMONALL      = $(CURDIR)/../../../common_all/trunk

# Navigation to libraries specific to the f4 and discoveryf4
LIBF4D		= $(CURDIR)/../../../sw_discoveryf4/trunk

# Routes to USB 
LIBUSB		= $(LIBF4D)/STM32F4_USB_CDC

# Includes--ST USB
CFLAGS += -I$(LIBUSB)/inc
CFLAGS += -I$(LIBUSB)/inc/StdPeriph/inc
CFLAGS += -I$(LIBUSB)/lib/StdPeriph/inc
CFLAGS += -I$(LIBUSB)/lib/USB_OTG/inc
CFLAGS += -I$(LIBUSB)/inc/USB_Device/Core/inc
CFLAGS += -I$(LIBUSB)/inc/USB_Device/Class/cdc/inc
CFLAGS += -I$(LIBUSB)/lib/Core/stm32
CFLAGS += -I$(LIBUSB)/lib/Core/cmsis
CFLAGS += -I$(LIBUSB)/lib/USB_Device/Class/cdc/inc
CFLAGS += -I$(LIBUSB)/lib/USB_Device/Core/inc/

# Includes--our stuff
CFLAGS += -I$(LIBCOMMONALL)
CFLAGS += -I$(LIBF4D)/lib/libdiscoveryf4
CFLAGS += -I$(LIBF4D)/lib/libmiscstm32f4 
CFLAGS += -I$(LIBF4D)/lib/libusartstm32f4
CFLAGS += -I$(LIBF4D)/lib

# Library paths--ST USB
LIBPATHS  = -L$(LIBUSB)
LIBPATHS += -L$(LIBUSB)/lib/StdPeriph
LIBPATHS += -L$(LIBUSB)/lib/USB_Device/Core
LIBPATHS += -L$(LIBUSB)/lib/USB_Device/Class/cdc
LIBPATHS += -L$(LIBUSB)/lib/USB_OTG

# Library paths--our stuff
LIBPATHS += -L$(LIBF4D)/lib/libdiscoveryf4
LIBPATHS += -L$(LIBF4D)/lib/libmiscstm32f4 
LIBPATHS += -L$(LIBCOMMONALL)/stm32
LIBPATHS += -L$(LIBF4D)/lib/libusartstm32f4

# Libraries to link
#   USB
LIBS 	 = -lm -lusbf4 -lstdperiph -lusbdevcdc -lusbcore -lusbdevcore  -lusbf4  -lstdperiph -lcommon_all
#   our stuff
LIBS	+= -ldiscoveryf4 -lmiscstm32f4 -lusartstm32f4

LDFLAGS	 = -T$(LDFILE)

OBJS = $(SRCS:.c=.o)

###################################################

.PHONY: proj

all: proj
	$(SIZE) $(NAME).elf

proj: 	$(NAME).elf

$(NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBPATHS) $(LIBS) $(LDFLAGS)
	$(OBJCOPY) -O ihex   $(NAME).elf $(NAME).hex
	$(OBJCOPY) -O binary $(NAME).elf $(NAME).bin
	$(OBJDUMP) -S $(NAME).elf > $(NAME).list

clean:
	rm -f *.o
	rm -f $(NAME).elf
	rm -f $(NAME).hex
	rm -f $(NAME).bin
#	$(MAKE) clean -C lib # Remove this line if you don't want to clean the libs as well
	
flash: images
	@printf "  FLASHING   $(NAME)\n"
	@# IMPORTANT: Don't use "resume", only "reset" will work correctly!
	# Kill running instances of the server
#	killall st-util
	st-flash write $(NAME).bin 0x08000000

.PHONY: images clean

