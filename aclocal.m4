dnl -*- m4 -*-
dnl
dnl  Authors:
dnl    Denys Duchier (duchier@ps.uni-sb.de)
dnl    Michael Mehl (mehl@dfki.de)
dnl 
dnl  Copyright:
dnl    Denys Duchier (1998)
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

dnl ==================================================================
dnl SRCDIR, SRCTOP, BUILDTOP
dnl ==================================================================

dnl ------------------------------------------------------------------
dnl OZ_PATH_SRCDIR
dnl
dnl makes sure that $srcdir is absolute and sets SRCDIR.  Obviously
dnl this one _must_ not be cached or else recursive configure will
dnl become very confused
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_PATH_SRCDIR,[
  srcdir=`cd $srcdir && pwd`
  SRCDIR="$srcdir"
  AC_SUBST(SRCDIR)
])

dnl ------------------------------------------------------------------
dnl OZ_PATH_UPWARD(VAR,DIR,FILES)
dnl
dnl looks upward from DIR for a directory that contains one of the
dnl FILES and sets VAR to the abolute path to this directory, else
dnl leaves VAR unchanged.  First it looks upward for the 1st file, if
dnl it does not find it, then it tries with the next file, etc...
dnl Thus you should list first the files that are more surely
dnl characteristic of the directory that you are looking for, and last
dnl those files that are strongly indicative, but not necessarily
dnl sure bets.
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_PATH_UPWARD,[
  oz_tmp_dir=[$2]
  oz_tmp_ok=
  oz_for="[$3]"
  for oz_tmp1 in $oz_for; do
    for oz_tmp2 in $oz_tmp_dir \
		   $oz_tmp_dir/.. \
		   $oz_tmp_dir/../.. \
		   $oz_tmp_dir/../../.. \
		   $oz_tmp_dir/../../../.. \
		   $oz_tmp_dir/../../../../..; do
      if test -r "$oz_tmp2/$oz_tmp1"; then
        [$1]=`cd $oz_tmp2 && pwd`
        oz_tmp_ok=yes
        break
      fi
    done
    test -n "$oz_tmp_ok" && break
  done])

dnl ------------------------------------------------------------------
dnl OZ_PATH_SRCTOP
dnl
dnl sets SRCTOP by looking upward from $srcdir for a directory
dnl containing file OZVERSION
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_PATH_SRCTOP,[
  AC_CACHE_CHECK([for SRCTOP],oz_cv_path_SRCTOP,[
    OZ_PATH_UPWARD(oz_cv_path_SRCTOP,$srcdir,[OZVERSION])
    if test -z "$oz_cv_path_SRCTOP"; then
      AC_MSG_ERROR([cannot find SRCTOP])
    fi])
  SRCTOP=$oz_cv_path_SRCTOP
  AC_SUBST(SRCTOP)])

dnl ------------------------------------------------------------------
dnl OZ_PATH_BUILDTOP
dnl
dnl sets BUILDTOP by looking upward from the current directory
dnl for a directory containing either contrib or config.cache. We
dnl look for config.cache last because some people may want to run
dnl configure only in a subdirectory of the source tree.
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_PATH_BUILDTOP,[
  AC_CACHE_CHECK([for BUILDTOP],oz_cv_path_BUILDTOP,[
    OZ_PATH_UPWARD(oz_cv_path_BUILDTOP,.,[contrib config.cache])
    if test -z "$oz_cv_path_BUILDTOP"; then
      AC_MSG_ERROR([cannot find BUILDTOP])
    fi])
  BUILDTOP=$oz_cv_path_BUILDTOP
  AC_SUBST(BUILDTOP)])

AC_DEFUN(OZ_PROG_INSTALL,[
  if test "${INSTALL+set}" = set; then
    AC_CACHE_CHECK([whether to unset broken INSTALL],
      oz_cv_unset_INSTALL,[
        echo >conftest.$$
        if $INSTALL -c -m 644 conftest.$$ /tmp >/dev/null 2>&1; then
          rm -f /tmp/conftest.$$ 2>/dev/null
          oz_cv_unset_INSTALL=no
          oz_cv_given_INSTALL="$INSTALL"
        else
          oz_cv_unset_INSTALL=yes
        fi
        rm -f conftest.$$ 2>/dev/null])
    test "$oz_cv_unset_INSTALL" = yes && unset INSTALL
  fi
  AC_PROG_INSTALL
  if test "$INSTALL" = "$oz_cv_given_INSTALL"; then
    ac_given_INSTALL="$oz_cv_given_INSTALL"
  fi
  OZ_PATH_PROG(INSTALL_DIR,  mkinstalldirs)])

AC_DEFUN(OZ_BUILD_DATE,[
  AC_CACHE_CHECK([for build date],oz_cv_build_date,[
    oz_cv_build_date=`date +"%Y%m%d"`])
  OZBUILDDATE=$oz_cv_build_date;
  AC_SUBST(OZBUILDDATE)])

AC_DEFUN(OZ_VERSION,[
  AC_CACHE_CHECK([for oz version],oz_cv_ozversion,[
    oz_cv_ozversion=`sh $SRCTOP/OZVERSION`])
  OZVERSION=$oz_cv_ozversion;
  AC_SUBST(OZVERSION)])

AC_DEFUN(OZ_INIT, [
  AC_PREFIX_DEFAULT(/usr/local/oz)
  OZ_PATH_SRCDIR
  OZ_PATH_SRCTOP
  OZ_PATH_BUILDTOP
  AC_CONFIG_AUX_DIR($SRCTOP)
  if test "$host_os" = ""; then  
    AC_CANONICAL_HOST
  fi
  OZ_PROG_MAKE
  AC_PROG_MAKE_SET
  OZ_PROG_INSTALL
  OZ_VERSION
  HOMEURL="http://www.mozart-oz.org/home-$OZVERSION"
  HOMECACHE="http/www.mozart-oz.org/home-$OZVERSION"
  AC_SUBST(HOMEURL)
  AC_SUBST(HOMECACHE)
  case "$target" in
    i386-mingw32|i386-mingw32msvc)
	PLATFORM=win32-i486
	platform="win32-i486"
	ozplatform=$platform

	cross_compiling=yes

	CXX=${target}-gcc
	CC=${CXX}
	RANLIB=${target}-ranlib
	AR=${target}-ar
	STRIP=${target}-strip
	WINDRES=${target}-windres
	AS=${target}-as
	DLLTOOL=${target}-dlltool
	DLLWRAP=${target}-dllwrap
	OZTOOL="sh $BUILDTOP/platform/emulator/${target}-oztool"
	enable_contrib_psql=no
	enable_contrib_gdbm=yes
	enable_contrib_regex=yes
	enable_contrib_os=no
    ;;

    *)
	OZ_PATH_PROG(OZPLATFORM,ozplatform)
	PLATFORM=`$OZPLATFORM || exit 1`
	OZ_PATH_PROG(OZTOOL,oztool,[OZTOOL="sh $BUILDTOP/platform/emulator/oztool.sh"])
    ;;
  esac
  OZ_ARG_WITH_INC_DIR
  OZ_ARG_WITH_LIB_DIR
  AC_SUBST(CPPFLAGS)
  AC_SUBST(LDFLAGS)
  OZ_BUILD_DATE
  AC_SUBST(PLATFORM)
  OZ_OZLOADSEP
  OZ_OZLOADWIN
])

dnl ==================================================================
dnl VERSION CHECKING
dnl ==================================================================

dnl ------------------------------------------------------------------
dnl OZ_CHECK_VERSION(OK,GOT,WANT)
dnl
dnl GOT and WANT consist of integers separated by single periods, e.g.
dnl 23.1 or 3.5.7, and OK is set to yes or no according to whether
dnl GOT >= WANT with respect to the obvious ordering.  To accomodate
dnl such program as perl, we also permit `_' and `-' instead of `.'
dnl
dnl for example
dnl	OZ_CHECK_VERSION(OK,$VERSION,2.5.4)
dnl sets OK to yes or no, according to whether $VERSION is greater
dnl than 2.5.4
dnl
dnl Note that we now support having one letter at the end: this
dnl interpreted as a "beta" version.  Thus 1.3.23c is less that
dnl 1.3.23, but more than 1.3.23a.
dnl
dnl `set $oz_tmp_got DONE' results in the component integers being
dnl assigned to positional parameters $1, $2 etc... DONE marks the
dnl of this sequence. The for loop goes through what we `want' but
dnl also through what correspondingly we `got' through positional
dnl parameter $1.  the `shift' at the end of the loop moves left
dnl all positional parameters, thus bringing the next thing we `got'
dnl into $1.
dnl
dnl Note that if GOT is empty the test succeeds (so don't do it)
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_PREPARE_VERSION,[
  oz_tmp="$[$1]"
changequote(<,>)
  oz_tmp=`echo $oz_tmp | sed -e 's/[a-z]/.\0/'`
  oz_tmp=`echo $oz_tmp | sed -e 's/[._-]/ /g'`
changequote([,])
  [$1]=$oz_tmp])

AC_DEFUN(OZ_CHECK_VERSION,[
  oz_tmp_got="[$2]"
  oz_tmp_want="[$3]"
  OZ_PREPARE_VERSION(oz_tmp_got)
  OZ_PREPARE_VERSION(oz_tmp_want)
  set $oz_tmp_got DONE
  oz_tmp__ok=yes
  for oz_tmp_cur in $oz_tmp_want; do
    case $oz_tmp_cur in
      DONE)
         case $[1] in
           [a-z]) oz_tmp__ok=no;;
           *);;
         esac
         break
         ;;
      [a-z])
         case $[1] in
           DONE) break;;
           [a-z])
              if test `expr $oz_tmp_cur '<=' $[1]` -eq 0; then
                oz_tmp__ok=no
                break
              fi
           ;;
           *) break
           ;;
         esac
         ;;
      *)
         case $[1] in
           DONE) oz_tmp__ok=no
             break
             ;;
           [a-z]) oz_tmp__ok=no
             break
             ;;
           *) if test "$oz_tmp_cur" -lt "$[1]"; then
                break
              elif test "$oz_tmp_cur" -gt "$[1]"; then
                oz_tmp__ok=no
                break
              fi
           ;;
         esac
         ;;
    esac
    [shift]
  done
  [$1]=$oz_tmp__ok])

dnl ------------------------------------------------------------------
dnl OZ_PROG_VERSION_CHECK(VAR,PROG,VERSION [,HOW])
dnl
dnl gets the --version of PROG and compares it to VERSION
dnl sets VAR to yes or no accordingly. HOW is the command to execute
dnl to get the text containing the string `version XXXX' where XXXX
dnl is the version number.  If HOW is not supplied we use the
dnl --version option.
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_PROG_VERSION_CHECK,[
  AC_MSG_CHECKING([$2 version is at least $3])
  [$1]=no
  if oz_tmp_version1=`ifelse([$4],[],[$2 --version],[$4]) 2>/dev/null`; then
    # first we try to locate the string "version"
    oz_tmp_version2=`echo "${oz_tmp_version1}" | tr '\012' ' '`
changequote(<,>)
    oz_tmp_version=`expr "${oz_tmp_version2}" : '.*version \([0-9._-]*[a-z]\{0,1\}\)'`
    # if that failed: we look at the end of the first line
    if test -z "$oz_tmp_version"; then
      oz_tmp_IFS="$IFS"
      IFS='
'
      for oz_tmp_version3 in ${oz_tmp_version1}; do
        oz_tmp_version=`expr "${oz_tmp_version3}" : '.* \([0-9._-]*[a-z]\{0,1\}\)$'`
	# else try to match the entire first line
	if test -z "$oz_tmp_version"; then
	  oz_tmp_version=`expr "${oz_tmp_version3}" : '\([0-9._-]*[a-z]\{0,1\}\)$'`
	fi
	break
      done
      IFS="$oz_tmp_IFS"
    fi
changequote([,])
    if test -n "$oz_tmp_version"; then
      OZ_CHECK_VERSION([$1],$oz_tmp_version,[$3])
    fi
  fi
  if test -z "$oz_tmp_version"; then
    AC_MSG_RESULT([no (cannot find version)])
  elif test $[$1] = no; then
    AC_MSG_RESULT([no])
  else
    AC_MSG_RESULT([yes ($oz_tmp_version)])
  fi])

dnl ==================================================================
dnl CHOOSE C++ COMPILER
dnl
dnl choose only if the choice is not already in the cache.  At
dnl Christian's request, CXXFLAGS is set to -O by default if it is not
dnl already set in the environment
dnl ==================================================================

AC_DEFUN(OZ_VERSION_GXX,[2.7])
AC_DEFUN(OZ_CXX_CHOOSE,[
  if test -z "$oz_cv_cxx__chosen"; then
    : ${CXXFLAGS="-O"}
    OZ_ARG_WITH_CXX
    AC_PROG_CXX
    if test "${GXX}" = yes; then
      if oz_tmp=`$CXX -dumpversion 2>/dev/null` || oz_tmp=`$CXX --version 2>/dev/null`; then
        if expr "$oz_tmp" : egcs >/dev/null; then
dnl I don't know what the appropriate version number is for egcs
          AC_MSG_WARN([dont know how to check egcs version, assuming ok])
	else
	  OZ_PROG_VERSION_CHECK(oz_tmp_ok,$CXX,OZ_VERSION_GXX,$CXX -dumpversion || $CXX --version)
	  if test "$oz_tmp_ok" = no; then
            AC_MSG_ERROR([
configure found the GNU C++ compiler $CXX version $oz_tmp_version
but version] OZ_VERSION_GXX [or higher is required to build the
system.  It can be retrieved from:

	ftp://ftp.gnu.org/pub/gnu/

The latest version at this time is 2.95.2 and is available
packaged as the following archive:

	gcc-2.95.2.tar.gz 

You may find a mirror archive closer to you by consulting:

	http://www.gnu.org/order/ftp.html
])
          fi
        fi
      else
        AC_MSG_WARN([Could not check $CXX version, assuming ok])
      fi
    fi
    case "$PLATFORM" in
    *win32*)
      case "$CXX" in
      *mno-cygwin*)
      ;;
      *)
        OZ_CXX_OPTIONS(-mno-cygwin,oz_tmp)
	CXX="$CXX${oz_tmp:+ }$oz_tmp"
      ;;
      esac
    ;;
    *)
    ;;
    esac
    AC_PROG_CXXCPP
    oz_cv_CXX=$CXX
    oz_cv_CXXCPP=$CXXCPP
    oz_cv_GXX=$GXX
    oz_cv_CXXFLAGS=$CXXFLAGS
    oz_cv_cxx__chosen=yes
  else
    OZ_FROM_CACHE(CXX,[for C++ compiler])
    OZ_FROM_CACHE(GXX,[whether we are using GNU C++])
    OZ_FROM_CACHE(CXXCPP,[for C++ preprocessor])
    if test "${CXXFLAGS+set}" != set; then
      OZ_FROM_CACHE(CFLAGS,[for default CXXFLAGS])
    fi
  fi
  AC_SUBST(CXX)
  AC_SUBST(CXXCPP)])

dnl ------------------------------------------------------------------
dnl OZ_FROM_CACHE(VAR,MSG)
dnl
dnl sets VAR to $oz_cv_VAR while printing an appropriate MSG
dnl this is useful to give feedback to the user.  use this when
dnl whether to compute a value is decided directly through an `if'
dnl test rather than by calling e.g. AC_CACHE_CHECK.
dnl
dnl OZ_MSG_FRONT(MSG)
dnl
dnl Same idea as AC_MSG_CHECKING, but doesn't say `Checking ', it just
dnl displays the MSG
dnl
dnl OZ_MSG_CACHING(MSG,VAR)
dnl
dnl gives feedback when caching a value that was computed as a side
dnl effect of doing something else.
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_FROM_CACHE,[
  OZ_MSG_FRONT([checking $2])
  [$1]=$oz_cv_$1
  AC_MSG_RESULT([(cached) $$1])])

define(OZ_MSG_FRONT,
[echo $ac_n "$1""... $ac_c" 1>&AC_FD_MSG
echo "configure:__oline__: $1" >&AC_FD_CC])

define(OZ_MSG_CACHING,[
  OZ_MSG_FRONT([Caching $1])
  AC_MSG_RESULT($$2)])

dnl ------------------------------------------------------------------
dnl OZ_ARG_WITH_CXX
dnl
dnl maybe sets CCC to a specific C++ compiler
dnl	1. --with-cxx=<CXX>	(command line)
dnl	2. oz_cv_with_cxx	(config.cache)
dnl	3. oz_with_cxx=<CXX>	(config.site)
dnl this is intended to be used by AC_PROG_CXX because it checks
dnl $CCC first in its list of alternative C++ compiler names
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_ARG_WITH_CXX,[
  AC_ARG_WITH(cxx,
    [--with-cxx=<CXX>	use <CXX> as C++ compiler (default=NONE)],
    oz_cv_with_cxx=$with_cxx)
  : ${CCC:=$oz_cv_with_cxx}
  : ${CCC:=$oz_with_cxx}
  : ${CXX:=$CCC}
])

dnl ==================================================================
dnl CHOOSE C COMPILER
dnl
dnl At Christian's request CFLAGS is set to -O by default if it is not
dnl already set in the environment.
dnl ==================================================================

AC_DEFUN(OZ_VERSION_GCC,[2.7])
AC_DEFUN(OZ_CC_CHOOSE,[
  if test -z "$oz_cv_cc__chosen"; then
    : ${CFLAGS="-O"}
    AC_PROG_CC
    if test "$GCC" = yes; then
      if oz_tmp=`$CC -dumpversion 2>/dev/null` || oz_tmp=`$CC --version 2>/dev/null`; then
        if expr "$oz_tmp" : egcs >/dev/null; then
dnl I don't know what the appropriate version number is for egcs
          AC_MSG_WARN([dont know how to check egcs version, assuming ok])
	else
          OZ_PROG_VERSION_CHECK(oz_tmp_ok,$CC,OZ_VERSION_GCC,$CC -dumpversion || $CC --version)
          if test "$oz_tmp_ok" = no; then
            AC_MSG_ERROR([
configure found the GNU C compiler $CC version $oz_tmp_version
but version] OZ_VERSION_GCC [or higher is required to build the
system.  It can be retrieved from:

	ftp://ftp.gnu.org/pub/gnu/

The latest version at this time is ??? and is available
packaged as the following archive:

	???

You may find a mirror archive closer to you by consulting:

	http://www.gnu.org/order/ftp.html
])
          fi
        fi
      else
        AC_MSG_WARN([Could not check $CC version, assuming ok])
      fi
    fi
    case "$PLATFORM" in
    *win32*)
      case "$CC" in
      *mno-cygwin*)
      ;;
      *)
        OZ_CC_OPTIONS(-mno-cygwin,oz_tmp)
	CC="$CC${oztmp:+ }$oz_tmp"
      ;;
      esac
    ;;
    *)
    ;;
    esac
    AC_PROG_CPP
    oz_cv_CC=$CC
    oz_cv_CPP=$CPP
    oz_cv_GCC=$GCC
    oz_cv_CFLAGS=$CFLAGS
    oz_cv_cc__chosen=yes
  else
    OZ_FROM_CACHE(CC,[for C compiler])
    OZ_FROM_CACHE(GCC,[whether we are using GNU C])
    OZ_FROM_CACHE(CPP,[for C preprocessor])
    if test "${CFLAGS+set}" != set ; then
      OZ_FROM_CACHE(CFLAGS,[for default CFLAGS])
    fi
  fi
  AC_SUBST(CC)
  AC_SUBST(CPP)])

dnl ==================================================================
dnl LOCATE UTILITIES AND CHECK VERSION NUMBERS
dnl ==================================================================

dnl ------------------------------------------------------------------
dnl OZ_PROG_FLEX or OZ_PROG_LEX
dnl
dnl locate GNU flex and check its version number. sets LEX and FLEX
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_PROG_LEX,[OZ_PROG_FLEX])
AC_DEFUN(OZ_VERSION_FLEX,[2.5.3])
AC_DEFUN(OZ_PROG_LEX_GNU,[
  if `$LEX -S/dev/null --version 2>/dev/null >/dev/null`; then
    GNU_LEX=yes
  else
    GNU_LEX=no
  fi])

AC_DEFUN(OZ_PROG_FLEX,[
  if test -z "$oz_cv_LEX"; then
    AC_PROG_LEX
    OZ_PROG_LEX_GNU
    if test "$GNU_LEX" = no; then
      oz_tmp_ok=no
      AC_MSG_WARN([$LEX is not GNU flex])
    else
      OZ_PROG_VERSION_CHECK(oz_tmp_ok,$LEX,OZ_VERSION_FLEX)
    fi
    if test "$oz_tmp_ok" = yes; then
      oz_cv_LEX=$LEX
    else
      if test "$oz_coolness" = yes; then
         LEX=flex_not_found
         AC_MSG_WARN(GNU flex not found)
      else
         AC_MSG_ERROR([
GNU flex version] OZ_VERSION_FLEX [or higher is needed to build the
system.  It can be retrieved from:

	ftp://ftp.gnu.org/pub/gnu/

The latest version at this time is 2.5.4 and is available
packaged as the following archive:

	flex-2.5.4a.tar.gz

You may find a mirror archive closer to you by consulting:

	http://www.gnu.org/order/ftp.html
])
      fi
    fi
  else
    OZ_FROM_CACHE(LEX,[for GNU flex])
  fi
  FLEX=$LEX
  AC_SUBST(LEX)
  AC_SUBST(FLEX)])

dnl ------------------------------------------------------------------
dnl OZ_PROG_BISON or OZ_PROG_YACC
dnl
dnl locate GNU bison and check its version number. sets YACC and BISON
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_PROG_YACC,[OZ_PROG_BISON])
AC_DEFUN(OZ_VERSION_BISON,[1.25])
AC_DEFUN(OZ_PROG_YACC_GNU,[
  if oz_tmp=`$YACC --version 2>/dev/null` && \
     oz_tmp=`expr "$oz_tmp" : '.*GNU'`; then
    GNU_YACC=yes;
  else
    GNU_YACC=no
  fi])

AC_DEFUN(OZ_PROG_BISON,[
  if test -z "$oz_cv_YACC"; then
    AC_PROG_YACC
    OZ_PROG_YACC_GNU
    if test "$GNU_YACC" = no; then
      oz_tmp_ok=no
      AC_MSG_WARN([$YACC is not GNU bison])
    else
      OZ_PROG_VERSION_CHECK(oz_tmp_ok,$YACC,OZ_VERSION_BISON)
    fi
    if test "$oz_tmp_ok" = yes; then
      oz_cv_YACC=$YACC
    else
      if test "$oz_coolness" = yes; then
        YACC=bison_not_found
        AC_MSG_WARN(bison not found)
      else
        AC_MSG_ERROR([
GNU bison version] OZ_VERSION_BISON [or higher is needed to build the system.
It can be retrieved from:

	ftp://ftp.gnu.org/pub/gnu/

The latest version at this time is 1.30 and is available
packaged as the following archive:

	bison-1.30.tar.gz


You may find a mirror archive closer to you by consulting:

	http://www.gnu.org/order/ftp.html
])
      fi
    fi
  else
    OZ_FROM_CACHE(YACC,[for GNU bison])
  fi
  BISON=$YACC
  AC_SUBST(YACC)
  AC_SUBST(BISON)])

dnl ------------------------------------------------------------------
dnl OZ_PROG_PERL
dnl
dnl locate perl and checks its version number. sets PERL
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_VERSION_PERL,[5])
AC_DEFUN(OZ_PROG_PERL,[
  if test -z "$oz_cv_PERL"; then
    AC_CHECK_PROGS(PERL,perl perl5,NONE)
    if test "$PERL" = NONE; then
      oz_tmp_ok=no;
    else
changequote(<,>)
      oz_tmp_version=`$PERL -e 'print "[$]\n";' | sed 's|^.||'`
changequote([,])
      OZ_CHECK_VERSION(oz_tmp_ok,$oz_tmp_version,OZ_VERSION_PERL)
    fi
    if test "$oz_tmp_ok" = yes; then
      oz_cv_PERL=$PERL
    else
      if test "$oz_coolness" = yes; then
        PERL=perl_not_found
        AC_MSG_WARN(perl not found)
      else
        AC_MSG_ERROR([
Perl version] OZ_VERSION_PERL [or higher is needed to build the system.
It can be retrieved from:

	http://www.perl.org/get.html

The latest version at this time is 5.8.2.  You may find further
information on the Perl site:

	http://www.perl.org/
])
      fi
    fi
  else
    OZ_FROM_CACHE(PERL,[for perl])
  fi
  AC_SUBST(PERL)])

dnl ------------------------------------------------------------------
dnl OZ_PROG_M4
dnl
dnl locate GNU m4. sets M4 and M4_S
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_PROG_M4_GNU,[
  if oz_tmp=`$M4 --version 2>/dev/null` && \
     oz_tmp=`expr "$oz_tmp" : GNU`; then
    GNU_M4=yes
  else
    GNU_M4=no
  fi])

AC_DEFUN(OZ_PROG_M4,[
  if test -z "$oz_cv_M4"; then
    AC_CHECK_PROGS(M4,gm4 m4,NONE)
    if test "$M4" = NONE; then
      oz_tmp_ok=no;
    else
      OZ_PROG_M4_GNU
      if test "$GNU_M4" = no; then
        oz_tmp_ok=no
        AC_MSG_WARN([$M4 is not GNU m4])
      else
        oz_tmp_ok=yes
      fi
      if test "$oz_tmp_ok" = yes; then
        oz_cv_M4=$M4
      else
      if test "$oz_coolness" = yes; then
	M4=m4_not_found
        AC_MSG_WARN(m4 not found)
      else
        AC_MSG_ERROR([
GNU m4 is needed to build the system.
It can be retrieved from:

	ftp://ftp.gnu.org/pub/gnu/m4/

The latest version at this time is 1.4 and is available
packaged as the following archive:

	m4-1.4.tar.gz

You may find a mirror archive closer to you by consulting:

	http://www.gnu.org/order/ftp.html
])
        fi
      fi
    fi
  else
    OZ_FROM_CACHE(M4,[for GNU m4])
  fi
  M4_S="-s"
  AC_SUBST(M4)
  AC_SUBST(M4_S)])

dnl ------------------------------------------------------------------
dnl OZ_PROG_MAKE
dnl
dnl locate GNU make. sets MAKE
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_PROG_MAKE_GNU,[
  if oz_tmp=`$MAKE -n --version 2>/dev/null` && \
     oz_tmp=`expr "$oz_tmp" : GNU`; then
    GNU_MAKE=yes
  else
    GNU_MAKE=no
  fi])

AC_DEFUN(OZ_PROG_MAKE,[
  if test -z "$oz_cv_MAKE"; then
    AC_CHECK_PROGS(MAKE,gmake make,NONE)
    if test "$MAKE" = NONE; then
      oz_tmp_ok=no
    else
      OZ_PROG_MAKE_GNU
      if test "$GNU_MAKE" = no; then
        oz_tmp_ok=no
        AC_MSG_WARN([$MAKE is not GNU make])
      else
        oz_tmp_ok=yes
      fi
    fi
    if test "$oz_tmp_ok" = yes; then
      oz_cv_MAKE=$MAKE
    else
      AC_MSG_ERROR([
GNU make is needed to build the system.
It can be retrieved from:

	ftp://ftp.gnu.org/pub/gnu/make/

The latest version at this time is 3.80 and is available
packaged as the following archive:

	make-3.80.tar.gz

You may find a mirror archive closer to you by consulting:

	http://www.gnu.org/order/ftp.html
])
    fi
  else
    OZ_FROM_CACHE(MAKE,[for GNU make])
  fi
  AC_SUBST(MAKE)])

dnl ------------------------------------------------------------------
dnl OZ_LIB_GMP
dnl
dnl locates gmp.h and libgmp and checks the version number
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_VERSION_GMP,[2])

AC_DEFUN(OZ_LIB_GMP,[
AC_MSG_CHECKING(for --with-gmp)
AC_ARG_WITH(gmp,
	[--with-gmp=<dir>   search gmp library and header in <dir> (default: yes)],
    with_gmp=$withval,
    with_gmp=yes)
	    
if test "$with_gmp" != no
then
    AC_MSG_RESULT($with_gmp)

    if test "${with_gmp}" != yes
    then
	oz_gmp_lib_dir=$with_gmp
	oz_gmp_inc_dir=$with_gmp
    fi
    
    oz_inc_path="$oz_gmp_inc_dir $oz_inc_path"
    oz_lib_path="$oz_gmp_lib_dir $oz_lib_path"

else
    AC_MSG_RESULT(no)
fi

OZ_CHECK_HEADER_PATH(gmp.h,
 [oz_gmp_inc_found=yes],
dnl try adding /usr/include/gmp2 for Debian
 [oz_inc_path_sav=$oz_inc_path
  oz_inc_path="$oz_inc_path /usr/include/gmp2"
  OZ_CHECK_HEADER_PATH(gmp.h,
    [oz_gmp_inc_found=yes],
    [oz_gmp_inc_found=no
     oz_inc_path=$oz_inc_path_sav])])

if test "$oz_gmp_inc_found" = yes; then
dnl first check for GMP 3
  OZ_CHECK_LIB_PATH(gmp, __gmpz_init,
    oz_gmp_lib_found=gmp,
dnl if that fail, try GMP 2
    OZ_CHECK_LIB_PATH(gmp, mpz_init,
      oz_gmp_lib_found=gmp,
      OZ_CHECK_LIB_PATH(gmp2, mpz_init,
      oz_gmp_lib_found=gmp2,
      oz_gmp_lib_found=no)))
fi

if test "$oz_gmp_lib_found" != no; then
  AC_MSG_CHECKING(gmp version is at least OZ_VERSION_GMP)
  if test -z "$oz_cv_gmp_version_ok"; then
    cat > conftest.$ac_ext <<EOF
#include <gmp.h>
TheVersion __GNU_MP_VERSION __GNU_MP_VERSION_MINOR
EOF
    oz_tmp=`$CXXCPP $CPPFLAGS conftest.$ac_ext | egrep TheVersion`;
    rm -f conftest.$ac_ext 2>/dev/null
changequote(<,>)
    OZ_GMP_MAJOR=`expr "$oz_tmp" : 'TheVersion *\([0-9]*\) '`
    OZ_GMP_MINOR=`expr "$oz_tmp" : 'TheVersion *[0-9]* *\([0-9]*\) *$'`
changequote([,])
    if oz_tmp=`expr "$oz_tmp" : 'TheVersion \(.*\)$'`; then
      OZ_CHECK_VERSION(oz_tmp_ok,$oz_tmp,OZ_VERSION_GMP)
      test "$oz_tmp_ok" = yes && oz_cv_gmp_version_ok=$oz_tmp_ok
    else
      oz_tmp_ok=no
    fi
    oz_cv_gmp_version_major=$OZ_GMP_MAJOR
    oz_cv_gmp_version_minor=$OZ_GMP_MINOR
    AC_MSG_RESULT($oz_tmp_ok)
  else
    oz_tmp_ok=$oz_cv_gmp_version_ok
    OZ_GMP_MAJOR=$oz_cv_gmp_version_major
    OZ_GMP_MINOR=$oz_cv_gmp_version_minor
    AC_MSG_RESULT([(cached) $oz_tmp_ok])
  fi
  oz_gmp_version_ok=$oz_tmp_ok
fi

if test "$oz_gmp_inc_found" = no; then
  AC_MSG_WARN([required GNU MP include file not found])
elif test "$oz_gmp_lib_found" = no; then
  AC_MSG_WARN([required GNU MP lib not found])
elif test "$oz_gmp_version_ok" = no; then
  AC_MSG_WARN([GNU MP version too old])
fi

if test "$oz_gmp_inc_found"  = no || \
   test "$oz_gmp_lib_found"  = no || \
   test "$oz_gmp_version_ok" = no; then
  AC_MSG_ERROR([
The GNU Multiple Precision Arithmetic Library (gmp)
version] OZ_VERSION_GMP [or higher is required
to build the system.  It can be retrieved from:

	ftp://ftp.gnu.org/pub/gnu/gmp/

The latest version at this time is 4.1.2 and is available
packaged as the following archive:

	ftp://ftp.gnu.org/pub/gnu/gmp/gmp-4.1.2.tar.gz

You may find a mirror archive closer to you by consulting:

	http://www.gnu.org/order/ftp.html
])
fi])

dnl ------------------------------------------------------------------
dnl OZ_LIB_ZLIB
dnl
dnl locates libz and zlib.h
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_LIB_ZLIB,[
  AC_MSG_CHECKING(for --with-zlib)
  AC_ARG_WITH(zlib,
	[--with-zlib=<dir>  search zlib library and header in <dir> (default: yes)],
	with_zlib=$withval,
	with_zlib=yes)
if test "$with_zlib" != no
then
    AC_MSG_RESULT($with_zlib)
	    
    if test "${with_zlib}" != yes
    then
	oz_zlib_lib_dir=$with_zlib
	oz_zlib_inc_dir=$with_zlib
    fi

    oz_inc_path="$oz_zlib_inc_dir $oz_inc_path"
    oz_lib_path="$oz_zlib_lib_dir $oz_lib_path"

else
    AC_MSG_RESULT(no)
fi

OZ_CHECK_HEADER_PATH(zlib.h,oz_zlib_inc_found=yes,oz_zlib_inc_found=no)

if test "$oz_zlib_inc_found" = yes; then
  OZ_CHECK_LIB_PATH(z, zlibVersion, oz_zlib_lib_found=yes,
    OZ_CHECK_LIB_PATH(gz, zlibVersion, oz_zlib_lib_found=yes,
	oz_zlib_lib_found=no))
fi

if test "$oz_zlib_inc_found" = no; then
  AC_MSG_WARN([required ZLIB include file zlib.h not found])
elif test "$oz_zlib_lib_found" = no; then
  AC_MSG_WARN([required ZLIB library libz not found])
fi

if test "$oz_zlib_inc_found" = no || \
   test "$oz_zlib_lib_found" = no; then
  AC_MSG_ERROR([
The ZLIB general purpose compression library is required to
build the system.  It can be retrieved from:

        http://www.gzip.org/zlib/

])
fi
])

dnl ------------------------------------------------------------------
dnl OZ_NEEDS_FUNC(FUNCTION)
dnl
dnl makes sure that FUNCTION is now available, else signals an error.
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_NEEDS_FUNC,[
  AC_CHECK_FUNC([$1],[oz_tmp_ok=yes],[oz_tmp_ok=no])
  if test "$oz_tmp_ok" = no; then
    AC_MSG_ERROR([Function $1 is not available.
The system cannot be built.
])
  fi])

dnl ------------------------------------------------------------------
dnl OZ_ARG_WITH_GLOBAL_OZ
dnl
dnl typically, we do NOT want to bootstrap the system using an existing
dnl mozart installation.  This can be overriden with an explicit
dnl --with-global-oz
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_ARG_WITH_GLOBAL_OZ,[
  AC_MSG_CHECKING([for --with-global-oz])
  AC_ARG_WITH(global-oz,
    [--with-global-oz allows to use an existing oz installation to build the system (default: no)],
    [oz_cv_global_oz="$with_global_oz"],
    [oz_cv_global_oz=no])
  WITH_GLOBAL_OZ="$oz_cv_global_oz"
  AC_MSG_RESULT($WITH_GLOBAL_OZ)
])

dnl ------------------------------------------------------------------
dnl OZ_PATH_PROG(VAR,PROGRAM,ACTION-IF-NOT-FOUND)
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_PATH_PROG, [
    if test -z "$WITH_GLOBAL_OZ"; then
      OZ_ARG_WITH_GLOBAL_OZ
    fi
    dummy_PATH="$PATH"
    if test "$WITH_GLOBAL_OZ" = no; then
      for oz_prog_tmp in $2; do
        case $oz_prog_tmp in
          oz|ozc|ozl|oztool|ozengine|ozplatform|ozdoc)
            dummy_PATH="$SRCTOP/share/bin:$SRCTOP"
          ;;
        esac
      done
    fi
    dummy_PWD=`pwd | sed 's/\//\\\\\//g'`
    dummy_PATH=`echo $dummy_PATH | sed -e 's/:://g' | sed -e 's/:$//g'`
    dummy_PATH=`echo $dummy_PATH | sed -e "s/^\.:/$dummy_PWD:/g"`
    dummy_PATH=`echo $dummy_PATH | sed -e "s/^\.\//$dummy_PWD\//g"`
    dummy_PATH=`echo $dummy_PATH | sed -e "s/:\.\$/:$dummy_PWD/g"`
    dummy_PATH=`echo $dummy_PATH | sed -e "s/:\.:/:$dummy_PWD:/g"`
    dummy_PATH=`echo $dummy_PATH | sed -e "s/:.\//:$dummy_PWD\//g"`
    oz_for="$dummy_PATH:$SRCTOP/share/bin:$SRCTOP"
    AC_PATH_PROG($1,$2,,$oz_for)
    case "$host_os" in
      cygwin*) ;;
      *) $1=`echo $$1 | sed -e "s|//|/|g"`;;
    esac
    if test ! -n "$$1"
    then
	$1=undefined
	$3
    fi
    ])

AC_DEFUN(OZ_TRY_LINK, [
	AC_TRY_LINK(
#ifdef __cplusplus
extern "C"
#endif
char $1();
,
$1(),$2,$3)])

AC_DEFUN(OZ_CHECK_LIB, [
  oz_saved_LIBS=$LIBS
  OZ_TRY_LINK($2,$3,
   [if test "${oz_enable_link_static}" = yes; then
      oz_add_libs="-Xlinker -Bstatic -l$1 -Xlinker -Bdynamic"
      LIBS="$oz_add_libs${oz_saved_LIBS:+ }$oz_saved_LIBS"
      OZ_TRY_LINK($2, $3,
        [LIBS="-l$1 $oz_saved_LIBS"
         oz_add_libs="-l$1"
         OZ_TRY_LINK($2,$3,
           [LIBS=$oz_saved_LIBS
            oz_add_libs=no
            $4])])
    else
      LIBS="-l$1 $oz_saved_LIBS"
      oz_add_libs="-l$1"
      OZ_TRY_LINK($2,$3,
        [LIBS=$oz_saved_LIBS
         oz_add_libs=no
         $4])
    fi])])

AC_DEFUN(OZ_CXX_OPTIONS, [
	ozm_out=
	if test -n "$1"
	then
	    echo 'void f(){}' > oz_conftest.c
	    oz_for="$1"
	    for ozm_opt in $oz_for
	    do
		AC_MSG_CHECKING(c++ compiler option $ozm_opt)
		ozm_ropt=`echo $ozm_opt | sed -e 's/[[^a-zA-Z0-9_]]/_/g'`
		AC_CACHE_VAL(oz_cv_gxxopt_$ozm_ropt,
		    if test -z "`${CXX} ${ozm_out} ${ozm_opt} -c oz_conftest.c 2>&1`"; then
			eval "oz_cv_gxxopt_$ozm_ropt=yes"
		    else
			eval "oz_cv_gxxopt_$ozm_ropt=no"
		    fi)
		if eval "test \"`echo '$''{'oz_cv_gxxopt_$ozm_ropt'}'`\" = yes"; then
		    ozm_out="$ozm_out $ozm_opt"
		    AC_MSG_RESULT(yes)
		else
		    AC_MSG_RESULT(no)
		fi
	    done
	    rm -f oz_conftest*
	fi
	$2="$ozm_out"
	])

AC_DEFUN(OZ_CXX_FIRST_OPTION, [
	ozm_out=
	if test -n "$1"
	then
	    echo 'void f(){}' > oz_conftest.c
	    oz_for="$1"
	    for ozm_opt in $oz_for
	    do
		AC_MSG_CHECKING(c++ compiler option $ozm_opt)
		ozm_ropt=`echo $ozm_opt | sed -e 's/[[^a-zA-Z0-9_]]/_/g'`
		AC_CACHE_VAL(oz_cv_gxxopt_$ozm_ropt,
		    if test -z "`${CXX} ${ozm_out} ${ozm_opt} -c oz_conftest.c 2>&1`"; then
			eval "oz_cv_gxxopt_$ozm_ropt=yes"
		    else
			eval "oz_cv_gxxopt_$ozm_ropt=no"
		    fi)
		if eval "test \"`echo '$''{'oz_cv_gxxopt_$ozm_ropt'}'`\" = yes"; then
		    ozm_out="$ozm_out $ozm_opt"
		    AC_MSG_RESULT(yes)
		else
		    AC_MSG_RESULT(no)
		fi
                if test -n "$ozm_out"; then
                  break;
                fi
	    done
	    rm -f oz_conftest*
	fi
	$2="$ozm_out"
	])

AC_DEFUN(OZ_CC_OPTIONS, [
	ozm_out=
	if test -n "$1"
	then
	    echo 'void f(){}' > oz_conftest.c
	    oz_for="$1"
	    for ozm_opt in $oz_for
	    do
		AC_MSG_CHECKING(cc compiler option $ozm_opt)
		ozm_ropt=`echo $ozm_opt | sed -e 's/[[^a-zA-Z0-9_]]/_/g'`
		AC_CACHE_VAL(oz_cv_gccopt_$ozm_ropt,
		    if test -z "`${CC} ${ozm_out} ${ozm_opt} -c oz_conftest.c 2>&1`"; then
			eval "oz_cv_gccopt_$ozm_ropt=yes"
		    else
			eval "oz_cv_gccopt_$ozm_ropt=no"
		    fi)
		if eval "test \"`echo '$''{'oz_cv_gccopt_$ozm_ropt'}'`\" = yes"; then
		    ozm_out="$ozm_out $ozm_opt"
		    AC_MSG_RESULT(yes)
		else
		    AC_MSG_RESULT(no)
		fi
	    done
	    rm -f oz_conftest*
	fi
	$2="$ozm_out"
	])

AC_DEFUN(OZ_CC_FIRST_OPTION, [
	ozm_out=
	if test -n "$1"
	then
	    echo 'void f(){}' > oz_conftest.c
	    oz_for="$1"
	    for ozm_opt in $oz_for
	    do
		AC_MSG_CHECKING(cc compiler option $ozm_opt)
		ozm_ropt=`echo $ozm_opt | sed -e 's/[[^a-zA-Z0-9_]]/_/g'`
		AC_CACHE_VAL(oz_cv_gccopt_$ozm_ropt,
		    if test -z "`${CC} ${ozm_out} ${ozm_opt} -c oz_conftest.c 2>&1`"; then
			eval "oz_cv_gccopt_$ozm_ropt=yes"
		    else
			eval "oz_cv_gccopt_$ozm_ropt=no"
		    fi)
		if eval "test \"`echo '$''{'oz_cv_gccopt_$ozm_ropt'}'`\" = yes"; then
		    ozm_out="$ozm_out $ozm_opt"
		    AC_MSG_RESULT(yes)
		else
		    AC_MSG_RESULT(no)
		fi
                if test -n "$ozm_out"; then
                  break;
                fi
	    done
	    rm -f oz_conftest*
	fi
	$2="$ozm_out"
	])

AC_DEFUN(OZ_ADDTO_LDFLAGS,[
  if test "[$1]" != yes && test "[$1]" != no; then
    oz_tmp_ok=yes
    for oz_tmp in $LDFLAGS NONE; do
      if test "$oz_tmp" = "[$1]"; then
        oz_tmp_ok=no
        break
      fi
    done
    test "$oz_tmp_ok" = yes && LDFLAGS="[$1]${LDFLAGS:+ }$LDFLAGS"
  fi
])

AC_DEFUN(OZ_ADDTO_LIBS,[
  if test -n "[$1]" && test "[$1]" != yes && test "[$1]" != no; then
    LIBS="[$1]${LIBS:+ }$LIBS"
  fi])

AC_DEFUN(OZ_CHECK_LIB_PATH,[
  if test -n "$[oz_cv_lib_path_ldflags_]patsubst($1_$2,[[^a-zA-Z0-9_]],_)"; then
    AC_MSG_CHECKING([for library $1])
    oz_add_ldflags=$[oz_cv_lib_path_ldflags_]patsubst($1_$2,[[^a-zA-Z0-9_]],_)
    oz_add_libs=$[oz_cv_lib_path_libs_]patsubst($1_$2,[[^a-zA-Z0-9_]],_)
    if test "$oz_add_ldflags" = no; then
      oz_tmp=no
    else
     if test -n "$oz_add_ldflags" && test "$oz_add_ldflags" != yes; then
       oz_tmp="$oz_add_ldflags (LDFLAGS)"
     else
       oz_tmp=
     fi
     if test -n "$oz_add_libs" && \
        test "$oz_add_libs" != yes && \
        test "$oz_add_libs" != no; then
       oz_tmp="$oz_tmp${oz_tmp:+ }$oz_add_libs (LIBS)"
     fi
    fi
    AC_MSG_RESULT([(cached) $oz_tmp])
  else
    oz_tmp_ldflags=$LDFLAGS
    oz_tmp_libs=$LIBS
    oz_add_ldflags=no
    oz_add_libs=no
    AC_MSG_CHECKING([for $2 in -l$1 (default)])
    OZ_CHECK_LIB($1,$2,
      [AC_MSG_RESULT(yes)
       oz_add_ldflags=yes
      ],
      [AC_MSG_RESULT(no)
       for p in $oz_lib_path; do
         LDFLAGS="-L$p $oz_tmp_ldflags"
         AC_MSG_CHECKING([for $2 in -L$p -l$1])
         OZ_CHECK_LIB($1,$2,
           [AC_MSG_RESULT(yes)
            oz_add_ldflags="-L$p"
            break],
           [AC_MSG_RESULT(no)])
       done])
    LDFLAGS=$oz_tmp_ldflags
    LIBS=$oz_tmp_libs
    [oz_cv_lib_path_ldflags_]patsubst($1_$2,[[^a-zA-Z0-9_]],_)=$oz_add_ldflags
    [oz_cv_lib_path_libs_]patsubst($1_$2,[[^a-zA-Z0-9_]],_)=$oz_add_libs
  fi
  if test "$oz_add_ldflags" = no; then
    ifelse([$4],[],:,[$4])
  else
    OZ_ADDTO_LDFLAGS($oz_add_ldflags)
    OZ_ADDTO_LIBS($oz_add_libs)
    $3
  fi])


AC_DEFUN(OZ_ADDTO_CPPFLAGS,[
  if test "[$1]" != yes && test "[$1]" != no; then
    oz_tmp_ok=yes
    for oz_tmp in $CPPFLAGS NONE; do
      if test "$oz_tmp" = "[$1]"; then
        oz_tmp_ok=no
        break
      fi
    done
    test "$oz_tmp_ok" = yes && CPPFLAGS="$CPPFLAGS${CPPFLAGS:+ }[$1]"
  fi
])


AC_DEFUN(OZ_CHECK_HEADER_PATH, [
  AC_CACHE_CHECK([for $1],dnl
changequote(`,')oz_cv_header_`'patsubst($1,[^a-zA-Z0-9],_),
changequote([,])
    [
      oz_tmp_cppflags="$CPPFLAGS"
      oz_tmp_ok=no
      for oz_tmp in $oz_inc_path; do
	CPPFLAGS="$oz_tmp_cppflags -I$oz_tmp"
	AC_TRY_CPP([#include "$1"],[
        oz_tmp_ok="-I$oz_tmp"
        break])
      done
      CPPFLAGS="$oz_tmp_cppflags"
      if test "$oz_tmp_ok" = no; then
	AC_TRY_CPP([#include "$1"],[
          oz_tmp_ok=yes],)
      fi
changequote(`,')oz_cv_header_`'patsubst($1,[^a-zA-Z0-9],_)="$oz_tmp_ok"
changequote([,])
    ])
changequote(`,')
  oz_tmp_val=$oz_cv_header_`'patsubst($1,[^a-zA-Z0-9],_)
changequote([,])
  if test "$oz_tmp_val" != no; then
    if test "$oz_tmp_val" != yes; then
      OZ_ADDTO_CPPFLAGS($oz_tmp_val)
    fi
    $2
  else
    ifelse([$3],[],:,$3)
  fi])

AC_DEFUN(OZ_CONTRIB_INIT,[
    OZ_INIT
    OZ_PATH_PROG(OZC,ozc)
    OZ_PATH_PROG(OZL,ozl)
    OZ_PATH_PROG(OZE,ozengine)
])

AC_DEFUN(OZ_CONTRIB_INIT_CXX,[
    OZ_CONTRIB_INIT
dnl we actually need the c++ compiler to test for libraries
dnl just oztool does not suffice since configure doesn't use
dnl oztool but calls the compiler directly
    OZ_CXX_CHOOSE
    AC_SUBST(CPPFLAGS)
    AC_SUBST(CXXFLAGS)
    AC_LANG_CPLUSPLUS
dnl    OZ_PATH_PROG(OZTOOL,oztool,[OZTOOL="sh $BUILDTOP/platform/emulator/oztool.sh"])
])


dnl ------------------------------------------------------------------
dnl OZ_ENABLE
dnl
dnl a canonical macro to check for enable options
dnl  OZ_ENABLE(name,comment,default, on_yes, on_no)
dnl   tests for the option --enable/disable-name
dnl   the 'default' can be either 'yes' or 'no'
dnl   if the option is enabled 'on_yes' is executed, else 'on_no'
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_ENABLE, [
    AC_MSG_CHECKING(for --enable-$1)
    AC_ARG_ENABLE($1, [--enable-$1 $2 (default=$3)])
    : ${[oz_enable_]translit($1,-,_)=$3}
    if test -n "${[enable_]translit($1,-,_)}"; then
	[oz_enable_]translit($1,-,_)=$[enable_]translit($1,-,_)
    fi
    if test "${[oz_enable_]translit($1,-,_)}" != no; then
	ifelse($4,[],AC_MSG_RESULT(yes),$4)
    else
	ifelse($5,[],AC_MSG_RESULT(no),$5)
    fi
    ])

dnl ------------------------------------------------------------------
dnl OZ_COMPILE_ELISP
dnl	check for --enable-compile-elisp
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_COMPILE_ELISP,
  [OZ_ENABLE(compile-elisp,[whether to compile elisp files],yes,
	COMPILE_ELISP=yes,
	COMPILE_ELISP=no)
   AC_MSG_RESULT($COMPILE_ELISP)
   AC_SUBST(COMPILE_ELISP)])

dnl ------------------------------------------------------------------
dnl OZ_EMACS
dnl	tries to locate emacs or xemacs
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_EMACS,[
  AC_CHECK_PROGS(THEEMACS, emacs xemacs, emacs)
  AC_SUBST(THEEMACS)])

dnl ------------------------------------------------------------------
dnl OZ_EMACS_OPTIONS
dnl	check with what options to start an emacs subprocess, e.g.
dnl	to perform highlighting when processing the documentation
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_EMACS_OPTIONS,[
  AC_REQUIRE([OZ_EMACS])
  AC_MSG_CHECKING([for --with-emacs-options])
  AC_ARG_WITH(emacs-options,
    [--with-emacs-options=OPTIONS command-[line] options for emacs subprocess (default: -q --no-site-[file])],
    [oz_cv_emacs_options="$with_emacs_options"],
    [ if $THEEMACS --version 2>&1 | egrep XEmacs >/dev/null; then
        oz_cv_emacs_options="-q -no-site-[file]"
      else
        oz_cv_emacs_options="-q --no-site-[file]"
      fi ])
  EMACS_OPTIONS="$oz_cv_emacs_options"
  AC_SUBST(EMACS_OPTIONS)
  AC_MSG_RESULT($EMACS_OPTIONS)])

dnl ------------------------------------------------------------------
dnl OZ_DENYS_EVENTS
dnl	check for --enable-denys-events
dnl sets DENYS_EVENTS accordingly
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_DENYS_EVENTS,
  [OZ_ENABLE(denys-events,[enable new events mechanism],no,
	DENYS_EVENTS=yes,
	DENYS_EVENTS=no)
   AC_MSG_RESULT($DENYS_EVENTS)
   AC_SUBST(DENYS_EVENTS)])

dnl ------------------------------------------------------------------
dnl OZ_SITE_PROPERTY
dnl	check for --enable-site-property
dnl sets SITE_PROPERTY accordingly
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_SITE_PROPERTY,
  [OZ_ENABLE(site-property,[enable site property support],no,
	SITE_PROPERTY=yes,
	SITE_PROPERTY=no)
   AC_MSG_RESULT($SITE_PROPERTY)
   AC_SUBST(SITE_PROPERTY)])

dnl ------------------------------------------------------------------
dnl OZ_ARG_WITH_LIB_DIR
dnl
dnl maybe adds some directories to LDFLAGS and oz_lib_path
dnl --with-lib-dirs=d1,...,dn
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_ARG_WITH_LIB_DIR,[
  AC_MSG_CHECKING(for --with-lib-dir)
  AC_ARG_WITH(lib-dir,
    [--with-lib-dir=d1,...,dn	add these dirs to LDFLAGS],
    [oz_cv_with_lib_dirs=$with_lib_dir],
    [: ${oz_cv_with_lib_dirs=$oz_with_lib_dir}])
  AC_MSG_RESULT($with_lib_dir)
  oz_tmp_IFS="$IFS"
  IFS=","
  # reverse the list of dirs
  oz_tmp_dirs=""
  for oz_tmp in $oz_cv_with_lib_dirs DONE; do
    if test "$oz_tmp" != DONE; then
      oz_tmp_dirs="$oz_tmp${oz_tmp_dirs:+ }$oz_tmp_dirs"
    fi
  done
  # add them to LDFLAGS and oz_lib_path
  IFS=$oz_tmp_IFS
  : ${oz_lib_path=""}
  for oz_tmp1 in $oz_tmp_dirs DONE; do
    if test "$oz_tmp1" != DONE; then
      OZ_ADDTO_LDFLAGS(-L$oz_tmp1)
      oz_lib_path="$oz_tmp1${oz_lib_path:+ }$oz_lib_path"
    fi
  done
])

dnl ------------------------------------------------------------------
dnl OZ_ARG_WITH_INC_DIR
dnl
dnl maybe adds some directories to CPPFLAGS
dnl --with-inc-dirs=d1,...,dn
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_ARG_WITH_INC_DIR,[
  AC_MSG_CHECKING(for --with-inc-dir)
  AC_ARG_WITH(inc-dir,
    [--with-inc-dir=d1,...,dn	add these dirs to CPPFLAGS],
    [oz_cv_with_inc_dirs=$with_inc_dir],
    [: ${oz_cv_with_inc_dirs=$oz_with_inc_dir}])
  AC_MSG_RESULT($with_inc_dir)
  oz_tmp_IFS="$IFS"
  IFS=","
  # reverse the list of dirs
  oz_tmp_dirs=""
  for oz_tmp in $oz_cv_with_inc_dirs DONE; do
    if test "$oz_tmp" != DONE; then
      oz_tmp_dirs="$oz_tmp${oz_tmp_dirs:+ }$oz_tmp_dirs"
    fi
  done
  # add them to CPPFLAGS and oz_inc_path
  IFS=$oz_tmp_IFS
  : ${oz_inc_path=""}
  for oz_tmp1 in $oz_tmp_dirs DONE; do
    if test "$oz_tmp1" != DONE; then
      OZ_ADDTO_CPPFLAGS(-I$oz_tmp1)
      oz_inc_path="$oz_tmp1${oz_inc_path:+ }$oz_inc_path"
    fi
  done
])

dnl ------------------------------------------------------------------
dnl defines some vars that need special care under Windows
dnl
dnl OZ_OZLOADSEP
dnl   define OZLOADSEP to be ":" on Unix and ";" on Windows
dnl
dnl OZ_LOADWIN
dnl
dnl on Unix we typically added to OZ_LOAD specs like prefix=/=/ and
dnl prefix=./=./ to say that absolute paths should be tried as such
dnl but on Windows these specs don't match path that begin with a
dnl drive.  Instead we need to add rules like prefix=y:/=y:/ and for
dnl robustness, we need to add both the lowercase and the uppercase
dnl versions.  We define variable OZLOADWIN to contain such specs
dnl both for the srcdir and for the build dir; each spec is prefixed
dnl by the character `%' which serves as the portable path separator
dnl to be subsequently substituted by the platform specific one.
dnl OZLOADWIN can be cached because the drives for source dir and
dnl build dir are the same for the entire build.
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_OZLOADSEP,[
  AC_CACHE_CHECK([for OZLOADSEP],oz_cv_OZLOADSEP,[
    case $PLATFORM in
    win32*) oz_cv_OZLOADSEP=';'
    ;;
    *) oz_cv_OZLOADSEP=':'
    ;;
    esac
  ])
  OZLOADSEP="$oz_cv_OZLOADSEP";
  AC_SUBST(OZLOADSEP)
])

AC_DEFUN(OZ_OZLOADWIN,[
  AC_CACHE_CHECK([for OZLOADWIN],oz_cv_OZLOADWIN,[
changequote(<,>)
  case $PLATFORM in
    win32*)
	oztmp=`pwd`
	oztmp=`cygpath -a -w "$oztmp"`
	oztmp=`expr "$oztmp" : "\(.\):"`
	if test -n "$oztmp"; then
	  oztmplo=`echo $oztmp | tr '[:upper:]' '[:lower:]'`
	  oztmphi=`echo $oztmp | tr '[:lower:]' '[:upper:]'`
	  oz_cv_OZLOADWIN="%prefix=${oztmplo}:/=${oztmplo}:/%prefix=${oztmphi}:/=${oztmphi}:/"
	fi
	oztmp=`cygpath -a -w "$srcdir"`
	oztmp=`expr "$oztmp" : "\(.\):"`
	if test -n "$oztmp"; then
	  oztmplo=`echo $oztmp | tr '[:upper:]' '[:lower:]'`
	  oztmphi=`echo $oztmp | tr '[:lower:]' '[:upper:]'`
	  oz_cv_OZLOADWIN="${oz_cv_OZLOADWIN}%prefix=${oztmplo}:/=${oztmplo}:/%prefix=${oztmphi}:/=${oztmphi}:/"
	fi
	;;
    *) oz_cv_OZLOADWIN=
	;;
  esac
changequote([,])
  ])
  OZLOADWIN="$oz_cv_OZLOADWIN"
  AC_SUBST(OZLOADWIN)
])
