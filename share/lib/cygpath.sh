#!/bin/sh

: ${SRCDIR=.}
: ${OZPLATFORM=`$SRCDIR/../bin/ozplatform`}

case $OZPLATFORM in
    win32*)
	cygpath -w -s $1 | sed 's|\\|/|g'
	;;
    *)
	echo $1
	;;
esac
