#!/bin/sh
#
# use this for bootstrapping the system:
#
#    make OZBATCH=./ozbatch.sh
#

: ${OZEMULATOR=$HOME/Oz/Emulator/oz.emulator.bin}
: ${OZQUIET=-quiet}
: ${SRCDIR=.}
OZINIT=${OZMAINIT}
export OZINIT

exec $OZEMULATOR $OZQUIET -b $SRCDIR/ozbatch.ozm -a "$@"
