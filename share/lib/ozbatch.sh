#!/bin/sh
#
# use this for bootstrapping the system:
#
#    make OZBATCH=./ozbatch.sh
#

: ${SRCDIR=.}
: ${OZMAFILE=$SRCDIR/ozbatch.ozm}
: ${OZPLATFORM=`$SRCDIR/../bin/ozplatform`}
: ${OZPREFIX=/usr/local/oz}

if test -z "$OZEMULATOR"
then
    for d in $OZPREFIX/platform/$OZPLATFORM \
	$SRCDIR/../Emulator $SRCDIR/../Emulator/$OZPLATFORM
    do
	if test -x $d/oz.emulator.bin
	then
	    OZEMULATOR=$d/oz.emulator.bin
	    break
	fi
    done
fi

OZINIT=${OZMAINIT}
export OZINIT

echo "Using OZEMULATOR: $OZEMULATOR"
echo "Using OZMAFILE: $OZMAFILE"

exec $OZEMULATOR -b $OZMAFILE -a "$@"
