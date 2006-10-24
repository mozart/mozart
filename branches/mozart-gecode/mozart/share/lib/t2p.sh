#! /bin/sh
: ${BUILDTOP=../..}
: ${SRCDIR=.}
: ${SRCTOP=$SRCDIR/../..}
: ${OZPLATFORM=`$SRCTOP/share/bin/ozplatform`}
: ${OZPREFIX=/opt/mozart-1.1.0}

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
    echo Fatal error: text2pickle not found 2>&1
    exit 1
fi

if test "x$OZMAKE_VERBOSE" = xyes
then
    echo "Using TEXT2PICKLE: $TEXT2PICKLE" 1>&2
fi

exec $TEXT2PICKLE "$@"
