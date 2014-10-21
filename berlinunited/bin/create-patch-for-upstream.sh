#!/bin/bash

if [[ $# != 1 ]]; then
	echo "Usage: $0 <sha1>"
fi

filename=`git format-patch -1 $1`
wrongfiles=`grep -e "^+++ b/" $filename | grep -v " b/berlinunited"`
if [[ ! -z $wrongfiles ]]; then
	echo "Your patch contains files outside the berlinunited folder."
	echo "Please remove them, or check that you exported the correct"
	echo "commit. The files in question:"
	echo $wrongfiles
	echo
fi

# fix paths
sed -i -e "s_ \([a-b]\)/berlinunited/_ \1/_g" $filename

echo "The patch was stored in $filename."
echo "Please send it by email to the upstream project's maintainer."

