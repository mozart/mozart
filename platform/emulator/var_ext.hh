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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
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

enum {
  OZ_EVAR_PROXY,
  OZ_EVAR_MANAGER,
  OZ_EVAR_OBJECT,
  OZ_EVAR_LAST
};

class ExtVar : public OzVariable {
public:
  ExtVar(Board *bb) : OzVariable(OZ_VAR_EXT,bb) {}
  virtual int           getIdV() = 0;
  virtual OzVariable*   gcV() = 0;
  virtual void          gcRecurseV() = 0;
  virtual OZ_Return     unifyV(TaggedRef*, TaggedRef*) = 0;
  virtual OZ_Return     bindV(TaggedRef*, TaggedRef) = 0;
  virtual Bool          validV(TaggedRef) = 0;
  virtual OZ_Term       statusV() = 0;
  virtual VarStatus     checkStatusV() = 0;
  virtual void          disposeV() = 0;

  virtual OZ_Return addSuspV(TaggedRef *, Suspension susp,
                             int unstable = TRUE) {
    addSuspSVar(susp, unstable);
    return PROCEED;
  }
  virtual int getSuspListLengthV() { return getSuspListLengthS(); }

  virtual void printStreamV(ostream &out,int depth = 10) {
    out << "<extvar: #" << getIdV() << ">";
  }
  virtual void printLongStreamV(ostream &out,int depth = 10, int offset = 0) {
    printStreamV(out,depth); out << endl;
  }
  void print(void) { printStreamV(cerr); cerr << endl; cerr.flush(); }
  void printLong(void) { printLongStreamV(cerr); cerr.flush(); }
  virtual OZ_Term inspectV() { return 0; }
  virtual OZ_Return     forceBindV(TaggedRef*p, TaggedRef v) {
    return bindV(p,v);
  }
};

inline
int oz_isExtVar(TaggedRef r)
{
  return isCVar(r) && tagged2CVar(r)->getType()==OZ_VAR_EXT;
}

inline
ExtVar *oz_getExtVar(TaggedRef r) {
  Assert(oz_isExtVar(r));
  return (ExtVar *) tagged2CVar(r);
}

inline
OZ_Term oz_makeExtVar(ExtVar *ev) {
  return makeTaggedCVar(ev);
}

unsigned int oz_newVarId();

#endif
