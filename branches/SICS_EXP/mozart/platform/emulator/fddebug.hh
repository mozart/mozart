#ifndef __FDDEBUG_HH__
#define __FDDEBUG_HH__


#if defined(DEBUG_CHECK) && defined(DEBUG_FD)

extern "C" {
void error( char *format ...);
}

extern ostream * cpi_cout;

#  define DEBUG_FD_IR(COND, CODE) if (COND) { *cpi_cout << CODE << flush;}

#  define AssertFD(C) \
if (!(C)) error("FD assertion '%s' failed at %s:%d.", #C, __FILE__, __LINE__); 

#  define DebugCodeFD(C) C

#else

#  define DEBUG_FD_IR(COND, CODE)
#  define AssertFD(C)
#  define DebugCodeFD(C)

#endif


#  define FORCE_ALL 0

#endif







