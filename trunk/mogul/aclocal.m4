AC_DEFUN(MOZART_PATH,[
  srcdir=`cd $srcdir 2> /dev/null && pwd`
  VAR_PATH=${srcdir}:${PATH}:${HOME}/.oz/bin:${OZHOME}/bin:/usr/local/mozart/bin:/usr/local/oz/bin
])

AC_DEFUN(MOZART_PATH_PROG, [
    dummy_PWD=`pwd | sed 's/\//\\\\\//g'`
    dummy_PATH=`echo $VAR_PATH | sed -e 's/:://g'`
    dummy_PATH=`echo $dummy_PATH | sed -e "s/^\.:/$dummy_PWD:/g"`
    dummy_PATH=`echo $dummy_PATH | sed -e "s/^\.\//$dummy_PWD\//g"`
    dummy_PATH=`echo $dummy_PATH | sed -e "s/:\.\$/:$dummy_PWD/g"`
    dummy_PATH=`echo $dummy_PATH | sed -e "s/:\.:/:$dummy_PWD:/g"`
    dummy_PATH=`echo $dummy_PATH | sed -e "s/:.\//:$dummy_PWD\//g"`
    mozart_for="$dummy_PATH"
    AC_PATH_PROG($1,$2,,$mozart_for)
    case "$host_os" in
      cygwin32) ;;
      *) $1=`echo $$1 | sed -e "s|//|/|g"`;;
    esac
    if test ! -n "$$1"
    then
	$1=undefined
	ifelse([$3],[],[AC_MSG_ERROR([$2 not found])],$3)
    fi])

dnl this assumes that VAR_OZE is set to the full path of the
dnl ozengine.  OZHOME is derived from it.
AC_DEFUN(MOZART_OZHOME,[
  if test "${OZHOME:-NONE}" = NONE; then
changequote(<,>)
    OZHOME=`expr "${VAR_OZE}" : '\(.*\)/[^/]*/[^/]*$' || echo "."`
changequote([,])
  fi
  AC_SUBST(OZHOME)
])

AC_DEFUN(MOZART_PROG_INSTALL,[
  if test "${INSTALL+set}" = set; then
    AC_CACHE_CHECK([whether to unset broken INSTALL],
      mozart_cv_unset_INSTALL,[
        echo >conftest.$$
        if $INSTALL -c -m 644 conftest.$$ /tmp >/dev/null 2>&1; then
          rm -f /tmp/conftest.$$ 2>/dev/null
          mozart_cv_unset_INSTALL=no
        else
          mozart_cv_unset_INSTALL=yes
        fi
        rm -f conftest.$$ 2>/dev/null])
    test "$mozart_cv_unset_INSTALL" = yes && unset INSTALL
  fi
  AC_PROG_INSTALL
  MOZART_PATH_PROG(VAR_INSTALL_DIR,mkinstalldirs)])

AC_DEFUN(MOZART_INIT,[
  AC_CANONICAL_HOST
  AC_PROG_MAKE_SET
  MOZART_PATH
  MOZART_PROG_INSTALL
  MOZART_PATH_PROG(VAR_OZTOOL,oztool)
  MOZART_PATH_PROG(VAR_OZC,ozc)
  MOZART_PATH_PROG(VAR_OZL,ozl)
  MOZART_PATH_PROG(VAR_OZE,ozengine)
  MOZART_OZHOME
])
