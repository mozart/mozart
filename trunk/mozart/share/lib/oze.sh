#!/bin/sh

: ${BUILDTOP=../..}
: ${SRCDIR=.}
: ${OZPLATFORM=`$SRCDIR/../bin/ozplatform`}
: ${OZPREFIX=/usr/local/oz}
: ${OZINIT="$BUILDTOP/share/lib/Init.ozf"}

case $OZPLATFORM in
    win32*)
	exe=emulator.dll
	;;
    *)
	exe=emulator.exe
	;;
esac

if test -z "$OZEMULATOR"
then
    for d in \
	$BUILDTOP/platform/emulator \
	$OZPREFIX/platform/$OZPLATFORM
    do
	if test -x $d/$exe
	then
	    OZEMULATOR=$d/$exe
	    break
	fi
    done
fi

if test "x$OZMAKE_VERBOSE" = xyes
then
    echo "Using OZEMULATOR: $OZEMULATOR"
fi

url=$1
shift

case $OZPLATFORM in
    win32*)
	OZEMULATOR=`cygpath -w $OZEMULATOR`
	OZINIT=`cygpath -w $OZINIT`
	export OZEMULATOR OZINIT
	exec $BUILDTOP/platform/mswindows/ozengine $url "$@"
	;;
    *)
	exec $OZEMULATOR -init $OZINIT -u $url -- "$@"
	;;
esac
