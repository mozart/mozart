dnl aclocal.m4 generated automatically by aclocal 1.4

dnl Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

# Configure paths for ESD
# Manish Singh    98-9-30
# stolen back from Frank Belew
# stolen from Manish Singh
# Shamelessly stolen from Owen Taylor

dnl AM_PATH_ESD([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for ESD, and define ESD_CFLAGS and ESD_LIBS
dnl
AC_DEFUN(AM_PATH_ESD,
[dnl 
dnl Get the cflags and libraries from the esd-config script
dnl
AC_ARG_WITH(esd-prefix,[  --with-esd-prefix=PFX   Prefix where ESD is installed (optional)],
            esd_prefix="$withval", esd_prefix="")
AC_ARG_WITH(esd-exec-prefix,[  --with-esd-exec-prefix=PFX Exec prefix where ESD is installed (optional)],
            esd_exec_prefix="$withval", esd_exec_prefix="")
AC_ARG_ENABLE(esdtest, [  --disable-esdtest       Do not try to compile and run a test ESD program],
		    , enable_esdtest=yes)

  if test x$esd_exec_prefix != x ; then
     esd_args="$esd_args --exec-prefix=$esd_exec_prefix"
     if test x${ESD_CONFIG+set} != xset ; then
        ESD_CONFIG=$esd_exec_prefix/bin/esd-config
     fi
  fi
  if test x$esd_prefix != x ; then
     esd_args="$esd_args --prefix=$esd_prefix"
     if test x${ESD_CONFIG+set} != xset ; then
        ESD_CONFIG=$esd_prefix/bin/esd-config
     fi
  fi

  AC_PATH_PROG(ESD_CONFIG, esd-config, no)
  min_esd_version=ifelse([$1], ,0.2.5,$1)
  AC_MSG_CHECKING(for ESD - version >= $min_esd_version)
  no_esd=""
  if test "$ESD_CONFIG" = "no" ; then
    no_esd=yes
  else
    ESD_CFLAGS=`$ESD_CONFIG $esdconf_args --cflags`
    ESD_LIBS=`$ESD_CONFIG $esdconf_args --libs`

    esd_major_version=`$ESD_CONFIG $esd_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    esd_minor_version=`$ESD_CONFIG $esd_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    esd_micro_version=`$ESD_CONFIG $esd_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_esdtest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $ESD_CFLAGS"
      LIBS="$LIBS $ESD_LIBS"
dnl
dnl Now check if the installed ESD is sufficiently new. (Also sanity
dnl checks the results of esd-config to some extent
dnl
      rm -f conf.esdtest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esd.h>

char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.esdtest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_esd_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_esd_version");
     exit(1);
   }

   if (($esd_major_version > major) ||
      (($esd_major_version == major) && ($esd_minor_version > minor)) ||
      (($esd_major_version == major) && ($esd_minor_version == minor) && ($esd_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'esd-config --version' returned %d.%d.%d, but the minimum version\n", $esd_major_version, $esd_minor_version, $esd_micro_version);
      printf("*** of ESD required is %d.%d.%d. If esd-config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If esd-config was wrong, set the environment variable ESD_CONFIG\n");
      printf("*** to point to the correct copy of esd-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}

],, no_esd=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_esd" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$ESD_CONFIG" = "no" ; then
       echo "*** The esd-config script installed by ESD could not be found"
       echo "*** If ESD was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the ESD_CONFIG environment variable to the"
       echo "*** full path to esd-config."
     else
       if test -f conf.esdtest ; then
        :
       else
          echo "*** Could not run ESD test program, checking why..."
          CFLAGS="$CFLAGS $ESD_CFLAGS"
          LIBS="$LIBS $ESD_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include <esd.h>
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding ESD or finding the wrong"
          echo "*** version of ESD. If it is not finding ESD, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means ESD was incorrectly installed"
          echo "*** or that you have moved ESD since it was installed. In the latter case, you"
          echo "*** may want to edit the esd-config script: $ESD_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     ESD_CFLAGS=""
     ESD_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(ESD_CFLAGS)
  AC_SUBST(ESD_LIBS)
  rm -f conf.esdtest
])

# Configure paths for AUDIOFILE
# Bertrand Guiheneuf 98-10-21
# stolen from esd.m4 in esound :
# Manish Singh    98-9-30
# stolen back from Frank Belew
# stolen from Manish Singh
# Shamelessly stolen from Owen Taylor

dnl AM_PATH_AUDIOFILE([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for AUDIOFILE, and define AUDIOFILE_CFLAGS and AUDIOFILE_LIBS
dnl
AC_DEFUN(AM_PATH_AUDIOFILE,
[dnl 
dnl Get the cflags and libraries from the audiofile-config script
dnl
AC_ARG_WITH(audiofile-prefix,[  --with-audiofile-prefix=PFX   Prefix where AUDIOFILE is installed (optional)],
            audiofile_prefix="$withval", audiofile_prefix="")
AC_ARG_WITH(audiofile-exec-prefix,[  --with-audiofile-exec-prefix=PFX Exec prefix where AUDIOFILE is installed (optional)],
            audiofile_exec_prefix="$withval", audiofile_exec_prefix="")
AC_ARG_ENABLE(audiofiletest, [  --disable-audiofiletest       Do not try to compile and run a test AUDIOFILE program],
		    , enable_audiofiletest=yes)

  if test x$audiofile_exec_prefix != x ; then
     audiofile_args="$audiofile_args --exec-prefix=$audiofile_exec_prefix"
     if test x${AUDIOFILE_CONFIG+set} != xset ; then
        AUDIOFILE_CONFIG=$audiofile_exec_prefix/bin/audiofile-config
     fi
  fi
  if test x$audiofile_prefix != x ; then
     audiofile_args="$audiofile_args --prefix=$audiofile_prefix"
     if test x${AUDIOFILE_CONFIG+set} != xset ; then
        AUDIOFILE_CONFIG=$audiofile_prefix/bin/audiofile-config
     fi
  fi

  AC_PATH_PROG(AUDIOFILE_CONFIG, audiofile-config, no)
  min_audiofile_version=ifelse([$1], ,0.2.5,$1)
  AC_MSG_CHECKING(for AUDIOFILE - version >= $min_audiofile_version)
  no_audiofile=""
  if test "$AUDIOFILE_CONFIG" = "no" ; then
    no_audiofile=yes
  else
    AUDIOFILE_LIBS=`$AUDIOFILE_CONFIG $audiofileconf_args --libs`
    AUDIOFILE_CFLAGS=`$AUDIOFILE_CONFIG $audiofileconf_args --cflags`
    audiofile_major_version=`$AUDIOFILE_CONFIG $audiofile_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    audiofile_minor_version=`$AUDIOFILE_CONFIG $audiofile_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    audiofile_micro_version=`$AUDIOFILE_CONFIG $audiofile_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_audiofiletest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $AUDIOFILE_CFLAGS"
      LIBS="$LIBS $AUDIOFILE_LIBS"
dnl
dnl Now check if the installed AUDIOFILE is sufficiently new. (Also sanity
dnl checks the results of audiofile-config to some extent
dnl
      rm -f conf.audiofiletest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <audiofile.h>

char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.audiofiletest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_audiofile_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_audiofile_version");
     exit(1);
   }

   if (($audiofile_major_version > major) ||
      (($audiofile_major_version == major) && ($audiofile_minor_version > minor)) ||
      (($audiofile_major_version == major) && ($audiofile_minor_version == minor) && ($audiofile_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'audiofile-config --version' returned %d.%d.%d, but the minimum version\n", $audiofile_major_version, $audiofile_minor_version, $audiofile_micro_version);
      printf("*** of AUDIOFILE required is %d.%d.%d. If audiofile-config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If audiofile-config was wrong, set the environment variable AUDIOFILE_CONFIG\n");
      printf("*** to point to the correct copy of audiofile-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}

],, no_audiofile=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_audiofile" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$AUDIOFILE_CONFIG" = "no" ; then
       echo "*** The audiofile-config script installed by AUDIOFILE could not be found"
       echo "*** If AUDIOFILE was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the AUDIOFILE_CONFIG environment variable to the"
       echo "*** full path to audiofile-config."
     else
       if test -f conf.audiofiletest ; then
        :
       else
          echo "*** Could not run AUDIOFILE test program, checking why..."
          CFLAGS="$CFLAGS $AUDIOFILE_CFLAGS"
          LIBS="$LIBS $AUDIOFILE_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include <audiofile.h>
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding AUDIOFILE or finding the wrong"
          echo "*** version of AUDIOFILE. If it is not finding AUDIOFILE, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means AUDIOFILE was incorrectly installed"
          echo "*** or that you have moved AUDIOFILE since it was installed. In the latter case, you"
          echo "*** may want to edit the audiofile-config script: $AUDIOFILE_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     AUDIOFILE_CFLAGS=""
     AUDIOFILE_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(AUDIOFILE_CFLAGS)
  AC_SUBST(AUDIOFILE_LIBS)
  rm -f conf.audiofiletest
])

# Checks for availability of various utmp fields
#
# Original code by Bernhard Rosenkraenzer (bero@linux.net.eu.org), 1998.
# Modifications by Timur Bakeyev (timur@gnu.org), 1999.
#

dnl AC_CHECK_UTMP()
dnl Test for presence of the field and define HAVE_UT_UT_field macro
dnl

AC_DEFUN(AC_CHECK_UTMP,[

AC_CHECK_HEADERS(sys/time.h utmp.h utmpx.h)
AC_HEADER_TIME

if test "$ac_cv_header_utmpx_h" = "yes"; then
    AC_DEFINE(UTMP,[struct utmpx])
else
    AC_DEFINE(UTMP,[struct utmp])
fi

dnl some systems (BSD4.4-like) require time.h to be included before utmp.h :/
AC_MSG_CHECKING(for ut_host field in the utmp structure)
AC_TRY_COMPILE([#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#ifdef HAVE_UTMP_H
#include <utmp.h>
#endif
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#endif],[UTMP ut; char *p; p=ut.ut_host;],result=yes,result=no)
if test "$result" = "yes"; then
  AC_DEFINE(HAVE_UT_UT_HOST)
fi
AC_MSG_RESULT($result)

AC_MSG_CHECKING(for ut_pid field in the utmp structure)
AC_TRY_COMPILE([#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#ifdef HAVE_UTMP_H
#include <utmp.h>
#endif
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#endif],[UTMP ut; int i; i=ut.ut_pid;],result=yes,result=no)
if test "$result" = "yes"; then
  AC_DEFINE(HAVE_UT_UT_PID)
fi
AC_MSG_RESULT($result)

AC_MSG_CHECKING(for ut_id field in the utmp structure)
AC_TRY_COMPILE([#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#ifdef HAVE_UTMP_H
#include <utmp.h>
#endif
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#endif],[UTMP ut; char *p; p=ut.ut_id;],result=yes,result=no)
if test "$result" = "yes"; then
  AC_DEFINE(HAVE_UT_UT_ID)
fi
AC_MSG_RESULT($result)

AC_MSG_CHECKING(for ut_name field in the utmp structure)
AC_TRY_COMPILE([#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#ifdef HAVE_UTMP_H
#include <utmp.h>
#endif
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#endif],[UTMP ut; char *p; p=ut.ut_name;],result=yes,result=no)
if test "$result" = "yes"; then
  AC_DEFINE(HAVE_UT_UT_NAME)
fi
AC_MSG_RESULT($result)

AC_MSG_CHECKING(for ut_type field in the utmp structure)
AC_TRY_COMPILE([#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#ifdef HAVE_UTMP_H
#include <utmp.h>
#endif
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#endif],[UTMP ut; int i; i=(int) ut.ut_type;],result=yes,result=no)
if test "$result" = "yes"; then
  AC_DEFINE(HAVE_UT_UT_TYPE)
fi
AC_MSG_RESULT($result)

AC_MSG_CHECKING(for ut_exit.e_termination field in the utmp structure)
AC_TRY_COMPILE([#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#ifdef HAVE_UTMP_H
#include <utmp.h>
#endif
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#endif],[UTMP ut; ut.ut_exit.e_termination=0;],result=yes,result=no)
if test "$result" = "yes"; then
  AC_DEFINE(HAVE_UT_UT_EXIT_E_TERMINATION)
fi
AC_MSG_RESULT($result)

AC_MSG_CHECKING(for ut_user field in the utmp structure)
AC_TRY_COMPILE([#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#ifdef HAVE_UTMP_H
#include <utmp.h>
#endif
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#endif],[UTMP ut; char *p; p=ut.ut_user;],result=yes,result=no)
if test "$result" = "yes"; then
  AC_DEFINE(HAVE_UT_UT_USER)
fi
AC_MSG_RESULT($result)

AC_MSG_CHECKING(for ut_time field in the utmp structure)
AC_TRY_COMPILE([#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#ifdef HAVE_UTMP_H
#include <utmp.h>
#endif
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#endif],[UTMP ut; ut.ut_time=0;],result=yes,result=no)
if test "$result" = "yes"; then
  AC_DEFINE(HAVE_UT_UT_TIME)
fi
AC_MSG_RESULT($result)

AC_MSG_CHECKING(for ut_tv field in the utmp structure)
AC_TRY_COMPILE([#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#ifdef HAVE_UTMP_H
#include <utmp.h>
#endif
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#endif],[UTMP ut; ut.ut_tv.tv_sec=0;],result=yes,result=no)
if test "$result" = "yes"; then
  AC_DEFINE(HAVE_UT_UT_TV)
fi
AC_MSG_RESULT($result)

AC_MSG_CHECKING(for ut_syslen field in the utmp structure)
AC_TRY_COMPILE([#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#ifdef HAVE_UTMP_H
#include <utmp.h>
#endif
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#endif],[UTMP ut; ut.ut_syslen=0;],result=yes,result=no)
if test "$result" = "yes"; then
  AC_DEFINE(HAVE_UT_UT_SYSLEN)
fi
AC_MSG_RESULT($result)

])

AC_DEFUN(AC_CHECK_LASTLOG,[

AC_CHECK_HEADERS(sys/time.h utmp.h lastlog.h)
AC_HEADER_TIME

dnl some systems (BSD4.4-like) require time.h to be included before utmp.h :/
AC_MSG_CHECKING(for existance of lastlog structure)
AC_TRY_COMPILE([#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#ifdef HAVE_UTMP_H
#include <utmp.h>
#endif
#ifdef HAVE_LASTLOG_H
#include <lastlog.h>
#endif],[struct lastlog ll; char *p; p=ll.ll_host;],result=yes,result=no)
if test "$result" = "yes"; then
  AC_DEFINE(HAVE_LASTLOG)
fi
AC_MSG_RESULT($result)

])

# Like AC_CONFIG_HEADER, but automatically create stamp file.

AC_DEFUN(AM_CONFIG_HEADER,
[AC_PREREQ([2.12])
AC_CONFIG_HEADER([$1])
dnl When config.status generates a header, we must update the stamp-h file.
dnl This file resides in the same directory as the config header
dnl that is generated.  We must strip everything past the first ":",
dnl and everything past the last "/".
AC_OUTPUT_COMMANDS(changequote(<<,>>)dnl
ifelse(patsubst(<<$1>>, <<[^ ]>>, <<>>), <<>>,
<<test -z "<<$>>CONFIG_HEADERS" || echo timestamp > patsubst(<<$1>>, <<^\([^:]*/\)?.*>>, <<\1>>)stamp-h<<>>dnl>>,
<<am_indx=1
for am_file in <<$1>>; do
  case " <<$>>CONFIG_HEADERS " in
  *" <<$>>am_file "*<<)>>
    echo timestamp > `echo <<$>>am_file | sed -e 's%:.*%%' -e 's%[^/]*$%%'`stamp-h$am_indx
    ;;
  esac
  am_indx=`expr "<<$>>am_indx" + 1`
done<<>>dnl>>)
changequote([,]))])

# Do all the work for Automake.  This macro actually does too much --
# some checks are only needed if your package does certain things.
# But this isn't really a big deal.

# serial 1

dnl Usage:
dnl AM_INIT_AUTOMAKE(package,version, [no-define])

AC_DEFUN(AM_INIT_AUTOMAKE,
[AC_REQUIRE([AC_PROG_INSTALL])
PACKAGE=[$1]
AC_SUBST(PACKAGE)
VERSION=[$2]
AC_SUBST(VERSION)
dnl test to see if srcdir already configured
if test "`cd $srcdir && pwd`" != "`pwd`" && test -f $srcdir/config.status; then
  AC_MSG_ERROR([source directory already configured; run "make distclean" there first])
fi
ifelse([$3],,
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version number of package]))
AC_REQUIRE([AM_SANITY_CHECK])
AC_REQUIRE([AC_ARG_PROGRAM])
dnl FIXME This is truly gross.
missing_dir=`cd $ac_aux_dir && pwd`
AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)
AC_REQUIRE([AC_PROG_MAKE_SET])])

#
# Check to make sure that the build environment is sane.
#

AC_DEFUN(AM_SANITY_CHECK,
[AC_MSG_CHECKING([whether build environment is sane])
# Just in case
sleep 1
echo timestamp > conftestfile
# Do `set' in a subshell so we don't clobber the current shell's
# arguments.  Must try -L first in case configure is actually a
# symlink; some systems play weird games with the mod time of symlinks
# (eg FreeBSD returns the mod time of the symlink's containing
# directory).
if (
   set X `ls -Lt $srcdir/configure conftestfile 2> /dev/null`
   if test "[$]*" = "X"; then
      # -L didn't work.
      set X `ls -t $srcdir/configure conftestfile`
   fi
   if test "[$]*" != "X $srcdir/configure conftestfile" \
      && test "[$]*" != "X conftestfile $srcdir/configure"; then

      # If neither matched, then we have a broken ls.  This can happen
      # if, for instance, CONFIG_SHELL is bash and it inherits a
      # broken ls alias from the environment.  This has actually
      # happened.  Such a system could not be considered "sane".
      AC_MSG_ERROR([ls -t appears to fail.  Make sure there is not a broken
alias in your environment])
   fi

   test "[$]2" = conftestfile
   )
then
   # Ok.
   :
else
   AC_MSG_ERROR([newly created file is older than distributed files!
Check your system clock])
fi
rm -f conftest*
AC_MSG_RESULT(yes)])

dnl AM_MISSING_PROG(NAME, PROGRAM, DIRECTORY)
dnl The program must properly implement --version.
AC_DEFUN(AM_MISSING_PROG,
[AC_MSG_CHECKING(for working $2)
# Run test in a subshell; some versions of sh will print an error if
# an executable is not found, even if stderr is redirected.
# Redirect stdin to placate older versions of autoconf.  Sigh.
if ($2 --version) < /dev/null > /dev/null 2>&1; then
   $1=$2
   AC_MSG_RESULT(found)
else
   $1="$3/missing $2"
   AC_MSG_RESULT(missing)
fi
AC_SUBST($1)])

# aclocal-include.m4
# 
# This macro adds the name macrodir to the set of directories
# that `aclocal' searches for macros.  

# serial 1

dnl AM_ACLOCAL_INCLUDE(macrodir)
AC_DEFUN([AM_ACLOCAL_INCLUDE],
[
	AM_CONDITIONAL(INSIDE_GNOME_COMMON, test x = y)

	test -n "$ACLOCAL_FLAGS" && ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

	for k in $1 ; do ACLOCAL="$ACLOCAL -I $k" ; done
])

# Define a conditional.

AC_DEFUN(AM_CONDITIONAL,
[AC_SUBST($1_TRUE)
AC_SUBST($1_FALSE)
if $2; then
  $1_TRUE=
  $1_FALSE='#'
else
  $1_TRUE='#'
  $1_FALSE=
fi])


# serial 40 AC_PROG_LIBTOOL
AC_DEFUN(AC_PROG_LIBTOOL,
[AC_REQUIRE([AC_LIBTOOL_SETUP])dnl

# Save cache, so that ltconfig can load it
AC_CACHE_SAVE

# Actually configure libtool.  ac_aux_dir is where install-sh is found.
CC="$CC" CFLAGS="$CFLAGS" CPPFLAGS="$CPPFLAGS" \
LD="$LD" LDFLAGS="$LDFLAGS" LIBS="$LIBS" \
LN_S="$LN_S" NM="$NM" RANLIB="$RANLIB" \
DLLTOOL="$DLLTOOL" AS="$AS" OBJDUMP="$OBJDUMP" \
${CONFIG_SHELL-/bin/sh} $ac_aux_dir/ltconfig --no-reexec \
$libtool_flags --no-verify $ac_aux_dir/ltmain.sh $lt_target \
|| AC_MSG_ERROR([libtool configure failed])

# Reload cache, that may have been modified by ltconfig
AC_CACHE_LOAD

# This can be used to rebuild libtool when needed
LIBTOOL_DEPS="$ac_aux_dir/ltconfig $ac_aux_dir/ltmain.sh"

# Always use our own libtool.
LIBTOOL='$(SHELL) $(top_builddir)/libtool'
AC_SUBST(LIBTOOL)dnl

# Redirect the config.log output again, so that the ltconfig log is not
# clobbered by the next message.
exec 5>>./config.log
])

AC_DEFUN(AC_LIBTOOL_SETUP,
[AC_PREREQ(2.13)dnl
AC_REQUIRE([AC_ENABLE_SHARED])dnl
AC_REQUIRE([AC_ENABLE_STATIC])dnl
AC_REQUIRE([AC_ENABLE_FAST_INSTALL])dnl
AC_REQUIRE([AC_CANONICAL_HOST])dnl
AC_REQUIRE([AC_CANONICAL_BUILD])dnl
AC_REQUIRE([AC_PROG_RANLIB])dnl
AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_PROG_LD])dnl
AC_REQUIRE([AC_PROG_NM])dnl
AC_REQUIRE([AC_PROG_LN_S])dnl
dnl

case "$target" in
NONE) lt_target="$host" ;;
*) lt_target="$target" ;;
esac

# Check for any special flags to pass to ltconfig.
#
# the following will cause an existing older ltconfig to fail, so
# we ignore this at the expense of the cache file... Checking this 
# will just take longer ... bummer!
#libtool_flags="--cache-file=$cache_file"
#
test "$enable_shared" = no && libtool_flags="$libtool_flags --disable-shared"
test "$enable_static" = no && libtool_flags="$libtool_flags --disable-static"
test "$enable_fast_install" = no && libtool_flags="$libtool_flags --disable-fast-install"
test "$ac_cv_prog_gcc" = yes && libtool_flags="$libtool_flags --with-gcc"
test "$ac_cv_prog_gnu_ld" = yes && libtool_flags="$libtool_flags --with-gnu-ld"
ifdef([AC_PROVIDE_AC_LIBTOOL_DLOPEN],
[libtool_flags="$libtool_flags --enable-dlopen"])
ifdef([AC_PROVIDE_AC_LIBTOOL_WIN32_DLL],
[libtool_flags="$libtool_flags --enable-win32-dll"])
AC_ARG_ENABLE(libtool-lock,
  [  --disable-libtool-lock  avoid locking (might break parallel builds)])
test "x$enable_libtool_lock" = xno && libtool_flags="$libtool_flags --disable-lock"
test x"$silent" = xyes && libtool_flags="$libtool_flags --silent"

# Some flags need to be propagated to the compiler or linker for good
# libtool support.
case "$lt_target" in
*-*-irix6*)
  # Find out which ABI we are using.
  echo '[#]line __oline__ "configure"' > conftest.$ac_ext
  if AC_TRY_EVAL(ac_compile); then
    case "`/usr/bin/file conftest.o`" in
    *32-bit*)
      LD="${LD-ld} -32"
      ;;
    *N32*)
      LD="${LD-ld} -n32"
      ;;
    *64-bit*)
      LD="${LD-ld} -64"
      ;;
    esac
  fi
  rm -rf conftest*
  ;;

*-*-sco3.2v5*)
  # On SCO OpenServer 5, we need -belf to get full-featured binaries.
  SAVE_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS -belf"
  AC_CACHE_CHECK([whether the C compiler needs -belf], lt_cv_cc_needs_belf,
    [AC_TRY_LINK([],[],[lt_cv_cc_needs_belf=yes],[lt_cv_cc_needs_belf=no])])
  if test x"$lt_cv_cc_needs_belf" != x"yes"; then
    # this is probably gcc 2.8.0, egcs 1.0 or newer; no need for -belf
    CFLAGS="$SAVE_CFLAGS"
  fi
  ;;

ifdef([AC_PROVIDE_AC_LIBTOOL_WIN32_DLL],
[*-*-cygwin* | *-*-mingw*)
  AC_CHECK_TOOL(DLLTOOL, dlltool, false)
  AC_CHECK_TOOL(AS, as, false)
  AC_CHECK_TOOL(OBJDUMP, objdump, false)
  ;;
])
esac
])

# AC_LIBTOOL_DLOPEN - enable checks for dlopen support
AC_DEFUN(AC_LIBTOOL_DLOPEN, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])])

# AC_LIBTOOL_WIN32_DLL - declare package support for building win32 dll's
AC_DEFUN(AC_LIBTOOL_WIN32_DLL, [AC_BEFORE([$0], [AC_LIBTOOL_SETUP])])

# AC_ENABLE_SHARED - implement the --enable-shared flag
# Usage: AC_ENABLE_SHARED[(DEFAULT)]
#   Where DEFAULT is either `yes' or `no'.  If omitted, it defaults to
#   `yes'.
AC_DEFUN(AC_ENABLE_SHARED, [dnl
define([AC_ENABLE_SHARED_DEFAULT], ifelse($1, no, no, yes))dnl
AC_ARG_ENABLE(shared,
changequote(<<, >>)dnl
<<  --enable-shared[=PKGS]  build shared libraries [default=>>AC_ENABLE_SHARED_DEFAULT],
changequote([, ])dnl
[p=${PACKAGE-default}
case "$enableval" in
yes) enable_shared=yes ;;
no) enable_shared=no ;;
*)
  enable_shared=no
  # Look at the argument we got.  We use all the common list separators.
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:,"
  for pkg in $enableval; do
    if test "X$pkg" = "X$p"; then
      enable_shared=yes
    fi
  done
  IFS="$ac_save_ifs"
  ;;
esac],
enable_shared=AC_ENABLE_SHARED_DEFAULT)dnl
])

# AC_DISABLE_SHARED - set the default shared flag to --disable-shared
AC_DEFUN(AC_DISABLE_SHARED, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
AC_ENABLE_SHARED(no)])

# AC_ENABLE_STATIC - implement the --enable-static flag
# Usage: AC_ENABLE_STATIC[(DEFAULT)]
#   Where DEFAULT is either `yes' or `no'.  If omitted, it defaults to
#   `yes'.
AC_DEFUN(AC_ENABLE_STATIC, [dnl
define([AC_ENABLE_STATIC_DEFAULT], ifelse($1, no, no, yes))dnl
AC_ARG_ENABLE(static,
changequote(<<, >>)dnl
<<  --enable-static[=PKGS]  build static libraries [default=>>AC_ENABLE_STATIC_DEFAULT],
changequote([, ])dnl
[p=${PACKAGE-default}
case "$enableval" in
yes) enable_static=yes ;;
no) enable_static=no ;;
*)
  enable_static=no
  # Look at the argument we got.  We use all the common list separators.
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:,"
  for pkg in $enableval; do
    if test "X$pkg" = "X$p"; then
      enable_static=yes
    fi
  done
  IFS="$ac_save_ifs"
  ;;
esac],
enable_static=AC_ENABLE_STATIC_DEFAULT)dnl
])

# AC_DISABLE_STATIC - set the default static flag to --disable-static
AC_DEFUN(AC_DISABLE_STATIC, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
AC_ENABLE_STATIC(no)])


# AC_ENABLE_FAST_INSTALL - implement the --enable-fast-install flag
# Usage: AC_ENABLE_FAST_INSTALL[(DEFAULT)]
#   Where DEFAULT is either `yes' or `no'.  If omitted, it defaults to
#   `yes'.
AC_DEFUN(AC_ENABLE_FAST_INSTALL, [dnl
define([AC_ENABLE_FAST_INSTALL_DEFAULT], ifelse($1, no, no, yes))dnl
AC_ARG_ENABLE(fast-install,
changequote(<<, >>)dnl
<<  --enable-fast-install[=PKGS]  optimize for fast installation [default=>>AC_ENABLE_FAST_INSTALL_DEFAULT],
changequote([, ])dnl
[p=${PACKAGE-default}
case "$enableval" in
yes) enable_fast_install=yes ;;
no) enable_fast_install=no ;;
*)
  enable_fast_install=no
  # Look at the argument we got.  We use all the common list separators.
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:,"
  for pkg in $enableval; do
    if test "X$pkg" = "X$p"; then
      enable_fast_install=yes
    fi
  done
  IFS="$ac_save_ifs"
  ;;
esac],
enable_fast_install=AC_ENABLE_FAST_INSTALL_DEFAULT)dnl
])

# AC_ENABLE_FAST_INSTALL - set the default to --disable-fast-install
AC_DEFUN(AC_DISABLE_FAST_INSTALL, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
AC_ENABLE_FAST_INSTALL(no)])

# AC_PROG_LD - find the path to the GNU or non-GNU linker
AC_DEFUN(AC_PROG_LD,
[AC_ARG_WITH(gnu-ld,
[  --with-gnu-ld           assume the C compiler uses GNU ld [default=no]],
test "$withval" = no || with_gnu_ld=yes, with_gnu_ld=no)
AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_CANONICAL_HOST])dnl
AC_REQUIRE([AC_CANONICAL_BUILD])dnl
ac_prog=ld
if test "$ac_cv_prog_gcc" = yes; then
  # Check if gcc -print-prog-name=ld gives a path.
  AC_MSG_CHECKING([for ld used by GCC])
  ac_prog=`($CC -print-prog-name=ld) 2>&5`
  case "$ac_prog" in
    # Accept absolute paths.
changequote(,)dnl
    [\\/]* | [A-Za-z]:[\\/]*)
      re_direlt='/[^/][^/]*/\.\./'
changequote([,])dnl
      # Canonicalize the path of ld
      ac_prog=`echo $ac_prog| sed 's%\\\\%/%g'`
      while echo $ac_prog | grep "$re_direlt" > /dev/null 2>&1; do
	ac_prog=`echo $ac_prog| sed "s%$re_direlt%/%"`
      done
      test -z "$LD" && LD="$ac_prog"
      ;;
  "")
    # If it fails, then pretend we aren't using GCC.
    ac_prog=ld
    ;;
  *)
    # If it is relative, then search for the first ld in PATH.
    with_gnu_ld=unknown
    ;;
  esac
elif test "$with_gnu_ld" = yes; then
  AC_MSG_CHECKING([for GNU ld])
else
  AC_MSG_CHECKING([for non-GNU ld])
fi
AC_CACHE_VAL(ac_cv_path_LD,
[if test -z "$LD"; then
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}${PATH_SEPARATOR-:}"
  for ac_dir in $PATH; do
    test -z "$ac_dir" && ac_dir=.
    if test -f "$ac_dir/$ac_prog" || test -f "$ac_dir/$ac_prog$ac_exeext"; then
      ac_cv_path_LD="$ac_dir/$ac_prog"
      # Check to see if the program is GNU ld.  I'd rather use --version,
      # but apparently some GNU ld's only accept -v.
      # Break only if it was the GNU/non-GNU ld that we prefer.
      if "$ac_cv_path_LD" -v 2>&1 < /dev/null | egrep '(GNU|with BFD)' > /dev/null; then
	test "$with_gnu_ld" != no && break
      else
	test "$with_gnu_ld" != yes && break
      fi
    fi
  done
  IFS="$ac_save_ifs"
else
  ac_cv_path_LD="$LD" # Let the user override the test with a path.
fi])
LD="$ac_cv_path_LD"
if test -n "$LD"; then
  AC_MSG_RESULT($LD)
else
  AC_MSG_RESULT(no)
fi
test -z "$LD" && AC_MSG_ERROR([no acceptable ld found in \$PATH])
AC_PROG_LD_GNU
])

AC_DEFUN(AC_PROG_LD_GNU,
[AC_CACHE_CHECK([if the linker ($LD) is GNU ld], ac_cv_prog_gnu_ld,
[# I'd rather use --version here, but apparently some GNU ld's only accept -v.
if $LD -v 2>&1 </dev/null | egrep '(GNU|with BFD)' 1>&5; then
  ac_cv_prog_gnu_ld=yes
else
  ac_cv_prog_gnu_ld=no
fi])
])

# AC_PROG_NM - find the path to a BSD-compatible name lister
AC_DEFUN(AC_PROG_NM,
[AC_MSG_CHECKING([for BSD-compatible nm])
AC_CACHE_VAL(ac_cv_path_NM,
[if test -n "$NM"; then
  # Let the user override the test.
  ac_cv_path_NM="$NM"
else
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}${PATH_SEPARATOR-:}"
  for ac_dir in $PATH /usr/ccs/bin /usr/ucb /bin; do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/nm || test -f $ac_dir/nm$ac_exeext ; then
      # Check to see if the nm accepts a BSD-compat flag.
      # Adding the `sed 1q' prevents false positives on HP-UX, which says:
      #   nm: unknown option "B" ignored
      if ($ac_dir/nm -B /dev/null 2>&1 | sed '1q'; exit 0) | egrep /dev/null >/dev/null; then
	ac_cv_path_NM="$ac_dir/nm -B"
	break
      elif ($ac_dir/nm -p /dev/null 2>&1 | sed '1q'; exit 0) | egrep /dev/null >/dev/null; then
	ac_cv_path_NM="$ac_dir/nm -p"
	break
      else
	ac_cv_path_NM=${ac_cv_path_NM="$ac_dir/nm"} # keep the first match, but
	continue # so that we can try to find one that supports BSD flags
      fi
    fi
  done
  IFS="$ac_save_ifs"
  test -z "$ac_cv_path_NM" && ac_cv_path_NM=nm
fi])
NM="$ac_cv_path_NM"
AC_MSG_RESULT([$NM])
])

# AC_CHECK_LIBM - check for math library
AC_DEFUN(AC_CHECK_LIBM,
[AC_REQUIRE([AC_CANONICAL_HOST])dnl
LIBM=
case "$lt_target" in
*-*-beos* | *-*-cygwin*)
  # These system don't have libm
  ;;
*-ncr-sysv4.3*)
  AC_CHECK_LIB(mw, _mwvalidcheckl, LIBM="-lmw")
  AC_CHECK_LIB(m, main, LIBM="$LIBM -lm")
  ;;
*)
  AC_CHECK_LIB(m, main, LIBM="-lm")
  ;;
esac
])

# AC_LIBLTDL_CONVENIENCE[(dir)] - sets LIBLTDL to the link flags for
# the libltdl convenience library, adds --enable-ltdl-convenience to
# the configure arguments.  Note that LIBLTDL is not AC_SUBSTed, nor
# is AC_CONFIG_SUBDIRS called.  If DIR is not provided, it is assumed
# to be `${top_builddir}/libltdl'.  Make sure you start DIR with
# '${top_builddir}/' (note the single quotes!) if your package is not
# flat, and, if you're not using automake, define top_builddir as
# appropriate in the Makefiles.
AC_DEFUN(AC_LIBLTDL_CONVENIENCE, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
  case "$enable_ltdl_convenience" in
  no) AC_MSG_ERROR([this package needs a convenience libltdl]) ;;
  "") enable_ltdl_convenience=yes
      ac_configure_args="$ac_configure_args --enable-ltdl-convenience" ;;
  esac
  LIBLTDL=ifelse($#,1,$1,['${top_builddir}/libltdl'])/libltdlc.la
  INCLTDL=ifelse($#,1,-I$1,['-I${top_builddir}/libltdl'])
])

# AC_LIBLTDL_INSTALLABLE[(dir)] - sets LIBLTDL to the link flags for
# the libltdl installable library, and adds --enable-ltdl-install to
# the configure arguments.  Note that LIBLTDL is not AC_SUBSTed, nor
# is AC_CONFIG_SUBDIRS called.  If DIR is not provided, it is assumed
# to be `${top_builddir}/libltdl'.  Make sure you start DIR with
# '${top_builddir}/' (note the single quotes!) if your package is not
# flat, and, if you're not using automake, define top_builddir as
# appropriate in the Makefiles.
# In the future, this macro may have to be called after AC_PROG_LIBTOOL.
AC_DEFUN(AC_LIBLTDL_INSTALLABLE, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
  AC_CHECK_LIB(ltdl, main,
  [test x"$enable_ltdl_install" != xyes && enable_ltdl_install=no],
  [if test x"$enable_ltdl_install" = xno; then
     AC_MSG_WARN([libltdl not installed, but installation disabled])
   else
     enable_ltdl_install=yes
   fi
  ])
  if test x"$enable_ltdl_install" = x"yes"; then
    ac_configure_args="$ac_configure_args --enable-ltdl-install"
    LIBLTDL=ifelse($#,1,$1,['${top_builddir}/libltdl'])/libltdl.la
    INCLTDL=ifelse($#,1,-I$1,['-I${top_builddir}/libltdl'])
  else
    ac_configure_args="$ac_configure_args --enable-ltdl-install=no"
    LIBLTDL="-lltdl"
    INCLTDL=
  fi
])

dnl old names
AC_DEFUN(AM_PROG_LIBTOOL, [indir([AC_PROG_LIBTOOL])])dnl
AC_DEFUN(AM_ENABLE_SHARED, [indir([AC_ENABLE_SHARED], $@)])dnl
AC_DEFUN(AM_ENABLE_STATIC, [indir([AC_ENABLE_STATIC], $@)])dnl
AC_DEFUN(AM_DISABLE_SHARED, [indir([AC_DISABLE_SHARED], $@)])dnl
AC_DEFUN(AM_DISABLE_STATIC, [indir([AC_DISABLE_STATIC], $@)])dnl
AC_DEFUN(AM_PROG_LD, [indir([AC_PROG_LD])])dnl
AC_DEFUN(AM_PROG_NM, [indir([AC_PROG_NM])])dnl

dnl This is just to silence aclocal about the macro not being used
ifelse([AC_DISABLE_FAST_INSTALL])dnl

# Add --enable-maintainer-mode option to configure.
# From Jim Meyering

# serial 1

AC_DEFUN(AM_MAINTAINER_MODE,
[AC_MSG_CHECKING([whether to enable maintainer-specific portions of Makefiles])
  dnl maintainer-mode is disabled by default
  AC_ARG_ENABLE(maintainer-mode,
[  --enable-maintainer-mode enable make rules and dependencies not useful
                          (and sometimes confusing) to the casual installer],
      USE_MAINTAINER_MODE=$enableval,
      USE_MAINTAINER_MODE=no)
  AC_MSG_RESULT($USE_MAINTAINER_MODE)
  AM_CONDITIONAL(MAINTAINER_MODE, test $USE_MAINTAINER_MODE = yes)
  MAINT=$MAINTAINER_MODE_TRUE
  AC_SUBST(MAINT)dnl
]
)

dnl GNOME_COMPILE_WARNINGS
dnl Turn on many useful compiler warnings
dnl For now, only works on GCC
AC_DEFUN([GNOME_COMPILE_WARNINGS],[
  AC_ARG_ENABLE(compile-warnings, 
    [  --enable-compile-warnings=[no/minimum/yes]	Turn on compiler warnings.],,enable_compile_warnings=minimum)

  AC_MSG_CHECKING(what warning flags to pass to the C compiler)
  warnCFLAGS=
  if test "x$GCC" != xyes; then
    enable_compile_warnings=no
  fi

  if test "x$enable_compile_warnings" != "xno"; then
    if test "x$GCC" = "xyes"; then
      case " $CFLAGS " in
      *[\ \	]-Wall[\ \	]*) ;;
      *) warnCFLAGS="-Wall -Wunused" ;;
      esac

      ## -W is not all that useful.  And it cannot be controlled
      ## with individual -Wno-xxx flags, unlike -Wall
      if test "x$enable_compile_warnings" = "xyes"; then
	warnCFLAGS="$warnCFLAGS -Wmissing-prototypes -Wmissing-declarations"
      fi
    fi
  fi
  AC_MSG_RESULT($warnCFLAGS)

  AC_ARG_ENABLE(iso-c,
    [  --enable-iso-c          Try to warn if code is not ISO C ],,
    enable_iso_c=no)

  AC_MSG_CHECKING(what language compliance flags to pass to the C compiler)
  complCFLAGS=
  if test "x$enable_iso_c" != "xno"; then
    if test "x$GCC" = "xyes"; then
      case " $CFLAGS " in
      *[\ \	]-ansi[\ \	]*) ;;
      *) complCFLAGS="$complCFLAGS -ansi" ;;
      esac

      case " $CFLAGS " in
      *[\ \	]-pedantic[\ \	]*) ;;
      *) complCFLAGS="$complCFLAGS -pedantic" ;;
      esac
    fi
  fi
  AC_MSG_RESULT($complCFLAGS)
  if test "x$cflags_set" != "xyes"; then
    CFLAGS="$CFLAGS $warnCFLAGS $complCFLAGS"
    cflags_set=yes
    AC_SUBST(cflags_set)
  fi
])

dnl For C++, do basically the same thing.

AC_DEFUN([GNOME_CXX_WARNINGS],[
  AC_ARG_ENABLE(cxx-warnings, 
    [  --enable-cxx-warnings=[no/minimum/yes]	Turn on compiler warnings.],,enable_cxx_warnings=minimum)

  AC_MSG_CHECKING(what warning flags to pass to the C++ compiler)
  warnCXXFLAGS=
  if test "x$GCC" != xyes; then
    enable_compile_warnings=no
  fi
  if test "x$enable_cxx_warnings" != "xno"; then
    if test "x$GCC" = "xyes"; then
      case " $CXXFLAGS " in
      *[\ \	]-Wall[\ \	]*) ;;
      *) warnCXXFLAGS="-Wall -Wno-unused" ;;
      esac

      ## -W is not all that useful.  And it cannot be controlled
      ## with individual -Wno-xxx flags, unlike -Wall
      if test "x$enable_cxx_warnings" = "xyes"; then
	warnCXXFLAGS="$warnCXXFLAGS -Wmissing-prototypes -Wmissing-declarations -Wshadow -Woverloaded-virtual"
      fi
    fi
  fi
  AC_MSG_RESULT($warnCXXFLAGS)

   AC_ARG_ENABLE(iso-cxx,
     [  --enable-iso-cxx          Try to warn if code is not ISO C++ ],,
     enable_iso_cxx=no)

   AC_MSG_CHECKING(what language compliance flags to pass to the C++ compiler)
   complCXXFLAGS=
   if test "x$enable_iso_cxx" != "xno"; then
     if test "x$GCC" = "xyes"; then
      case " $CXXFLAGS " in
      *[\ \	]-ansi[\ \	]*) ;;
      *) complCXXFLAGS="$complCXXFLAGS -ansi" ;;
      esac

      case " $CXXFLAGS " in
      *[\ \	]-pedantic[\ \	]*) ;;
      *) complCXXFLAGS="$complCXXFLAGS -pedantic" ;;
      esac
     fi
   fi
  AC_MSG_RESULT($complCXXFLAGS)
  if test "x$cxxflags_set" != "xyes"; then
    CXXFLAGS="$CXXFLAGS $warnCXXFLAGS $complCXXFLAGS"
    cxxflags_set=yes
    AC_SUBST(cxxflags_set)
  fi
])

dnl GTK_X_CHECKS
dnl
dnl Basic X11 related checks for X11.  At the end, the following will be
dnl defined/changed:
dnl   GTK_{CFLAGS,LIBS}      From AM_PATH_GTK
dnl   CPPFLAGS		     Will include $X_CFLAGS
dnl   XPM_LIBS               -lXpm if Xpm library is present, otherwise ""
dnl
dnl The following configure cache variables are defined (but not used):
dnl   gnome_cv_passdown_{x_libs,X_LIBS,X_CFLAGS}
dnl
AC_DEFUN([GTK_X_CHECKS],
[
	AM_PATH_GTK(1.2.0,,AC_MSG_ERROR(GTK not installed, or gtk-config not in path))
	dnl Hope that GTK_CFLAGS have only -I and -D.  Otherwise, we could
	dnl   test -z "$x_includes" || CPPFLAGS="$CPPFLAGS -I$x_includes"
	dnl
	dnl Use CPPFLAGS instead of CFLAGS because AC_CHECK_HEADERS uses
	dnl CPPFLAGS, not CFLAGS
        CPPFLAGS="$CPPFLAGS $GTK_CFLAGS"

        saved_ldflags="$LDFLAGS"
        LDFLAGS="$LDFLAGS $GTK_LIBS"

	gnome_cv_passdown_x_libs="$GTK_LIBS"
	gnome_cv_passdown_X_LIBS="$GTK_LIBS"
	gnome_cv_passdown_X_CFLAGS="$GTK_CFLAGS"
	gnome_cv_passdown_GTK_LIBS="$GTK_LIBS"

        LDFLAGS="$saved_ldflags $GTK_LIBS"

dnl We are requiring GTK >= 1.1.1, which means this will be fine anyhow.
	USE_DEVGTK=true

dnl	AC_MSG_CHECKING([whether to use features from (unstable) GTK+ 1.1.x])
dnl	AC_EGREP_CPP(answer_affirmatively,
dnl	[#include <gtk/gtkfeatures.h>
dnl	#ifdef GTK_HAVE_FEATURES_1_1_0
dnl	   answer_affirmatively
dnl	#endif
dnl	], dev_gtk=yes, dev_gtk=no)
dnl	if test "$dev_gtk" = "yes"; then
dnl	   USE_DEVGTK=true
dnl	fi
dnl	AC_MSG_RESULT("$dev_gtk")


	XPM_LIBS=""
	AC_CHECK_LIB(Xpm, XpmFreeXpmImage, [XPM_LIBS="-lXpm"], , $x_libs)
	AC_SUBST(XPM_LIBS)

        LDFLAGS="$saved_ldflags"

	AC_PROVIDE([GNOME_X_CHECKS])
])

# Configure paths for GTK+
# Owen Taylor     97-11-3

dnl AM_PATH_GTK([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for GTK, and define GTK_CFLAGS and GTK_LIBS
dnl
AC_DEFUN(AM_PATH_GTK,
[dnl 
dnl Get the cflags and libraries from the gtk-config script
dnl
AC_ARG_WITH(gtk-prefix,[  --with-gtk-prefix=PFX   Prefix where GTK is installed (optional)],
            gtk_config_prefix="$withval", gtk_config_prefix="")
AC_ARG_WITH(gtk-exec-prefix,[  --with-gtk-exec-prefix=PFX Exec prefix where GTK is installed (optional)],
            gtk_config_exec_prefix="$withval", gtk_config_exec_prefix="")
AC_ARG_ENABLE(gtktest, [  --disable-gtktest       Do not try to compile and run a test GTK program],
		    , enable_gtktest=yes)

  for module in . $4
  do
      case "$module" in
         gthread) 
             gtk_config_args="$gtk_config_args gthread"
         ;;
      esac
  done

  if test x$gtk_config_exec_prefix != x ; then
     gtk_config_args="$gtk_config_args --exec-prefix=$gtk_config_exec_prefix"
     if test x${GTK_CONFIG+set} != xset ; then
        GTK_CONFIG=$gtk_config_exec_prefix/bin/gtk-config
     fi
  fi
  if test x$gtk_config_prefix != x ; then
     gtk_config_args="$gtk_config_args --prefix=$gtk_config_prefix"
     if test x${GTK_CONFIG+set} != xset ; then
        GTK_CONFIG=$gtk_config_prefix/bin/gtk-config
     fi
  fi

  AC_PATH_PROG(GTK_CONFIG, gtk-config, no)
  min_gtk_version=ifelse([$1], ,0.99.7,$1)
  AC_MSG_CHECKING(for GTK - version >= $min_gtk_version)
  no_gtk=""
  if test "$GTK_CONFIG" = "no" ; then
    no_gtk=yes
  else
    GTK_CFLAGS=`$GTK_CONFIG $gtk_config_args --cflags`
    GTK_LIBS=`$GTK_CONFIG $gtk_config_args --libs`
    gtk_config_major_version=`$GTK_CONFIG $gtk_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    gtk_config_minor_version=`$GTK_CONFIG $gtk_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    gtk_config_micro_version=`$GTK_CONFIG $gtk_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_gtktest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $GTK_CFLAGS"
      LIBS="$GTK_LIBS $LIBS"
dnl
dnl Now check if the installed GTK is sufficiently new. (Also sanity
dnl checks the results of gtk-config to some extent
dnl
      rm -f conf.gtktest
      AC_TRY_RUN([
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

int 
main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.gtktest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = g_strdup("$min_gtk_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_gtk_version");
     exit(1);
   }

  if ((gtk_major_version != $gtk_config_major_version) ||
      (gtk_minor_version != $gtk_config_minor_version) ||
      (gtk_micro_version != $gtk_config_micro_version))
    {
      printf("\n*** 'gtk-config --version' returned %d.%d.%d, but GTK+ (%d.%d.%d)\n", 
             $gtk_config_major_version, $gtk_config_minor_version, $gtk_config_micro_version,
             gtk_major_version, gtk_minor_version, gtk_micro_version);
      printf ("*** was found! If gtk-config was correct, then it is best\n");
      printf ("*** to remove the old version of GTK+. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If gtk-config was wrong, set the environment variable GTK_CONFIG\n");
      printf("*** to point to the correct copy of gtk-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
    } 
#if defined (GTK_MAJOR_VERSION) && defined (GTK_MINOR_VERSION) && defined (GTK_MICRO_VERSION)
  else if ((gtk_major_version != GTK_MAJOR_VERSION) ||
	   (gtk_minor_version != GTK_MINOR_VERSION) ||
           (gtk_micro_version != GTK_MICRO_VERSION))
    {
      printf("*** GTK+ header files (version %d.%d.%d) do not match\n",
	     GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);
      printf("*** library (version %d.%d.%d)\n",
	     gtk_major_version, gtk_minor_version, gtk_micro_version);
    }
#endif /* defined (GTK_MAJOR_VERSION) ... */
  else
    {
      if ((gtk_major_version > major) ||
        ((gtk_major_version == major) && (gtk_minor_version > minor)) ||
        ((gtk_major_version == major) && (gtk_minor_version == minor) && (gtk_micro_version >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of GTK+ (%d.%d.%d) was found.\n",
               gtk_major_version, gtk_minor_version, gtk_micro_version);
        printf("*** You need a version of GTK+ newer than %d.%d.%d. The latest version of\n",
	       major, minor, micro);
        printf("*** GTK+ is always available from ftp://ftp.gtk.org.\n");
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the gtk-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of GTK+, but you can also set the GTK_CONFIG environment to point to the\n");
        printf("*** correct copy of gtk-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
      }
    }
  return 1;
}
],, no_gtk=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_gtk" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$GTK_CONFIG" = "no" ; then
       echo "*** The gtk-config script installed by GTK could not be found"
       echo "*** If GTK was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the GTK_CONFIG environment variable to the"
       echo "*** full path to gtk-config."
     else
       if test -f conf.gtktest ; then
        :
       else
          echo "*** Could not run GTK test program, checking why..."
          CFLAGS="$CFLAGS $GTK_CFLAGS"
          LIBS="$LIBS $GTK_LIBS"
          AC_TRY_LINK([
#include <gtk/gtk.h>
#include <stdio.h>
],      [ return ((gtk_major_version) || (gtk_minor_version) || (gtk_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding GTK or finding the wrong"
          echo "*** version of GTK. If it is not finding GTK, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"
          echo "***"
          echo "*** If you have a RedHat 5.0 system, you should remove the GTK package that"
          echo "*** came with the system with the command"
          echo "***"
          echo "***    rpm --erase --nodeps gtk gtk-devel" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means GTK was incorrectly installed"
          echo "*** or that you have moved GTK since it was installed. In the latter case, you"
          echo "*** may want to edit the gtk-config script: $GTK_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     GTK_CFLAGS=""
     GTK_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(GTK_CFLAGS)
  AC_SUBST(GTK_LIBS)
  rm -f conf.gtktest
])

dnl
dnl And better, use gthreads instead...
dnl

AC_DEFUN([GNOME_PTHREAD_CHECK],[
	PTHREAD_LIB=""
	AC_CHECK_LIB(pthread, pthread_create, PTHREAD_LIB="-lpthread",
		[AC_CHECK_LIB(pthreads, pthread_create, PTHREAD_LIB="-lpthreads",
		    [AC_CHECK_LIB(c_r, pthread_create, PTHREAD_LIB="-lc_r",
			[AC_CHECK_FUNC(pthread_create)]
		    )]
		)]
	)
	AC_SUBST(PTHREAD_LIB)
	AC_PROVIDE([GNOME_PTHREAD_CHECK])
])

dnl GNOME_SUPPORT_CHECKS
dnl    Check for various support functions needed by the standard
dnl    Gnome libraries.  Sets LIBOBJS, might define some macros.
dnl    This should only be used when building the Gnome libs; 
dnl    Gnome clients should not need this macro.
AC_DEFUN([GNOME_SUPPORT_CHECKS],[
  # we need an `awk' to build `gnomesupport.h'
  AC_REQUIRE([AC_PROG_AWK])

  # this should go away soon
  need_gnome_support=yes

  save_LIBOBJS="$LIBOBJS"
  LIBOBJS=

  AC_CHECK_FUNCS(getopt_long,,LIBOBJS="$LIBOBJS getopt.o getopt1.o")

  # for `scandir'
  AC_HEADER_DIRENT

  # copied from `configure.in' of `libiberty'
  vars="program_invocation_short_name program_invocation_name sys_errlist"
  for v in $vars; do
    AC_MSG_CHECKING([for $v])
    AC_CACHE_VAL(gnome_cv_var_$v,
      [AC_TRY_LINK([int *p;], [extern int $v; p = &$v;],
		   [eval "gnome_cv_var_$v=yes"],
		   [eval "gnome_cv_var_$v=no"])])
    if eval "test \"`echo '$gnome_cv_var_'$v`\" = yes"; then
      AC_MSG_RESULT(yes)
      n=HAVE_`echo $v | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`
      AC_DEFINE_UNQUOTED($n)
    else
      AC_MSG_RESULT(no)
    fi
  done

  AC_REPLACE_FUNCS(memmove mkstemp scandir strcasecmp strerror strndup strnlen)
  AC_REPLACE_FUNCS(strtok_r strtod strtol strtoul vasprintf vsnprintf)

  AC_CHECK_FUNCS(realpath,,LIBOBJS="$LIBOBJS canonicalize.o")

  # to include `error.c' error.c has some HAVE_* checks
  AC_CHECK_FUNCS(vprintf doprnt strerror_r)
  AM_FUNC_ERROR_AT_LINE

  # This is required if we declare setreuid () and setregid ().
  AC_TYPE_UID_T

  # see if we need to declare some functions.  Solaris is notorious for
  # putting functions into the `libc' but not listing them in the headers
  AC_CHECK_HEADERS(string.h strings.h stdlib.h unistd.h dirent.h)
  GCC_NEED_DECLARATIONS(gethostname setreuid setregid getpagesize)
  GCC_NEED_DECLARATION(scandir,[
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
])

  # Turn our LIBOBJS into libtool objects.  This is gross, but it
  # requires changes to autoconf before it goes away.
  LTLIBOBJS=`echo "$LIBOBJS" | sed 's/\.o/.lo/g'`
  AC_SUBST(need_gnome_support)
  AC_SUBST(LTLIBOBJS)

  LIBOBJS="$save_LIBOBJS"
  AM_CONDITIONAL(BUILD_GNOME_SUPPORT, test "$need_gnome_support" = yes)
])

dnl From Jim Meyering.  Use this if you use the GNU error.[ch].
dnl FIXME: Migrate into libit

AC_DEFUN(AM_FUNC_ERROR_AT_LINE,
[AC_CACHE_CHECK([for error_at_line], am_cv_lib_error_at_line,
 [AC_TRY_LINK([],[error_at_line(0, 0, "", 0, "");],
              am_cv_lib_error_at_line=yes,
	      am_cv_lib_error_at_line=no)])
 if test $am_cv_lib_error_at_line = no; then
   LIBOBJS="$LIBOBJS error.o"
 fi
 AC_SUBST(LIBOBJS)dnl
])

dnl See whether we need a declaration for a function.
dnl GCC_NEED_DECLARATION(FUNCTION [, EXTRA-HEADER-FILES])
AC_DEFUN(GCC_NEED_DECLARATION,
[AC_MSG_CHECKING([whether $1 must be declared])
AC_CACHE_VAL(gcc_cv_decl_needed_$1,
[AC_TRY_COMPILE([
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#else
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
$2],
[char *(*pfn) = (char *(*)) $1],
eval "gcc_cv_decl_needed_$1=no", eval "gcc_cv_decl_needed_$1=yes")])
if eval "test \"`echo '$gcc_cv_decl_needed_'$1`\" = yes"; then
  AC_MSG_RESULT(yes)
  gcc_need_declarations="$gcc_need_declarations $1"
  gcc_tr_decl=NEED_DECLARATION_`echo $1 | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`
  AC_DEFINE_UNQUOTED($gcc_tr_decl)
else
  AC_MSG_RESULT(no)
fi
])dnl

dnl Check multiple functions to see whether each needs a declaration.
dnl GCC_NEED_DECLARATIONS(FUNCTION... [, EXTRA-HEADER-FILES])
AC_DEFUN(GCC_NEED_DECLARATIONS,
[for ac_func in $1
do
GCC_NEED_DECLARATION($ac_func, $2)
done
]
)

# Macro to add for using GNU gettext.
# Ulrich Drepper <drepper@cygnus.com>, 1995.
#
# Modified to never use included libintl. 
# Owen Taylor <otaylor@redhat.com>, 12/15/1998
#
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU Public License
# but which still want to provide support for the GNU gettext functionality.
# Please note that the actual code is *not* freely available.

# serial 5

AC_DEFUN(AM_GNOME_WITH_NLS,
  [AC_MSG_CHECKING([whether NLS is requested])
    dnl Default is enabled NLS
    AC_ARG_ENABLE(nls,
      [  --disable-nls           do not use Native Language Support],
      USE_NLS=$enableval, USE_NLS=yes)
    AC_MSG_RESULT($USE_NLS)
    AC_SUBST(USE_NLS)

    USE_INCLUDED_LIBINTL=no

    dnl If we use NLS figure out what method
    if test "$USE_NLS" = "yes"; then
      AC_DEFINE(ENABLE_NLS)
#      AC_MSG_CHECKING([whether included gettext is requested])
#      AC_ARG_WITH(included-gettext,
#        [  --with-included-gettext use the GNU gettext library included here],
#        nls_cv_force_use_gnu_gettext=$withval,
#        nls_cv_force_use_gnu_gettext=no)
#      AC_MSG_RESULT($nls_cv_force_use_gnu_gettext)
      nls_cv_force_use_gnu_gettext="no"

      nls_cv_use_gnu_gettext="$nls_cv_force_use_gnu_gettext"
      if test "$nls_cv_force_use_gnu_gettext" != "yes"; then
        dnl User does not insist on using GNU NLS library.  Figure out what
        dnl to use.  If gettext or catgets are available (in this order) we
        dnl use this.  Else we have to fall back to GNU NLS library.
	dnl catgets is only used if permitted by option --with-catgets.
	nls_cv_header_intl=
	nls_cv_header_libgt=
	CATOBJEXT=NONE

	AC_CHECK_HEADER(libintl.h,
	  [AC_CACHE_CHECK([for gettext in libc], gt_cv_func_gettext_libc,
	    [AC_TRY_LINK([#include <libintl.h>], [return (int) gettext ("")],
	       gt_cv_func_gettext_libc=yes, gt_cv_func_gettext_libc=no)])

	   if test "$gt_cv_func_gettext_libc" != "yes"; then
	     AC_CHECK_LIB(intl, bindtextdomain,
	       [AC_CACHE_CHECK([for gettext in libintl],
		 gt_cv_func_gettext_libintl,
		 [AC_CHECK_LIB(intl, gettext,
		  gt_cv_func_gettext_libintl=yes,
		  gt_cv_func_gettext_libintl=no)],
		 gt_cv_func_gettext_libintl=no)])
	   fi

	   if test "$gt_cv_func_gettext_libc" = "yes" \
	      || test "$gt_cv_func_gettext_libintl" = "yes"; then
	      AC_DEFINE(HAVE_GETTEXT)
	      AM_PATH_PROG_WITH_TEST(MSGFMT, msgfmt,
		[test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], no)dnl
	      if test "$MSGFMT" != "no"; then
		AC_CHECK_FUNCS(dcgettext)
		AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)
		AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
		  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
		AC_TRY_LINK(, [extern int _nl_msg_cat_cntr;
			       return _nl_msg_cat_cntr],
		  [CATOBJEXT=.gmo
		   DATADIRNAME=share],
		  [CATOBJEXT=.mo
		   DATADIRNAME=lib])
		INSTOBJEXT=.mo
	      fi
	    fi

	    # Added by Martin Baulig 12/15/98 for libc5 systems
	    if test "$gt_cv_func_gettext_libc" != "yes" \
	       && test "$gt_cv_func_gettext_libintl" = "yes"; then
	       INTLLIBS=-lintl
	       LIBS=`echo $LIBS | sed -e 's/-lintl//'`
	    fi
	])

        if test "$CATOBJEXT" = "NONE"; then
	  AC_MSG_CHECKING([whether catgets can be used])
	  AC_ARG_WITH(catgets,
	    [  --with-catgets          use catgets functions if available],
	    nls_cv_use_catgets=$withval, nls_cv_use_catgets=no)
	  AC_MSG_RESULT($nls_cv_use_catgets)

	  if test "$nls_cv_use_catgets" = "yes"; then
	    dnl No gettext in C library.  Try catgets next.
	    AC_CHECK_LIB(i, main)
	    AC_CHECK_FUNC(catgets,
	      [AC_DEFINE(HAVE_CATGETS)
	       INTLOBJS="\$(CATOBJS)"
	       AC_PATH_PROG(GENCAT, gencat, no)dnl
#	       if test "$GENCAT" != "no"; then
#		 AC_PATH_PROG(GMSGFMT, gmsgfmt, no)
#		 if test "$GMSGFMT" = "no"; then
#		   AM_PATH_PROG_WITH_TEST(GMSGFMT, msgfmt,
#		    [test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], no)
#		 fi
#		 AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
#		   [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
#		 USE_INCLUDED_LIBINTL=yes
#		 CATOBJEXT=.cat
#		 INSTOBJEXT=.cat
#		 DATADIRNAME=lib
#		 INTLDEPS='$(top_builddir)/intl/libintl.a'
#		 INTLLIBS=$INTLDEPS
#		 LIBS=`echo $LIBS | sed -e 's/-lintl//'`
#		 nls_cv_header_intl=intl/libintl.h
#		 nls_cv_header_libgt=intl/libgettext.h
#              fi
            ])
	  fi
        fi

        if test "$CATOBJEXT" = "NONE"; then
	  dnl Neither gettext nor catgets in included in the C library.
	  dnl Fall back on GNU gettext library.
	  nls_cv_use_gnu_gettext=yes
        fi
      fi

      if test "$nls_cv_use_gnu_gettext" != "yes"; then
        AC_DEFINE(ENABLE_NLS)
      else
         # Unset this variable since we use the non-zero value as a flag.
         CATOBJEXT=
#        dnl Mark actions used to generate GNU NLS library.
#        INTLOBJS="\$(GETTOBJS)"
#        AM_PATH_PROG_WITH_TEST(MSGFMT, msgfmt,
#	  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], msgfmt)
#        AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)
#        AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
#	  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
#        AC_SUBST(MSGFMT)
#	USE_INCLUDED_LIBINTL=yes
#        CATOBJEXT=.gmo
#        INSTOBJEXT=.mo
#        DATADIRNAME=share
#	INTLDEPS='$(top_builddir)/intl/libintl.a'
#	INTLLIBS=$INTLDEPS
#	LIBS=`echo $LIBS | sed -e 's/-lintl//'`
#        nls_cv_header_intl=intl/libintl.h
#        nls_cv_header_libgt=intl/libgettext.h
      fi

      dnl Test whether we really found GNU xgettext.
      if test "$XGETTEXT" != ":"; then
	dnl If it is no GNU xgettext we define it as : so that the
	dnl Makefiles still can work.
	if $XGETTEXT --omit-header /dev/null 2> /dev/null; then
	  : ;
	else
	  AC_MSG_RESULT(
	    [found xgettext program is not GNU xgettext; ignore it])
	  XGETTEXT=":"
	fi
      fi

      # We need to process the po/ directory.
      POSUB=po
    else
      DATADIRNAME=share
      nls_cv_header_intl=intl/libintl.h
      nls_cv_header_libgt=intl/libgettext.h
    fi
    AC_LINK_FILES($nls_cv_header_libgt, $nls_cv_header_intl)
    AC_OUTPUT_COMMANDS(
     [case "$CONFIG_FILES" in *po/Makefile.in*)
        sed -e "/POTFILES =/r po/POTFILES" po/Makefile.in > po/Makefile
      esac])


#    # If this is used in GNU gettext we have to set USE_NLS to `yes'
#    # because some of the sources are only built for this goal.
#    if test "$PACKAGE" = gettext; then
#      USE_NLS=yes
#      USE_INCLUDED_LIBINTL=yes
#    fi

    dnl These rules are solely for the distribution goal.  While doing this
    dnl we only have to keep exactly one list of the available catalogs
    dnl in configure.in.
    for lang in $ALL_LINGUAS; do
      GMOFILES="$GMOFILES $lang.gmo"
      POFILES="$POFILES $lang.po"
    done

    dnl Make all variables we use known to autoconf.
    AC_SUBST(USE_INCLUDED_LIBINTL)
    AC_SUBST(CATALOGS)
    AC_SUBST(CATOBJEXT)
    AC_SUBST(DATADIRNAME)
    AC_SUBST(GMOFILES)
    AC_SUBST(INSTOBJEXT)
    AC_SUBST(INTLDEPS)
    AC_SUBST(INTLLIBS)
    AC_SUBST(INTLOBJS)
    AC_SUBST(POFILES)
    AC_SUBST(POSUB)
  ])

AC_DEFUN(AM_GNOME_GETTEXT,
  [AC_REQUIRE([AC_PROG_MAKE_SET])dnl
   AC_REQUIRE([AC_PROG_CC])dnl
   AC_REQUIRE([AC_PROG_RANLIB])dnl
   AC_REQUIRE([AC_ISC_POSIX])dnl
   AC_REQUIRE([AC_HEADER_STDC])dnl
   AC_REQUIRE([AC_C_CONST])dnl
   AC_REQUIRE([AC_C_INLINE])dnl
   AC_REQUIRE([AC_TYPE_OFF_T])dnl
   AC_REQUIRE([AC_TYPE_SIZE_T])dnl
   AC_REQUIRE([AC_FUNC_ALLOCA])dnl
   AC_REQUIRE([AC_FUNC_MMAP])dnl

   AC_CHECK_HEADERS([argz.h limits.h locale.h nl_types.h malloc.h string.h \
unistd.h sys/param.h])
   AC_CHECK_FUNCS([getcwd munmap putenv setenv setlocale strchr strcasecmp \
strdup __argz_count __argz_stringify __argz_next])

   if test "${ac_cv_func_stpcpy+set}" != "set"; then
     AC_CHECK_FUNCS(stpcpy)
   fi
   if test "${ac_cv_func_stpcpy}" = "yes"; then
     AC_DEFINE(HAVE_STPCPY)
   fi

   AM_LC_MESSAGES
   AM_GNOME_WITH_NLS

   if test "x$CATOBJEXT" != "x"; then
     if test "x$ALL_LINGUAS" = "x"; then
       LINGUAS=
     else
       AC_MSG_CHECKING(for catalogs to be installed)
       NEW_LINGUAS=
       if test "x$LINGUAS" = "x"; then
           LINGUAS=$ALL_LINGUAS
       fi
       for lang in $LINGUAS; do
         case "$ALL_LINGUAS" in
          *\ $lang\ *|$lang\ *|*\ $lang) NEW_LINGUAS="$NEW_LINGUAS $lang" ;;
         esac
       done
       LINGUAS=$NEW_LINGUAS
       AC_MSG_RESULT($LINGUAS)
     fi

     dnl Construct list of names of catalog files to be constructed.
     if test -n "$LINGUAS"; then
       for lang in $LINGUAS; do CATALOGS="$CATALOGS $lang$CATOBJEXT"; done
     fi
   fi

   dnl The reference to <locale.h> in the installed <libintl.h> file
   dnl must be resolved because we cannot expect the users of this
   dnl to define HAVE_LOCALE_H.
   if test $ac_cv_header_locale_h = yes; then
     INCLUDE_LOCALE_H="#include <locale.h>"
   else
     INCLUDE_LOCALE_H="\
/* The system does not provide the header <locale.h>.  Take care yourself.  */"
   fi
   AC_SUBST(INCLUDE_LOCALE_H)

   dnl Determine which catalog format we have (if any is needed)
   dnl For now we know about two different formats:
   dnl   Linux libc-5 and the normal X/Open format
   test -d intl || mkdir intl
   if test "$CATOBJEXT" = ".cat"; then
     AC_CHECK_HEADER(linux/version.h, msgformat=linux, msgformat=xopen)

     dnl Transform the SED scripts while copying because some dumb SEDs
     dnl cannot handle comments.
     sed -e '/^#/d' $srcdir/intl/$msgformat-msg.sed > intl/po2msg.sed
   fi
   dnl po2tbl.sed is always needed.
   sed -e '/^#.*[^\\]$/d' -e '/^#$/d' \
     $srcdir/intl/po2tbl.sed.in > intl/po2tbl.sed

   dnl In the intl/Makefile.in we have a special dependency which makes
   dnl only sense for gettext.  We comment this out for non-gettext
   dnl packages.
   if test "$PACKAGE" = "gettext"; then
     GT_NO="#NO#"
     GT_YES=
   else
     GT_NO=
     GT_YES="#YES#"
   fi
   AC_SUBST(GT_NO)
   AC_SUBST(GT_YES)

   dnl If the AC_CONFIG_AUX_DIR macro for autoconf is used we possibly
   dnl find the mkinstalldirs script in another subdir but ($top_srcdir).
   dnl Try to locate is.
   MKINSTALLDIRS=
   if test -n "$ac_aux_dir"; then
     MKINSTALLDIRS="$ac_aux_dir/mkinstalldirs"
   fi
   if test -z "$MKINSTALLDIRS"; then
     MKINSTALLDIRS="\$(top_srcdir)/mkinstalldirs"
   fi
   AC_SUBST(MKINSTALLDIRS)

   dnl *** For now the libtool support in intl/Makefile is not for real.
   l=
   AC_SUBST(l)

   dnl Generate list of files to be processed by xgettext which will
   dnl be included in po/Makefile.
   test -d po || mkdir po
   if test "x$srcdir" != "x."; then
     if test "x`echo $srcdir | sed 's@/.*@@'`" = "x"; then
       posrcprefix="$srcdir/"
     else
       posrcprefix="../$srcdir/"
     fi
   else
     posrcprefix="../"
   fi
   rm -f po/POTFILES
   sed -e "/^#/d" -e "/^\$/d" -e "s,.*,	$posrcprefix& \\\\," -e "\$s/\(.*\) \\\\/\1/" \
	< $srcdir/po/POTFILES.in > po/POTFILES
  ])


# Search path for a program which passes the given test.
# Ulrich Drepper <drepper@cygnus.com>, 1996.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU Public License
# but which still want to provide support for the GNU gettext functionality.
# Please note that the actual code is *not* freely available.

# serial 1

dnl AM_PATH_PROG_WITH_TEST(VARIABLE, PROG-TO-CHECK-FOR,
dnl   TEST-PERFORMED-ON-FOUND_PROGRAM [, VALUE-IF-NOT-FOUND [, PATH]])
AC_DEFUN(AM_PATH_PROG_WITH_TEST,
[# Extract the first word of "$2", so it can be a program name with args.
set dummy $2; ac_word=[$]2
AC_MSG_CHECKING([for $ac_word])
AC_CACHE_VAL(ac_cv_path_$1,
[case "[$]$1" in
  /*)
  ac_cv_path_$1="[$]$1" # Let the user override the test with a path.
  ;;
  *)
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
  for ac_dir in ifelse([$5], , $PATH, [$5]); do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      if [$3]; then
	ac_cv_path_$1="$ac_dir/$ac_word"
	break
      fi
    fi
  done
  IFS="$ac_save_ifs"
dnl If no 4th arg is given, leave the cache variable unset,
dnl so AC_PATH_PROGS will keep looking.
ifelse([$4], , , [  test -z "[$]ac_cv_path_$1" && ac_cv_path_$1="$4"
])dnl
  ;;
esac])dnl
$1="$ac_cv_path_$1"
if test -n "[$]$1"; then
  AC_MSG_RESULT([$]$1)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST($1)dnl
])

# Check whether LC_MESSAGES is available in <locale.h>.
# Ulrich Drepper <drepper@cygnus.com>, 1995.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU Public License
# but which still want to provide support for the GNU gettext functionality.
# Please note that the actual code is *not* freely available.

# serial 1

AC_DEFUN(AM_LC_MESSAGES,
  [if test $ac_cv_header_locale_h = yes; then
    AC_CACHE_CHECK([for LC_MESSAGES], am_cv_val_LC_MESSAGES,
      [AC_TRY_LINK([#include <locale.h>], [return LC_MESSAGES],
       am_cv_val_LC_MESSAGES=yes, am_cv_val_LC_MESSAGES=no)])
    if test $am_cv_val_LC_MESSAGES = yes; then
      AC_DEFINE(HAVE_LC_MESSAGES)
    fi
  fi])

# Configure paths for IMLIB
# Frank Belew     98-8-31
# stolen from Manish Singh
# Shamelessly stolen from Owen Taylor

dnl AM_PATH_IMLIB([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for IMLIB, and define IMLIB_CFLAGS and IMLIB_LIBS
dnl
AC_DEFUN(AM_PATH_IMLIB,
[dnl 
dnl Get the cflags and libraries from the imlib-config script
dnl
AC_ARG_WITH(imlib-prefix,[  --with-imlib-prefix=PFX   Prefix where IMLIB is installed (optional)],
            imlib_prefix="$withval", imlib_prefix="")
AC_ARG_WITH(imlib-exec-prefix,[  --with-imlib-exec-prefix=PFX Exec prefix where IMLIB is installed (optional)],
            imlib_exec_prefix="$withval", imlib_exec_prefix="")
AC_ARG_ENABLE(imlibtest, [  --disable-imlibtest       Do not try to compile and run a test IMLIB program],
		    , enable_imlibtest=yes)

  if test x$imlib_exec_prefix != x ; then
     imlib_args="$imlib_args --exec-prefix=$imlib_exec_prefix"
     if test x${IMLIB_CONFIG+set} != xset ; then
        IMLIB_CONFIG=$imlib_exec_prefix/bin/imlib-config
     fi
  fi
  if test x$imlib_prefix != x ; then
     imlib_args="$imlib_args --prefix=$imlib_prefix"
     if test x${IMLIB_CONFIG+set} != xset ; then
        IMLIB_CONFIG=$imlib_prefix/bin/imlib-config
     fi
  fi

  AC_PATH_PROG(IMLIB_CONFIG, imlib-config, no)
  min_imlib_version=ifelse([$1], ,1.8.2,$1)
  AC_MSG_CHECKING(for IMLIB - version >= $min_imlib_version)
  no_imlib=""
  if test "$IMLIB_CONFIG" = "no" ; then
    no_imlib=yes
  else
    IMLIB_CFLAGS=`$IMLIB_CONFIG $imlibconf_args --cflags`
    IMLIB_LIBS=`$IMLIB_CONFIG $imlibconf_args --libs`

    imlib_major_version=`$IMLIB_CONFIG $imlib_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    imlib_minor_version=`$IMLIB_CONFIG $imlib_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    imlib_micro_version=`$IMLIB_CONFIG $imlib_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_imlibtest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $IMLIB_CFLAGS"
      LIBS="$LIBS $IMLIB_LIBS"
dnl
dnl Now check if the installed IMLIB is sufficiently new. (Also sanity
dnl checks the results of imlib-config to some extent
dnl
      rm -f conf.imlibtest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Imlib.h>

char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.imlibtest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_imlib_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_imlib_version");
     exit(1);
   }

    if (($imlib_major_version > major) ||
        (($imlib_major_version == major) && ($imlib_minor_version > minor)) ||
	(($imlib_major_version == major) && ($imlib_minor_version == minor) &&
	($imlib_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'imlib-config --version' returned %d.%d, but the minimum version\n", $imlib_major_version, $imlib_minor_version);
      printf("*** of IMLIB required is %d.%d. If imlib-config is correct, then it is\n", major, minor);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If imlib-config was wrong, set the environment variable IMLIB_CONFIG\n");
      printf("*** to point to the correct copy of imlib-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}

],, no_imlib=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_imlib" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$IMLIB_CONFIG" = "no" ; then
       echo "*** The imlib-config script installed by IMLIB could not be found"
       echo "*** If IMLIB was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the IMLIB_CONFIG environment variable to the"
       echo "*** full path to imlib-config."
     else
       if test -f conf.imlibtest ; then
        :
       else
          echo "*** Could not run IMLIB test program, checking why..."
          CFLAGS="$CFLAGS $IMLIB_CFLAGS"
          LIBS="$LIBS $IMLIB_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include <Imlib.h>
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding IMLIB or finding the wrong"
          echo "*** version of IMLIB. If it is not finding IMLIB, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means IMLIB was incorrectly installed"
          echo "*** or that you have moved IMLIB since it was installed. In the latter case, you"
          echo "*** may want to edit the imlib-config script: $IMLIB_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     IMLIB_CFLAGS=""
     IMLIB_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(IMLIB_CFLAGS)
  AC_SUBST(IMLIB_LIBS)
  rm -f conf.imlibtest
])

# Check for gdk-imlib
AC_DEFUN(AM_PATH_GDK_IMLIB,
[dnl 
dnl Get the cflags and libraries from the imlib-config script
dnl
AC_ARG_WITH(imlib-prefix,[  --with-imlib-prefix=PFX   Prefix where IMLIB is installed (optional)],
            imlib_prefix="$withval", imlib_prefix="")
AC_ARG_WITH(imlib-exec-prefix,[  --with-imlib-exec-prefix=PFX Exec prefix where IMLIB is installed (optional)],
            imlib_exec_prefix="$withval", imlib_exec_prefix="")
AC_ARG_ENABLE(imlibtest, [  --disable-imlibtest       Do not try to compile and run a test IMLIB program],
		    , enable_imlibtest=yes)

  if test x$imlib_exec_prefix != x ; then
     imlib_args="$imlib_args --exec-prefix=$imlib_exec_prefix"
     if test x${IMLIB_CONFIG+set} != xset ; then
        IMLIB_CONFIG=$imlib_exec_prefix/bin/imlib-config
     fi
  fi
  if test x$imlib_prefix != x ; then
     imlib_args="$imlib_args --prefix=$imlib_prefix"
     if test x${IMLIB_CONFIG+set} != xset ; then
        IMLIB_CONFIG=$imlib_prefix/bin/imlib-config
     fi
  fi

  AC_PATH_PROG(IMLIB_CONFIG, imlib-config, no)
  min_imlib_version=ifelse([$1], ,1.8.2,$1)
  AC_MSG_CHECKING(for IMLIB - version >= $min_imlib_version)
  no_imlib=""
  if test "$IMLIB_CONFIG" = "no" ; then
    no_imlib=yes
  else
    GDK_IMLIB_CFLAGS=`$IMLIB_CONFIG $imlibconf_args --cflags-gdk`
    GDK_IMLIB_LIBS=`$IMLIB_CONFIG $imlibconf_args --libs-gdk`

    imlib_major_version=`$IMLIB_CONFIG $imlib_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    imlib_minor_version=`$IMLIB_CONFIG $imlib_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    if test "x$enable_imlibtest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $GDK_IMLIB_CFLAGS"
      LIBS="$LIBS $GDK_IMLIB_LIBS"
dnl
dnl Now check if the installed IMLIB is sufficiently new. (Also sanity
dnl checks the results of imlib-config to some extent
dnl
      rm -f conf.imlibtest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <gdk_imlib.h>

int main ()
{
  int major, minor;
  char *tmp_version;

  system ("touch conf.gdkimlibtest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = g_strdup("$min_imlib_version");
  if (sscanf(tmp_version, "%d.%d", &major, &minor) != 2) {
     printf("%s, bad version string\n", "$min_imlib_version");
     exit(1);
   }

    if (($imlib_major_version > major) ||
        (($imlib_major_version == major) && ($imlib_minor_version >= minor)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'imlib-config --version' returned %d.%d, but the minimum version\n", $imlib_major_version, $imlib_minor_version);
      printf("*** of IMLIB required is %d.%d. If imlib-config is correct, then it is\n", major, minor);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If imlib-config was wrong, set the environment variable IMLIB_CONFIG\n");
      printf("*** to point to the correct copy of imlib-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}

],, no_imlib=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_imlib" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$IMLIB_CONFIG" = "no" ; then
       echo "*** The imlib-config script installed by IMLIB could not be found"
       echo "*** If IMLIB was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the IMLIB_CONFIG environment variable to the"
       echo "*** full path to imlib-config."
     else
       if test -f conf.gdkimlibtest ; then
        :
       else
          echo "*** Could not run IMLIB test program, checking why..."
          CFLAGS="$CFLAGS $GDK_IMLIB_CFLAGS"
          LIBS="$LIBS $GDK_IMLIB_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include <gdk_imlib.h>
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding IMLIB or finding the wrong"
          echo "*** version of IMLIB. If it is not finding IMLIB, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means IMLIB was incorrectly installed"
          echo "*** or that you have moved IMLIB since it was installed. In the latter case, you"
          echo "*** may want to edit the imlib-config script: $IMLIB_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     IMLIB_CFLAGS=""
     IMLIB_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(GDK_IMLIB_CFLAGS)
  AC_SUBST(GDK_IMLIB_LIBS)
  rm -f conf.gdkimlibtest
])

dnl
dnl GNOME_ORBIT_HOOK (script-if-orbit-found, failflag)
dnl
dnl if failflag is "failure" it aborts if orbit is not found.
dnl

AC_DEFUN([GNOME_ORBIT_HOOK],[
	AC_PATH_PROG(ORBIT_CONFIG,orbit-config,no)
	AC_PATH_PROG(ORBIT_IDL,orbit-idl,no)
	AC_CACHE_CHECK([for working ORBit environment],gnome_cv_orbit_found,[
		if test x$ORBIT_CONFIG = xno -o x$ORBIT_IDL = xno; then
			gnome_cv_orbit_found=no
		else
			gnome_cv_orbit_found=yes
		fi
	])
	AM_CONDITIONAL(HAVE_ORBIT, test x$gnome_cv_orbit_found = xyes)
	if test x$gnome_cv_orbit_found = xyes; then
		$1
		ORBIT_CFLAGS=`orbit-config --cflags client server`
		ORBIT_LIBS=`orbit-config --use-service=name --libs client server`
		AC_SUBST(ORBIT_CFLAGS)
		AC_SUBST(ORBIT_LIBS)
	else
    		if test x$2 = xfailure; then
			AC_MSG_ERROR(ORBit not installed or installation problem)
    		fi
	fi
])

AC_DEFUN([GNOME_ORBIT_CHECK], [
	GNOME_ORBIT_HOOK([],failure)
])

