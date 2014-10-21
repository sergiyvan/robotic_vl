#!/bin/bash
#
# Compile the berlinunited project - this is for testing purposes only.
#
###############################################################################
# SETTINGS
###############################################################################

# processor count.
PROCESSORCOUNT=`cat /proc/cpuinfo | grep -i ^processor | wc -l`

###############################################################################

# the script assumes you are in the root folder of BerlinUnited, make sure this
# is so
cd `dirname $0`/..

# if verbose is not set, disable verbosity
if [ -z "${VERBOSE+xxx}" ]; then
	VERBOSE=""
elif [ ! -z "${VERBOSE}" ]; then
	VERBOSE="verbose=1"
fi

PLATTFORM=PC
CONFIG=debugpc

# execute the protobuf makefiles
make -C src/messages $3 || exit
#make -C src/messages python
#echo \\n\\n\\n

# create make file and execute it (make sure to have -R in there as otherwise the wrong compiler will be used)
premake_cmd="./tools/premake/premake4.sh --platform=$PLATTFORM gmake"
   make_cmd="make config=$CONFIG ${VERBOSE} -R -j${PROCESSORCOUNT} $3"

exitcode=$?
if [ $exitcode -eq 0 ]; then
	echo "Create Makefile: $premake_cmd"
	$premake_cmd
	exitcode=$?
fi

if [ $exitcode -eq 0 ]; then
	echo "Create Target:   $make_cmd"
	$make_cmd
	exitcode=$?
fi

exit $exitcode
