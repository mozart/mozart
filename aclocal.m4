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

echo $P | sed -e "s/^\.:/$X:/g" | sed -e "s/:\.\$/:$X/g" | sed -e "s/:\.:/:$X:/g" | sed "s/:\.\//:$X\//g"

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
