AC_DEFUN(OZ_INIT, [
    AC_PREFIX_DEFAULT(/usr/local/oz)

    AC_CANONICAL_HOST

    if test -z "$TOPDIR"
    then
	for TOPDIR in 	$srcdir \
			$srcdir/.. \
			$srcdir/../.. \
			$srcdir/../../.. \
			$srcdir/../../../..; do
	  if test -r $TOPDIR/OZVERSION
	  then
		break
	  fi
	done
    fi
    if test ! -r $TOPDIR/OZVERSION
    then
	AC_MSG_ERROR([can't find TOPDIR in $TOPDIR])
    fi
    AC_SUBST(TOPDIR)
    oz_topdira=`cd $TOPDIR; pwd`
    TOPDIR=$oz_topdira
    ])

AC_DEFUN(OZ_PATH_PROG, [
    dummy_PWD=`pwd | sed 's/\//\\\\\//g'`
    dummy_PATH=`echo $PATH | sed -e "s/:://g" | sed -e "s/\(^\|:\)\.\(\$\|:\|\/\)/\1$dummy_PWD\2/g"`
    AC_PATH_PROG($1,$2,,$dummy_PATH:$oz_topdira/share/bin:$oz_topdira)
    if test ! -n "$$1"
    then
	$1=undefined
	$3
    fi
    ])
