/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Michael Mehl (1997)
 *    Christian Schulte, 2000
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


/* OS and CPU */
#define WINDOWS 1

#define ARCH_I486 1

/*
 * emulator optimization
 */

/* define if optimized register access (pre-shifted) */
#define FASTREGACCESS 1

/* define if optimized X-register access (resolved to direct address) */
#ifdef FASTREGACCESS
#define FASTERREGACCESS 1
#endif

/* define if modules are linked statically */
#define MODULES_LINK_STATIC 1

/* Define if you want support for virtual sites */
#undef VIRTUALSITES

/* Define if you want to include misc builtins */
#undef MISC_BUILTINS

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the dlopen function.  */
#undef HAVE_DLOPEN

/* Define if you have <dlfcn.h>.  */
#define HAVE_DLFCN_H 1

/* Define if you want to allocate memory with malloc.  */
#define USE_MALLOC

/* Define if you want to allocate memory via mmap.  */
#undef USE_MMAP

/* Define if you have the sbrk function.  */
#undef USE_SBRK

/* do we have setpgid? */
#undef HAVE_SETPGID


/* Define if dlopen needs a leading underscore. */
#undef DLOPEN_UNDERSCORE

/* for virtual sites */
#define key_t unsigned int

/* type check */
#undef HAVE_SOCKLEN_T

#ifndef HAVE_SOCKLEN_T
typedef int socklen_t;
#endif

