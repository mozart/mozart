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


/* a 32 bit integer, the same on alpha and other machines */
#define int32 int

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
 * 0x20000000 and 0x40000000. We choose to use the -n option.
 * See also man page of ld(1)
 */




#ifdef MIPS
const intlong mallocBase = 0x10000000;
#else
#ifdef AIX3_RS6000
const intlong mallocBase = 0x20000000;
#else
#ifdef HPUX_700
const intlong mallocBase = 0x40000000;
#else
#ifdef OSF1_ALPHA
const intlong mallocBase = 0x40000000;
#else
const intlong mallocBase = 0x0;
#endif
#endif
#endif
#endif


/* where does the text segment start
 * needed for threaded code
 */

const intlong textBase = 0x0;

/* convert an int32 to a pointer and vice versa */
inline void* ToPointer(int32 i) {
  return (void*) i;
}

inline int32 ToInt32(void *p) { return (int32)(p); }


/* (un)set bits in a pointer */
inline void *orPointer(void *p, int i)  { return (void*) ((intlong)p|(intlong)i); }
inline void *andPointer(void *p, int i) { return (void*) ((intlong)p&(intlong)i); }


/* consistency check */
inline int isPointer(void *p) { return (((intlong)p) & mallocBase) != 0;}

#endif
