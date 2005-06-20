/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Per Brand
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


#ifndef __FAULTMODULE_HH
#define __FAULTMODULE_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "tagged.hh"
#include "dss_object.hh"

class Watcher {
public: 
  TaggedRef proc;
  FaultState fs; 
  Watcher *next; 
  
  Watcher(TaggedRef p, FaultState f, Watcher *n);
  
  void winvoke(FaultState cond, TaggedRef entity);
  void gCollect();
};


#endif

