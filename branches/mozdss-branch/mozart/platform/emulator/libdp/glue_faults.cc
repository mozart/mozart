/*
 *  Authors:
 *    Zacharias El Banna (zeb@sics.se)
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

#if defined(INTERFACE)
#pragma implementation "glue_faults.hh"
#endif

#include "glue_faults.hh"
#include "glue_tables.hh"
#include "glue_base.hh"

OZ_Term FaultPort; 

Watcher::Watcher(TaggedRef p, FaultState f, Watcher *n):
  proc(p),fs(f), next(n)
{
  ; 
}

void Watcher::winvoke(FaultState cond, TaggedRef entity){
  OZ_Term msg = OZ_recordInit(oz_atom("watcher"),oz_cons(oz_pairA("entity",entity),oz_cons(oz_pairA("action",proc),oz_cons(oz_pairAI("condition",cond),oz_nil()))));
  doPortSend(tagged2Port(FaultPort), msg, NULL);
}

void Watcher::gCollect(){
  oz_gCollectTerm(proc, proc);
}
