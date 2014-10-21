#! /bin/bash
#
# Run install.py with the correct settings and in the correct directory.


# get to project root
cd "$(readlink -f "$(dirname "$0")/..")"

# call install.py with the necessary information
./berlinunited/install/install.py --fork `readlink -f ./build/install.info`
