#! /bin/sh
#
: ${BUILDTOP=../..}
: ${SRCDIR=.}
: ${SRCTOP=$SRCDIR/../..}
: ${OZPLATFORM=`$SRCTOP/share/bin/ozplatform`}
: ${OZPREFIX=/usr/local/oz}
: ${OZBOOTINIT="$BUILDTOP/share/lib/boot-init"}
: ${OZBOOTOZC="$BUILDTOP/share/lib/boot-ozc"}

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
	$BUILDTOP/platform/emulator/$OZPLATFORM \
	$OZPREFIX/platform/$OZPLATFORM
    do
	if test -x $d/$exe
	then
	    OZEMULATOR=$d/$exe
	    break
	fi
    done
fi

if test -z "$OZEMULATOR"
then
    echo Fatal error: $exe not found
    exit 1
fi

if test "x$OZMAKE_VERBOSE" = xyes
then
    echo "Using OZEMULATOR: $OZEMULATOR"
    echo "Using OZBOOTINIT: $OZBOOTINIT"
    echo "Using OZBOOTOZC:  $OZBOOTOZC"
fi

case $OZPLATFORM in
    win32*)
	OZEMULATOR=`cygpath -w $OZEMULATOR`
	OZINIT=`cygpath -w $OZBOOTINIT`
	export OZEMULATOR OZINIT
	exec $BUILDTOP/platform/mswindows/ozengine `cygpath -w $OZBOOTOZC` "$@"
	;;
    *)
	exec $OZEMULATOR -init $OZBOOTINIT -u $OZBOOTOZC -- "$@"
	;;
esac
