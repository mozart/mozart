/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *    Kostja Popov <kost@sics.se>
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

//
template class GenDistEntryTable<RHTNode>;
#include "hashtblDefs.cc"

//
ResourceHashTable *resourceTable;

// 
// This must be done after the ownerTable has been gced. The resource
// will stay in the resource table if there is an entry for it in the
// ownertable.
//

// The table is flushed because keys (OZ_Term"s) are changed over GC,
// so nodes need to be inserted anew;
struct GCRTEntry {
  OZ_Term term;
  OB_TIndex oti;
};

//
void ResourceHashTable::gcResourceTable()
{
  const int num = getUsed();
  struct GCRTEntry *entries;
  int ai = 0;
  DebugCode(int cnt = 0;);

  //
  if (num == 0) return;
  entries = new struct GCRTEntry[num];

  //
  for (int i = getSize(); i--; ) {
    RHTNode **np = getFirstNodeRef(i);
    RHTNode *n = *np;
    while (n) {
      DebugCode(cnt++;);
      // This ('n') resource table entity (RTE) references an OZ_Term
      // ('te') and an owner table entity (OE). RTE is alive iff OE is
      // alive, and OTE's OZ_Term is the same as the 'te'. 
      // NOTE: "the same" means pointer equality!!
      // Let's compare them NOW;
      OZ_Term te = n->getEntity();
      Bool teWasVar = oz_isRef(te);

      // If 'te' is NOT marked as GC"ed, then both terms will be
      // [pointer] unequal even we collect 'te'.
      if (isGCMarkedTerm(te)) {
	OB_TIndex oti = n->getOTI();
	OwnerEntry *oe = ownerIndex2ownerEntry(oti);

	//
	if (oe && oe->isRef()) {
	  // It can be alive (we still have the entry, and it is of the
	  // 'ref' type): now we really have to compare both entitles;
	  OZ_Term oer = oe->getRef();
	  DEREF(oer, oerp);
	  // must not be marked since the GC of owner table is finished:
	  Assert(!isGCMarkedTerm(oer));

	  // Now we have an entity in the owner table.
	  // Let's just collect our [resource table] entity (which may
	  // garbage, but we cannot tell right now) and the compare
	  // them;
	  oz_gCollectTerm(te, te);
	  DEREF(te, tep);

	  //
	  Assert(!oz_isRef(oer));
	  if (oz_isVarOrRef(oer)) {
	    // compare pointers to if we see a variable;
	    Assert(oerp);
	    // 'tep' can be anything (zero if 'te' is a value), however;
	    if (oerp == tep) {
	      entries[ai].term = makeTaggedRef(tep);
	      entries[ai++].oti = oti;
	    }
	  } else {		// non-variable:
	    // if the original entry was for a variable, then
	    // discard it now: the variable has got bound;
	    if (oer == te && !teWasVar) {
	      entries[ai].term = te;
	      entries[ai++].oti = oti;
	    }
	  }
	} else {
	  // something else in the owner table;
	}
      } else {
	// 'te' is NOT collected, so terms are unequal;
      }

      //
      deleteNode(n, np);
      delete n;

      //
      n = *np;
    }
  }
  Assert(cnt == num);

  //
  for (int i = 0; i < ai; i++)
    add(entries[i].term, entries[i].oti);

  //
  delete entries;
}

// kost@ : what is this doing here ?
ConstTerm* gcDistResourceImpl(ConstTerm* term)
{
  gcEntityInfoImpl((Tertiary *)term);
  term = (ConstTerm *) oz_hrealloc((void*)term,sizeof(DistResource));
  gcProxyRecurseImpl((Tertiary *)term);
  return term;
}
