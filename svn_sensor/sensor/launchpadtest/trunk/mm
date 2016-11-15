#!/bin/bash
echo ==================================================================================================================================================================
export FLOAT_TYPE=soft
echo $FLOAT_TYPE
cd ../../../../svn_pod/sw_stm32/trunk/devices
#make clean
make
cd ../lib/libmi*
make clean
make
cd ../libu*
#make clean
make
cd ../libsu*
make clean
make
cd ../../../../../svn_sensor/sw_f103/trunk/lib/libdev*
make clean
make
cd ../lib*sensormisc
make clean
make
cd ../../../../../svn_common/trunk
make clean
make
cd ../../svn_sensor/sensor/launchpadtest/trunk
make clean
make


