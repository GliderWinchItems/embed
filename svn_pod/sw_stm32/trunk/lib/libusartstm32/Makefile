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
		  -mcpu=cortex-m3 -mthumb -Wstrict-prototypes

AFLAGS  = -mcpu=cortex-m3 -mthumb

ARFLAGS	= rcsv
#ARFLAGS		= rcs

OBJS = \
mymalloc.o \
 \
usartx_setbaud.o \
usartx_putsS.o \
 \
usartx_rxmin_init.o \
usartx_txmin_init.o \
 \
USART1_txmin.o \
USART1_rxmin.o \
USART1_rxmin_getchar.o \
USART1_rxmin_rxready.o \
USART1_txmin_putc.o \
USART1_txmin_puts.o \
USART1_txmin_txbusy.o \
 \
USART2_txmin.o \
USART2_rxmin.o \
USART2_rxmin_getchar.o \
USART2_rxmin_rxready.o \
USART2_txmin_putc.o \
USART2_txmin_puts.o \
USART2_txmin_txbusy.o \
 \
USART3_txmin.o \
USART3_rxmin.o \
USART3_rxmin_getchar.o \
USART3_rxmin_rxready.o \
USART3_txmin_putc.o \
USART3_txmin_puts.o \
USART3_txmin_txbusy.o \
 \
UART5_txmin.o \
UART5_rxmin.o\
UART5_rxmin_getchar.o \
UART5_rxmin_rxready.o \
UART5_txmin_putc.o \
UART5_txmin_puts.o \
UART5_txmin_txbusy.o \
\
usartx_txdma_init.o \
usartx_txdma_putc.o \
usartx_txdma_send.o \
usartx_tx_advlnptr.o \
 \
USART1_txdma.o \
USART1_txdma_putc.o \
USART1_txdma_puts.o \
USART1_txdma_busy.o \
USART1_txdma_send.o \
 \
USART2_txdma.o \
USART2_txdma_putc.o \
USART2_txdma_puts.o \
USART2_txdma_busy.o \
USART2_txdma_send.o \
 \
USART3_txdma.o \
USART3_txdma_putc.o \
USART3_txdma_puts.o \
USART3_txdma_busy.o \
USART3_txdma_send.o \
 \
usartx_rxdma_init.o \
usartx_rx_advlnptr.o \
 \
USART1_rxdma.o \
USART1_rxdma_getcount.o \
USART1_rxdma_getchar.o \
USART1_rxdma_getline.o \
 \
usartx_rxint_init.o \
usartx_rxint_rxisrptradvRTC.o \
usartx_rxint_rxisrptradvRTC_p1.o \
usartx_rxint_getline.o \
 \
USART2_rxdma.o \
USART2_rxdma_getcount.o \
USART2_rxdma_getchar.o \
USART2_rxdma_getline.o \
  \
USART3_rxdma.o \
USART3_rxdma_getcount.o \
USART3_rxdma_getchar.o \
USART3_rxdma_getline.o \
  \
USART1_rxint.o \
USART1_rxint_getcount.o \
USART1_rxint_getline.o \
USART1_rxint_getlineboth.o \
 \
USART2_rxint.o \
USART2_rxint_getcount.o \
USART2_rxint_getline.o \
USART2_rxint_getlineboth.o \
 \
USART3_rxint.o \
USART3_rxint_getcount.o \
USART3_rxint_getline.o \
USART3_rxint_getlineboth.o \
 \
usartx_txcir_putsS.o \
usartx_txcir_init.o \
usartx_txcir_putc.o \
usartx_txcir_puts.o \
usartx_txcir_putcount.o \
 \
USART1_rxinttxcir.o \
USART1_txcir_putcount.o \
USART1_txcir_putc.o \
USART1_txcir_puts.o \
 \
USART2_rxinttxcir.o \
USART2_txcir_putcount.o \
USART2_txcir_putc.o \
USART2_txcir_puts.o \
 \
USART3_rxinttxcir.o \
USART3_txcir_putcount.o \
USART3_txcir_putc.o \
USART3_txcir_puts.o \
 \
usartx_txint_send.o \
usartx_txint_init.o \
 \
USART1_rxinttxint.o \
USART1_rxinttxintRTC.o \
USART1_txint_putc.o \
USART1_rxinttxintRTC_p1.o \
USART1_txint_puts.o \
USART1_txint_send.o \
USART1_txint_busy.o \
 \
USART2_rxinttxint.o \
USART2_rxinttxintRTC.o \
USART2_txint_putc.o \
USART2_txint_puts.o \
USART2_txint_send.o \
USART2_txint_busy.o \
 \
USART3_rxinttxint.o \
USART3_rxinttxintRTC.o \
USART3_txint_putc.o \
USART3_txint_puts.o \
USART3_txint_send.o \
USART3_txint_busy.o \
\
UART4_rxinttxintRTC.o \
UART4_txint_putc.o \
UART4_txint_puts.o \
UART4_txint_send.o \
UART4_txint_busy.o \
UART4_rxint_getlineboth.o \
\
UART5_rxinttxint.o \
UART5_rxinttxintRTC.o \
UART5_txint_putc.o \
UART5_txint_puts.o \
UART5_txint_send.o \
UART5_txint_busy.o \
UART5_rxint_getlineboth.o \
\
USARTremap.o \
\
USART2_setbaud.o \
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

