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
# kost (1539) ll *gz
# -r--r--r--    1 kost     dsl       7284401 May 10 14:42 binutils-2.10.1.tar.gz
# -r--r--r--    1 kost     dsl        420341 May 11 12:00 bison-1.28.tar.gz
# -r--r--r--    1 kost     dsl        380995 May 11 12:06 flex-2.5.4a.tar.gz
# -r--r--r--    1 kost     dsl      12911721 May 10 14:42 gcc-2.95.3.tar.gz
# -r--r--r--    1 kost     dsl        134080 May 10 14:42 gdbm-1.8.0.tar.gz
# -r--r--r--    1 kost     dsl       1033780 May 10 14:42 gmp-3.1.1.tar.gz
# -r--r--r--    1 kost     dsl        317588 May 10 14:42 m4-1.4.tar.gz
# -r--r--r--    1 kost     dsl       3679040 May 10 14:42 perl5.005_03.tar.gz
# -r--r--r--    1 kost     dsl        297790 May 10 14:42 regex-0.12.tar.gz
# -r--r--r--    1 kost     dsl       2502194 May 10 14:42 tcl8.2.3.tar.gz
# -r--r--r--    1 kost     dsl       2336472 May 10 14:42 tk8.2.3.tar.gz
# -r--r--r--    1 kost     dsl        168463 May 10 14:42 zlib-1.1.3.tar.gz

#PLAT=$1
#BASE=$2
PLAT=`ozplatform`
BASE=`pwd`

PREFIX=$BASE/packages/$PLAT

case $PLAT in
    linux-i486)
	 CFLAGS=
	 CXXFLAGS=
	 GCC=gcc-2.95.3
	 BINUTILS=binutils-2.10.1
	 zcat $GCC.tar.gz | tar xf -
	 cd $GCC
	 zcat ../$BINUTILS.tar.gz | tar xf -
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
	 GCC=gcc-2.95.3
	 zcat $GCC.tar.gz | tar xf -
	 mkdir $GCC-build
	 (cd $GCC-build; \
	  ../$GCC/configure --prefix=$PREFIX --enable-languages=c,c++; \
	  make CFLAGS="$CFLAGS" LIBCFLAGS="$CFLAGS" \
	       LIBCXXFLAGS="$CXXFLAGS" bootstrap; \
	  make install)
	 rm -rf $GCC-build $GCC
	 CFLAGS="-O3 -mcpu=v8 -fdelayed-branch"
	 CXXFLAGS="$CFLAGS"
	# fix v8 for the time being; 
         GMP_TARGET="--target=sparcv8-sun-solaris2.7"
         ;;

    openbsd-sparc)
	 CFLAGS=
	 CXXFLAGS=
	 GCC=gcc-2.95.3
	 zcat $GCC.tar.gz | tar xf -
	 mkdir $GCC-build
	 (cd $GCC-build; \
	  ../$GCC/configure --prefix=$PREFIX --enable-languages=c,c++; \
	  make CFLAGS="$CFLAGS" LIBCFLAGS="$CFLAGS" \
	       LIBCXXFLAGS="$CXXFLAGS" bootstrap; \
	  make install)
	 rm -rf $GCC-build
	 CFLAGS="-O3 -mcpu=v8 -fdelayed-branch"
	 CXXFLAGS="$CFLAGS"
         GMP_TARGET=
         ;;

    netbsd-sparc)
	 CFLAGS=
	 CXXFLAGS=
	 GCC=gcc-2.95.3
	 zcat $GCC.tar.gz | tar xf -
	 mkdir $GCC-build
	 (cd $GCC-build; \
	  ../$GCC/configure --prefix=$PREFIX --enable-languages=c,c++; \
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
	 GCC=gcc-2.95.3
	 zcat $GCC.tar.gz | tar xf -
	 mkdir $GCC-build
	 (cd $GCC-build; \
	  ../$GCC/configure --prefix=$PREFIX --enable-languages=c,c++; \
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
FLEX=flex-2.5.4a
FLEXDIR=flex-2.5.4
BISON=bison-1.28

zcat $GDBM.tar.gz | tar xf -
(cd $GDBM; \
 ./configure --disable-shared; \
 mv Makefile Makefile.orig; \
 sed -e '1,$s/-o $(BINOWN) -g $(BINGRP) //g' < Makefile.orig > Makefile; \
 make CFLAGS="$CFLAGS -fpic"; \
 make prefix=$PREFIX install )

zcat $ZLIB.tar.gz | tar xf -
(cd $ZLIB; \
 ./configure --prefix=$PREFIX; \
 make CFLAGS="$CFLAGS"; \
 make install )

zcat $GMP.tar.gz | tar xf -
(cd $GMP; \
 ./configure --prefix=$PREFIX $GMP_TARGET; \
 make CFLAGS="$CFLAGS"; \
 make install )

zcat $REGEX.tar.gz | tar xf -
(cd $REGEX; \
 ./configure --prefix=$PREFIX; \
 make CFLAGS="$CFLAGS"; \
 make install )

zcat $TCL.tar.gz | tar xf -
(cd $TCL/unix; \
 ./configure --prefix=$PREFIX --disable-shared --enable-gcc; \
 make CFLAGS="$CFLAGS"; \
 make install )

zcat $TK.tar.gz | tar xf -
(cd $TK/unix; \
 ./configure --prefix=$PREFIX --disable-shared --enable-gcc; \
 make CFLAGS="$CFLAGS"; \
 make install )

zcat $PERL.tar.gz | tar xf -
(cd $PERL; \
 S_CFLAGS=$CFLAGS \
 S_CXXFLAGS=$CXXFLAGS \
 S_CC=$CC \
 CFLAGS= \
 CXXFLAGS= \
 CC=gcc \
 export CFLAGS CXXFLAGS CC ; \
 ./configure.gnu --prefix=$PREFIX; \
 make; \
 make install; \
 CFLAGS=$S_CFLAGS \
 CXXFLAGS=$S_CXXFLAGS \
 CC=$S_CC \
 export CFLAGS CXXFLAGS CC )

zcat $M4.tar.gz | tar xf -
(cd $M4; \
 ./configure --prefix=$PREFIX; \
 make CFLAGS="$CFLAGS"; \
 make install )

zcat $BISON.tar.gz | tar xf -
(cd $BISON; \
 ./configure --prefix=$PREFIX; \
 make CFLAGS="$CFLAGS"; \
 make install )

zcat $FLEX.tar.gz | tar xf -
(cd $FLEXDIR; \
 ./configure --prefix=$PREFIX; \
 make CFLAGS="$CFLAGS"; \
 make install )

# no dynamic libraries: we want that part static;
rm -f $PREFIX/lib/*.so*
rm -rf $GDBM $ZLIB $GMP $REGEX $TCL $TK $PERL $M4 $FLEXDIR $BISON
