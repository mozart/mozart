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
  Entry_Propagator,
  Entry_Variable
};

class ReflectStack : protected Stack {
public:
  ReflectStack(void) : Stack(1024, Stack_WithMalloc) { }

  Bool isEmpty(void) {
    return Stack::isEmpty();
  }

  void push(Propagator * p) {
    Stack::push((StackEntry) makeTaggedRef2p((TypeOfTerm) Entry_Variable,
                                             (OZ_Term) p));
  }

  void push(OZ_Term * v) {
    Stack::push((StackEntry) makeTaggedRef2p((TypeOfTerm) Entry_Variable,
                                             (OZ_Term) v));
  }

  void * pop(void) {
    return Stack::pop();
  }
};

//-----------------------------------------------------------------------------

template <class T_WHAT>
class TableClass : protected HashTable {
private:
  int id_counter;
public:
  TableClass(void) : HashTable(HT_INTKEY, 2000), id_counter(0) {}

  Bool isIn(T_WHAT k) { return HashTable::htFind((intlong) k) != htEmpty; }

  int add(T_WHAT k, Bool &is_reflected) {
    int i = (int) HashTable::htFind((intlong) k);
    is_reflected = ((i != (int) htEmpty) && (i >= 0));
    if (i != (int) htEmpty) {
      return abs(i);
    }
    id_counter -= 1;
    HashTable::htAdd((intlong) k, (void *) id_counter);
    return abs(id_counter);
  }

  void reflected(T_WHAT k) {
    int i = (int) HashTable::htFind((intlong) k);
    Assert(!((i != (int) htEmpty) && (i >= 0)));
    HashTable::htAdd((intlong) k, (void *) id_counter);
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
