# Makefile 
# 10/06/2013


# Current directory
SRCDIR=.

SUBDIRS =  PC stm32
     
subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

clean:
	for dir in $(SUBDIRS); do \
		rm -f $$dir/*.o; \
	done

.PHONY: clean

