# Makefile - Cortex-M3, STM32F103
# svn_sensor/sw_f103/trunk/lib level
# 11-10-2012


# Current directory
SRCDIR=.

SUBDIRS = libdevicestm32 libsensormisc
     
subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

clean:
	for dir in $(SUBDIRS); do \
		rm -f $$dir/*.o; \
	done

.PHONY: clean

