#!/bin/sh
#
# use this for bootstrapping the system:
#
#    make OZBATCH=./ozbatch.sh
#

if test -z "$OZ_EMULATOR_DIR"
then
    for d in ../Emulator ../Emulator/`platform` \
            /usr/local/oz/platform/`platform`
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
: ${SRCDIR=.}
OZINIT=${OZMAINIT}
export OZINIT

exec $OZEMULATOR $OZQUIET $OZMA_LIB -b $SRCDIR/ozbatch.ozm -a "$@"
