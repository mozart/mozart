/*
 *  Authors:
 *    Konstantin Popov <kost@sics.se>
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Konstantin Popov (2000)
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

#ifndef __VAR_EMANAGER_HH
#define __VAR_EMANAGER_HH

#if defined(INTERFACE)
#pragma interface
#endif

#include "dpBase.hh"
#include "var_ext.hh"
#include "var.hh"
#include "table.hh"

//
// ExportedManagerVar keeps information that will be put into a stream
// during marshaling (see dpMarshaler.hh, comments before the
// DPMarshaler's definition).
//

//
class ExportedManagerVar : public ExtVar {
private:
  Bool isMarshaled;
  int oti;
  Credit credit;
  int tag;			// isFuture + isAuto;

  //
public:
  ExportedManagerVar(ManagerVar *mv, DSite *dest);
  virtual ~ExportedManagerVar() { Assert(0); }

  //
  virtual ExtVarType    getIdV() { return (OZ_EVAR_EMANAGER); }
  virtual ExtVar*       gCollectV() { return new ExportedManagerVar(*this); }
  virtual void          gCollectRecurseV() {
    DebugCode(PD((GC, "ExportedProxyVar oti:%d", oti)););
  }
  virtual ExtVar*   sCloneV() {
    Assert(0);
    return ((ExportedManagerVar *) 0);
  }
  virtual void          sCloneRecurseV() { Assert(0); }

  //
  virtual OZ_Return     unifyV(TaggedRef *lPtr, TaggedRef *rPtr) {
    Assert(0);
    return (FAILED);
  }
  virtual OZ_Return     bindV(TaggedRef *lPtr, TaggedRef valIn) {
    Assert(0);
    return (FAILED);
  }

  //    
  virtual Bool          validV(TaggedRef) {
    Assert(0);
    return (TRUE);
  }
  virtual OZ_Term       statusV() {
    Assert(0);
    return ((OZ_Term) 0);
  }
  virtual VarStatus     checkStatusV() {
    return (EVAR_STATUS_UNKNOWN);
  }

  //
  virtual void          disposeV();

  //
  virtual OZ_Return addSuspV(TaggedRef *, Suspendable *susp) {
    Assert(0);
    return (FAILED);
  }
  virtual int getSuspListLengthV() {
    Assert(0);
    return (0);
  }

  //
  // virtual void printStreamV(ostream &out, int depth);
  // virtual void printLongStreamV(ostream &out, int depth, int offset);
  // void print(void);
  // void printLong(void);

  //
  virtual OZ_Return forceBindV(TaggedRef *p, TaggedRef v) {
    Assert(0);
    return (FAILED);
  }

  //
  void marshal(ByteBuffer *bs);
};

//
inline
Bool oz_isEManagerVar(TaggedRef v) {
  return (oz_isExtVar(v) && oz_getExtVar(v)->getIdV() == OZ_EVAR_EMANAGER);
}

//
inline
ExportedManagerVar* oz_getEManagerVar(TaggedRef v) {
  Assert(oz_isEManagerVar(v));
  return ((ExportedManagerVar *) oz_getExtVar(v));
}
inline
ExportedManagerVar* getEManagerVar(TaggedRef *tPtr) {
  return (oz_getEManagerVar(*tPtr));
}

#endif // __VAR_EMANAGER_HH
