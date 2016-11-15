# Makefile 
# 02/02/2015


# Current directory
SRCDIR=.

# SUBDIRS =  PC STM32
SUBDIRS =  PC f103
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
export OBJS     
subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

clean:
	for dir in $(SUBDIRS); do \
		rm -f $$dir/*.o; \
		rm -f $$dir/*.a; \
	printf "DIRECTORY $$dir\n"; \
		cd $$dir; \
		$(MAKE) clean; \
		cd ..; \
	done
.PHONY: clean

