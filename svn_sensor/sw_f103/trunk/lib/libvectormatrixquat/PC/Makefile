#/******************************************************************************
#* File Name          : Makefile
#* Date First Issued  : 02/02/2015
#* Description        : Build library common to PC and STM32--for PC
#*******************************************************************************/ 
LIBNAME		= libvmq

# Current directory
THISDIR=.

# Current directory
CURDIR	=.

# Define path to source files.
VPATH=../

# Navigation to libraries specific to the f4 and discoveryf4
LIBF103	= $(CURDIR)/../../../../sw_f103/trunk/lib

# Compiler flags -Os = minimum space, -O3 = fastest
CFLAGS		= -Wall 
CFLAGS += -I$(LIBF103)/lib
CFLAGS += -I$(LIBF103)/lib/libvectormatrixquat


# Routines to be compiled into the library
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

