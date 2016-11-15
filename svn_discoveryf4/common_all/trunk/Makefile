# Makefile 
# 10/06/2013
# 02/17/2015 updated to descend to subdirectories for 'make clean'


# Current directory
SRCDIR=.

# Build common routines for all processors
# stm32 original f4
# f4 same as original (leave stm32 so's not break Makefiles)
# SUBDIRS =  PC stm32
SUBDIRS =  PC f4 f103 libvectormatrixquat stm32
     
subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

clean:
	for dir in $(SUBDIRS); do \
		cd $$dir; \
		$(MAKE) clean; \
		cd ..; \
	done

.PHONY: clean

