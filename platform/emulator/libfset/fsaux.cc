#include "fsaux.hh"

OZ_C_proc_begin(fsp_init, 0)
{
#ifdef OZ_DEBUG
  cout << "*** DEBUG-FSETLIB ***" << endl << flush;
#elif OZ_PROFILE
  cout << "*** PROFILE-FSETLIB ***" << endl << flush;
#endif
  //  cout << "fsetlib " << __DATE__ << ' ' << __TIME__ << endl << flush;
  return PROCEED;
}
OZ_C_proc_end

