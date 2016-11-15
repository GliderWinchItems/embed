# Makefile - PC
# 05/20/2014

# Name of the .c file with 'main' as well as output file name
NAME		= sdcopy

# Current directory
SRCDIR=.

LIBCOMMONALL    =$(SRCDIR)/../../../../svn_discoveryf4/common_all/trunk

# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS		= -Wall -Wstrict-prototypes \
-I../../../sw_f103/trunk/lib/libsensormisc \
-I../../../../svn_pod/sw_stm32/trunk/lib/libopenstm32 \
-I../../../../svn_pod/sw_stm32/trunk/lib/libsupportstm32 \
-I../../../../svn_pod/sw_stm32/trunk/devices \
-I../../../../svn_discoveryf4/sw_discoveryf4/trunk/lib \
-I../../../../svn_discoveryf4/common_all/trunk \
-I$(LIBCOMMONALL) \


# Libraries (.a)
LDFLAGS         = -L../../../../svn_discoveryf4/common_all/trunk/PC -lcommon_all

# List of files to be compiled
OBJS = $(NAME).o \
packet_extract.o \
packet_search.o \
packet_print.o \
can_write.o \


canldr: $(OBJS)
	gcc -o $(NAME) $(OBJS) $(LDFLAGS)

clean:
	rm -f $(NAME)\
	rm -f *.o

.PHONY: clean


