#/******************** (C) COPYRIGHT 2010 **************************************
#* File Name          : Makefile
#* Hackor             : deh
#* Date First Issued  : 10/04/2010 deh
#* Description        : Build library: libusartstm32.a
#*******************************************************************************/ 
LIBNAME		= libusartstm32

# Prefix the name for the particular toolchain
PREFIX		= arm-none-eabi
# The path for the commands to find the compiler, linker, et al.
TOOLDIR 	= $(HOME)/CodeSourcery/Sourcery_G++_Lite/bin
CC		= $(TOOLDIR)/$(PREFIX)-gcc
LD		= $(TOOLDIR)/$(PREFIX)-ld
OBJCOPY		= $(TOOLDIR)/$(PREFIX)-objcopy
OBJDUMP		= $(TOOLDIR)/$(PREFIX)-objdump
AR		= $(TOOLDIR)/$(PREFIX)-ar

# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS		= -Os -g -Wall -Wextra -Iinclude -fno-common \
		  -mcpu=cortex-m3 -mthumb -Wstrict-prototypes -save-temps

AFLAGS  = -mcpu=cortex-m3 -mthumb

ARFLAGS	= rcsv
#ARFLAGS		= rcs

OBJS = \
usartx_setbaud.o \
usartx_putsS.o \
 \
usartx_tx_advlnptr.o \
 \
usartx_rxint_init.o \
usartx_rxint_getline.o \
 \
USART1_rxint_getcount.o \
USART1_rxint_getline.o \
USART1_rxint_getlineboth.o \
usartx_txdma_init.o \
 \
usartx_txint_send.o \
usartx_txint_init.o \
 \
USART1_rxinttxint.o \
USART1_txint_putc.o \
USART1_txint_puts.o \
USART1_txint_send.o \
USART1_txint_busy.o \
\
USARTremap.o \
\




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

