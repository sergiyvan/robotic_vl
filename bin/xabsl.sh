#!/bin/bash
#
# Start the XABSLEditor

(
	cd "$(readlink -f "$(dirname "$0")")"
	java -jar XabslEditor/XabslEditor.jar
) &

