#!/bin/sh

case `uname -s` in
    CYGWIN*)
	cygpath -w -s $1 | sed 's|\\|/|g'
	;;
    *)
	echo $1
	;;
esac
