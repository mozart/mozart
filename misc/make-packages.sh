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
#    Konstantin Popov, 2001, 2004
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
# -r--r--r--    2 kost      14269432 Mar  1 00:44 binutils-2.14.tar.gz
# -r--r--r--    2 kost      31242089 Oct 20 15:21 gcc-3.3.2.tar.gz
# -r--r--r--    2 kost      18115189 Mar  1 00:47 glibc-2.3.2.tar.gz
#These 3 packages should probably be updated (not tested while building on linux-i486)
# -rw-r--r--  1 yjaradin stafinfo 16378360 2006-05-31 11:44 binutils-2.16.1.tar.gz
# -rw-r--r--  1 yjaradin stafinfo  1384574 2006-05-31 11:48 bison-2.2.tar.gz
# -rw-r--r--  1 yjaradin stafinfo   380995 2006-05-31 11:49 flex-2.5.4a.tar.gz
# -rw-r--r--  1 yjaradin stafinfo   228695 2006-05-31 11:51 gdbm-1.8.3.tar.gz
# -rw-r--r--  1 yjaradin stafinfo  2249464 2006-05-31 11:52 gmp-4.2.1.tar.gz
# -rw-r--r--  1 yjaradin stafinfo   384582 2006-05-31 11:52 m4-1.4.4.tar.gz
# -rw-r--r--  1 yjaradin stafinfo 12829188 2006-05-31 11:55 perl-5.8.8.tar.gz
# -rw-r--r--  1 yjaradin stafinfo   297790 2006-05-31 11:55 regex-0.12.tar.gz
# -rw-r--r--  1 yjaradin stafinfo  3513852 2006-05-31 11:57 tcl8.4.13-src.tar.gz
# -rw-r--r--  1 yjaradin stafinfo  3253070 2006-05-31 11:57 tk8.4.13-src.tar.gz
# -rw-r--r--  1 yjaradin stafinfo   496597 2006-05-31 12:09 zlib-1.2.3.tar.gz

#PLAT=$1
#BASE=$2
PLAT=`ozplatform`
BASE=`pwd`

CPPFLAGS=
CFLAGS=
CXXFLAGS=
LDFLAGS=
LD_LIBRARY_PATH=
export CPPFLAGS CFLAGS CXXFLAGS LDFLAGS LD_LIBRARY_PATH

PREFIX=$BASE/packages/$PLAT

case $PLAT in
    linux-i486)
         # Requires gcc 3.3+, as stated
         # Building compiler from scratch is not supported
         # since in general one must also have the right
         # binutils (at least 2.12.1) and glibc (at least 2.2.5)
         # Use the 'toolchain.csh' script if in dire straights..
         CFLAGS="-O3 -pipe -fomit-frame-pointer -march=pentium -mcpu=pentiumpro -fno-tracer -static-libgcc"
         CXXFLAGS="$CFLAGS"
         HOST="--host=i586-*-linux"
         BUILD="--build=i586-*-linux"
         ;;

    solaris-sparc)
         GCC=gcc-3.3.2
         zcat $GCC.tar.gz | tar xf -
         mkdir $GCC-build
         (cd $GCC-build; \
          ../$GCC/configure --prefix=$PREFIX \
                  --enable-languages=c,c++ --disable-nls --disable-multilib; \
          make bootstrap; \
          make install)
         rm -rf $GCC-build $GCC
         CFLAGS="-O3 -pipe -mcpu=v8 -fdelayed-branch -fno-tracer -static-libgcc"
         CXXFLAGS="$CFLAGS"
         # fix v8 for the time being;
         BUILD="--build=supersparc-sun-solaris"
         ;;

    openbsd-sparc)
         # not verified;
         CFLAGS="-O3 -pipe -mcpu=v8 -fdelayed-branch -fno-tracer -static-libgcc"
         CXXFLAGS="$CFLAGS"
         # fix v8 for the time being;
         BUILD="--build=supersparc-sun-solaris"
         ;;

    netbsd-sparc)
         # not verified;
         CFLAGS="-O3 -pipe -mcpu=v8 -fdelayed-branch -fno-tracer -static-libgcc"
         CXXFLAGS="$CFLAGS"
         # fix v8 for the time being;
         BUILD="--build=sparcv8-sun-solaris"
         ;;

    freebsdelf-i486)
         # not verified;
         CFLAGS="-O3 -pipe -fomit-frame-pointer -march=pentium -mcpu=pentiumpro -fno-tracer -static-libgcc"
         CXXFLAGS="$CFLAGS"
         BUILD="--build=i586-*-linux"
         ;;

    *)
         echo "Unknown platform: $plat" 2>& 1
         exit 1
         ;;
esac

PATH=$PREFIX/bin:$PATH
LD_LIBRARY_PATH=$PREFIX/lib:$LD_LIBRARY_PATH
export CFLAGS CXXFLAGS PATH LD_LIBRARY_PATH

BISON=bison-2.2
FLEX=flex-2.5.4a
GDBM=gdbm-1.8.3
GMP=gmp-4.2.1
M4=m4-1.4.4
PERL=perl-5.8.8
REGEX=regex-0.12
TCL=tcl8.4.13
TK=tk8.4.13
ZLIB=zlib-1.2.3
FLEXDIR=flex-2.5.4

zcat $GDBM.tar.gz | tar xf -
(cd $GDBM; \
 ./configure --disable-shared $HOST $BUILD; \
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
 cp libtool tmplt; \
 sed -e '1,$s/CC -shared/CC -shared -static-libgcc/g' < tmplt > libtool; \
 rm tmplt; \
 ./configure --prefix=$PREFIX --disable-shared $BUILD; \
 make CFLAGS="$CFLAGS"; \
 make install )

zcat $REGEX.tar.gz | tar xf -
(cd $REGEX; \
 ./configure --prefix=$PREFIX; \
 make CFLAGS="$CFLAGS"; \
 make install )

zcat $TCL-src.tar.gz | tar xf -
(cd $TCL/unix; \
 ./configure --prefix=$PREFIX --disable-shared --enable-gcc; \
 make CFLAGS="$CFLAGS"; \
 make install )

zcat $TK-src.tar.gz | tar xf -
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
