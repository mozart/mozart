#!/bin/sh
#
# use this for bootstrapping the system:
#
#    make OZC=./ozc.sh
#

: ${BUILDTOP=../..}
: ${SRCDIR=.}
: ${SRCTOP=$SRCDIR/../..}
: ${OZMAFILE="-b $SRCTOP/share/lib/ozc.ozm"}
: ${OZPLATFORM=`$SRCTOP/share/bin/ozplatform`}
: ${OZPREFIX=/usr/local/oz}

echo Fatal error: ozc.sh should no longer be called
exit 1

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

if test -z "$OZEMULATOR"
then
    echo Fatal error: no oz.emulator.bin found
    exit 1
fi

OZINIT=${OZMAINIT}
export OZINIT

if test "x$OZMAKE_VERBOSE" = xyes
then
    echo "Using OZEMULATOR: $OZEMULATOR"
    echo "Using OZMAFILE: $OZMAFILE"
fi

exec $OZEMULATOR $OZMAFILE -- "$@"
