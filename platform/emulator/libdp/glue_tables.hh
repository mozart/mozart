/*
 *  Authors:
 *    Zacharias El Banna, 2002
 *    Erik Klintskog, 2002
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
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
#ifndef __GLUE_TABLES_HH
#define __GLUE_TABLES_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "value.hh"
#include "hashtbl.hh"
#include "glue_mediators.hh"


/*
  The mediator table handles mediator lookups, and their garbage
  collection.

  Note that mediator lookup is only available for detached mediators
  (glue_mediators.hh).  Those mediators are stored in a hash table for
  that purpose.  Otherwise all mediators are simply kept in a list.
  If you change this, make sure that the passive mediators are never
  inserted in the hash table (they have no valid key).

  The garbage collection of mediators is done in three phases:

  - The primary phase collects the mediators that are primary roots
  for the DSS.

  - The weak phase should happen after local garbage collection.  It
  determines which weak roots should be removed.  The detached
  mediators are also collected during this phase.  Indeed, those are
  not directly marked by local garbage collection.  Therefore one must
  check whether their entity has been marked after local gc.

  - The cleanup phase removes unused mediators from the table.  It
  localizes entities that are only referred to locally, and deletes
  mediators of dead entities.

 */

class MediatorTable {

private:
  // list of all mediators
  Mediator* medList;

  // The detached mediators are also put in this hash table
  AddressHashTableO1Reset *medTable;

  // used during garbage collection only
  Mediator* noneList;     // mediators with status DSS_GC_NONE
  Mediator* weakList;     // mediators with status DSS_GC_WEAK
  Mediator* primaryList;  // mediators with status DSS_GC_PRIMARY
  Mediator* localizeList; // mediators with status DSS_GC_LOCALIZE

  // useful functions for manipulating lists
  inline void push(Mediator* &lst, Mediator *med) {
    med->next = lst; lst = med;
  }
  inline Mediator* pop(Mediator* &lst) {
    Mediator *med = lst;
    if (med) lst = med->next;
    return med;
  }

public:
  MediatorTable();
  ~MediatorTable();

  void insert(Mediator *med);
  Mediator *lookup(TaggedRef ref);

  // gc
  void gcPrimary();
  void gcWeak();
  void gcCleanUp();

  // debugging
  void print();
};


// THE mediator table
extern MediatorTable *mediatorTable;


/*************************** GC **************************/
void gcMediatorTablePrimary();
void gcMediatorTableWeak();
void gcMediatorTableCleanUp();

/************************ LOOK-UPS ***********************/

Mediator *taggedref2Me(TaggedRef tr);
Mediator *index2Me(const int&);

// Get a ProxyInterface * from a engine name
AbstractEntity *index2AE(const int&);
CoordinatorAssistantInterface *index2CAI(const int&);

// Get a ProxyInterface * from a TaggedRef, returns
// the ProxyInterface * 0 if no entry was found. 


#endif
