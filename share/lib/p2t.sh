#! /bin/sh
: ${BUILDTOP=../..}
: ${SRCDIR=.}
: ${SRCTOP=$SRCDIR/../..}
: ${OZPLATFORM=`$SRCTOP/share/bin/ozplatform`}
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

if test -z "$OZEMULATOR"
then
    echo Fatal error: oz.emulator.bin not found
    exit 1
fi

if test -z "$TEXT2PICKLE"
then
    for d in \
	$BUILDTOP/platform/emulator \
	$BUILDTOP/platform/emulator/$OZPLATFORM \
	$OZPREFIX/platform/$OZPLATFORM
    do
	if test -x $d/text2pickle
	then
	    TEXT2PICKLE=$d/text2pickle
	    break
	fi
    done
fi

if test -z "$TEXT2PICKLE"
then
    echo Fatal error: text2pickle not found
    exit 1
fi

exec $OZEMULATOR --pickle2text "$@" | $TEXT2PICKLE --textmode
