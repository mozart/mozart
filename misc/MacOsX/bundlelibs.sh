#!/bin/sh

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
