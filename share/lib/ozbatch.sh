#!/bin/sh
#
# use this for bootstrapping the system:
#
#    make OZBATCH=./ozbatch.sh
#

: ${SRCDIR=.}
: ${OZMAFILE=$SRCDIR/ozbatch.ozm}
: ${OZPLATFORM=`$SRCDIR/../bin/ozplatform`}

if test -z "$OZ_EMULATOR_DIR"
then
    for d in ../Emulator ../Emulator/$OZPLATFORM \
            /usr/local/oz/platform/$OZPLATFORM
    do
        if test -r $d/ozma.so && test -x $d/oz.emulator.bin
        then
            OZ_EMULATOR_DIR=$d
            break
        fi
    done
fi

: ${OZEMULATOR=$OZ_EMULATOR_DIR/oz.emulator.bin}
: ${OZMA_LIB="-B $OZ_EMULATOR_DIR/ozma.so"}
: ${OZQUIET=-quiet}
OZINIT=${OZMAINIT}
export OZINIT

echo "Using OZMAFILE: $OZMAFILE"

exec $OZEMULATOR $OZQUIET $OZMA_LIB -b $OZMAFILE -a "$@"
