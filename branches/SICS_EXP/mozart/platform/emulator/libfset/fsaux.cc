/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include <stdarg.h>

#include "fsaux.hh"

OZ_C_proc_begin(fsp_init, 1)
{
#ifdef OZ_DEBUG
  oz_fsetdebugprint("*** DEBUG-FSETLIB ***");
#elif OZ_PROFILE
  oz_fsetdebugprint("*** PROFILE-FSETLIB ***");
#endif
  //  cout << "fsetlib " << __DATE__ << ' ' << __TIME__ << endl << flush;
  return OZ_unifyAtom(OZ_args[0], __DATE__ " (" __TIME__ ")");
}
OZ_C_proc_end


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
