#!/bin/sh

: ${BUILDTOP=../..}
: ${SRCDIR=.}
: ${OZPLATFORM=`$SRCDIR/../bin/ozplatform`}
: ${OZPREFIX=/usr/local/oz}

if test "x$OZMAKE_COPYALWAYS" = xyes
then
    OZCOPYALWAYS=yes
    export OZCOPYALWAYS
fi

if test -z "$OZEMULATOR"
then
    for d in \
	$BUILDTOP/platform/emulator \
	$BUILDTOP/platform/emulator/$OZPLATFORM \
	$OZPREFIX/platform/$OZPLATFORM
    do
	if test -x $d/oz.emulator.bin
	then
	    OZEMULATOR=$d/oz.emulator.bin
	    break
	fi
    done
fi

if test "x$OZMAKE_VERBOSE" = xyes
then
    echo "Using OZEMULATOR: $OZEMULATOR"
fi

url=$1
shift

exec $OZEMULATOR -u $url -- $*
