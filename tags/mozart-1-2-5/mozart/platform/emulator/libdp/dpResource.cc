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
#include "cac.hh"


char *dpresource_names[UD_last] = {
  "unknown",
  
  "thread",
  "array",
  "dictionary"

};

ResourceHashTable *resourceTable;

/* 
 *  This must be done after the ownerTable has been gced. The resource
 *  will stay in the resource table if there is an entry for it in the
 *  ownertable.
 */

struct GCRTEntry {
  OZ_Term term;
  int oti;
};

//
void ResourceHashTable::gcResourceTable()
{
  const int num = getUsed();
  struct GCRTEntry *entries;
  int index;
  GenHashNode *aux;
  int ai = 0;

  //
  if (num == 0) 
    return;

  //
  entries = new struct GCRTEntry[num];
  aux = getFirst(index);
  Assert(aux);
  //
  do {
    OZ_Term te;
    Bool teWasVar;

    //
    te = (OZ_Term) aux->getBaseKey();
    teWasVar = oz_isRef(te);

    //
    if (isGCMarkedTerm(te)) {
      // ... can be alive - somebody copied it;
      int oti;
      OwnerEntry *oe;

      //
      oti = (int) aux->getEntry();
      oe = OT->getEntry(oti);

      //
      if (oe && oe->isRef()) {
	OZ_Term oer = oe->getRef();
	DEREF(oer, oerp);
	Assert(!isGCMarkedTerm(oer));
	// just extract the collected term:
	oz_gCollectTerm(te, te);
	DEREF(te, tep);

	//
	Assert(!oz_isRef(oer));
	if (oz_isVarOrRef(oer)) {
	  Assert(oerp);
	  // 'tep' can be anything, however;
	  if (oerp == tep) {
	    entries[ai].term = makeTaggedRef(tep);
	    entries[ai].oti = oti;
	  } else {
	    // nothing: dead entry;
	    entries[ai].term = (OZ_Term) 0;
	  }
	} else {		// non-variable:
	  // if the original entry was for a variable, then
	  // discard it now: the variable has got bound;
	  if (oer == te && !teWasVar) {
	    entries[ai].term = te;
	    entries[ai].oti = oti;
	  } else {
	    entries[ai].term = (OZ_Term) 0;
	  }
	}
      } else {
	// oe entry is gone, so the RHT entry is gone too:
	entries[ai].term = (OZ_Term) 0;
      }
    } else {
      // even the term itself is gone: 
      entries[ai].term = (OZ_Term) 0;
    }

    //
    ai++;
    aux = getNext(aux, index);
  } while (aux);
  Assert(ai == num);

  //
  clear();

  //
  for (ai = 0; ai < num; ai++) {
    OZ_Term te = entries[ai].term;
    if (te)
      add(te, entries[ai].oti);
  }

  //
  delete entries;
}

//
ConstTerm* gcDistResourceImpl(ConstTerm* term)
{
  term = (ConstTerm *) oz_hrealloc((void*)term,sizeof(DistResource));
  gcProxyRecurseImpl((Tertiary *)term);
  return term;
}




