/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *
 *  Copyright:
 *    Michael Mehl (1998)
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

#ifndef __EXTVAR__H__
#define __EXTVAR__H__

#ifdef INTERFACE
#pragma interface
#endif

#include "var_base.hh"

//
typedef enum {
  OZ_EVAR_PROXY,
  OZ_EVAR_MANAGER,
  OZ_EVAR_LAZY,
  // Patches are not really a variable, but serve for marshaling
  // (see marshalerPatch.hh and libdp/dpMarshaler.hh);
  OZ_EVAR_MGRVARPATCH,
  OZ_EVAR_PXYVARPATCH,
  OZ_EVAR_MVARPATCH,
  OZ_EVAR_LAST
} ExtVarType;

//
class ExtVar;

inline
OzVariable* extVar2Var(ExtVar*p) {
  return reinterpret_cast<OzVariable*>(reinterpret_cast<OzVariable*>(p)-1);
}

class ExtVar {
public:
  static void* operator new(size_t n)
  {
    return reinterpret_cast<void*>
      (reinterpret_cast<char*>(oz_freeListMalloc(n+sizeof(OzVariable)))
       + sizeof(OzVariable));
  }
  static void  operator delete(void*,size_t) { Assert(NO); }
public:
  ExtVar(Board *bb) { extVar2Var(this)->initAsExtension(bb); }
  virtual ExtVarType    getIdV() = 0;
  virtual ExtVar*       gCollectV() = 0;
  virtual void          gCollectRecurseV() = 0;
  virtual ExtVar*       sCloneV() = 0;
  virtual void          sCloneRecurseV() = 0;
  virtual OZ_Return     unifyV(TaggedRef*, TaggedRef*) = 0;
  virtual OZ_Return     bindV(TaggedRef*, TaggedRef) = 0;
  virtual Bool          validV(TaggedRef) = 0;
  virtual OZ_Term       statusV() = 0;
  virtual VarStatus     checkStatusV() = 0;
  virtual void          disposeV() = 0;

  virtual OZ_Return addSuspV(TaggedRef *, Suspendable * susp) {
    extVar2Var(this)->addSuspSVar(susp);
    return PROCEED;
  }
  virtual int getSuspListLengthV() {
    return extVar2Var(this)->getSuspListLengthS();
  }

  virtual void printStreamV(ostream &out,int depth = 10) {
    out << "<extvar: #" << (int) getIdV() << ">";
  }
  virtual void printLongStreamV(ostream &out,int depth = 10, int offset = 0) {
    printStreamV(out,depth); out << endl;
  }
  void print(void) { printStreamV(cerr); cerr << endl; cerr.flush(); }
  void printLong(void) { printLongStreamV(cerr); cerr.flush(); }
  virtual OZ_Return     forceBindV(TaggedRef*p, TaggedRef v) {
    return bindV(p,v);
  }

  // helpers for disposeV()
  void disposeS() { extVar2Var(this)->disposeS(); }
  void freeListDispose(size_t n) {
    oz_freeListDispose(extVar2Var(this),n+sizeof(void*));
  }
};

inline
int oz_isExtVar(TaggedRef r)
{
  return oz_isVar(r) && tagged2Var(r)->getType()==OZ_VAR_EXT;
}

inline
ExtVar* var2ExtVar(OzVariable*p) {
  return reinterpret_cast<ExtVar*>(p+1);
}

inline
ExtVar *oz_getExtVar(TaggedRef r) {
  Assert(oz_isExtVar(r));
  return var2ExtVar(tagged2Var(r));
}

inline
OZ_Term oz_makeExtVar(ExtVar *ev) {
  return makeTaggedVar(extVar2Var(ev));
}

#endif
