#include "fsaux.hh"

#ifdef FSET_FILE_PRINT
ofstream * fscout;
#else
ostream * fscout;
#endif

#define FS_DEBUG_FN "fset_propagator.debug"

OZ_C_proc_begin(fsp_init, 0)
{
#ifdef FSET_FILE_PRINT
  static ofstream _fscout(FS_DEBUG_FN, ios::out);
  cerr << endl << "Output fset debug info to '" << FS_DEBUG_FN << "'." 
       << endl << flush;
  if (! _fscout ) {
    cerr << endl << "Cannot open '" << FS_DEBUG_FN << "' for output." 
	 << endl << flush;
  }
  fscout = & _fscout;
#else
  fscout = & cout;
#endif
  
#ifdef OZ_DEBUG
  cout << "*** DEBUG-FSETLIB ***" << endl << flush;
#elif OZ_PROFILE
  cout << "*** PROFILE-FSETLIB ***" << endl << flush;
#endif
  return PROCEED;
}
OZ_C_proc_end

