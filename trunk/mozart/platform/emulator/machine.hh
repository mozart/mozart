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

/*
 * This file contains machine specific settings
 */


/*
 * We use 4 bits for tags --> adress space is 2^28 == 256MB
 *
 * We can do better, if you define
 *     #define LARGEADRESSES
 *
 * if the value part of a TaggedRef contains a pointer (this holds for all
 * except small ints , than it will be word aligned,
 * i.e. its two lower bits are 00
 * --> makeTaggedRefs shifts only up by 2 bits
 * --> tagValueOf shifts down by 2 bits AND zeros the two lowest bits
 *
 */

#ifndef LINUX_M68K
#define LARGEADRESSES
#endif



/* a 32 bit integer, the same on alpha and other machines */
#define int32 int

/* a 32 bit unsigned integer */
#define uint32 unsigned int

#define intlong long


/*
 * Note for DEC Alpha:
 * by default the text segment on the alpha starts at adress 
 * 0x120000000 and the data segment at 0x140000000 (i.e. a bit in the upper
 * 4 bytes of addresses is set).
 * This makes converting an int32 to a void* and converting an
 * int32 to a program adress (used in threaded code) more expensive.
 * So we can either use the -T and -D option of the linker or create an
 * NMAGIC file using the -n link option, which lets start segments at
 * 0x20000000 and 0x40000000. We choose to use -T/-D since -n requires
 * static linking, which does not support dlopen/dlsym dynamic linking
 * We also let the data segment start at 0x3000000 --> if LARGEADRESSES is 
 * defined this makes mallocBase==0x0
 * See also man page of ld(1)
 */




#if defined(MIPS) && !defined(LARGEADRESSES)
#define mallocBase 0x10000000
#else
#if defined(AIX3_RS6000) && !defined(LARGEADRESSES)
#define mallocBase 0x20000000
#else
#if defined(OSF1_ALPHA) && !defined(LARGEADRESSES)
#define mallocBase 0x30000000
#else
#if defined(LINUX_M68K)
#define mallocBase 0x80000000
#else
#ifdef HPUX_700
#define mallocBase 0x40000000
#else
#define mallocBase 0x0
#endif
#endif
#endif
#endif
#endif


/* where does the text segment start
 * needed for threaded code
 */

#define textBase 0x0

/* convert an uint32 to a pointer and vice versa */
#define _ToPointer(i) ((void*) (i)) 
#define _ToInt32(p) ((uint32)(p))
#ifdef DEBUG_CHECK
inline void* ToPointer(uint32 i) { return _ToPointer(i); }
inline uint32 ToInt32(void *p) { return _ToInt32(p); }
#else
#define ToPointer(i) _ToPointer(i)
#define ToInt32(p)   _ToInt32(p)
#endif

/* (un)set bits in a pointer */
inline void *orPointer(void *p, int i)  { return (void*) ((intlong)p|(intlong)i); }
inline void *andPointer(void *p, int i) { return (void*) ((intlong)p&(intlong)i); }


/* consistency check */
inline int isPointer(void *p) { return (((intlong)p) & mallocBase) != 0;}

#endif

