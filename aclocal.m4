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
dnl     http://mozart.ps.uni-sb.de
dnl 
dnl  See the file "LICENSE" or
dnl     http://mozart.ps.uni-sb.de/LICENSE.html
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
  AC_REQUIRE([OZ_WORKING_TEST])
  oz_tmp_dir=[$2]
  oz_tmp_ok=
  for oz_tmp1 in [$3]; do
    for oz_tmp2 in $oz_tmp_dir \
		   $oz_tmp_dir/.. \
		   $oz_tmp_dir/../.. \
		   $oz_tmp_dir/../../.. \
		   $oz_tmp_dir/../../../.. \
		   $oz_tmp_dir/../../../../..; do
      if $TEST -e "$oz_tmp2/$oz_tmp1"; then
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
dnl for a directory called mozart
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_PATH_BUILDTOP,[
  AC_CACHE_CHECK([for BUILDTOP],oz_cv_path_BUILDTOP,[
    OZ_PATH_UPWARD(oz_cv_path_BUILDTOP,.,[contrib config.cache])
    if test -z "$oz_cv_path_BUILDTOP"; then
      AC_MSG_ERROR([cannot find BUILDTOP])
    fi])
  BUILDTOP=$oz_cv_path_BUILDTOP
  AC_SUBST(BUILDTOP)])

AC_DEFUN(OZ_INIT, [
  AC_PREFIX_DEFAULT(/usr/local/oz)
  OZ_PATH_SRCDIR
  OZ_PATH_SRCTOP
  OZ_PATH_BUILDTOP
  AC_PROG_MAKE_SET
  AC_PROG_INSTALL
  OZ_PATH_PROG(INSTALL_DIR,  mkinstalldirs)
#OZ_PATH_PROG(PLATFORMSCRIPT, ozplatform)
#OZ_PATH_PROG(DYNLD,          ozdynld)
])

dnl ------------------------------------------------------------------
dnl OZ_PROG_TEST
dnl
dnl finds working version of test, sets TEST
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_WORKING_TEST,[
  AC_CACHE_CHECK([for test],oz_cv_TEST,[
    if test -e . 2>/dev/null; then
      oz_cv_TEST=test
    else
      oz_tmp_IFS="$IFS"
      IFS="$IFS:"
      for oz_tmp in $PATH; do
        if $oz_tmp/test -e . 2>/dev/null; then
          oz_tmp=`cd $oz_tmp && pwd`
          oz_cv_TEST="$oz_tmp/test"
          break
        fi
      done
      IFS=$oz_tmp_IFS
      if test -n "$oz_cv_TEST"; then
        AC_MSG_ERROR([cannot locate a working test])
      fi
    fi])
  TEST=$oz_cv_TEST
  AC_SUBST(TEST)])

dnl ==================================================================
dnl VERSION CHECKING
dnl ==================================================================

dnl ------------------------------------------------------------------
dnl OZ_CHECK_VERSION(OK,GOT,WANT)
dnl
dnl GOT and WANT consist of integers separated by single periods, e.g.
dnl 23.1 or 3.5.7, and OK is set to yes or no according to whether
dnl GOT >= WANT with respect to the obvious ordering.
dnl
dnl for example
dnl	OZ_CHECK_VERSION(OK,$VERSION,2.5.4)
dnl sets OK to yes or no, according to whether $VERSION is greater
dnl than 2.5.4
dnl
dnl IFS="$IFS." gives me the implicit break at periods in both the
dnl `set' and the `for' statements.  Actually I am now using
dnl `.' and `_' for breaks (to accommodate perl).
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

AC_DEFUN(OZ_CHECK_VERSION,[
  oz_tmp_got="[$2]"
  oz_tmp_want="[$3]"
  oz_tmp_IFS="$IFS"
  IFS="$IFS._"
  set $oz_tmp_got DONE
  oz_tmp__ok=yes
  for oz_tmp_cur in $oz_tmp_want; do
    if test "$[1]" = DONE; then
      break
    elif test "$oz_tmp_cur" -lt "$[1]"; then
      break
    elif test "$oz_tmp_cur" -gt "$[1]"; then
      oz_tmp__ok=no
      break
    fi
    [shift]
  done
  IFS=$oz_tmp_IFS
  [$1]=$oz_tmp__ok])

dnl ------------------------------------------------------------------
dnl OZ_PROG_VERSION_CHECK(VAR,PROG,VERSION)
dnl
dnl gets the --version of PROG and compares it to VERSION
dnl sets VAR to yes or no accordingly
dnl ------------------------------------------------------------------

AC_DEFUN(OZ_PROG_VERSION_CHECK,[
  AC_MSG_CHECKING([$2 version is at least $3])
  [$1]=no
  if oz_tmp_version=`[$2] --version 2>/dev/null`; then
changequote(<,>)
    oz_tmp_version=`expr "$oz_tmp_version" : '.*version \([0-9._]\+\)'`
changequote([,])
    if test -n "$oz_tmp_version"; then
      OZ_CHECK_VERSION([$1],$oz_tmp_version,[$3])
    fi
  fi
  if test -z "$oz_tmp_version"; then
    AC_MSG_RESULT([no (cannot find version)])
  else
    AC_MSG_RESULT($[$1])
  fi])

dnl ==================================================================
dnl CHOOSE C++ COMPILER
dnl
dnl choose only if the choice is not already in the cache.
dnl WE DONT DO THIS ANYMORE
dnl -- note that CXXFLAGS is set to the empty string to avoid that it be
dnl -- set by AC_PROG_CXX (we take care of our own defaults)
dnl ==================================================================

AC_DEFUN(OZ_VERSION_GXX,[2.7])
AC_DEFUN(OZ_CXX_CHOOSE,[
  if test -z "$oz_cv_cxx__chosen"; then
dnl    CXXFLAGS=
    OZ_ARG_WITH_CXX
    AC_PROG_CXX
    if test "${GXX}" = yes; then
      if oz_tmp=`$CXX --version 2>/dev/null`; then
        if expr "$oz_tmp" : egcs >/dev/null; then
dnl I don't know what the appropriate version number is for egcs
          :
changequote(<,>)
        elif oz_tmp=`expr "$oz_tmp" : '\([0-9.]\+\)'`; then
changequote([,])
          AC_MSG_CHECKING($CXX version is at least OZ_VERSION_GXX)
          OZ_CHECK_VERSION(oz_tmp_ok,$oz_tmp,OZ_VERSION_GXX)
          AC_MSG_RESULT($oz_tmp_ok)
          if test "$oz_tmp_ok" = no; then
            AC_MSG_ERROR([
configure found the GNU C++ compiler $CXX version $oz_tmp
but version] OZ_VERSION_GXX [or higher is required to build the
system.  It can be retrieved from:

	ftp://ftp.gnu.org/pub/gnu/

The latest version at this time is 2.8.1 and is available
packaged as the following archive:

	gcc-2.8.1.tar.gz 

You may find a mirror archive closer to you by consulting:

	http://www.gnu.org/order/ftp.html
])
          fi
        else
          AC_MSG_WARN([Could not check $CXX version, assuming ok])
        fi
      else
        AC_MSG_WARN([Could not check $CXX version, assuming ok])
      fi
    fi
    AC_PROG_CXXCPP
    oz_cv_CXX=$CXX
    oz_cv_CXXCPP=$CXXCPP
    oz_cv_GXX=$GXX
    oz_cv_cxx__chosen=yes
  else
    OZ_FROM_CACHE(CXX,[for C++ compiler])
    OZ_FROM_CACHE(GXX,[whether we are using GNU C++])
    OZ_FROM_CACHE(CXXCPP,[for C++ preprocessor])
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
  OZ_MSG_FRONT([Looking up in cache $2])
  [$1]=$oz_cv_$1
  AC_MSG_RESULT([$$1])])

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
])

dnl ==================================================================
dnl CHOOSE C COMPILER
dnl ==================================================================

AC_DEFUN(OZ_VERSION_GCC,[2.7])
AC_DEFUN(OZ_CC_CHOOSE,[
  if test -z "$oz_cv_cc__chosen"; then
    CFLAGS=
    AC_PROG_CC
    if test "$GCC" = yes; then
      if oz_tmp=`$CC --version 2>/dev/null`; then
        if expr "$oz_tmp" : egcs >/dev/null; then
dnl I don't know what the appropriate version number is for egcs
          :
changequote(<,>)
        elif oz_tmp=`expr "$oz_tmp" : '\([0-9.]\+\)'`; then
changequote([,])
          AC_MSG_CHECKING($CC version is at least OZ_VERSION_GCC)
          OZ_CHECK_VERSION(oz_tmp_ok,$oz_tmp,OZ_VERSION_GCC)
          AC_MSG_RESULT($oz_tmp_ok)
          if test "$oz_tmp_ok" = no; then
            AC_MSG_ERROR([
configure found the GNU C compiler $CC version $oz_tmp
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
        else
          AC_MSG_WARN([Could not check $CC version, assuming ok])
        fi
      else
        AC_MSG_WARN([Could not check $CC version, assuming ok])
      fi
    fi
    AC_PROG_CPP
    oz_cv_CC=$CC
    oz_cv_CPP=$CPP
    oz_cv_GCC=$GCC
    oz_cv_cc__chosen=yes
  else
    OZ_FROM_CACHE(CC,[for C compiler])
    OZ_FROM_CACHE(GCC,[whether we are using GNU C])
    OZ_FROM_CACHE(CPP,[for C preprocessor])
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
  if `$LEX -S --version 2>/dev/null >/dev/null`; then
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
     oz_tmp=`expr "$oz_tmp" : GNU`; then
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
      AC_MSG_ERROR([
GNU bison version] OZ_VERSION_BISON [or higher is needed to build the system.
It can be retrieved from:

	ftp://ftp.gnu.org/pub/gnu/

The latest version at this time is 1.25 and is available
packaged as the following archive:

	bison-1.25.tar.gz


You may find a mirror archive closer to you by consulting:

	http://www.gnu.org/order/ftp.html
])
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
      OZ_PROG_VERSION_CHECK(oz_tmp_ok,$PERL,OZ_VERSION_PERL)
    fi
    if test "$oz_tmp_ok" = yes; then
      oz_cv_PERL=$PERL
    else
      AC_MSG_ERROR([
Perl version] OZ_VERSION_PERL [or higher is needed to build the system.
It can be retrieved from:

	http://language.perl.com/info/software.html

The latest version at this time is 5.004_04 and is available
packaged as the following archive:

	http://language.perl.com/CPAN/src/latest.tar.gz

You may find further information on the Perl site:

	http://www.perl.org/
])
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
        AC_MSG_ERROR([
GNU m4 is needed to build the system.
It can be retrieved from:

	ftp://ftp.gnu.org/pub/gnu/

The latest version at this time is 1.4 and is available
packaged as the following archive:

	m4-1.4.tar.gz

You may find a mirror archive closer to you by consulting:

	http://www.gnu.org/order/ftp.html
])
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
  if oz_tmp=`$MAKE --version 2>/dev/null` && \
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

	ftp://ftp.gnu.org/pub/gnu/

The latest version at this time is 3.77 and is available
packaged as the following archive:

	make-3.77.tar.gz

You may find a mirror archive closer to you by consulting:

	http://www.gnu.org/order/ftp.html
])
    fi
  else
    OZ_FROM_CACHE(MAKE,[for GNU make])
  fi
  AC_SUBST(MAKE)])

dnl ==================================================================
dnl OLD STUFF
dnl ==================================================================

AC_DEFUN(OZ_PATH_PROG, [
    dummy_PWD=`pwd | sed 's/\//\\\\\//g'`
    dummy_PATH=`echo $PATH | sed -e 's/:://g'`
    dummy_PATH=`echo $dummy_PATH | sed -e "s/^\.:/$dummy_PWD:/g"`
    dummy_PATH=`echo $dummy_PATH | sed -e "s/^\.\//$dummy_PWD\//g"`
    dummy_PATH=`echo $dummy_PATH | sed -e "s/:\.\$/:$dummy_PWD/g"`
    dummy_PATH=`echo $dummy_PATH | sed -e "s/:\.:/:$dummy_PWD:/g"`
    dummy_PATH=`echo $dummy_PATH | sed -e "s/:.\//:$dummy_PWD\//g"`
    AC_PATH_PROG($1,$2,,$dummy_PATH:$SRCTOP/share/bin:$SRCTOP)
    $1=`echo $$1 | sed -e "s|//|/|g"`
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
		if test "${enable_link_static}" = yes
		then
			LIBS="-Xlinker -Bstatic -l$1 -Xlinker -Bdynamic $LIBS"
			OZ_TRY_LINK($2, $3,
				LIBS="-l$1 $oz_saved_LIBS"
				OZ_TRY_LINK($2,$3,
					LIBS=$oz_saved_LIBS
					$4)
				)
		else
			LIBS="-l$1 $oz_saved_LIBS"
			OZ_TRY_LINK($2,$3,
				LIBS=$oz_saved_LIBS
				$4)
		fi)
	])

AC_DEFUN(OZ_CXX_OPTIONS, [
	ozm_out=
	if test -n "$1"
	then
	    echo 'void f(){}' > oz_conftest.c
	    for ozm_opt in $1
	    do
		AC_MSG_CHECKING(compiler option $ozm_opt)
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

AC_DEFUN(OZ_CHECK_LIB_PATH, [
	oz_check_lib_path=no
	oz_saved_LDFLAGS=$LDFLAGS
    	oz_v=`echo $1_$2 | sed -e 's/[[^a-zA-Z0-9_]]/_/g'`
	AC_MSG_CHECKING(for $2 in -l$1 (default))
	OZ_CHECK_LIB($1,$2,
		AC_MSG_RESULT(yes)
		oz_check_lib_path=yes,
		AC_MSG_RESULT(no)
		for p in $oz_lib_path
		do
			LDFLAGS="-L$p $oz_saved_LDFLAGS"
			AC_MSG_CHECKING(for $2 in -L$p -l$1)
			OZ_CHECK_LIB($1,$2,
				AC_MSG_RESULT(yes)
				oz_check_lib_path=yes
				break
				,
				AC_MSG_RESULT(no)
				)
		done)
	if test $oz_check_lib_path = yes
	then
		:
		$3
	else
		LDFLAGS=$oz_saved_LDFLAGS
		$4
	fi
	])

AC_DEFUN(OZ_ADDTO_CPPFLAGS,[
  oz_tmp_ok=yes
  for oz_tmp in $CPPFLAGS NONE; do
    if test "$oz_tmp" = "[$1]"; then
      oz_tmp_ok=no
      break
    fi
  done
  test "$oz_tmp_ok" = yes && CPPFLAGS="$CPPFLAGS${CPPFLAGS:+ }[$1]"
])


AC_DEFUN(OZ_CHECK_HEADER_PATH, [
  AC_CACHE_CHECK([for $1],dnl
changequote(`,')oz_cv_header_`'patsubst($1,[^a-zA-Z0-9],_),
changequote([,])
    [
      oz_tmp_cppflags="$CPPFLAGS"
      oz_tmp_ok=no
      AC_TRY_CPP([#include "$1"],[
        oz_tmp_ok=yes],[
        for oz_tmp in $oz_inc_path; do
          CPPFLAGS="$oz_tmp_cppflags -I$p"
          AC_TRY_CPP([#include "$1"],[
            oz_tmp_ok="-I$p"
            break])
        done])
      CPPFLAGS=$oz_tmp_cppflags
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
    OZ_PATH_PROG(OZC,ozc,OZC=$SRCTOP/share/lib/ozc.sh)
    OZ_PATH_PROG(OZPLATFORM,ozplatform)
    PLATFORM=`$OZPLATFORM`
    AC_SUBST(PLATFORM)
])

AC_DEFUN(OZ_CONTRIB_INIT_CXX,[
    OZ_CONTRIB_INIT
    OZ_CXX_CHOOSE
    if test "${GXX}" = yes; then
      OZ_CXX_OPTIONS(-fno-rtti -fno-exceptions,CXXAVOID)
    else
      CXXAVOID=
    fi
    AC_SUBST(CXXAVOID)
    AC_PROG_CXXCPP
    AC_LANG_CPLUSPLUS
    OZ_PATH_PROG(OZDYNLD,ozdynld)
])
