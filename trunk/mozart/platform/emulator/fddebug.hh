#ifndef __FDDEBUG_HH__
#define __FDDEBUG_HH__

#if defined(DEBUG_CHECK) && defined(DEBUG_FD)

extern "C" {
void error( char *format ...);
}

#  define DEBUG_FD_IR(COND, CODE) if (COND) CODE;

#  define AssertFD(C) \
if (!(C)) error("FD assertion '%s' failed at %s:%d.", #C, __FILE__, __LINE__); 

#  define DebugCodeFD(C) C

#else

#  define DEBUG_FD_IR(COND, CODE)
#  define AssertFD(C)
#  define DebugCodeFD(C)

#endif

/*
#define FD_DEBUG_T(TEXT, SIZE, T1, COND)
#define FD_DEBUG_TTI(TEXT, SIZE, T1, T2, I, COND)
#define FD_DEBUG_ITI(TEXT, SIZE, I1, T, I2, COND)
#define FD_DEBUG_XYZ(TEXT, X, Y, Z, COND)
#define FD_DEBUG_XYC(TEXT, X, Y, C, COND)
#define FD_DEBUG_XY(TEXT, X, Y, COND)
*/

#  define FORCE_ALL 0

#  define FD_DEBUG_T(TEXT, SIZE, T, COND) \
if (FORCE_ALL || COND) { \
  cout << TEXT << endl; \
  for (int _i = 0; _i < SIZE; _i += 1) \
    cout << "x[" << _i << "]=", taggedPrint(T[_i]), cout << endl; \
  cout.flush(); \
}

#  define FD_DEBUG_TTI(TEXT, SIZE, T1, T2, I, COND) \
if (FORCE_ALL || COND) { \
  cout << TEXT << endl; \
  int _i; \
  for (_i = 0; _i < SIZE; _i += 1) \
    cout << "a[" << _i << "]=", taggedPrint(T1[_i]), cout << endl; \
  for (_i = 0; _i < SIZE; _i += 1) \
    cout << "x[" << _i << "]=", taggedPrint(T2[_i]), cout << endl; \
  cout << "c=" << I  << endl; \
  cout.flush(); \
}

#  define FD_DEBUG_ITI(TEXT, SIZE, I1, T, I2, COND) \
if (FORCE_ALL || COND) { \
  cout << TEXT << endl; \
  cout << "n=" << I1  << endl; \
  for (int _i = 0; _i < SIZE; _i += 1) \
    cout << "l[" << _i << "]=", taggedPrint(T[_i]), cout << endl; \
  cout << "v=" << I2  << endl; \
  cout.flush(); \
}

#  define FD_DEBUG_XYZ(TEXT, X, Y, Z, COND) \
if (FORCE_ALL || COND) { \
  cout << TEXT << endl; \
  cout << "x="; taggedPrint(X); cout << endl; \
  cout << "y="; taggedPrint(Y); cout << endl; \
  cout << "z="; taggedPrint(Z); cout << endl; \
  cout.flush(); \
}

#  define FD_DEBUG_XYC(TEXT, X, Y, C, COND) \
if (FORCE_ALL || COND) { \
  cout << TEXT << endl; \
  cout << "x="; taggedPrint(X); cout << endl; \
  cout << "y="; taggedPrint(Y); cout << endl; \
  cout << "c=" << C << endl; \
  cout.flush(); \
}

#  define FD_DEBUG_XY(TEXT, X, Y, COND) \
if (FORCE_ALL || COND) { \
  cout << TEXT << endl; \
  cout << "x="; taggedPrint(X); cout << endl; \
  cout << "y="; taggedPrint(Y); cout << endl; \
  cout.flush(); \
}


#endif







