#!/bin/sh
#
# use this for bootstrapping the system:
#
#    make OZBATCH=./ozbatch.sh
#

: ${SRCDIR=.}
: ${OZMAFILE=$SRCDIR/ozbatch.ozm}
: ${OZPLATFORM=`$SRCDIR/../bin/ozplatform`}

if test -z "$OZEMULATOR"
then
    for d in /usr/local/oz/platform/$OZPLATFORM \
	$SRCDIR/../Emulator $SRCDIR/../Emulator/$OZPLATFORM \
	    
    do
	if test -x $d/oz.emulator.bin
	then
	    OZEMULATOR=$d/oz.emulator.bin
	    break
	fi
    done
fi

: ${OZQUIET=-quiet}
OZINIT=${OZMAINIT}
export OZINIT

echo "Using OZEMULATOR: $OZEMULATOR"
echo "Using OZMAFILE: $OZMAFILE"

exec $OZEMULATOR $OZQUIET -b $OZMAFILE -a "$@"
