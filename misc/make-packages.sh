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

# The list of packages (that have to be in the directory where
# 'make-packages.sh' is called):
#
# kost (534) ll *gz
# -r--r--r--    1 kost     sics      7284401 Feb 14 19:06 binutils-2.10.1.tar.gz
# -r--r--r--    1 kost     sics     12973458 Feb 14 19:06 gcc-2.95.2.tar.gz
# -r--r--r--    1 kost     sics       134080 Feb 14 19:06 gdbm-1.8.0.tar.gz
# -r--r--r--    1 kost     sics      1033780 Feb 14 19:06 gmp-3.1.1.tar.gz
# -r--r--r--    1 kost     sics       297790 Feb 14 19:06 regex-0.12.tar.gz
# -r--r--r--    1 kost     sics      2502194 Feb 14 19:06 tcl8.2.3.tar.gz
# -r--r--r--    1 kost     sics      2336472 Feb 14 19:06 tk8.2.3.tar.gz
# -r--r--r--    1 kost     sics       168463 Feb 14 19:06 zlib-1.1.3.tar.gz
# -rw-r--r--    1 kost     sics      3679040 Feb 14 19:06 perl5.005_03.tar.gz
# -rw-r--r--    1 kost     sics       317588 Feb 14 19:06 m4-1.4.tar.gz

#PLAT=$1
#BASE=$2
PLAT=`ozplatform`
BASE=`pwd`

PREFIX=$BASE/packages/$PLAT

case $PLAT in
    linux-i486)
	 CFLAGS=
	 CXXFLAGS=
	 GCC=gcc-2.95.2
	 BINUTILS=binutils-2.10.1
	 zcat < $GCC.tar.gz | tar xf -
	 cd $GCC
	 zcat < ../$BINUTILS.tar.gz | tar xf -
	 mv $BINUTILS binutils
	 cd ..
	 mkdir $GCC-build
	 (cd $GCC-build; \
	  ../$GCC/configure --prefix=$PREFIX --enable-languages=c,c++; \
	  make MAKE="make" CFLAGS="$CFLAGS" LIBCFLAGS="$CFLAGS" \
	       LIBCXXFLAGS="$CXXFLAGS" bootstrap; \
	  make install)
	 rm -rf $GCC-build $GCC
	 CFLAGS="-O3 -fomit-frame-pointer -mcpu=pentium"
	 CXXFLAGS="$CFLAGS"
         GMP_TARGET=
         ;;

    solaris-sparc)
	 CFLAGS=
	 CXXFLAGS=
	 GCC=gcc-2.95.2
	 zcat < $GCC.tar.gz | tar xf -
	 mkdir $GCC-build
	 (cd $GCC-build; \
	  ../$GCC/configure --prefix=$PREFIX; \
	  make CFLAGS="$CFLAGS" LIBCFLAGS="$CFLAGS" \
	       LIBCXXFLAGS="$CXXFLAGS" bootstrap; \
	  make install)
	 rm -rf $GCC-build $GCC
	 CFLAGS="-O3 -mcpu=v8 -fdelayed-branch"
	 CXXFLAGS="$CFLAGS"
	# fix v8 for the time being; 
         GMP_TARGET="--target=sparcv8-sun-solaris2.7"
         ;;

    netbsd-sparc)
	 CFLAGS=
	 CXXFLAGS=
	 GCC=gcc-2.95.2
	 # zcat < $GCC.tar.gz | tar xf -
	 mkdir $GCC-build
	 (cd $GCC-build; \
	  ../$GCC/configure --prefix="$PREFIX"; \
	  make CFLAGS="$CFLAGS" LIBCFLAGS="$CFLAGS" \
	       LIBCXXFLAGS="$CXXFLAGS" bootstrap; \
	  make install)
	 rm -rf $GCC-build
	 CFLAGS="-O3 -mcpu=v8 -fdelayed-branch"
	 CXXFLAGS="$CFLAGS"
         GMP_TARGET=
         ;;

    freebsdelf-i486)
	 CFLAGS=
	 CXXFLAGS=
	 GCC=gcc-2.95.2
	 zcat < $GCC.tar.gz | tar xf -
	 mkdir $GCC-build
	 (cd $GCC-build; \
	  ../$GCC/configure --prefix="$PREFIX" --enable-languages=c,c++; \
	  make CFLAGS="$CFLAGS" LIBCFLAGS="$CFLAGS" \
	       LIBCXXFLAGS="$CXXFLAGS" bootstrap; \
	  make install)
	 rm -rf $GCC-build
	 CFLAGS="-O3 -fomit-frame-pointer -mcpu=pentium"
	 CXXFLAGS="$CFLAGS"
         GMP_TARGET=
         ;;

    *)
	 echo "Unknown platform: $plat" 2>& 1
	 exit 1
         ;;
esac

PATH=$PREFIX/bin:$PATH
export CFLAGS CXXFLAGS PATH

GDBM=gdbm-1.8.0
ZLIB=zlib-1.1.3
GMP=gmp-3.1.1
REGEX=regex-0.12
TCL=tcl8.2.3
TK=tk8.2.3
PERL=perl5.005_03
M4=m4-1.4

zcat < $GDBM.tar.gz | tar xf -
(cd $GDBM; \
 ./configure --disable-shared; \
 make CFLAGS="$CFLAGS -fpic"; \
 make prefix=$PREFIX install )

zcat < $ZLIB.tar.gz | tar xf -
(cd $ZLIB; \
 ./configure --prefix=$PREFIX; \
 make CFLAGS="$CFLAGS"; \
 make install )

zcat < $GMP.tar.gz | tar xf -
(cd $GMP; \
 ./configure --prefix=$PREFIX $GMP_TARGET; \
 make CFLAGS="$CFLAGS"; \
 make install )

zcat < $REGEX.tar.gz | tar xf -
(cd $GMP; \
 ./configure --prefix=$PREFIX; \
 make CFLAGS="$CFLAGS"; \
 make install )

zcat < $TCL.tar.gz | tar xf -
(cd $TCL/unix; \
 ./configure --prefix=$PREFIX --disable-shared --enable-gcc; \
 make CFLAGS="$CFLAGS"; \
 make install )

zcat < $TK.tar.gz | tar xf -
(cd $TK/unix; \
 ./configure --prefix=$PREFIX --disable-shared --enable-gcc; \
 make CFLAGS="$CFLAGS"; \
 make install )

zcat < $PERL.tar.gz | tar xf -
(cd $PERL; \
 S_CFLAGS=$CFLAGS \
 S_CXXFLAGS=$CXXFLAGS \
 CFLAGS= \
 CXXFLAGS= \
 ./configure.gnu --prefix=$PREFIX; \
 make; \
 make install; \
 CFLAGS=$S_CFLAGS \
 CXXFLAGS=$S_CXXFLAGS )

zcat < $M4.tar.gz | tar xf -
(cd $M4; \
 ./configure --prefix=$PREFIX; \
 make CFLAGS="$CFLAGS"; \
 make install )

# no dynamic libraries: we want that part static;
rm -f $PREFIX/lib/*.so*
rm -rf $GDBM $ZLIB $GMP $REGEX $TCL $TK $PERL $M4
