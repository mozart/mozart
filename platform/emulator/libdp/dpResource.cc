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

// kost@ : WHAT IS ALL THAT?!! Not used anymore:
/*
inline
Bool isReallyBuiltin(TaggedRef b) {
  if (!oz_isConst(b))
    return NO;

  ConstTerm * c = tagged2Const(b);

  if (c->cacIsMarked())
    return NO;

  return isBuiltin(c);
}
*/

//
void ResourceHashTable::gcResourceTable()
{
  const int num = getUsed();
  int *entries;
  int index;
  GenHashNode *aux;
  int ai = 0;

  //
  if (num == 0)
    return;

  //
  entries = new int[num];
  aux = getFirst(index);
  Assert(aux);
  //
  do {
    // kost@ : WHAT IS ALL THAT?!! Not used anymore:
    /*
      TaggedRef entity = (TaggedRef) aux->getBaseKey();
      if(!isReallyBuiltin(entity)) {
      entity = oz_deref(entity);
      Assert(!oz_isVariable(entity));
      }
    */
    int OTI;
    GenCast(aux->getEntry(), GenHashEntry*, OTI, int);
    entries[ai++] = OTI;

    //
    aux = getNext(aux, index);
  } while (aux);
  Assert(ai == num);

  //
  clear();

  //
  for (ai = 0; ai < num; ai++) {
    int OTI = entries[ai];
    OwnerEntry *oe = OT->getEntry(OTI);
    // kost@ : i've replaced next two lines:
    //    if(oe && (!oe->isFree()) && oe->getRef()==entity)
    //      add(entity, OTI);
    // kost@ : Now, like this:
    // Must be alive, and, therefore, a reference, since RHT entries are
    // discarded explicitly:
    Assert(oe);
    Assert(!oe->isFree());
    Assert(oe->isRef());

    //
    OZ_Term t = oe->getRef();
    DEREF(t, tPtr, _tag);
    if (oz_isVariable(t)) {
      add(makeTaggedRef(tPtr), OTI);
    } else {
      add(t, OTI);
    }
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
