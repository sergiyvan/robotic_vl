#!/bin/bash


case "$OSTYPE" in
	linux*)      $(dirname $0)/premake4_linux $@ ;;
	darwin*)     $(dirname $0)/premake4_macosx $@ ;;
	*)           premake $@ ;;
esac

