/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se) 
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 2004
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

#ifndef __MSG_TYPE_HH
#define __MSG_TYPE_HH

#ifdef INTERFACE  
#pragma interface
#endif
namespace _dss_internal{ //Start namespace


  enum MessageType {
    M_NONE = 0,
    M_PROXY_PROTOCOL,  // SITE INDEX + AS (+ MSG)
    M_MANAGER_PROTOCOL,// SITE INDEX + AS (+ MSG) 
    M_MANAGER_AS,
    M_MANAGER_AS_FAILED,
    M_PROXY_AS,
    M_PROXY_AS_FAILED,
    M_HOME_DGCI,          // SITE INDEX + AS (+ MSG)
    M_REMOTE_DGCI,        // SITE INDEX + AS (+ MSG)
    M_HOME_ENTITY_FAILED, // SITE INDEX + AS (+ MSG)
    M_ALGORITHM_REMOVED,  // SITE INDEX + AS (+ MSG)
    M_DKS_MSG,
    M_LAST
  };


  extern char *mess_names[];
    
  


} //End namespace
#endif
