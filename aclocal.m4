AC_DEFUN(OZ_INIT, [
    AC_PREFIX_DEFAULT(/usr/local/oz)

    AC_CANONICAL_HOST

    if test -z "$SRCTOP"
    then
        for SRCTOP in   $srcdir \
                        $srcdir/.. \
                        $srcdir/../.. \
                        $srcdir/../../.. \
                        $srcdir/../../../..; do
          if test -r $SRCTOP/OZVERSION
          then
                break
          fi
        done
    fi
    if test ! -r $SRCTOP/OZVERSION
    then
        AC_MSG_ERROR([can't find SRCTOP])
    fi
    SRCTOP=`cd $SRCTOP && pwd`
    AC_SUBST(SRCTOP)

    AC_PROG_MAKE_SET
    AC_PROG_INSTALL
    OZ_PATH_PROG(INSTALL_DIR,  mkinstalldirs)
    OZ_PATH_PROG(INSTALL_SRC,  ozinstallsrc)
    #OZ_PATH_PROG(PLATFORMSCRIPT, ozplatform)
    #OZ_PATH_PROG(DYNLD,          ozdynld)
    ])

AC_DEFUN(OZ_PATH_PROG, [
    dummy_PWD=`pwd | sed 's/\//\\\\\//g'`
    dummy_PATH=`echo $PATH | sed -e "s/:://g" | sed -e "s/\(^\|:\)\.\(\$\|:\|\/\)/\1$dummy_PWD\2/g"`
    AC_PATH_PROG($1,$2,,$dummy_PATH:$SRCTOP/share/bin:$SRCTOP)
    if test ! -n "$$1"
    then
        $1=undefined
        $3
    fi
    ])
