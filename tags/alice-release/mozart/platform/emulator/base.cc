/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "base.hh"
#endif

#include "base.hh"

#include "ozconfig.hh"
#include "os.hh"
#include "value.hh"

#include <stdarg.h>
#include <errno.h>
#include <stdio.h>

void prefixError()
{
  if (ozconf.runningUnderEmacs) {
    fputc(MSG_ERROR,stderr);
    fflush(stderr);
  }
}

void prefixWarning()
{
  prefixError();
}

extern char *AMVersion, *AMDate, *ozplatform;

void OZ_error(OZ_CONST char *format, ...)
{
  va_list ap;

  va_start(ap,format);

  if (ozconf.runningUnderEmacs) 
    prefixError();
  else
    fprintf(stderr, "\a");

#ifdef DEBUG_CHECK
  fflush(stdout); // To let any printouts be printed before quitting
  vfprintf(stderr,format,ap);
  fprintf(stderr, "\n(going to report an error in pid %d)", osgetpid());
  while (OK) {}
#endif
  fprintf(stderr, "\n*** Internal Error: "
#ifndef DEBUG_CHECK
	  "Please send a bug report to bugs@mozart-oz.org ***\n"
	  "*** with the following information:\n"
	  "*** version:  %s\n"
	  "*** platform: %s\n"
	  "*** date:     %s\n\n",
	  AMVersion,ozplatform,AMDate
#endif
	   );
  vfprintf(stderr,format,ap);
  fprintf( stderr, "\n");
  fflush(stderr);

  va_end(ap);

  osStackDump();

  DebugCheckT(osUnblockSignals());

  // fprintf(stderr, "\n(going to report an error in pid %d)", osgetpid());
  // while (OK) {}

  //
  // send a signal to all forked processes, including the emulator itself
  DebugCode(ossleep(30));
#if defined(SIGQUIT) && defined(SIGUSR1)
  oskill(0,ozconf.dumpCore?SIGQUIT:SIGUSR1);
#else
  oskill(0,SIGINT);
#endif
}

void OZ_warning(OZ_CONST char *format, ...)
{
  va_list ap;

  va_start(ap,format);

  prefixWarning();

  fprintf( stderr, "*** Warning: ");
  vfprintf(stderr,format,ap);
  fprintf( stderr, "\n");
  fflush(stderr);

  va_end(ap);
}

void ozperror(const char *msg)
{
  OZ_error("UNIX ERROR: %s: %s",msg,OZ_unixError(errno));
}

void ozpwarning(const char *msg)
{
  int err = ossockerrno();
  OZ_warning("OS ERROR: %s: %s (%d)",msg,OZ_unixError(err),err);
}


void message(const char *format, ...)
{
  va_list ap;

  va_start(ap,format);

  fprintf( stdout,"*** ");
  vfprintf(stdout,format,ap);
  fflush(  stdout);

  va_end(ap);
}

// 
// kost@ : very quick hack to kill emulators that are lost...
Bool isDeadSTDOUT()
{
  char *buf = "\n";
  fflush(stdout);
  if (write(fileno(stdout), buf, 1) == -1)
    return (TRUE);
  else 
    return (FALSE);
}


void errorTrailer()
{
  message("----------------------------------------\n\n");
  fflush(stderr);
}

void errorHeader()
{
  printf("\n");
  prefixError();
  message("****************************************\n");
}

/*
 * destructively replace 'from' with 'to' in 's'
 */
char *replChar(char *s,char from,char to) {
  for (char *help = s; *help != '\0'; help++) {
    if (*help == from) {
      *help = to;
    }
  }
  return s;
}

/*
 * destructively delete a character from string
 */
char *delChar(char *s,char c) {
  char *p = s;
  char *help;
  for (help = s; *help != '\0'; help++) {
    if (*help != c) {
      *p++=*help;
    }
  }
  *p=*help; /* don't forget the \0 */
  return s;
}

Atom * DBG_STEP_ATOM, * DBG_NOSTEP_ATOM, * DBG_EXIT_ATOM;

#if defined(DEBUG_CHECK)
extern "C" {
void *__real_malloc(size_t sz)
{
  return ((void *) 0);
}
};
#endif // DEBUG_CHECK

