/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/


#ifndef __MACHINEHH
#define __MACHINEHH

#include <stdlib.h>


/*
 * This file contains machine specific settings
 */


/* a 32 bit integer, the same on alpha and other machines */
#define int32 int

#define intlong long


#ifdef MIPS
const int mallocBase = 0x10000000;
#else
#ifdef AIX3_RS6000
const int mallocBase = 0x20000000;
#else
#ifdef HPUX_700
const int mallocBase = 0x40000000;
#else
#ifdef OSF1_ALPHA
const long int mallocBase = 0x140000000;
#else
const int mallocBase = 0x0;
#endif
#endif
#endif
#endif


/* convert an int32 to a pointer and vice versa */
inline void* ToPointer(int32 i) { 
#ifdef OSF1_ALPHA
  return (void*) (mallocBase|i);
#else
  return (void*) i;
#endif
}


inline void *orPointer(void *p, int i)
{
  return (void*) ((long)p|(long)i);
}


inline void *andPointer(void *p, int i)
{
  return (void*) ((long)p&(long)i);
}

inline int32 ToInt32(void *p)   { return (int32)(p); } ;

inline int isPointer(void *p) { return (((intlong)p) & mallocBase) != 0;}

#endif

