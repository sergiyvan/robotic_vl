#!/bin/bash

# the script assumes you are in the root folder of the project, make sure this is so
cd "$(readlink -f "$(dirname "$0")/..")"

# install for the required platforms
./berlinunited/bin/update-sdk.sh -p odroid-x2
