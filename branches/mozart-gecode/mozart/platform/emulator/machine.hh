/*
 *  Authors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
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


#ifndef __MACHINEHH
#define __MACHINEHH

#include <limits.h>

/*
 * This file contains machine specific settings
 */


/* a 32 bit integer, the same on alpha and other machines */
#define int32 int

/* a 32 bit unsigned integer */
#define uint32 unsigned int

#define intlong long


/*
 * Note for DEC Alpha:
 * by default the text segment on the alpha starts at adress
 * 0x120000000 and the data segment at 0x140000000 (i.e. a bit in the
 * upper 4 bytes of addresses is set).  This makes converting an int32
 * to a void* and converting an int32 to a program adress (used in
 * threaded code) more expensive.  So we can either use the -T and -D
 * option of the linker or create an NMAGIC file using the -n link
 * option, which lets start segments at 0x20000000 and 0x40000000. We
 * choose to use -T/-D since -n requires static linking, which does
 * not support dlopen/dlsym dynamic linking.  See also man page of
 * ld(1)
 */


#if defined(ARCH_MIPS) || defined(ARCH_SPARC) || defined(OSF1_ALPHA)
#define DELAY_SLOT
#endif

#ifndef ARCH_I486
#define MANY_REGISTERS
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
inline uint32 ToInt32(const void *p) { return _ToInt32(p); }
inline uint32 ToInt32(uint32 t) { return t; }
#else
#define ToPointer(i) _ToPointer(i)
#define ToInt32(p)   _ToInt32(p)
#endif

/* (un)set bits in a pointer */
inline void *orPointer(void *p, int i)  { return (void*) ((intlong)p|(intlong)i); }
inline void *andPointer(void *p, int i) { return (void*) ((intlong)p&(intlong)i); }

#ifdef THREADED

#if defined(AS_HAS_MULTIPLE_ALIGN) || defined(AS_HAS_POWER_ALIGN)

#define INLINEOPCODEMAP
#define OPCODEALIGN 4

#ifdef AS_HAS_MULTIPLE_ALIGN
#define OPCODEALIGNINSTR ".align 16"
#endif

#ifdef AS_HAS_POWER_ALIGN
#define OPCODEALIGNINSTR ".align 4"
#endif

#endif
#endif


#endif


