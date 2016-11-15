# Makefile - PC
# 10/06/2013




# Name of the .c file with 'main' as well as output file name
NAME		= canldr

# Current directory
SRCDIR=.

LIBCOMMONALL    =$(SRCDIR)/../../../../common_all/trunk

# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS		= -Wall -Wstrict-prototypes \
-I../../../../sw_f103/trunk/lib/libsensormisc \
-I../../../../../svn_pod/sw_stm32/trunk/lib/libopenstm32 \
-I$(LIBCOMMONALL) \


# Libraries (.a)
LDFLAGS         = -L../../../../common_all/trunk/PC -lcommon_all

# List of files to be compiled
OBJS = canldr.o \
commands.o \
cmd_a.o \
cmd_l.o \
cmd_n.o \
cmd_m.o \
cmd_p.o \
cmd_r.o \



canldr: $(OBJS)
	gcc -o canldr $(OBJS) $(LDFLAGS)

clean:
	rm -f $(NAME)\
	rm -f *.o

.PHONY: clean


