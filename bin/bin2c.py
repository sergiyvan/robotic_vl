#!/usr/bin/python2
#
# Converts a binary file into a C++ source file, so it can
# be incorporated into a binary.
#
# Usage:   bin2c.py <name> <binary> <cppfile>
#
# where <name> is the name of the object containing the data. The
# name will be prefaced with "BINARY_".

import textwrap
import sys

if (len(sys.argv) != 4):
	print "Usage: " + sys.argv[0] + " <name> <infile> <outfile>"
	quit()

name    = sys.argv[1]
infile  = sys.argv[2]
outfile = sys.argv[3]

binf = open(infile, 'rb')
bind = binf.read()
bin  = bind.encode("hex").upper(); 
data = "".join(["\\x" + x + y
	for (x,y) in zip(bin[0::2], bin[1::2])])

out  = open(outfile, 'w')
out.write("#ifndef BINARY_" + name + "_H\n")
out.write("#define BINARY_" + name + "_H\n")
out.write("static char BINARY_" + name + "[] = \\\n\t\"%s\";"%"\" \\\n\t\"".join(textwrap.wrap(data, 80)))
out.write("\n");
out.write("static int BINARY_" + name + "_SIZE = " + str(len(bind)) + ";\n")
out.write("#endif\n");
out.close()
