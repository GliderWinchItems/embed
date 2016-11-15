# Makefile - Cortex-M3, STM32F103
# svn_pod/sw_stm32/trunk level
# 06-01-2011


# Current directory
SRCDIR=.

SUBDIRS = libdiscoveryf4 libmiscstm32f4 libusartstm32f4 
     
subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

