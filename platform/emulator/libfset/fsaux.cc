#include "fsaux.hh"

OZ_C_proc_begin(fsp_init, 0)
{
  cout << "Finite Set Propagator Module of " 
       << __DATE__ << " (" << __TIME__ << ')'
#ifdef OZ_DEBUG
       << " (DEBUG)"
#elif OZ_PROFILE
       << " (PROFILE)"
#endif
       << '.' << endl << flush;
  return PROCEED;
}
OZ_C_proc_end

