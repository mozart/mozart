#!/bin/sh

# windows step
MOZART=/src1/dragan/usr/mozart
#MOZART=/home/dragan/mozart
rm -rf /src1/dragan/usr/mozart-win/win-part;
mkdir /src1/dragan/usr/mozart-win/win-part;
cd /src1/dragan/usr/mozart-win/win-part;
PATH=/src1/dragan/cross-tools/bin:/src1/dragan/usr/mozart-win/install/bin:$PATH;
packageroot=/src1/dragan/build/packages/win32-i486;
export packageroot
LDFLAGS="-s"
CFLAGS="-O"
CXXFLAGS="-O"
windlldir=${packageroot}/dlls
with_lib_dir=${packageroot}/lib
with_inc_dir=${packageroot}/include
with_tcl=${packageroot}
with_tk=${packageroot}
export PATH packageroot LDFLAGS CFLAGS CXXFLAGS windlldir with_lib_dir with_inc_dir with_tcl with_tk;
${MOZART}/configure --target=i386-mingw32 --prefix=/src1/dragan/usr/mozart-win/install  --disable-contrib-micq --enable-opt=yes;

cd platform;
make bootstrap install;
cd ..;

cd contrib;
make all install;
cd ..;

make win32-i486;
