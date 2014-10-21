#!/bin/bash

FOUND=0
FILELIST=`find . | grep -E "src/.*(\.cpp|\.h)$"`
for i in $FILELIST; do
	RESULT=`cat $i | grep -Hn -P $" .*\t"`

	if [ "x$RESULT" != "x" ]; then
		echo "Found stray tabs in $i"
		echo "$RESULT" | sed -e "s#(standard input)#$i#"
		FOUND=1
	fi
done

exit $FOUND
