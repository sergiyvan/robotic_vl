#!/bin/bash
#
# Generic installation script for cross-compiler and SDK
#
# Conventions:
#	* the SDK will be named as "sdk-[date/version].suffix
#	* the cross compiler will be named as "crosscompiler-[date/version]-[os].suffix
#	* date/version must not contain dots and sort lexically
#	* supported suffixes: tgz, tar.gz, tbz, tar.bz2, txz, tar.xz, zip
#
################################################################################

TERM_RESET="\033[0m"
TERM_UNDERLINE="\033[4m"
TERM_UNDERLINE_OFF="\033[24m"

TERM_BLINK_SLOW="\033[5m"
TERM_BLINK_FAST="\033[6m"
TERM_BLINK_OFF="\033[25m"

TERM_BLACK="\033[30m"
TERM_RED="\033[31m"
TERM_GREEN="\033[32m"
TERM_BROWN="\033[33m"
TERM_BLUE="\033[34m"
TERM_PURPLE="\033[35m"
TERM_CYAN="\033[36m"
TERM_LIGHT_GRAY="\033[37m"


################################################################################

function usage {
	cat << EOF
Usage: $0 <options>

Installs a cross-compiler and SDK.

OPTIONS:
   -h      Show this message
   -p      platform
   -x      cross compiler URI
   -X      cross compiler root directory
   -s      sdk URI
   -S      sdk root directory
   -i      installation directory
   -f      force installation
   -v      verbose
EOF
}


################################################################################

function execute {
	CMD=$@
	echo -e "${TERM_RED}Execute ${TERM_LIGHT_GRAY}${CMD}${TERM_RESET}"
	$CMD
}


################################################################################

# init variables
PLATFORM=
CROSS_URI=
CROSS_ROOT="svn+https://maserati.mi.fu-berlin.de/svn/binaries"
SDK_URI=
SDK_ROOT="svn+https://maserati.mi.fu-berlin.de/svn/binaries"
INSTALL_PATH="/opt"
HOST_OS=linux
TMPDIR=/tmp/

while getopts “hp:x:X:s:S:i:fv” OPTION
do
	case $OPTION in
		h)
			usage
			exit 1
			;;
		p)
			PLATFORM=${OPTARG,,}
			;;
		x)
			CROSS_URI=$OPTARG
			;;
		X)
			CROSS_ROOT=$OPTARG
			;;
		s)
			SDK_URI=$OPTARG
			;;
		S)
			SDK_ROOT=$OPTARG
			;;
		i)
			INSTALL_PATH=$OPTARG
			;;
		f)
			FORCE=yes
			;;
		v)
			VERBOSE=1
			;;
		?)
			usage
			exit
			;;
	esac
done


# check that either URIs or platform is specified
if [[ ( -z $PLATFORM ) && ( -z $SDK_URI || -z $CROSS_URI ) ]]; then
	echo "ERROR: Please specify the platform or both URIs."
	exit 1
fi

# add platform to install path
if [[ ( ! -z $PLATFORM ) && ( -z $SDK_URI && -z $CROSS_URI ) ]]; then
	INSTALL_PATH="${INSTALL_PATH}/$PLATFORM"
fi

# check that install path exists
if [[ ! -d $INSTALL_PATH ]]; then
	echo "WARNING: Install path ${INSTALL_PATH} does not exist. Creating it now."
	execute sudo mkdir $INSTALL_PATH
	execute sudo chown $USER $INSTALL_PATH
fi


################################################################################

function detect_file {
	LOCATION=$1
	
	if [[ $LOCATION == svn* ]]; then
		CMD="svn ls ${LOCATION#*+} | grep -e "^$2" | grep "$3\." | tail -1"
		echo -e "${TERM_RED}Execute ${TERM_LIGHT_GRAY}${CMD}${TERM_RESET}"
		FILE=`eval $CMD`
	elif [[ -d $LOCATION ]]; then
		FILE=`ls ${LOCATION}  | grep -e "^$2" | grep "$3\." | tail -1`
	else
		echo "ERROR: Could not access ${LOCATION}"
		exit 1
	fi
	
	RESULT="${LOCATION}/${FILE}"
}


################################################################################

function get_version {
	FILE=${1##*/}
	TMP=${FILE#*-}
	echo ${TMP%%.*}
}


################################################################################

function needs_update {
	DIR=$1
	URI=$2
	TYPE=$3
	
	if [[ ! -z $FORCE ]]; then
		return 1
	fi

	# get installed versions
	if [[ -e $DIR/$TYPE/.version ]]; then
		current_version=`cat $DIR/$TYPE/.version`
	else
		current_version=""
	fi
	
	# extract version from URI
	new_version=$(get_version $URI)

	if [[ $current_version < $new_version ]]; then
		echo "Installed $TYPE version needs an update (from ${current_version:-none} to ${new_version})"
		return 1
	elif [[ $current_version == $new_version ]]; then
		echo "Installed $TYPE version ${current_version} is the latest available version"
		return 0
	else
		echo "Installed $TYPE version ${current_version} is newer than the latest available version ${new_version}"
		return 0
	fi
}


################################################################################

function install_file {
	if [[ $# -ne 3 ]]; then
		echo ERROR: wrong number of arguments
		exit 1
	fi

	DIR=$1
	URI=$2
	TYPE=$3
	
	FILE=${URI##*/}
	SUFFIX=${FILE#*.}
	BASE=`basename $FILE .$SUFFIX`

	cd $TMPDIR
	if [[ -e $FILE ]]; then
		echo "File $FILE already downloaded, skipping new download. If file is corrupt, please delete $TMPDIR/$FILE manually and re-start"
	elif [[ $URI == svn* ]]; then
		execute svn export ${LOCATION#*+}/$FILE || exit 1
	elif [[ -e $URI ]]; then
		execute ln -s $URI
	else
		echo "ERROR: Could not access ${DIR}"
		exit 1
	fi

	execute cd $DIR
	execute rm -rf \"${DIR}/${TYPE}\"
	case $SUFFIX in
		tbz|tar.bz|tbz2|tar.bz2)
			execute tar xjf /tmp/$FILE
			;;
		tgz|tar.gz)
			execute tar xzf /tmp/$FILE
			;;
		txz|tar.xz)
			execute tar xJf /tmp/$FILE
			;;
		tar)
			execute tar xf /tmp/$FILE
			;;
		zip)
			execute unzip /tmp/$FILE
			;;
		?)
			echo "ERROR: $FILE is of unknown type"
			exit
			;;
	esac

	echo $(get_version $URI) > $TYPE/.version
}


################################################################################

if [[ -z $CROSS_URI ]]; then
	detect_file $CROSS_ROOT/$PLATFORM/ compiler ${HOST_OS}
	CROSS_URI=$RESULT
fi

if [[ -z $SDK_URI ]]; then
	detect_file $CROSS_ROOT/$PLATFORM/ sdk
	SDK_URI=$RESULT
fi

echo "Paths:"
echo "             SDK: ${SDK_URI}"
echo "  Cross compiler: ${CROSS_URI}"

################################################################################

needs_update $INSTALL_PATH $CROSS_URI compiler
if [[ $? -eq 1 ]]; then
	install_file $INSTALL_PATH $CROSS_URI compiler
fi

needs_update $INSTALL_PATH $SDK_URI sdk
if [[ $? -eq 1 ]]; then
	install_file $INSTALL_PATH $SDK_URI sdk
fi

