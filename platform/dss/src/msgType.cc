/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Per Brand, 1998
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

#if defined(INTERFACE)
#pragma implementation "msgType.hh"
#endif

#include "msgType.hh"

namespace _dss_internal{ //Start namespace


  char *mess_names[M_LAST] = {
    "none",
    "proxy_protocol_msg",
    "manager_protocol_msg",
    "manager_as",
    "manager_as_failed",
    "proxy_as",
    "proxy_as_failed",
    "home dgci",
    "remote dgci",
    "entity failed",
    "algorithm removed",
    "dks_msg"
    };



} //End namespace
