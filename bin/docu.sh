#!/bin/bash

# Create the documentatin for the FUmanoid project.

cd "$(readlink -f "$(dirname "$0")/..")"

python bin/module2pdf.py
doxygen FUmanoid.doxyfile

cd src/modules/cognition/behaviorLayer/xabsl
make DOC
