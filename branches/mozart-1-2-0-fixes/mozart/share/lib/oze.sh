#!/bin/sh

: ${BUILDTOP=../..}
: ${SRCDIR=.}
: ${OZPLATFORM=`$SRCDIR/../bin/ozplatform`}
: ${OZPREFIX=/usr/local/oz}
: ${OZINIT="$BUILDTOP/share/lib/Init.ozf"}

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
	if test -x $d/emulator.exe
	then
	    OZEMULATOR=$d/emulator.exe
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

exec $OZEMULATOR -init $OZINIT -u $url -- $*
