#!/bin/sh
#
#  Authors:
#    Raphael Collet <raphael.collet@uclouvain.be>
#
#  Contributors:
#    Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
#
#  Copyright:
#    Raphael Collet, 2008
#
#  Last change:
#    $Date$ by $Author$
#    $Revision$
# 
#  This file is part of Mozart, an implementation 
#  of Oz 3:
#     http://www.mozart-oz.org
# 
#  See the file "LICENSE" or
#     http://www.mozart-oz.org/LICENSE.html
#  for information on usage and redistribution 
#  of this file, and for a DISCLAIMER OF ALL 
#  WARRANTIES.
#


# This script copies the libraries used by emulator.exe into its
# application bundle, and modifies the references to these libraries
# inside emulator.exe accordingly.

# path to the emulator
EMULATOR=$1

# path to the bundle
BUNDLE=$2

# relative path to the frameworks inside the bundle
RELFWKS=@executable_path/../../../Frameworks

# paths of imported shared libraries that do not match the patterns
# shown below.  Add more grep -v's if necessary.
BUNDLELIBS=`otool -LX $EMULATOR | sed 's/(.*$//' |
    grep -v /usr/lib/libgcc |
    grep -v /usr/lib/libstdc\+\+ |
    grep -v /usr/lib/libz |
    grep -v /System/Library/Frameworks/Tk.framework/Versions/8.4/Tk |
    grep -v /usr/lib/libSystem`

# copy libraries in the bundle, and patch the emulator
for FILE in $BUNDLELIBS; do
    NAME=`basename $FILE`
    cp $FILE $BUNDLE/Contents/Frameworks/
    install_name_tool -change $FILE $RELFWKS/$NAME $EMULATOR
done

# done!
