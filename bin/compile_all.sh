#!/bin/bash

cd "$(readlink -f "$(dirname "$0")/..")"

bash bin/compile.sh PC debugpc           || exit -1
bash bin/compile.sh Robot releaserobot   || exit -1

echo
echo "ALL OKAY!"
echo
