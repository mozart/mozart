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
//  					     (OZ_Term) p));
    Stack::push((StackEntry) __stag_ptr(p, Entry_Propagator));
  }

  void push(OZ_Term * v) {
    DEBUGPRINT(("ReflectStack::push(OZ_Term *)"));
//      Stack::push((StackEntry) makeTaggedRef2p((TypeOfTerm) Entry_Variable, 
//  					     (OZ_Term) v));
    Stack::push((StackEntry) __stag_ptr(v, Entry_Variable));
  }

  void * pop(void) { 
    return Stack::pop(); 
  }
};

//-----------------------------------------------------------------------------

// This kind of table stores the id of an item. In case the id is
// preliminary, i.e., the item has not been reflected yet, the id is
// (internally) negative. The id of a reflected item is (internally)
// positive. To the outside a table provides only positive ids.

template <class T_WHAT>
class TableClass : protected AddressHashTable {
private:
  int id_counter;

public:
  TableClass(void) : AddressHashTable(2000), id_counter(-1) {}

  int add(T_WHAT k, Bool &is_reflected) {
    DEBUGPRINT(("TableClass::add -- in --"));

    int i = (int) htFind((void *) k);
    DEBUGPRINT(("TableClass::add -- htFind --"));
    is_reflected = ((i != (int) htEmpty) && (i >= 0));
    if (i != (int) htEmpty) {
      return abs(i)-1;
    }
    DEBUGPRINT(("TableClass::add -- after loop --"));
    id_counter -= 1;
    htAdd((void *) k, (void *) id_counter);
    DEBUGPRINT(("TableClass::add -- out --"));
    return abs(id_counter)-1;
  }

  void reflected(T_WHAT k) {
    DEBUGPRINT(("TableClass::reflected -- in --"));

    int i = (int) htFind((void *) k);
    DEBUG_ASSERT((i != (int) htEmpty) && (i < 0));
    htAdd((void *) k, (void *) -i);

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
