#!/bin/bash

OZHOME=$1/Contents/Resources
CD="$1/Contents/Resources/CocoaDialog.app/Contents/MacOS/CocoaDialog"
#CD="Mozart.app/Contents/Resources/CocoaDialog.app/Contents/MacOS/CocoaDialog"


detectAquamacs()
{
    if [ ! -e "/Applications/Aquamacs Emacs.app" ]; then
	`$CD ok-msgbox --no-cancel --icon info --timeout 20 --title "Aquamacs not found" --text "This application is needed to run Mozart" --informative-text "You can download it from http://aquamacs.org/"`
	exit 2
    fi
}

detectAquamacs


######################################################################
# you should not edit below this line

# where Oz resides:


export OZHOME

GEC_LIBS="$OZHOME/Framworks"
EXT_LIBS="$OZHOME/Framworks"
: ${OZPLATFORM=`"$OZHOME/bin/ozplatform"`}
: ${OZEMULATOR="$OZHOME/platform/$OZPLATFORM/emulator.exe"}
: ${OZVERSION="1.4.0"}
: ${OZ_DOTOZ="$HOME/.oz/$OZVERSION"}
: ${OZ_LD_LIBRARY_PATH="$OZ_DOTOZ/platform/$OZPLATFORM/lib:$OZHOME/platform/$OZPLATFORM/lib${LD_LIBRARY_PATH:+:}${LD_LIBRARY_PATH}"}
: ${OZ_DYLD_LIBRARY_PATH="$OZ_DOTOZ/platform/$OZPLATFORM/lib:$OZHOME/platform/$OZPLATFORM/lib${DYLD_LIBRARY_PATH:+:}${DYLD_LIBRARY_PATH}:$GEC_LIBS:EXT_LIBS"}


LD_LIBRARY_PATH="$OZ_LD_LIBRARY_PATH"
export LD_LIBRARY_PATH
DYLD_LIBRARY_PATH="$OZ_DYLD_LIBRARY_PATH"
export DYLD_LIBRARY_PATH

# set OZPATH & PATH
if test -z "${OZ_PI}"
then
  # where Oz searches for files:
  if test -z "${OZPATH}"
  then
     OZPATH=.
  fi
  OZPATH="${OZPATH}:${OZHOME}/share"
  export OZPATH
  # increment path
  PATH="${OZHOME}/bin:${PATH}"
  export PATH
  OZ_PI=1
  export OZ_PI
fi



exec /Applications/Aquamacs\ Emacs.app/Contents/MacOS/Aquamacs\ Emacs --eval '(setq load-path (cons "'$OZHOME'/share/elisp" load-path))' -l oz.elc -f run-oz $2 &

