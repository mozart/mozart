#!/bin/sh

if test -z "${OZHOME}"
then
    OZHOME=/usr/local/oz
fi
export OZHOME

exec emacs --batch \
-L $OZHOME/share/elisp -l oz.elc \
-L `dirname $0` -l ozdoc.elc \
-f ozdoc-fontify $*
