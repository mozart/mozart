/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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

#include <stdarg.h>

#include "fsaux.hh"

template _OZ_ParamIterator<OZ_Return>;
extern FILE *cpi_fileout;

void oz_fsetdebugprint(char *format, ...)
{
  va_list ap;
  va_start(ap,format);
  vfprintf(cpi_fileout,format,ap);
  va_end(ap);

  fprintf(cpi_fileout, "\n");
  fflush(cpi_fileout);
}
