/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Michael Mehl (1997,1998)
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

#ifndef __VAR_OBJ__HH__
#define __VAR_OBJ__HH__

#if defined(INTERFACE)
#pragma interface
#endif

#include "dpBase.hh"
#include "var_ext.hh"

enum PV_TYPES {
  PV_OBJECTCLASSAVAIL,     // class available
  PV_OBJECTCLASSNOTAVAIL   // only the class's gname known
};

class ObjectVar : public ExtVar {
protected:
  short pvtype;
  short requested;
  Object *obj;
  union {
    ObjectClass *aclass;
    GName *gnameClass;
  } u;

protected:
  void setpvType(PV_TYPES t) { pvtype = (short) t; }
  PV_TYPES getpvType()       { return (PV_TYPES) pvtype; }

public:
  ObjectVar(Board *bb,Object *o,ObjectClass *cl) : ExtVar(bb) {
    setpvType(PV_OBJECTCLASSAVAIL);
    obj   = o;
    u.aclass=cl;
    requested = 0;
  }
  ObjectVar(Board *bb,Object *o,GName *gn) : ExtVar(bb) {
    setpvType(PV_OBJECTCLASSNOTAVAIL);
    obj   = o;
    u.gnameClass=gn;
    requested = 0;
  }

  int getIdV() { return OZ_EVAR_OBJECT; }
  OZ_Term statusV();
  VarStatus checkStatusV();
  OZ_Return addSuspV(TaggedRef *v, Suspension susp, int unstable);
  Bool validV(TaggedRef v) { return FALSE; }
  ExtVar *gcV() { return new ObjectVar(*this); }
  void gcRecurseV(void);
  void printStreamV(ostream &out,int depth = 10) { out << "<dist:oprxy>"; }
  OZ_Return bindV(TaggedRef *vptr, TaggedRef t);
  OZ_Return unifyV(TaggedRef *vptr, TaggedRef *tPtr);
  void disposeV(void);

private:
  Bool isObjectClassAvail()    { return getpvType()==PV_OBJECTCLASSAVAIL; }
  Bool isObjectClassNotAvail() { return getpvType()==PV_OBJECTCLASSNOTAVAIL; }

  void setClass(ObjectClass *cl) {
    Assert(isObjectClassAvail());
    u.aclass=cl;
  }

  GName *getGNameClass() {
    Assert(isObjectClassNotAvail());
    return u.gnameClass;
  }
  Object *getObject() { return obj; }
  ObjectClass *getClass() { Assert(isObjectClassAvail()); return u.aclass; }

public:
  void marshal(MsgBuffer*);
  void sendObject(DSite*, int, ObjectFields&, BorrowEntry*);
  void sendObjectAndClass(ObjectFields&, BorrowEntry*);
};

inline
Bool oz_isObjectVar(TaggedRef v) {
  return oz_isExtVar(v) && oz_getExtVar(v)->getIdV()==OZ_EVAR_OBJECT;
}

inline
ObjectVar *oz_getObjectVar(TaggedRef v) {
  Assert(oz_isObjectVar(v));
  return (ObjectVar*) oz_getExtVar(v);
}


TaggedRef newObjectProxy(Object*, GName*, GName*, TaggedRef);

#endif
