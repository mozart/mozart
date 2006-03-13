/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
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

#ifndef __REFLECT_SPACE__HH__
#define __REFLECT_SPACE__HH__

#include "reflect.hh"
#include "hashtbl.hh"
#include "stack.hh"
#include "tagged.hh"

//-----------------------------------------------------------------------------

inline
intlong abs(intlong i) { return i >= 0 ? i : -i; }

//-----------------------------------------------------------------------------

enum TypeOfReflStackEntry {
  Entry_Propagator = 0,
  Entry_Variable
};

class ReflectStack : protected Stack {
public:
  ReflectStack(void) : Stack(1024, Stack_WithMalloc) { }

  Bool isEmpty(void) {
    return Stack::isEmpty();
  }

  void push(Propagator * p) {
    DEBUGPRINT(("ReflectStack::push(Propagator *)"));
//      Stack::push((StackEntry) makeTaggedRef2p((TypeOfTerm) Entry_Propagator,
//                                           (OZ_Term) p));
    Stack::push((StackEntry) __stag_ptr(p, Entry_Propagator));
  }

  void push(OZ_Term * v) {
    DEBUGPRINT(("ReflectStack::push(OZ_Term *)"));
//      Stack::push((StackEntry) makeTaggedRef2p((TypeOfTerm) Entry_Variable,
//                                           (OZ_Term) v));
    Stack::push((StackEntry) __stag_ptr(v, Entry_Variable));
  }

  void * pop(void) {
    return Stack::pop();
  }
};

//-----------------------------------------------------------------------------

// This kind of table stores the id of an item.  (Ids are positive.)
// Internally two hash tables are used.  The table 'preliminary'
// stores preliminary ids of items (before they are reflected).  Once
// reflected, the items are also put in table 'reflected'.  This
// separation makes up for the limitations of AddressHashTable.

template <class T_WHAT>
class TableClass {
private:
  AddressHashTable pretable;     // ids of possibly non reflected items
  AddressHashTable reftable;     // ids of reflected items
  int id_counter;                // next id available (positive)

public:
  TableClass(void) : pretable(2000), reftable(2000), id_counter(1) {}

  int add(T_WHAT k, Bool &is_reflected) {
    DEBUGPRINT(("TableClass::add -- in --"));

    // First look whether the item has been reflected
    int id = (int) reftable.htFind((void*) k);
    is_reflected = (id != (int) htEmpty);
    if (is_reflected) return id;

    // It hasn't been reflected yet, maybe it has a preliminary id...
    DEBUGPRINT(("TableClass::add -- not reflected yet --"));
    id = (int) pretable.htFind((void*) k);
    if (id != (int) htEmpty) return id;

    // It has no id, so create a preliminary id
    id = id_counter++;
    pretable.htAdd((void*) k, (void*) id);
    DEBUGPRINT(("TableClass::add -- out --"));
    return id;
  }

  void reflected(T_WHAT k) {
    DEBUGPRINT(("TableClass::reflected -- in --"));

    // The item must have a preliminary id
    int id = (int) pretable.htFind((void*) k);
    DEBUG_ASSERT(id != (int) htEmpty);

    // Store the id as a reflected one
    reftable.htAdd((void*) k, (void*) id);

    DEBUGPRINT(("TableClass::reflected -- out --"));
  }
};

typedef TableClass<OZ_Term *>    VarTable;
typedef TableClass<Propagator *> PropTable;

//-----------------------------------------------------------------------------

OZ_Term reflect_space_variable(ReflectStack &,
                               OZ_Term &,
                               VarTable &,
                               PropTable &,
                               OZ_Term);

OZ_Term reflect_space_prop(ReflectStack &,
                           OZ_Term &,
                           VarTable &,
                           PropTable &,
                           Propagator *);

OZ_Term reflect_space_susplist(ReflectStack &,
                               VarTable &vt,
                               PropTable &pt,
                               SuspList * sl);

//-----------------------------------------------------------------------------
#define ADD_TO_LIST(LIST, ELEM) LIST = OZ_cons(ELEM, LIST)

#endif /* __REFLECT_SPACE__HH__ */
