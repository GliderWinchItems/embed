# Makefile - Cortex-M3, STM32F103
# svn_pod/sw_stm32/trunk level
# 06-01-2011
# 03-24-2012 Added libxbeestm32


# Current directory
SRCDIR=.

SUBDIRS = libmiscstm32 libusartstm32 libsupportstm32 libxbeestm32
     
subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

