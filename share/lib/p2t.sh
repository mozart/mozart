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
	if test -x $d/emulator.exe
	then
	    OZEMULATOR=$d/emulator.exe
	    break
	fi
    done
fi

if test -z "$OZEMULATOR"
then
    echo Fatal error: emulator.exe not found 1>&2
    exit 1
fi

if test -z "$TEXT2PICKLE"
then
    for d in \
	$BUILDTOP/platform/emulator \
	$BUILDTOP/platform/emulator/$OZPLATFORM \
	$OZPREFIX/platform/$OZPLATFORM
    do
	if test -x $d/text2pickle.exe
	then
	    TEXT2PICKLE=$d/text2pickle.exe
	    break
	fi
    done
fi

if test -z "$TEXT2PICKLE"
then
    echo Fatal error: text2pickle not found 1>&2
    exit 1
fi

if test "x$OZMAKE_VERBOSE" = xyes
then
    echo "Using OZEMULATOR:  $OZEMULATOR" 1>&2
    echo "Using TEXT2PICKLE: $TEXT2PICKLE" 1>&2
fi

exec $OZEMULATOR --pickle2text "$@" | $TEXT2PICKLE --textmode
