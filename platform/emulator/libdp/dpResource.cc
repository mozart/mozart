/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
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

#if defined(INTERFACE)
#pragma implementation "dpResource.hh"
#endif

#include "base.hh"
#include "builtins.hh"
#include "value.hh"
#include "dpBase.hh"
#include "perdio.hh"
#include "table.hh"
#include "controlvar.hh"
#include "dpMarshaler.hh"
#include "dpInterface.hh"
#include "dpResource.hh"


char *dpresource_names[UD_last] = {
  "unknown",
  
  "thread",
  "array",
  "dictionary"

};
  

ResourceHashTable *resourceTable;

/* 
   This must be done after the ownerTable has
   been gced. The resource will stay in the resource table
   if there is an entry for it in the ownertable.
   */

void ResourceHashTable::gcResourceTable(){
  PD((GC,"Resource gc"));
  int index;
  int  OTI;
  OwnerEntry *oe;
  GenHashNode *aux = getFirst(index);
  while (aux!=NULL) {
    TaggedRef entity = (TaggedRef) aux->getBaseKey();
    if(!oz_isBuiltin(entity)){
      entity = oz_deref(entity);
      Assert(!oz_isVariable(entity));
      OTI =  (int) aux->getEntry();
      if(htSub(index,aux)) ;
      oe = OT->getEntry(OTI);
      if(oe && oe->getRef()==entity)
	add(entity, OTI);}
    else{
      OTI =  (int) aux->getEntry();
      oe = OT->getEntry(OTI);
      if(oe==NULL || oe->getRef()!=entity)
	if(htSub(index,aux)) ;
    }
    aux = getNext(aux,index);
  }
}

ConstTerm* gcDistResourceImpl(ConstTerm* term){
  term = (ConstTerm *) OZ_hrealloc((void*)term,sizeof(DistResource));
  gcProxyRecurseImpl((Tertiary *)term);
  return term;
}




