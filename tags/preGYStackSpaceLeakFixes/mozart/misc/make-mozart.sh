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

echo "prefix: $PREFIX/$PLAT"

packageroot="$PREFIX/packages/$PLAT"
PATH=$packageroot/bin:$PATH

echo "Packages in: $packageroot"

use_src=../mozart
build=$PREFIX/build-$PLAT

case $PLAT in
    linux-i486)
        LDFLAGS=-s
	moreargs="--enable-opt=yes --with-documents=all --enable-index --enable-chm"
    ;;
    solaris-sparc)
        LDFLAGS=-s
	moreargs="--enable-opt=yes"
    ;;
    freebsdelf-i486)
        LDFLAGS=-s
	moreargs="--enable-opt=yes"
    ;;
    openbsd-sparc)
        LDFLAGS=-s
	moreargs="--enable-opt=yes"
    ;;
    win32-i486)
        LDFLAGS=-s
        windlldir="$packageroot/dlls"
	moreargs="--enable-opt=yes --target=i386-mingw32"
        use_src=$PREFIX/build-$PLAT/mozart
        build=$PREFIX/build-$PLAT/mozart
        export windlldir
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
with_tk="$packageroot/lib"
with_gmp="$packageroot"
with_zlib="$packageroot"
with_gdbm="$packageroot"
with_regex="$packageroot"

# echo $CFLAGS $CXXFLAGS $LDFLAGS
# echo $with_lib_dir $with_inc_dir $with_tcl $with_tk
export PATH CFLAGS CXXFLAGS LDFLAGS
unset CONFIG_SITE 
export with_lib_dir with_inc_dir
export with_tcl with_tk with_gmp with_zlib with_gdbm with_regex

set -x
mkdir -p install
mkdir $build
cd $build
#
$use_src/configure $moreargs --prefix=$PREFIX/install
#
make depend bootstrap install
# src & doc are used by make-mozart-rpm.sh
make src doc ${PLAT}
