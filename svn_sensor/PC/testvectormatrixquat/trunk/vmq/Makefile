# Makefile - PC
# 10/06/2013




# Name of the .c file with 'main' as well as output file name
NAME		= pctestq

# Current directory
SRCDIR=.

LIBF103    =$(SRCDIR)/../../../../sw_f103/trunk/lib/libvectormatrixquat

# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS  = -Wall -Wstrict-prototypes
CFLAGS += -I../../../../sw_f103/trunk/lib/libvectormatrixquat


# Libraries (.a)
LDFLAGS         = -lm -L../../../../sw_f103/trunk/lib/libvectormatrixquat/PC -lvmq -lm
LDFLAGS = -L ../../../../sw_f103/trunk/lib/libvectormatrixquat/PC -lvmq -lm


# List of files to be compiled
OBJS = pctestq.o



$(NAME): $(OBJS)
	gcc -o $(NAME) $(OBJS) $(LDFLAGS) -lm -lvmq

clean:
	rm -f $(NAME)\
	rm -f *.o

.PHONY: clean


