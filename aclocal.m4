AC_DEFUN(OZ_INIT, [
    AC_PREFIX_DEFAULT(/usr/local/oz)

    AC_CANONICAL_HOST

    if test -z "$TOPDIR"
    then
	TOPDIR=$srcdir
	if test ! -r $TOPDIR/OZVERSION
	then
	    TOPDIR=$srcdir/..
	fi
    fi
    if test ! -r $TOPDIR/OZVERSION
    then
	AC_MSG_ERROR([can't find TOPDIR])
    fi
    AC_SUBST(TOPDIR)
    oz_topdira=`cd $TOPDIR; pwd`
    ])

AC_DEFUN(OZ_PATH_PROG, [
    AC_PATH_PROG($1,$2,,$PATH:$oz_topdira/bin:$oz_topdira)
    if test ! -n "$$1"
    then
	$1=undefined
	$3
    fi
    ])
