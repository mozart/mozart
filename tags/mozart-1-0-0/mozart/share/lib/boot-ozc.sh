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
	if test -x $d/emulator.exe
	then
	    OZEMULATOR=$d/emulator.exe
	    break
	fi
    done
fi

if test -z "$OZEMULATOR"
then
    echo Fatal error: emulator.exe not found
    exit 1
fi

if test "x$OZMAKE_VERBOSE" = xyes
then
    echo "Using OZEMULATOR: $OZEMULATOR"
    echo "Using OZBOOTINIT: $OZBOOTINIT"
    echo "Using OZBOOTOZC:  $OZBOOTOZC"
fi

exec $OZEMULATOR -init $OZBOOTINIT -u $OZBOOTOZC -- "$@"
