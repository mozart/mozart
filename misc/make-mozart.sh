#!/bin/sh -x
#
#  Authors:
#    Christian Schulte <schulte@ps.uni-sb.de>
#    Konstantin Popov <kost@sics.se>
#
#  Contributors:
#
#  Copyright:
#    Christian Schulte, 1998
#    Konstantin Popov, 2001
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

#PLAT=$1
#PREFIX=$2
PLAT=`ozplatform`
PREFIX=`pwd`

packageroot="$PREFIX/packages/$PLAT"
PATH=$packageroot/bin:$PATH
LD_LIBRARY_PATH=$packageroot/lib:$LD_LIBRARY_PATH

echo "Packages in: $packageroot"

use_src=../mozart
use_src-stdlib=../mozart-stdlib
build=$PREFIX/build-$PLAT
build-stdlib=$PREFIX/build-stdlib-$PLAT

case $PLAT in
linux-i486)
    LDFLAGS=-s
    moreargs="--enable-opt=yes --with-documents=all --enable-index --enable-chm"
    ;;

solaris-sparc)
    LDFLAGS=-s
    moreargs="--enable-opt=yes --disable-doc"
    ;;

freebsdelf-i486)
    LDFLAGS=-s
    moreargs="--enable-opt=yes --disable-doc"
    ;;

openbsd-sparc)
    LDFLAGS=-s
    moreargs="--enable-opt=yes --disable-doc "
    ;;

*)
    echo "Unknown platform: $PLAT" 2>& 1
    exit 1
    ;;
esac

CXXFLAGS="$CFLAGS"

with_lib_dir="$packageroot/lib"
with_inc_dir="$packageroot/include"
with_tcl="$packageroot/lib"
with_tclinclude="$packageroot/include"
with_tk="$packageroot/lib"
with_tkinclude="$packageroot/include"
with_gmp="$packageroot"
with_zlib="$packageroot"
with_gdbm="$packageroot"
with_regex="$packageroot"

export PATH LD_LIBRARY_PATH CFLAGS CXXFLAGS LDFLAGS
unset CONFIG_SITE
export with_lib_dir with_inc_dir
export with_tcl with_tclinclude with_tk with_tkinclude
export with_gmp with_zlib with_gdbm with_regex

set -x
mkdir -p install

# if libstdc++.a is not in $packageroot/lib, locate it and make a link
# to it into the $packageroot/lib
if [ ! -f $packageroot/lib/libstdc++.a ]; then
    found=""
    for e in `echo $LD_LIBRARY_PATH | tr ':' ' '` ; do
        if [ -f $e/libstdc++.a ]; then
            found=$e/libstdc++.a
            break
        fi
    done
    if [ -f "$found" ]; then
        ln -s $found $packageroot/lib/libstdc++.a
    fi
fi

#
cd $PREFIX
mkdir -p $build
cd $build
#
$use_src/configure $moreargs --with-stdlib=$use_src-stdlib --prefix=$PREFIX/install
#
make depend bootstrap install

# inspect the emulator.exe for being dynamically linked against
# libstdc++.so.5 and libgcc_s.so, and if so - copy the library(s) into
# install/platform/$PLAT/lib:
found=""
dynlib=`ldd $PREFIX/install/platform/$PLAT/emulator.exe | grep libgcc_s.so`
for e in $dynlib ; do
    match=`echo $e | grep "/.*libgcc_s.so"`
    if [ "$match" != "" ]; then
        found=$match
        break
    fi
done
if [ "$found" != "" ]; then
    lib=""
    for e in `echo $found | tr '/' ' '` ; do
        lib=$e
    done
    mkdir -p $PREFIX/install/platform/$PLAT/lib
    /bin/cp $found $PREFIX/install/platform/$PLAT/lib/$lib
    /bin/chmod a+x $PREFIX/install/platform/$PLAT/lib/$lib
    unset lib
fi
unset found
unset dynlib
found=""
dynlib=`ldd $PREFIX/install/platform/$PLAT/emulator.exe | grep "libstdc++.so"`
for e in $dynlib ; do
    match=`echo $e | grep "/.*libstdc++.so"`
    if [ "$match" != "" ]; then
        found=$match
        break
    fi
done
if [ "$found" != "" ]; then
    lib=""
    for e in `echo $found | tr '/' ' '` ; do
        lib=$e
    done
    mkdir -p $PREFIX/install/platform/$PLAT/lib
    /bin/cp $found $PREFIX/install/platform/$PLAT/lib/$lib
    /bin/chmod a+x $PREFIX/install/platform/$PLAT/lib/$lib
    unset lib
fi
unset found
unset dynlib

# compile&install stdlib
cd $PREFIX
mkdir $build-stdlib
cd $build-stdlib
#
$use_src-stdlib/configure --prefix=$PREFIX/install
#
make
make install

# src, doc & std are used by make-mozart-rpm.sh
cd $PREFIX
cd $build
(USER=root; make src doc std ${PLAT})
