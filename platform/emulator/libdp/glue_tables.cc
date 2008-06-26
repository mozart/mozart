/*
 *  Authors:
 *    Zacharias El Banna, 2002
 *
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 *    Boriss Mejias (bmc@info.ucl.ac.be)
 *
 *  Copyright:
 *    Zacharias El Banna, 2002
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
#pragma implementation "glue_tables.hh"
#endif

#include "am.hh"
#include "glue_tables.hh"
#include "pstContainer.hh"
#include "engine_interface.hh"


// The mediator table
MediatorTable *mediatorTable;



/************************* Mediator Table *************************/

MediatorTable::MediatorTable() :
  medList(NULL), noneList(NULL), weakList(NULL), primaryList(NULL),
  localizeList(NULL)
{
  medTable = new AddressHashTableO1Reset(100);
}

MediatorTable::~MediatorTable() {
  Mediator *med;
  while (med = pop(medList)) delete med;
  delete medTable;
}

// put med in the list (and in the hash table if detached)
void
MediatorTable::insert(Mediator *med) {
  med->resetGCStatus();     // simpler when gc cleanup
  push(medList, med);
  if (!med->isAttached())
    medTable->htAddOverWrite(reinterpret_cast<void*>(med->getEntity()), med);
}

// lookup a mediator (for detached mediators only)
Mediator*
MediatorTable::lookup(TaggedRef ref) {
  void *med = medTable->htFind(reinterpret_cast<void*>(ref));
  return (med == htEmpty ? NULL : static_cast<Mediator*>(med));
}

// primary phase of gc
void
MediatorTable::gcPrimary() {
  // first put each element in its gc list
  Mediator *med;
  while (med = pop(medList)) {
    switch (med->getDssGCStatus()) {
    case DSS_GC_NONE:     push(noneList, med); break;
    case DSS_GC_WEAK:     push(weakList, med); break;
    case DSS_GC_PRIMARY:  push(primaryList, med); break;
    case DSS_GC_LOCALIZE: push(localizeList, med); break;
    default: Assert(0);
    }
    med->gCollectPrepare();     // preliminary stuff
  }
  Assert(medList == NULL);

  // now mark all primary roots
  for (med = primaryList; med; med = med->next) med->gCollect();
}

// check weak roots
void
MediatorTable::gcWeak() {
  // check weak mediators; clear non-marked weak roots
  for (Mediator* med = weakList; med; med = med->next) {
    if (!med->isCollected()) {
      med->getCoordinatorAssistant()->clearWeakRoot();
      med->gCollect();
    }
  }

  // this is the right time to check detached mediators.
  AHT_HashNodeCnt *node;
  for (node = medTable->getFirst(); node; node = medTable->getNext(node))
    static_cast<Mediator*>(node->getValue())->checkGCollect();

  // Note.  The latter used to be incomplete, but it is now correct.
  // The method checkGCollect() does not mark new entities; it only
  // updates the reference to the entity.  It was not the case before.
}

// cleanup the table
void
MediatorTable::gcCleanUp() {
  Mediator* med;

  // cleanup hash table first
  medTable->mkEmpty();
  Assert(medList == NULL);

  // primary and weak roots have been collected, reinsert them
  while (med = pop(primaryList)) insert(med);
  while (med = pop(weakList)) insert(med);

  // some in noneList may have been collected, reinsert if marked
  while (med = pop(noneList)) {
    if (med->isCollected()) insert(med); else delete med;
  }

  // localize if marked, delete otherwise.
  while (med = pop(localizeList)) {
    if (med->isCollected()) { med->localize(); insert(med); } else delete med;
  }

  Assert(noneList == NULL);
  Assert(weakList == NULL);
  Assert(primaryList == NULL);
  Assert(localizeList == NULL);
}

// print the table
void
MediatorTable::print() {
  printf("----- contents of the mediator table -----\n");
  for (Mediator *med = medList; med; med = med->next) med->print();
  printf("------------------------------------------\n");
}



/************************* Annotation Table *************************/

static Annotation defaultAnnotation[GLUE_LAST];

Annotation getDefaultAnnotation(GlueTag type) {
  return defaultAnnotation[type];
}

void setDefaultAnnotation(GlueTag type, Annotation annotation) {
  defaultAnnotation[type] = annotation;
}

void setDefaultAnnotation(GlueTag type, ProtocolName pn,
                          AccessArchitecture aa, RCalg rc) {
  setDefaultAnnotation(type, Annotation(pn, aa, rc));
}



/************************* RPC wrapper *************************/

static TaggedRef rpc_wrapper = makeTaggedNULL();

TaggedRef getRPC() {
  if (rpc_wrapper == makeTaggedNULL()) {
    rpc_wrapper = oz_newVariable();
  }
  return rpc_wrapper;
}



/************************* Interface functions *************************/

void gcMediatorTablePrimary(){
  oz_gCollectTerm(rpc_wrapper, rpc_wrapper);
  mediatorTable->gcPrimary();
};

void gcMediatorTableWeak(){
  mediatorTable->gcWeak();
};

void gcMediatorTableCleanUp(){
  mediatorTable->gcCleanUp();
};
