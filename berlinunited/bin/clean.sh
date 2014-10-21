#!/bin/bash
#
# Clean the project

cd "$(readlink -f "$(dirname "$0")/..")"

rm -vrf build/pc/BerlinUnited

rm -vrf obj/*

rm -vrf src/messages/*.pb.*

rm -vrf install/protobuf/*_pb2.py
rm -vrf install/protobuf/*.pyc

rm -vf Makefile
rm -vf BerlinUnited.make
