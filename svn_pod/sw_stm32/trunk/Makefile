# Makefile - Cortex-M3, STM32F103
# svn_pod/sw_stm32/trunk level
# 06-01-2011


# Current directory
SRCDIR=.

SUBDIRS =  lib devices gpiotest Usartall sdcardtest sdcardutil
     
subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

