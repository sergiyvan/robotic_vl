#!/bin/bash

cd "$(readlink -f "$(dirname "$0")/..")"
bash bin/compile.sh Robot releaserobot
