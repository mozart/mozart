#!/bin/sh

case `uname -s` in
    CYGWIN*)
	cygpath -w "$1" | sed 's|\\|/|g'
	;;
    *)
	echo "$1"
	;;
esac
