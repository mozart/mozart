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

#ifndef __WSOCK_H__
#define __WSOCK_H__

#include "conf.h"

#ifdef WINDOWS

/* "windows.h" defines some constants, that are also used in Oz,
 * so this file MUST BE INCLUDED BEFORE ANY OTHER FILE
 */

#define NOMINMAX
#define Bool WinBool
#define min winmin
#define max winmax

/* The windows header files can sometimes #define INTERFACE.
 * We make sure here that if it was undefined before we include
 * the Windows files then it will be undefined afterwards.
 */

#ifndef INTERFACE
 #define OZNOINTERFACE
#endif 

 #include <windows.h>
 #include <winsock.h>

#ifdef OZNOINTERFACE
  #undef INTERFACE
#endif

#undef min
#undef max
#undef FAILED /* used in mozart.h as well */
#undef Bool


/* these errors were not defined */
#define EINPROGRESS            WSAEINPROGRESS
#define EADDRINUSE             WSAEADDRINUSE
#define ECONNRESET             WSAECONNRESET
#define ENOBUFS                WSAENOBUFS
#define EADDRNOTAVAIL          WSAEADDRNOTAVAIL
#define EWOULDBLOCK            WSAEWOULDBLOCK
#define ECONNREFUSED           WSAECONNREFUSED
#define ETIMEDOUT              WSAETIMEDOUT
#define EHOSTUNREACH           WSAEHOSTUNREACH

#endif

#endif

