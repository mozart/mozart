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

#if defined(__GNUC__)
#pragma interface
#endif

#include "term.hh"
#include "unify.hh"

//-----------------------------------------------------------------------------
//                       Generic Constrained Variable
//-----------------------------------------------------------------------------


enum TypeOfGenCVariable {
  FDVariable,
  OFSVariable
};

class GenCVariable: public SVariable {
protected:
  TypeOfGenCVariable type;
  // takes the suspensionlist of var and  appends it to the
  // suspensionlist of leftVar
  void relinkSuspListTo(GenCVariable * lv, Bool reset_local = FALSE);

  void propagate(TaggedRef, SuspList * &, TaggedRef, PropCaller);

public:
  USEFREELISTMEMORY;

  // the constructor creates per default a local variable (wrt curr. node)
  GenCVariable(TypeOfGenCVariable , TaggedRef = AtomVoid, Board * = NULL);

  TypeOfGenCVariable getType(void){return type;}

  void setType(TypeOfGenCVariable t){ type = t;}

  Bool isLocalVariable(void);

  // methods relevant for term copying (gc and solve)
  void gc(void);
  size_t getSize(void);

  // unifies a generic variable with another generic variable
  // or a non-variable
  // invariant: left term == *this
  Bool unify(TaggedRef *, TaggedRef, TaggedRef *, TaggedRef, Bool);

  int getSuspListLength(void);

  // is X=val still valid
  Bool valid(TaggedRef val);

  OZPRINT;
  OZPRINTLONG;
};

#include "fdgenvar.hh"
#include "ofgenvar.hh"

#ifndef OUTLINE
#include "genvar.icc"
#endif


#endif
