#!/bin/bash
#
# compile the fumanoid project.
#
# this includes:
# * xabsl
# * protobuf
# * fumanoid

###############################################################################
# SETTINGS
###############################################################################

# processor count.
if [[ -z $PROCESSORCOUNT ]]; then
	PROCESSORCOUNT=`cat /proc/cpuinfo | grep -i ^processor | wc -l`
fi


ALLTARGETS="BerlinUnited Cognition Motion FUmanoid"


###############################################################################

# the script assumes you are in the root folder of FUmanoid, make sure this
# is so
cd "$(readlink -f "$(dirname "$0")/..")"

# custom pre build command (if exist)
if [ -e bin/custom_prebuild_commands.sh ]; then
	bin/custom_prebuild_commands.sh
fi

# if verbose is not set, disable verbosity
if [[ -z ${VERBOSE+xxx} ]]; then
	VERBOSE=""
elif [[ ! -z $VERBOSE ]]; then
	VERBOSE="verbose=1"
fi

# By setting the USE_CCACHE environment variable, we will use ccache to 
# speed up compilation of unchanged files. This makes only sense if you
# do a "make clean" often.
if [[ -z $USE_CCACHE ]]; then
	USE_CCACHE=0
fi
export USE_CCACHE

# disable distcc support
export USE_DISTCC=0

# get build information
BRANCHNAME="`git symbolic-ref HEAD 2> /dev/null | cut -b 12-`"
LASTHASH="`git log --pretty=format:%h -1`"
DATE="`date +%Y%m%d-%H%M`"
GITUSERNAME="`git config user.name`"
GITEMAIL="`git config user.email`"

# check that everything is set up in properly
ABORT=0
if [[ -z $GITUSERNAME ]]; then
	echo 
	echo "==============================================================="
	echo "WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING"
	echo
	echo "You did not yet set your git user.name, please do this now by"
	echo "opening a terminal, cd'ing into this folder and issuing the"
	echo "following command:"
	echo
	echo "   git config --global user.name <Firstname Lastname>"
	echo
	echo "(If you only want to set it for this specific project and not"
	echo "globally, you can leave out --global)"
	ABORT=1
fi

if [[ -z $GITEMAIL ]]; then
	echo 
	echo "==============================================================="
	echo "WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING"
	echo
	echo "You did not yet set your git user.email, please do this now by"
	echo "opening a terminal, cd'ing into this folder and issuing the"
	echo "following command:"
	echo
	echo "   git config --global user.email <Email used for Redmine/git>"
	echo
	echo "(If you only want to set it for this specific project and not"
	echo "globally, you can leave out --global)"
	ABORT=1
fi

if [[ $ABORT -eq 1 ]]; then exit; fi

USERINFO="$GITUSERNAME <$GITEMAIL>"
DIRTY=""
git diff-files --quiet || DIRTY=" (dirty)"
BUILDINFO="$BRANCHNAME-$LASTHASH$DIRTY build on $DATE by $USERINFO"
echo -e "#ifdef BUILDINFO\n#undef BUILDINFO\n#endif\n#define BUILDINFO \""$BUILDINFO"\"" > src/buildInfo.h

# compile for PC if no arguments are passed to the script
if [[ $# -eq 0 ]]; then
	PLATTFORM=PC
	CONFIG=debugpc
elif [[ $# -eq 1 ]]; then
	echo "Usage: $0 <PLATFORM> <CONFIG>, e.g. $0 PC debugpc"
	exit -1
else
	PLATTFORM=$1
	CONFIG=$2
fi

# check gcc version when compiling for PC
if [[ $PLATTFORM == "PC" ]]; then
	GCCVERSION_MAJOR=$(gcc -dumpversion | cut -f1 -d.)
	GCCVERSION_MINOR=$(gcc -dumpversion | cut -f2 -d.)

	if [[ $GCCVERSION_MAJOR -ne 4 ]]; then
		echo "This code has only been tested with gcc 4.x, you are using $(gcc --version | grep ^gcc | sed 's/^.* //g')."
		exit -1
	fi

	if [[ $GCCVERSION_MINOR -lt 7 ]]; then
		echo "This code needs at least gcc 4.7 - you are using $(gcc --version | grep ^gcc | sed 's/^.* //g')."
		echo "Please install an updated gcc version. If you are on an older"
		echo "Ubuntu version (e.g. 12.04), please follow these steps:"
		echo "   sudo add-apt-repository ppa:ubuntu-toolchain-r/test"
		echo "   sudo apt-get update"
		echo "   sudo apt-get install gcc-4.7 g++-4.7"
		echo "   sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.7 20"
		echo "   sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.7 20"
		echo "   sudo update-alternatives --config gcc"
		echo "   sudo update-alternatives --config g++"
		echo "(N.B.: instead of 4.7 you could also install 4.8)"
		exit -1
	fi
fi

# make sure we execute a clean if the makefile is older than the premake file
# TODO

# execute the protobuf makefiles
make -C src/messages $3 || exit
make -C berlinunited/src/messages $3 || exit
make -C berlinunited/src/messages python
echo \\n\\n\\n


# commmand to create make file
premake_cmd="./berlinunited/tools/premake/premake4.sh --platform=$PLATTFORM gmake"

# command to run the makefile (make sure to have -R in there as otherwise the wrong compiler will be used)
MAKETARGETS=${3:-$ALLTARGETS}
make_cmd="make config=$CONFIG ${VERBOSE} -R -j${PROCESSORCOUNT}"

exitcode=$?
if [[ $exitcode -eq 0 ]]; then
	echo "Create Makefile: $premake_cmd"
	$premake_cmd
	exitcode=$?
fi

if [[ $exitcode -eq 0 ]]; then
	echo "Create target(s) $MAKETARGETS via $make_cmd \$TARGET"
	for target in $MAKETARGETS; do
		$make_cmd $target || exit;
	done
	exitcode=$?
fi

# check that the pre-commit hook is in place
if [[ ! -e .git/hooks/pre-commit ]]; then
	ln -s ../../berlinunited/bin/pre-commit .git/hooks/pre-commit
	chmod +x .git/hooks/pre-commit
fi

# custom post build command (if exist)
if [[ -e bin/custom_postbuild_commands.sh ]]; then
	./bin/custom_postbuild_commands.sh
fi

exit $exitcode
