#!/bin/sh

: ${BUILDTOP=../..}
: ${SRCDIR=.}
: ${OZPLATFORM=`$BUILDTOP/share/bin/ozplatform`}
: ${OZPREFIX=/usr/local/oz}

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

echo "Using OZEMULATOR: $OZEMULATOR"

url=$1
shift

exec $OZEMULATOR -u $url -- $*
