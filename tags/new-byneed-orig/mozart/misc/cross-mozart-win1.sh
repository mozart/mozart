#!/bin/sh

# Set right path
# for now you have to log on costello.sics.se
#export PATH=/home/dragan/gcc-2.95/bin:/src1/dragan/cross-tools/bin:$PATH;
#CFLAGS= "-D__restrict=-O3 -fstrict-aliasing -march=i586 -mcpu=i686";
MOZART=/src1/dragan/usr/mozart
#PATH=/home/simon/usr/bin/$PATH
#MOZART=/home/dragan/mozart
CFLAGS="-D__restrict=  -O3 -fstrict-aliasing -march=i586 -mcpu=i686"
CXXFLAGS="-D__restrict=  -O3 -fstrict-aliasing -march=i586 -mcpu=i686"
export CFLAGS CXXFLAGS;

rm -rf /src1/dragan/usr/mozart-win;
mkdir /src1/dragan/usr/mozart-win;
mkdir /src1/dragan/usr/mozart-win/linux-part;
mkdir /src1/dragan/usr/mozart-win/win-part;
mkdir /src1/dragan/usr/mozart-win/install;

cd /src1/dragan/usr/mozart-win/linux-part;
${MOZART}/configure --prefix=/src1/dragan/usr/mozart-win/install --enable-opt=yes --with-documents=all --enable-chm --enable-index;
make bootstrap install;
