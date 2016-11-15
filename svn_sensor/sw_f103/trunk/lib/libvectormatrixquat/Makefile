# Makefile 
# 02/02/2015


# Current directory
SRCDIR=.

# SUBDIRS =  PC STM32
SUBDIRS =  PC f103
     
subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

clean:
	for dir in $(SUBDIRS); do \
		rm -f *.o; \
		rm -f $$dir/*.a; \
	done

.PHONY: clean

