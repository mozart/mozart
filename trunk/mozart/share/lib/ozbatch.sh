#!/bin/sh
#
# use this for bootstrapping the system:
#
#    make OZBATCH=./ozbatch.sh
#

: ${OZEMULATOR=$HOME/Oz/Emulator/oz.emulator.bin}

exec $OZEMULATOR -quiet -b ./ozbatch.ozm -a "$@"
