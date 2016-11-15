#/******************************************************************************
#* File Name          : Makefile
#* Date First Issued  : 02/02/2015
#* Description        : Build library common to PC and STM32--for PC
#*******************************************************************************/ 
LIBNAME		= libvmq


# Prefix the name for the particular toolchain
PREFIX		= arm-none-eabi
# The path for the commands to find the compiler, linker, et al.
TOOLDIR 	= $(HOME)/CodeSourcery/Sourcery_G++_Lite/bin
CC		= $(TOOLDIR)/$(PREFIX)-gcc
LD		= $(TOOLDIR)/$(PREFIX)-ld
OBJCOPY		= $(TOOLDIR)/$(PREFIX)-objcopy
OBJDUMP		= $(TOOLDIR)/$(PREFIX)-objdump
AR		= $(TOOLDIR)/$(PREFIX)-ar


# Define path to source files.
VPATH=../

# Current directory
THISDIR=.

# Library directory path--common includes
LIBCOMMONALL = $(VPATH)
LIBPODLIB    = $(THISDIR)/../../../../svn_pod/sw_stm32/trunk/lib

# Library directory path prefix--svn_pod library
LIBPODLIB	= $(THISDIR)/../../../../svn_pod/sw_stm32/trunk/lib
LIBPODDEV	= $(THISDIR)/../../../../svn_pod/sw_stm32/trunk/devices
# Library directory path--svn_pod library 
LIBOPENSTM32	= $(LIBPODLIB)/libopenstm32

# Library directory path prefix--svn_sensor, f103 library
LIBSENSORF103	= $(THISDIR)/../../../sw_f103/trunk/lib
# Library directory path--svn_sensor, f103 library
LIBSENSORMISC	= $(LIBSENSORF103)/libsensormisc

# Library directory path--compile and stuff
LGCC		= $(HOME)/CodeSourcery/Sourcery_G++_Lite/lib/gcc/arm-none-eabi/4.5.2/thumb2
LOTHER		= $(HOME)/CodeSourcery/Sourcery_G++_Lite/arm-none-eabi/lib/thumb2


# Compiler flags -Os = no optimization, -Os = minimum space, -O3 = fastest
CFLAGS		= -Os -g -Wall -Wextra 
CFLAGS += -I$(LIBF103)/lib/libvectormatrixquat
CFLAGS += -I$(LIBCOMMONALL) 
CFLAGS += -I$(LIBOPENSTM32) 
CFLAGS += -I$(LIBSENSORMISC)
CFLAGS += -Iinclude -fno-common -mcpu=cortex-m3 -mthumb -Wstrict-prototypes -fno-builtin-putc

AFLAGS  = -mcpu=cortex-m3 -mthumb

ARFLAGS	= rcsv
#ARFLAGS		= rcs

OBJS =  m3x3.o
OBJS	+= qrotate.o
OBJS	+= quaternionmath.o
OBJS	+= qd_rotation_matrixB.o
OBJS	+= qd_rotation_matrix.o
OBJS    += qd_fromaxisangleandvector.o
OBJS	+= vd3_axisanglefromvector.o
OBJS	+= vd3_degtorad.o
OBJS	+= vd3_radtodeg.o
OBJS	+= vd3_normalize.o
OBJS	+= vs3_normalize.o
OBJS	+= qd_magnitude.o
OBJS	+= qd_normalize.o
OBJS	+= vd3_magnitude.o
OBJS	+= vs3_magnitude.o
OBJS	+= eulertoquatd.o
OBJS	+= eulertoquatdK.o
OBJS	+= cosinetoquat.o
OBJS	+= eulertodirectioncosines.o
OBJS	+= md33_rotationangle.o
OBJS	+= vd3_rotationanglefromeuler.o
OBJS	+= vd3_axisofrotation.o
OBJS	+= qd_mulpq.o
OBJS	+= vd3_313from123.o
OBJS	+= vd3_123from313.o
OBJS	+= qd_mulqp.o
OBJS	+= qd_muleff.o
OBJS	+= qd_conj.o
OBJS	+= qd_normsq.o
OBJS	+= qd_inverse.o
OBJS	+= md33_rotfromquatd.o
OBJS	+= qd_toeuler.o
OBJS	+= vd3_eulertovector.o
OBJS	+= axisangle_phi_fromtwoangletiltd.o
OBJS	+= vd3_fromaxisandphid.o
OBJS	+= qd_fromvd3andaxisangleandrotation.o
OBJS	+= vd3_euler123fromquatd.o
OBJS	+= qd_todirectioncosinesd.o
OBJS	+= qd_toeuleranglesd.o
OBJS	+= qd_quatunit.o
OBJS	+= qd_fromvd3.o

OBJS	+= md33transpose.o
OBJS	+= md33mulmd33.o
OBJS	+= md33mul31.o
OBJS	+= ms33mul31.o
OBJS	+= md33det.o
OBJS	+= ms33det.o
OBJS	+= md33toms33.o


# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q := @
endif

all: $(LIBNAME).a

$(LIBNAME).a: $(OBJS)
	@printf "  AR      $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	@printf "  CC      $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<

clean:
	@printf "  CLEAN   $(subst $(shell pwd)/,,$(OBJS))\n"
	$(Q)rm -f *.o
	@printf "  CLEAN   $(LIBNAME).a\n"
	$(Q)rm -f $(LIBNAME).a

.PHONY: clean

