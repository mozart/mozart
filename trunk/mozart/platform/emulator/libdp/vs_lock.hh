/*
 *  Authors:
 *    Konstantin Popov
 * 
 *  Contributors:
 *
 *  Copyright:
 *    Konstantin Popov 1997-1998
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

#ifndef __VS_LOCKHH
#define __VS_LOCKHH

#include "base.hh"

#ifdef VIRTUALSITES

#ifdef INTERFACE  
#pragma interface
#endif

//
// This code is cloned from the OzPar's "asm_core.hh".
//

#if defined(volatile)
#define __volatile_f
#undef  volatile
#else 
#undef  __volatile_f
#endif

/*
 * Basic types;
 */
typedef unsigned int Value;

//
// Basic swap;
#ifdef __GNUC__

#if defined(sparc)

#define ASM_SWAP(cell, value)					\
({ 								\
  Value out;							\
  __asm__ __volatile__ ("swap %3,%0"				\
		: "=r" (out),   "=m" (cell) 	/* output */	\
		: "0"  (value), "m"  (cell));	/* input  */	\
  out;								\
})
;

#elif defined(i386)

#define ASM_SWAP(cell, value)					\
({ 								\
  Value out;							\
  __asm__ __volatile__ ("xchgl %3,%0"				\
                        :"=r" (out), "=m" (cell)		\
                        :"0" (value), "m" (cell));		\
  out;								\
})
;

#else

WE DO NOT SUPPORT ANYTHING ELSE BUT SPARCs and i386 SYSTEMS!

#endif 

#else

WE DO NOT SUPPORT ANYTHING ELSE BUT GCC COMPILER!

#endif

//
#define PAR_UNLOCKED	0x0
#define PAR_LOCKED	0xffffffff

//
class LockObj {
private:
  volatile Value lc;

  //
  // Note that these really should be protected methods, since 
  // they are to be used by methods of its superclasses only;
protected:
  LockObj() : lc(PAR_UNLOCKED) {}

  //
  Bool isUnlocked()	{ return (lc == PAR_UNLOCKED); }
  Bool isLocked()	{ return (lc == PAR_LOCKED);   }

  //
  void lock() {
    do { 
      if (ASM_SWAP(lc, PAR_LOCKED) == PAR_UNLOCKED) break;
      while (lc == PAR_LOCKED) continue; // spin in local cache;
    } while(1);
  }

  //
  Bool tryToLock() {
    if (ASM_SWAP(lc, PAR_LOCKED) == PAR_UNLOCKED) return (TRUE);
    else return (FALSE);
  }

  //
  void unlock() { lc = PAR_UNLOCKED; }
};

#if defined(__volatile_f)
#define volatile
#undef __volatile_f
#endif

#endif // VIRTUALSITES

#endif __ASM_CORE_H
