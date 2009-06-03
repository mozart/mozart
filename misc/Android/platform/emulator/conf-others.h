/* conf-others.h.  Generated automatically by configure.  */
/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Michael Mehl (1997)
 *
 *  Last change:
 *    $Date: 2003-05-06 21:32:58 +0200 (Tue, 06 May 2003) $ by $Author: duchier $
 *    $Revision: 15486 $
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

/* OS and CPU */
/* #define LINUX_I486 1 */
/* #undef SUNOS_SPARC */
/* #undef SOLARIS_SPARC */
/* #undef OSF1_ALPHA */
/* #undef HPUX_700 */

/* #undef WINDOWS */
#define LINUX 1
/* #undef SOLARIS */
/* #undef SUNOS */
/* #undef IRIX6 */
/* #undef NETBSD */
/* #undef FREEBSD */
/* #undef OPENBSD */
/* #undef IRIX */
/* #undef OS2 */
/* #undef RHAPSODY */

/* #define ARCH_I486 1 */
/* #undef ARCH_SPARC */
/* #undef ARCH_MIPS */

/* #undef SUNPRO */

/*
 * emulator optimization
 */

/* define if threaded code (requires GCC) */
#ifdef __GNUC__
#define THREADED 1
#endif

/* define if optimized register access (pre-shifted) */
#define FASTREGACCESS 1

/* define if optimized X-register access (resolved to direct address) */
#ifdef FASTREGACCESS
#define FASTERREGACCESS 1
#endif

/* define if try to use assembly code for integer arithmetic */
/* #undef FASTARITH */

/* define if modules are linked statically */
/* #undef MODULES_LINK_STATIC */

/* debugging */
/* #undef DEBUG_EMULATOR */

/* profiling */
/* #undef PROFILE_EMULATOR */

/* RS profiling */
/* #undef RS_PROFILE */

/* use a debug malloc library */
/* #undef DMALLOC */
/* #undef CCMALLOC */

/* Define if you want support for virtual sites */
/* #undef VIRTUALSITES */

/* Define if you want to include misc builtins */
/* #undef MISC_BUILTINS */


/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the dlopen function.  */
#define HAVE_DLOPEN 1

/* Define if you have <dlfcn.h>.  */
#define HAVE_DLFCN_H 1

/* Define if you want to allocate memory with malloc.  */
#define USE_MALLOC 1

/* Define if you want to allocate memory via mmap.  */
/* #undef USE_MMAP */

/* Define if you have the sbrk function.  */
/* kost@ : comment it out so: the configure should figure it out;       */
/* #ifndef RHAPSODY     */
/* #undef USE_SBRK */
/* #endif               */

/* do we have setpgid? */
#define HAVE_SETPGID 1

/* does .align n align to 1<<n */
/* #undef AS_HAS_POWER_ALIGN */

/* does .align n align to n */
/* #undef AS_HAS_MULTIPLE_ALIGN */


/* Define if dlopen needs a leading underscore. */
/* #undef DLOPEN_UNDERSCORE */

/* for virtual sites */
/* #undef key_t */

/* type check */
#define HAVE_SOCKLEN_T 1
#ifndef HAVE_SOCKLEN_T
typedef int socklen_t;
#endif

#define HAVE_STDINT_H 1

/* whether gmp.h needs c++ */
#define GMP_NEEDS_CPLUSPLUS 1

/* whether we have the times/sysconf(_SC_CLK_TCK) bug */
/* #undef CLK_TCK_BUG_RATIO */

/* endianness */
#define ARCH_LITTLE_ENDIAN 1
/* #undef ARCH_BIG_WORDIAN */

#define SITE_PROPERTY 1
#include <stdio.h>
extern FILE __sF[3];
