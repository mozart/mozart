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
