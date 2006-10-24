#!/bin/csh -v
# 
#  Authors:
#    Konstantin Popov <kost@sics.se>
#
#  Contributors:
#
#  Copyright:
#    Konstantin Popov, 2004
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

# This file is ONLY necessary for building the mozart system on an
# older linux installation - the one that does NOT have the properly
# installed gcc 3.3+ together with required for it binutils-2.12.1 and
# glibc-2.2.5. It can be useful if the host system toolchain is broken
# for some other reason. This is a working [as,ld,libc*,gcc] toolchain
# bootstrapping procedure (there maybe some syntactic problems with
# the script as a whole though - I did not run entirely at once, but
# piecewise - but in THE sequence!)

# Building binutils requires the gcc3.2+ on its own, even if it is
# broken (i.e. installed over wrong versions of binutils/glibc, and
# therefore having problems with dynamic linking and C++ ABI). So make
# sure there one (binutils 'configure' will complain anyway)
#setenv PATH /usr/local/gcc-3.3.2/bin:${PATH}

unsetenv CPPFLAGS
unsetenv CFLAGS
unsetenv CXXFLAGS
unsetenv LDFLAGS
unsetenv LD_LIBRARY_PATH

setenv PREFIX /tmp/kost/install
setenv PATH ${PREFIX}/bin:${PATH}

#setenv CPPFLAGS "-I${PREFIX}/include"
#setenv LDFLAGS "-L${PREFIX}/lib"
#setenv LD_LIBRARY_PATH ${PREFIX}/lib


###
mkdir -p ${PREFIX}
# make sure it does not contain any wrong stuff. Best - purge it:
# rm -rf ${PREFIX}/*


# binutils come first (glibc & gcc check its features) 
cd /tmp/kost
zcat ~/compile/build-1.3.0/binutils-2.14.tar.gz | tar xf -
mkdir build-binutils
cd build-binutils
../binutils-2.14/configure --prefix=${PREFIX} --disable-nls >& conf.log
# tail -f conf.log
make configure-host >& pre-build.log ; make LDFLAGS="-all-static" >& build.log ; make install >& install.log
# tail -f pre-build.log
# tail -f build.log
# tail -f check.log
# tail -f install.log
make -C ld clean
make -C ld LDFLAGS="-all-static" LIB_PATH=${PREFIX}/lib >& build2.log
# tail -f build2.log
cd /tmp/kost
mv build-binutils build-binutils-stage1


# this compiler is built using the host's includes and libraries
# (also at run-time)
cd /tmp/kost
zcat ~/compile/build-1.3.0/gcc-3.3.2.tar.gz | tar xf -
mkdir build-gcc
cd build-gcc
../gcc-3.3.2/configure --prefix=${PREFIX} --enable-languages=c --enable-shared --enable-clocale=gnu --disable-nls --with-as=${PREFIX}/bin/as --with-ld=${PREFIX}/bin/ld --with-local-prefix=${PREFIX} >& conf.log
# NO: CFLAGS=${CFLAGS} LIBCFLAGS=${CFLAGS} LIBCXXFLAGS=${CFLAGS}
# NO: CPPFLAGS=${CPPFLAGS} HDEFINES="-I${PREFIX}/include" LDFLAGS=${LDFLAGS} 
# all these flags mess up with the environement ..
make BOOT_LDFLAGS="-static" bootstrap >& build.log ; make install >& install.log
# tail -f build.log
# tail -f install.log
cd /tmp/kost
mv build-gcc build-gcc-stage1


# kernel archives
setenv LINUX linux-2.4.22
zcat /usr/src/${LINUX}.tar.gz | tar xfv -
cd ${LINUX}
make mrproper
make include/linux/version.h
make symlinks
mkdir ${PREFIX}/include/asm
cp include/asm/* ${PREFIX}/include/asm
cp -R include/asm-generic ${PREFIX}/include
cp -R include/linux ${PREFIX}/include
touch ${PREFIX}/include/linux/autoconf.h


# takes advantage of the just install binutils;
cd /tmp/kost
zcat ~/compile/build-1.3.0/glibc-2.3.2.tar.gz | tar xf -
cd glibc-2.3.2
zcat ~/compile/build-1.3.0/glibc-linuxthreads-2.3.2.tar.gz | tar xf -
cat <<EOF > glibc-2.3.2.patch
Index: stdio-common/sscanf.c
===================================================================
--- stdio-common/sscanf.c       Sun Aug 11 00:09:08 2002 
+++ stdio-common/sscanf.c       Wed Oct 15 21:21:04 2003
@@ -25,13 +25,11 @@
 #endif
 
 /* Read formatted input from S, according to the format string FORMAT.  */
 /* VARARGS2 */
 int
-sscanf (s, format)
-     const char *s;
-     const char *format;
+sscanf (const char *s, const char *format,...)
 {
   va_list arg;
   int done;
 
   va_start (arg, format);
Index: Makerules
===================================================================
--- Makerules       Sun Feb 23 00:23:31 2003
+++ Makerules       Sun Mar  7 23:15:49 2004
@@ -970,6 +970,8 @@
               ')' \
         ) > $@.new
         mv -f $@.new $@
+	$(common-objpfx)elf/ldconfig $(addprefix -r ,$(install_root)) \
+	                                $(slibdir) $(libdir)
 
 endif
 
EOF
patch --verbose -u -l -F0 -p0 < glibc-2.3.2.patch
/bin/rm -f glibc-2.3.2.patch
cd ..
mkdir build-glibc
cd build-glibc
../glibc-2.3.2/configure --prefix=${PREFIX} --with-binutils=${PREFIX}/bin --with-headers=${PREFIX}/include --enable-add-ons=linuxthreads --disable-profile --without-gd --without-cvs >& conf.log
# tail -f conf.log
mkdir ${PREFIX}/etc
touch ${PREFIX}/etc/ld.so.conf
make >& build.log ; make check >& check.log ; make install >& install.log
# tail -f build.log
# tail -f check.log
# tail -f install.log
cd /tmp/kost
mv build-glibc build-glibc-stage1


# bolt-on the linker that BY DEFAULT will use the ${PREFIX} libraries.
cd /tmp/kost
cd build-binutils-stage1
make -C ld install


# fixing the stage1 compiler so that it tells the linker to 
# build dynamic binaries using the newling built ld-linux.so.2
setenv SPECFILE ${PREFIX}/lib/gcc-lib/*/*/specs
sed -e 's@ /lib/ld-linux.so.2@ '${PREFIX}'/lib/ld-linux.so.2@g' \
    $SPECFILE > tempspecfile
mv -f tempspecfile $SPECFILE
unset SPECFILE


# gcc install second stage.
# sources get patched now (a'la LFS - disabling "fix includes" and 
# also fixing the default include/library paths)
cd /tmp/kost
cat <<EOF > gcc-3.3.2-no_fixincludes-1.patch
Submitted By: Ronald Hummelink <ronald at hummelink dot xs4all dot nl>
Date: 2003-08-16
Initial Package Version: 3.3.1
Origin: Originally developed for GCC 3.2 by Greg Schafer <gschafer at zip dot com dot au>
Description: Prevent fixincludes script from running.

--- gcc-3.3.2/gcc/Makefile.in	2003-08-03 15:48:36.000000000 +0000
+++ gcc-3.3.2/gcc/Makefile.in	2003-08-15 23:40:28.000000000 +0000
@@ -2335,10 +2335,6 @@
 	rm -f include/limits.h
 	cp xlimits.h include/limits.h
 	chmod a+r include/limits.h
-# Install the README
-	rm -f include/README
-	cp $(srcdir)/README-fixinc include/README
-	chmod a+r include/README
 	$(STAMP) $@
 
 # fixinc.sh depends on this, not on specs directly.
@@ -2369,7 +2365,6 @@
 	(TARGET_MACHINE='$(target)'; srcdir=`cd $(srcdir); ${PWD_COMMAND}`; \
 	SHELL='$(SHELL)' ;\
 	export TARGET_MACHINE srcdir SHELL ; \
-	$(SHELL) ./fixinc.sh `${PWD_COMMAND}`/include $(SYSTEM_HEADER_DIR) $(OTHER_FIXINCLUDES_DIRS); \
 	rm -f include/syslimits.h; \
 	if [ -f include/limits.h ]; then \
 	  mv include/limits.h include/syslimits.h; \
EOF
cat <<EOF | sed -e '1,$s@/tools/@'${PREFIX}'/@g' > gcc-3.3.2-specs-1.patch
Submitted By: Oliver Brakmann <obrakmann at gmx dot net>
Date: 2003-09-20
Initial Package Version: 3.3.2
Origin: Idea originally developed by Ryan Oliver and Greg Schafer for the Pure LFS project.
        More architectures added by Zack Winkles.
        Further fine tunings by Greg Schafer.
	Modified for gcc 3.3.2 by Oliver Brakmann
Description: This patch modifies the location of the dynamic linker for the GCC Pass 2 build in LFS Chapter 5.
             It also removes /usr/include from the include search path.

             NOTE - !defined(USE_GNULIBC_1) is assumed i.e. libc5 is not supported.
             WARNING - Not all architectures addressed by this patch have been properly tested due
                       to lack of access to those architectures. If you notice any problems with
                       this patch on your architecture, please report them to
                       lfs-dev at linuxfromscratch dot org

diff -Naur gcc-3.3.2/gcc/config/alpha/linux-elf.h gcc-3.3.2-patched/gcc/config/alpha/linux-elf.h
--- gcc-3.3.2/gcc/config/alpha/linux-elf.h	2003-10-17 21:41:57.000000000 +0200
+++ gcc-3.3.2-patched/gcc/config/alpha/linux-elf.h	2003-10-17 21:37:06.000000000 +0200
@@ -30,7 +30,7 @@
 #ifdef USE_GNULIBC_1
 #define ELF_DYNAMIC_LINKER	"/lib/ld.so.1"
 #else
-#define ELF_DYNAMIC_LINKER	"/lib/ld-linux.so.2"
+#define ELF_DYNAMIC_LINKER	"/tools/lib/ld-linux.so.2"
 #endif
 
 #define LINK_SPEC "-m elf64alpha %{G*} %{relax:-relax}		\
diff -Naur gcc-3.3.2/gcc/config/arm/linux-elf.h gcc-3.3.2-patched/gcc/config/arm/linux-elf.h
--- gcc-3.3.2/gcc/config/arm/linux-elf.h	2003-10-17 21:41:57.000000000 +0200
+++ gcc-3.3.2-patched/gcc/config/arm/linux-elf.h	2003-10-17 21:37:07.000000000 +0200
@@ -84,7 +84,7 @@
    %{shared:-shared} \
    %{symbolic:-Bsymbolic} \
    %{rdynamic:-export-dynamic} \
-   %{!dynamic-linker:-dynamic-linker /lib/ld-linux.so.2} \
+   %{!dynamic-linker:-dynamic-linker /tools/lib/ld-linux.so.2} \
    -X \
    %{mbig-endian:-EB}" \
    SUBTARGET_EXTRA_LINK_SPEC
diff -Naur gcc-3.3.2/gcc/config/i386/linux64.h gcc-3.3.2-patched/gcc/config/i386/linux64.h
--- gcc-3.3.2/gcc/config/i386/linux64.h	2003-10-17 21:41:57.000000000 +0200
+++ gcc-3.3.2-patched/gcc/config/i386/linux64.h	2003-10-17 21:37:07.000000000 +0200
@@ -67,8 +67,8 @@
   %{!shared: \
     %{!static: \
       %{rdynamic:-export-dynamic} \
-      %{m32:%{!dynamic-linker:-dynamic-linker /lib/ld-linux.so.2}} \
-      %{!m32:%{!dynamic-linker:-dynamic-linker /lib64/ld-linux-x86-64.so.2}}} \
+      %{m32:%{!dynamic-linker:-dynamic-linker /tools/lib/ld-linux.so.2}} \
+      %{!m32:%{!dynamic-linker:-dynamic-linker /tools/lib64/ld-linux-x86-64.so.2}}} \
     %{static:-static}}"
 
 #undef  STARTFILE_SPEC
diff -Naur gcc-3.3.2/gcc/config/i386/linux.h gcc-3.3.2-patched/gcc/config/i386/linux.h
--- gcc-3.3.2/gcc/config/i386/linux.h	2003-10-17 21:41:57.000000000 +0200
+++ gcc-3.3.2-patched/gcc/config/i386/linux.h	2003-10-17 21:37:07.000000000 +0200
@@ -141,7 +141,7 @@
     %{!ibcs: \
       %{!static: \
 	%{rdynamic:-export-dynamic} \
-	%{!dynamic-linker:-dynamic-linker /lib/ld-linux.so.2}} \
+	%{!dynamic-linker:-dynamic-linker /tools/lib/ld-linux.so.2}} \
 	%{static:-static}}}"
 #endif
 
diff -Naur gcc-3.3.2/gcc/config/ia64/linux.h gcc-3.3.2-patched/gcc/config/ia64/linux.h
--- gcc-3.3.2/gcc/config/ia64/linux.h	2003-10-17 21:41:57.000000000 +0200
+++ gcc-3.3.2-patched/gcc/config/ia64/linux.h	2003-10-17 21:37:07.000000000 +0200
@@ -43,7 +43,7 @@
   %{!shared: \
     %{!static: \
       %{rdynamic:-export-dynamic} \
-      %{!dynamic-linker:-dynamic-linker /lib/ld-linux-ia64.so.2}} \
+      %{!dynamic-linker:-dynamic-linker /tools/lib/ld-linux-ia64.so.2}} \
       %{static:-static}}"
 
 
diff -Naur gcc-3.3.2/gcc/config/linux.h gcc-3.3.2-patched/gcc/config/linux.h
--- gcc-3.3.2/gcc/config/linux.h	2003-10-17 21:41:57.000000000 +0200
+++ gcc-3.3.2-patched/gcc/config/linux.h	2003-10-17 21:37:10.000000000 +0200
@@ -115,3 +115,7 @@
 #define HANDLE_PRAGMA_PACK_PUSH_POP
 
 #define TARGET_HAS_F_SETLKW
+
+/* Remove /usr/include from the end of the include search path.  */
+#undef STANDARD_INCLUDE_DIR
+#define STANDARD_INCLUDE_DIR 0
diff -Naur gcc-3.3.2/gcc/config/m68k/linux.h gcc-3.3.2-patched/gcc/config/m68k/linux.h
--- gcc-3.3.2/gcc/config/m68k/linux.h	2003-10-17 21:41:57.000000000 +0200
+++ gcc-3.3.2-patched/gcc/config/m68k/linux.h	2003-10-17 21:37:08.000000000 +0200
@@ -179,7 +179,7 @@
   %{!shared: \
     %{!static: \
       %{rdynamic:-export-dynamic} \
-      %{!dynamic-linker*:-dynamic-linker /lib/ld.so.1}} \
+      %{!dynamic-linker*:-dynamic-linker /tools/lib/ld.so.1}} \
     %{static}}"
 #endif
 
diff -Naur gcc-3.3.2/gcc/config/mips/linux.h gcc-3.3.2-patched/gcc/config/mips/linux.h
--- gcc-3.3.2/gcc/config/mips/linux.h	2003-10-17 21:41:57.000000000 +0200
+++ gcc-3.3.2-patched/gcc/config/mips/linux.h	2003-10-17 21:37:08.000000000 +0200
@@ -182,7 +182,7 @@
     %{!ibcs: \
       %{!static: \
         %{rdynamic:-export-dynamic} \
-        %{!dynamic-linker:-dynamic-linker /lib/ld.so.1}} \
+        %{!dynamic-linker:-dynamic-linker /tools/lib/ld.so.1}} \
         %{static:-static}}}"
 
 #undef SUBTARGET_ASM_SPEC
diff -Naur gcc-3.3.2/gcc/config/pa/pa-linux.h gcc-3.3.2-patched/gcc/config/pa/pa-linux.h
--- gcc-3.3.2/gcc/config/pa/pa-linux.h	2003-10-17 21:41:58.000000000 +0200
+++ gcc-3.3.2-patched/gcc/config/pa/pa-linux.h	2003-10-17 21:37:08.000000000 +0200
@@ -88,7 +88,7 @@
   %{!shared: \
     %{!static: \
       %{rdynamic:-export-dynamic} \
-      %{!dynamic-linker:-dynamic-linker /lib/ld.so.1}} \
+      %{!dynamic-linker:-dynamic-linker /tools/lib/ld.so.1}} \
       %{static:-static}}"
 
 /* glibc's profiling functions don't need gcc to allocate counters.  */
diff -Naur gcc-3.3.2/gcc/config/rs6000/linux64.h gcc-3.3.2-patched/gcc/config/rs6000/linux64.h
--- gcc-3.3.2/gcc/config/rs6000/linux64.h	2003-10-17 21:41:58.000000000 +0200
+++ gcc-3.3.2-patched/gcc/config/rs6000/linux64.h	2003-10-17 21:37:08.000000000 +0200
@@ -158,7 +158,7 @@
 #undef  LINK_OS_LINUX_SPEC
 #define LINK_OS_LINUX_SPEC "-m elf64ppc %{!shared: %{!static: \
   %{rdynamic:-export-dynamic} \
-  %{!dynamic-linker:-dynamic-linker /lib64/ld64.so.1}}}"
+  %{!dynamic-linker:-dynamic-linker /tools/lib64/ld64.so.1}}}"
 
 #ifdef NATIVE_CROSS
 #define STARTFILE_PREFIX_SPEC "/usr/local/lib64/ /lib64/ /usr/lib64/"
diff -Naur gcc-3.3.2/gcc/config/rs6000/sysv4.h gcc-3.3.2-patched/gcc/config/rs6000/sysv4.h
--- gcc-3.3.2/gcc/config/rs6000/sysv4.h	2003-10-17 21:41:58.000000000 +0200
+++ gcc-3.3.2-patched/gcc/config/rs6000/sysv4.h	2003-10-17 21:37:08.000000000 +0200
@@ -1146,7 +1146,7 @@
 
 #define LINK_OS_LINUX_SPEC "-m elf32ppclinux %{!shared: %{!static: \
   %{rdynamic:-export-dynamic} \
-  %{!dynamic-linker:-dynamic-linker /lib/ld.so.1}}}"
+  %{!dynamic-linker:-dynamic-linker /tools/lib/ld.so.1}}}"
 
 #if !defined(USE_GNULIBC_1) && defined(HAVE_LD_EH_FRAME_HDR)
 # define LINK_EH_SPEC "%{!static:--eh-frame-hdr} "
diff -Naur gcc-3.3.2/gcc/config/s390/linux.h gcc-3.3.2-patched/gcc/config/s390/linux.h
--- gcc-3.3.2/gcc/config/s390/linux.h	2003-10-17 21:41:58.000000000 +0200
+++ gcc-3.3.2-patched/gcc/config/s390/linux.h	2003-10-17 21:37:10.000000000 +0200
@@ -94,7 +94,7 @@
       %{static:-static} \
       %{!static: \
 	%{rdynamic:-export-dynamic} \
-	%{!dynamic-linker:-dynamic-linker /lib/ld.so.1}}}"
+	%{!dynamic-linker:-dynamic-linker /tools/lib/ld.so.1}}}"
 
 #define LINK_ARCH64_SPEC \
   "-m elf64_s390 \
@@ -103,7 +103,7 @@
       %{static:-static} \
       %{!static: \
 	%{rdynamic:-export-dynamic} \
-	%{!dynamic-linker:-dynamic-linker /lib/ld64.so.1}}}"
+	%{!dynamic-linker:-dynamic-linker /tools/lib/ld64.so.1}}}"
 
 #ifdef DEFAULT_TARGET_64BIT
 #undef  LINK_SPEC
diff -Naur gcc-3.3.2/gcc/config/sh/linux.h gcc-3.3.2-patched/gcc/config/sh/linux.h
--- gcc-3.3.2/gcc/config/sh/linux.h	2003-10-17 21:41:58.000000000 +0200
+++ gcc-3.3.2-patched/gcc/config/sh/linux.h	2003-10-17 21:37:10.000000000 +0200
@@ -48,7 +48,7 @@
   "%{shared:-shared} \
    %{!static: \
      %{rdynamic:-export-dynamic} \
-     %{!dynamic-linker:-dynamic-linker /lib/ld-linux.so.2}} \
+     %{!dynamic-linker:-dynamic-linker /tools/lib/ld-linux.so.2}} \
    %{static:-static}"
 
 /* The GNU C++ standard library requires that these macros be defined.  */
diff -Naur gcc-3.3.2/gcc/config/sparc/linux64.h gcc-3.3.2-patched/gcc/config/sparc/linux64.h
--- gcc-3.3.2/gcc/config/sparc/linux64.h	2003-10-17 21:41:58.000000000 +0200
+++ gcc-3.3.2-patched/gcc/config/sparc/linux64.h	2003-10-17 21:37:10.000000000 +0200
@@ -153,21 +153,21 @@
   { "link_arch_default", LINK_ARCH_DEFAULT_SPEC },	  \
   { "link_arch",	 LINK_ARCH_SPEC },
     
-#define LINK_ARCH32_SPEC "-m elf32_sparc -Y P,/usr/lib %{shared:-shared} \
+#define LINK_ARCH32_SPEC "-m elf32_sparc -Y P,/tools/lib %{shared:-shared} \
   %{!shared: \
     %{!ibcs: \
       %{!static: \
         %{rdynamic:-export-dynamic} \
-        %{!dynamic-linker:-dynamic-linker /lib/ld-linux.so.2}} \
+        %{!dynamic-linker:-dynamic-linker /tools/lib/ld-linux.so.2}} \
         %{static:-static}}} \
 "
 
-#define LINK_ARCH64_SPEC "-m elf64_sparc -Y P,/usr/lib64 %{shared:-shared} \
+#define LINK_ARCH64_SPEC "-m elf64_sparc -Y P,/tools/lib64 %{shared:-shared} \
   %{!shared: \
     %{!ibcs: \
       %{!static: \
         %{rdynamic:-export-dynamic} \
-        %{!dynamic-linker:-dynamic-linker /lib64/ld-linux.so.2}} \
+        %{!dynamic-linker:-dynamic-linker /tools/lib64/ld-linux.so.2}} \
         %{static:-static}}} \
 "
 
@@ -222,12 +222,12 @@
 #else /* !SPARC_BI_ARCH */
 
 #undef LINK_SPEC
-#define LINK_SPEC "-m elf64_sparc -Y P,/usr/lib64 %{shared:-shared} \
+#define LINK_SPEC "-m elf64_sparc -Y P,/tools/lib64 %{shared:-shared} \
   %{!shared: \
     %{!ibcs: \
       %{!static: \
         %{rdynamic:-export-dynamic} \
-        %{!dynamic-linker:-dynamic-linker /lib64/ld-linux.so.2}} \
+        %{!dynamic-linker:-dynamic-linker /tools/lib64/ld-linux.so.2}} \
         %{static:-static}}} \
 %{mlittle-endian:-EL} \
 %{!mno-relax:%{!r:-relax}} \
diff -Naur gcc-3.3.2/gcc/config/sparc/linux.h gcc-3.3.2-patched/gcc/config/sparc/linux.h
--- gcc-3.3.2/gcc/config/sparc/linux.h	2003-10-17 21:41:58.000000000 +0200
+++ gcc-3.3.2-patched/gcc/config/sparc/linux.h	2003-10-17 21:37:10.000000000 +0200
@@ -170,13 +170,13 @@
         %{static:-static}}}"
 #endif
 #else
-#define LINK_SPEC "-m elf32_sparc -Y P,/usr/lib %{shared:-shared} \
+#define LINK_SPEC "-m elf32_sparc -Y P,/tools/lib %{shared:-shared} \
   %{!mno-relax:%{!r:-relax}} \
   %{!shared: \
     %{!ibcs: \
       %{!static: \
         %{rdynamic:-export-dynamic} \
-        %{!dynamic-linker:-dynamic-linker /lib/ld-linux.so.2}} \
+        %{!dynamic-linker:-dynamic-linker /tools/lib/ld-linux.so.2}} \
         %{static:-static}}}"
 #endif
 
diff -Naur gcc-3.3.2/gcc/config/xtensa/linux.h gcc-3.3.2-patched/gcc/config/xtensa/linux.h
--- gcc-3.3.2/gcc/config/xtensa/linux.h	2003-10-17 21:41:58.000000000 +0200
+++ gcc-3.3.2-patched/gcc/config/xtensa/linux.h	2003-10-17 21:37:10.000000000 +0200
@@ -52,7 +52,7 @@
     %{!ibcs: \
       %{!static: \
         %{rdynamic:-export-dynamic} \
-        %{!dynamic-linker:-dynamic-linker /lib/ld.so.1}} \
+        %{!dynamic-linker:-dynamic-linker /tools/lib/ld.so.1}} \
       %{static:-static}}}"
 
 #undef LOCAL_LABEL_PREFIX
EOF
patch -Np0 -i gcc-3.3.2-no_fixincludes-1.patch
patch -Np0 -i gcc-3.3.2-specs-1.patch
/bin/rm -f gcc-3.3.2-no_fixincludes-1.patch gcc-3.3.2-specs-1.patch
#
mkdir build-gcc
cd build-gcc
../gcc-3.3.2/configure --prefix=${PREFIX} --enable-languages=c,c++ --enable-shared --enable-clocale=gnu --disable-nls --with-as=${PREFIX}/bin/as --with-ld=${PREFIX}/bin/ld --with-local-prefix=${PREFIX} --enable-threads=posix --enable-__cxa_atexit >& conf.log
# NO: CFLAGS=${CFLAGS} LIBCFLAGS=${CFLAGS} LIBCXXFLAGS=${CFLAGS}
# NO: CPPFLAGS=${CPPFLAGS} HDEFINES="-I${PREFIX}/include" LDFLAGS=${LDFLAGS} 
# all these flags mess up with the environement ..
make >& build.log ; make -k check >& check.log ; make install >& install.log
# tail -f build.log
# tail -f check.log
# tail -f install.log
cd /tmp/kost
mv build-gcc build-gcc-stage2


# binutils stage 2: this is rather a test - compilation, linking and
# run-time will depend on ${PREFIX} libraries only. Also can run the
# checks;
cd /tmp/kost
mkdir build-binutils
cd build-binutils
../binutils-2.14/configure --prefix=${PREFIX} --disable-nls --enable-shared --with-lib-path=${PREFIX}/lib >& conf.log
# tail -f conf.log
make >& build.log ; make check >& check.log ; make install >& install.log
# tail -f pre-build.log
# tail -f build.log
# tail -f check.log
# tail -f install.log
cd /tmp/kost
mv build-binutils build-binutils-stage2


# At this point, all the binaries produced by ${PREFIX}/bin/gcc will
# use ONLY the stuff in ${PREFIX}, including the run-time libraries in
# ${PREFIX}/lib, and the run-time linker ${PREFIX}/lib/ld-linux.so.2.
# Note since the path to run-time linker is hardwired, ${PREFIX} is
# NOT relocatable.
