/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

// I M P O R T A N T:
// This file defines the interface between the abstract machine
// and generic variables, which provide the basic functionality
// for concrete generic variables. The implementor of subclasses
// of GenCVariable is encouraged to include only this file and
// files related to the constraint system concerned.

#ifndef __GENVAR__H__
#define __GENVAR__H__

#if defined(INTERFACE)
#pragma interface
#endif

//-----------------------------------------------------------------------------
//                       Generic Constrained Variable
//-----------------------------------------------------------------------------


enum TypeOfGenCVariable {
  FDVariable,
  OFSVariable,
  MetaVariable,
  BoolVariable,
  AVAR
};

class GenCVariable: public SVariable {

friend class GenFDVariable;

private:
  TypeOfGenCVariable var_type;

protected:

  void propagate(TaggedRef, SuspList * &, PropCaller);

public:
  USEFREELISTMEMORY;

  // the constructor creates per default a local variable (wrt curr. node)
  GenCVariable(TypeOfGenCVariable, Board * = NULL);

  TypeOfGenCVariable getType(void){ return var_type; }
  void setType(TypeOfGenCVariable t){
    Assert(t == FDVariable || t == OFSVariable ||
           t == MetaVariable || t == BoolVariable || t==AVAR);
    var_type = t;
  }

  // methods relevant for term copying (gc and solve)
  void gc(void);
  size_t getSize(void);

  // unifies a generic variable with another generic variable
  // or a non-variable
  // invariant: left term == *this
  Bool unify(TaggedRef *, TaggedRef, TaggedRef *, TaggedRef, Bool);
  Bool unifyOutline(TaggedRef *, TaggedRef, TaggedRef *, TaggedRef, Bool);

  int getSuspListLength(void);

  // is X=val still valid
  Bool valid(TaggedRef val);

  void print(ostream &stream, int depth, int offset, TaggedRef v);
  void printLong(ostream &stream, int depth, int offset, TaggedRef v);

  void installPropagators(GenCVariable *, Bool prop);

  // takes the suspensionlist of var and  appends it to the
  // suspensionlist of leftVar
  void relinkSuspListTo(GenCVariable * lv, Bool reset_local = FALSE) {
    suspList = suspList->appendToAndUnlink(lv->suspList, reset_local);
  }

  void addDetSusp(Suspension *susp);
  void dispose(void);
};

#include "fdgenvar.hh"
#include "fdbvar.hh"
#include "ofgenvar.hh"
#include "metavar.hh"
#include "avar.hh"

#ifndef OUTLINE
#include "genvar.icc"
#endif


#endif
