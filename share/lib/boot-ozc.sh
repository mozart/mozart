#! /bin/sh
#
: ${BUILDTOP=../..}
: ${SRCDIR=.}
: ${SRCTOP=$SRCDIR/../..}
: ${OZPLATFORM=`$SRCTOP/share/bin/ozplatform`}
: ${OZPREFIX=/usr/local/oz}
: ${OZBOOTINIT="$BUILDTOP/share/lib/boot-init"}
: ${OZBOOTOZC="$BUILDTOP/share/lib/boot-ozc"}

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
    echo Fatal error: oz.emulator.bin not found
    exit 1
fi

if test "x$OZMAKE_VERBOSE" = xyes
then
    echo "Using OZEMULATOR: $OZEMULATOR"
    echo "Using OZBOOTINIT: $OZBOOTINIT"
    echo "Using OZBOOTOZC:  $OZBOOTOZC"
fi

exec $OZEMULATOR -init $OZBOOTINIT -u $OZBOOTOZC -- "$@"
