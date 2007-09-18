#!/bin/sh

# uncomment and adapt the following line if you want
# to draw links from say /usr/local/bin to OZHOME/bin
# see chapter "installation" in the users manual for more information

CD="CocoaDialog.app/Contents/MacOS/CocoaDialog"

OZHOME=$1/Contents/Resources/

######################################################################
# you should not edit below this line

# where Oz resides:


export OZHOME

GEC_LIBS="$OZHOME/lib"
EXT_LIBS="$OZHOME/ext_libs"
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


##
# TODO: Test for aquamacs or carbon emacs.
#if test -e ! "/Applications/Aquamacs\ Emacs.app/Contents/MacOS/Aquamacs\ Emacs"
#then
#    rv=`$1/Contents/Resources/$CD yesno-msgbox --string-output`
#    $1/Contents/Resources/$CD ok-msgbox --no-cancel --text "You pressed $rv"
#fi

exec /Applications/Aquamacs\ Emacs.app/Contents/MacOS/Aquamacs\ Emacs --eval '(setq load-path (cons "'$OZHOME'/share/elisp" load-path))' -l oz.elc -f run-oz
