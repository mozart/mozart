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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __WSOCK_H__
#define __WSOCK_H__

#ifdef HAVE_CONFIG_H
#include "conf.h"
#endif

#ifdef WINDOWS

/* "windows.h" defines some constants, that are also used in Oz,
 * so this file MUST BE INCLUDED BEFORE ANY OTHER FILE
 */

#define NOMINMAX
#define Bool WinBool
#define min winmin
#define max winmax

#include <windows.h>

#undef min
#undef max
#undef FAILED /* used in oz.h as well */
#undef Bool


/* sockets: */
#include <winsock.h>

/* these errors were not defined */
#define EINPROGRESS            WSAEINPROGRESS
#define EADDRINUSE             WSAEADDRINUSE
#define ECONNRESET             WSAECONNRESET
#define ENOBUFS                WSAENOBUFS
#define EADDRNOTAVAIL          WSAEADDRNOTAVAIL
#define EWOULDBLOCK            WSAEWOULDBLOCK
#define ECONNREFUSED           WSAECONNREFUSED

#endif

#endif


