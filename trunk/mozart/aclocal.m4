dnl -*- sh -*-
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
  oz_tmp_dir=[$2]
  oz_tmp_ok=
  for oz_tmp1 in [$3]; do
    for oz_tmp2 in $oz_tmp_dir \
		   $oz_tmp_dir/.. \
		   $oz_tmp_dir/../.. \
		   $oz_tmp_dir/../../.. \
		   $oz_tmp_dir/../../../.. \
		   $oz_tmp_dir/../../../../..; do
      if test -e "$oz_tmp2/$oz_tmp1"; then
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


dnl derived from tools/gdbm/configurea
AC_DEFUN(OZ_CHECK_HEADER_PATH, [
	my_cppflags="$CPPFLAGS"
	my_found=no
	AC_MSG_CHECKING([for $1 (default)])
	AC_TRY_CPP([#include "$1"],
		AC_MSG_RESULT(yes)
		my_found=yes
		,
		AC_MSG_RESULT(no)
		for p in $oz_inc_path
		do
			AC_MSG_CHECKING([[for $1 in $p]])
			CPPFLAGS="$my_cppflags -I$p"
			AC_TRY_CPP([#include "$1"],
				AC_MSG_RESULT(yes)
				my_found=yes
				break
				,
				AC_MSG_RESULT(no))
		done)
	if test $my_found = yes
	then
	   :
	   $2
	else
	   :
	   CPPFLAGS=$my_cppflags
	   $3
	fi
	])

AC_DEFUN(OZ_CONTRIB_INIT,[
    OZ_INIT
    OZ_PATH_PROG(OZC,ozc,OZC=$SRCTOP/share/lib/ozc.sh)
    OZ_PATH_PROG(OZPLATFORM,ozplatform)
    PLATFORM=`$OZPLATFORM`
    AC_SUBST(PLATFORM)
])

AC_DEFUN(OZ_CONTRIB_INIT_CXX,[
    OZ_CONTRIB_INIT
    AC_PROG_CXX
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
