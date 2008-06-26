dnl -*- sh -*-
dnl
dnl  Authors:
dnl    Zacharias El Banna (zeb@sics.se)
dnl 
dnl  Contributors:
dnl    optional, Contributor's name (Contributor's email address)
dnl 
dnl  Copyright:
dnl    Zacharias El Banna
dnl 
dnl  Last change:
dnl    $Date$ by $Author$
dnl    $Revision$
dnl 
dnl  This file is part of Mozart, an implementation 
dnl  of Oz 3:
dnl     http://www.mozart-oz.org
dnl 
dnl  See the file "LICENSE" or
dnl     http://www.mozart-oz.org/LICENSE.html
dnl  for information on usage and redistribution 
dnl  of this file, and for a DISCLAIMER OF ALL 
dnl  WARRANTIES.
dnl

# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.57)
AC_INIT(DSS, 0.9, "dss@sics.se")
AC_CONFIG_SRCDIR([src/dssBase.hh])
AC_LANG(C++)


# Check what kind of system we are on
AC_CANONICAL_HOST
tmp_host=`echo $host_cpu | sed 's/\./_/'`
AC_DEFINE_UNQUOTED(HAVE_HOST_CPU_$tmp_host)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET


# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(arpa/inet.h fcntl.h limits.h gmp.h pthread.h)

AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME

AC_FUNC_SELECT_ARGTYPES
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([sqrt])

dnl If installation does not work, use ordinary copy
# INSTALL="cp"
AC_PROG_INSTALL(INSTALL)


dnl Check for Programs
AR="ar"
AC_SUBST(AR)
AC_PROG_RANLIB(RANLIB)
AC_PROG_CXX(CXX)

AC_MSG_CHECKING(for $CXX version)
CXX_VERSION="`$CXX -dumpversion`"
case "$CXX_VERSION" in
    1* | 2*)
	VERSION=BELOW3
    ;;
    3*)
	VERSION=3
    ;;
    *)
	VERSION=ABOVE3
    ;;
esac
AC_MSG_RESULT($CXX_VERSION)


dnl Basis for warnings and CXXGLAGS
WARNINGS="-Wall -Wold-style-cast -Woverloaded-virtual -Wshadow"
CXX_BASE="-funsigned-char \$(INCS) \$(EXCEPTS) \$(WARNINGS)"

AC_MSG_RESULT()
AC_MSG_RESULT(*************** CHECKING OPTIONS ****************)
AC_MSG_RESULT()
AC_MSG_RESULT(Options can be found in README)
AC_MSG_RESULT()

AC_MSG_CHECKING(compiler alternative.)
AC_ARG_ENABLE(opt, [--enable-opt)],,)
case "$enable_opt" in
    debug)
	AC_MSG_RESULT(compile to debug mode)
	CXXFLAGS="-fno-inline -fno-default-inline -fno-inline-functions -DDEBUG_CHECK -DDSS_LOG -DINTERFACE -g3 -ggdb $CXX_BASE"
	WARNINGS="-pedantic-errors"
    ;;
    *)
	AC_MSG_RESULT(compile optimized mode)
	CXXFLAGS="-O3 -fomit-frame-pointer $CXX_BASE"
	WARNINGS="-Weffc++ $WARNINGS"
    ;;
esac

AC_MSG_CHECKING(exception handling...)
AC_ARG_ENABLE(exceptions, [--enable-exceptions)],,)
if test "$enable_exceptions" = yes -a "$enable_opt" != d; then
    EXCEPTS="$EXCEPTS -fexceptions"
    CXXFLAGS="-DEXCEPTIONS $CXXFLAGS"
    AC_MSG_RESULT(using exceptions)
else
    EXCEPTS="$EXCEPTS -fno-exceptions"
    AC_MSG_RESULT(no exceptions)
fi

AC_MSG_CHECKING(warning configuration)
AC_ARG_ENABLE(exceptions, [--enable-warnings)],,)
if test "$enable_warnings" = no; then
    AC_MSG_RESULT(no warnings (special))
    WARNINGS="-Wall"
else
    AC_MSG_RESULT(using extensive warnings)
    if test "$VERSION" = 3; then
	WARNINGS="$WARNINGS -Wswitch -Wreturn-type -Wunused"
    fi
    if test "$enable_opt" != d; then
	WARNINGS="-Winline $WARNINGS"
	if test "$VERSION" = 3; then
	    WARNINGS="-Wdisabled-optimization -Wuninitialized $WARNINGS"
	fi
    fi
    if test "$enable_warnings" = error; then
	AC_MSG_RESULT(WARNINGS TREATED AS ERRORS!!)	
	WARNINGS="$WARNINGS -Werror"
    fi
fi

AC_MSG_CHECKING(profiling information)
AC_ARG_ENABLE(profiling, [--enable-profiling)],,)
if test "$enable_profiling" != ""; then
    AC_MSG_RESULT(enabling profiler)
    CXXFLAGS="$CXXFLAGS -pg"
else
    AC_MSG_RESULT(no profiling)
fi

AC_MSG_CHECKING(what lib to build....)
AC_ARG_ENABLE(build, [--enable-build)],,)
case "$enable_build" in
    m*)
	AC_MSG_RESULT(will build stand-alone messaging component)
	ENV="MSL"
	COPY_HEADERS="$INSTALL $srcdir/include/dss_comService.hh $prefix/include"
    ;;
    *)
	AC_MSG_RESULT(will build full DSS)
        ENV="DSS"
        COPY_HEADERS="$INSTALL $srcdir/include/*.hh $prefix/include"
esac

AC_MSG_CHECKING(for linking choice...)
AC_ARG_ENABLE(modules-static,
    [--enable-modules-static)],,)
if test "$enable_modules_static" = yes; then
    AC_MSG_RESULT(static linking)
    ENVTYPE="lib$ENV.a"
else
    AC_MSG_RESULT(dynamic linking)
    ENVTYPE="lib$ENV.so"
fi

AC_MSG_CHECKING(inclusion of examples)
AC_ARG_ENABLE(dss-examples, [--enable-dss-examples],,)
AC_ARG_ENABLE(msgn-examples,[--enable-msgn-examples],,)
if test "$enable_dss_examples" != "" -o  "$enable_msgn_examples" != ""; then
    AC_MSG_RESULT(adding examples)
    EXAMPLES="if(cd examples && \$(MAKE) \$@); then true; else exit 1; fi;"
    AC_CONFIG_SUBDIRS(examples)
else
    AC_MSG_RESULT(no examples)
fi

dnl Configure subdirs
AC_MSG_CHECKING(inclusion of utils...)
AC_ARG_ENABLE(utils,[--enable-utils],,)
if test "$enable_utils" != "" -o "$EXAMPLES" != ""; then
    AC_MSG_RESULT(adding utils)
    UTILS="if(cd utils && \$(MAKE) \$@); then true; else exit 1; fi;"
    AC_CONFIG_SUBDIRS(utils)
    CXXFLAGS="-D_GNU_SOURCE $CXXFLAGS"
else
    AC_MSG_RESULT(no utils)
fi

AC_MSG_RESULT()
AC_MSG_RESULT(************* CHECKING ENVIRONMENT ***************)
AC_MSG_RESULT()

AC_MSG_CHECKING(environment)
AC_MSG_RESULT(found $host_os)
dnl Check for special cygwin environment
DYNLD="\$(CXX) -shared -Wl,-soname,\$@"
case "$host_os" in
    *cygwin)
        AC_MSG_RESULT(-- adding cygwin environment --)
	CREATE_OBJS="%.o : \$(SRCDIR)/%.cc"
	CREATE_OBJS_COMMAND="\$(CXX) \$(CXXFLAGS) -c \$(SRCDIR)/\$*.cc -o \$*.o"
	CXXFLAGS="-DWIN32 $CXXFLAGS -mno-cygwin"
    ;;
    solaris*)
	AC_MSG_RESULT(-- adding solaris environment --)
	CXXFLAGS="-DSOLARIS $CXXFLAGS"
    ;;
    darwin*)
	DYNLD="\$(CXX) -dynamic -bundle -flat_namespace -undefined suppress"
    ;;
esac

CPU_FLAGS=""
AC_MSG_CHECKING(CPU type)
case "$host_cpu" in
    sparc)
        AC_MSG_RESULT(sparc CPU)
	CPU_FLAGS="-mcpu=v8 -DBIG_ENDIAN_HOST"
    ;;
    *)
	AC_MSG_RESULT($host_cpu architecture)
	if test "$VERSION" = ABOVE3; then
	    CPU_FLAGS="-march=$host_cpu -mtune=$host_cpu"
	else
	    CPU_FLAGS="-march=$host_cpu -mcpu=$host_cpu"
	fi
    ;;
esac
CXXFLAGS="$CPU_FLAGS $CXXFLAGS"


dnl We need a prefix
if test "$prefix" = "" -o "$prefix" = "NONE"; then
    prefix=`pwd`
    AC_MSG_WARN(No prefix supplied)
    COPY_HEADERS=""
fi

dnl subst all variables!!
AC_SUBST(WARNINGS)
AC_SUBST(EXCEPTS)
AC_SUBST(ENVTYPE)
AC_SUBST(ENV)
AC_SUBST(COPY_HEADERS)
AC_SUBST(UTILS)
AC_SUBST(EXAMPLES)
AC_SUBST(CREATE_OBJS)
AC_SUBST(CREATE_OBJS_COMMAND)
AC_SUBST(CXXFLAGS)
AC_SUBST(DYNLD)

AC_CONFIG_FILES(Makefile Makefile.vars)
AC_OUTPUT

if test "$enable_modules_static" != "yes"; then
    AC_MSG_RESULT()
    AC_MSG_RESULT(**** REMEMBER TO SET LD_LIBRARY_PATH ****)
    AC_MSG_RESULT()
fi
