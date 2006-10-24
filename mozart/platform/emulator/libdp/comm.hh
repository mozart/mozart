/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
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

#ifndef __COMM_HH
#define __COMM_HH

#ifdef INTERFACE  
#pragma interface
#endif

//
#include "base.hh"

//
// Return codes;

//

enum MonitorReturn{
    MONITOR_OK,
    SIZE_THRESHOLD_REACHED,
    NO_MSGS_THRESHOLD_REACHED,
    MONITOR_ALREADY_EXISTS,
    NO_MONITOR_EXISTS,
    MONITOR_PERM
};

enum ProbeReturn{
  PROBE_INSTALLED,
  PROBE_ALREADY_INSTALLED,      
  PROBE_DEINSTALLED,
  PROBE_NONEXISTENT,
  PROBE_OF_DIFFERENT_KIND,
  PROBE_PERM,
  PROBE_TEMP,
  PROBE_OK
};

enum GiveUpInput{
  ALL_GIVEUP,         // debug purpose only
  TEMP_GIVEUP
};

enum GiveUpReturn{
  GIVES_UP,
  SITE_NOW_NORMAL,
  SITE_NOW_PERM
};

enum ProbeType{
  PROBE_TYPE_ALL,
  PROBE_TYPE_PERM,
  PROBE_TYPE_NONE
};

enum SiteStatus {
  SITE_OK,
  SITE_PERM,			// kost@ : redundant - for debugging only!
  SITE_TEMP
};

// dsite::send return
#define ACCEPTED       0
#define PERM_NOT_SENT ~1
// TEMP_NOT_SENT  >0

enum FaultCode{
  COMM_FAULT_PERM_NOT_SENT,
  COMM_FAULT_PERM_MAYBE_SENT,
  COMM_FAULT_TEMP_NOT_SENT,
  COMM_FAULT_TEMP_MAYBE_SENT
};

#endif // __COMM_HH
