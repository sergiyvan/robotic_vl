#!/bin/bash
#
# Clean the FUmanoid project
#

cd "$(readlink -f "$(dirname "$0")/..")"

rm -vrf build/debug-symbols.dat
rm -vrf build/behavior-ic.dat
rm -vrf build/pc/FUmanoid
rm -vrf build/robot/FUmanoid

rm -vrf obj/*

rm -vrf src/messages/*.pb.*
rm -vrf src/communication/protobuf # this folder does not exist anymore
rm -vrf berlinunited/src/messages/*.pb.*

rm -vrf install/protobuf/*_pb2.py
rm -vrf install/protobuf/*.pyc

rm -vf Makefile
rm -vf FUmanoid.make
rm -vf BerlinUnited.make
rm -vf Cognition.make
rm -vf Motion.make


#rm -vrf doc/xabsl
#rm -vrf doc/doxygen
#rm -vrf doc/module*
