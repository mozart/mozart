/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Per Brand (perbrand@sics.se)
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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
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

#include "var_lazy.hh"

typedef enum {
  OBJECT,
  OBJECT_AND_CLASS
} LazyFlag ;

enum PV_TYPES {
  PV_OBJECTCLASSAVAIL,     // class available
  PV_OBJECTCLASSNOTAVAIL   // only the class's gname known
};

class ObjectVar : public LazyVar {
protected:
  short pvtype;			// object class types (from above);
  union {
    TaggedRef aclass;
    GName *gnameClass;
  } u;

protected:
  void setpvType(PV_TYPES t) { pvtype = (short) t; }
  PV_TYPES getpvType()       { return (PV_TYPES) pvtype; }

public:
  ObjectVar(Board *bb, int indexIn, GName *gobjIn, ObjectClass *cl)
    : LazyVar(bb, indexIn, gobjIn)
  {
    setpvType(PV_OBJECTCLASSAVAIL);
    Assert(cl);
    u.aclass= makeTaggedConst(cl);
  }
  ObjectVar(Board *bb, int indexIn, GName *gobjIn, GName *gn)
    : LazyVar(bb, indexIn, gobjIn)
  {
    setpvType(PV_OBJECTCLASSNOTAVAIL);
    u.gnameClass=gn;
  }

  virtual LazyType getLazyType();
  // 'sendRequest' defines what is to be done for a particular lazy
  // var type:
  virtual void sendRequest();
  // New (extended) format;
  OzVariable * gCollectV() { return new ObjectVar(*this); }
  void gCollectRecurseV(void);

  void disposeV(void);

  Bool isObjectClassAvail() {
    return getpvType()==PV_OBJECTCLASSAVAIL;
  }
  Bool isObjectClassNotAvail() {
    return getpvType()==PV_OBJECTCLASSNOTAVAIL;
  }

  void setClassTerm(OZ_Term cl) {
    Assert(isObjectClassNotAvail());
    Assert(cl);
    setpvType(PV_OBJECTCLASSAVAIL);
    u.aclass = cl;
  }

  GName *getGNameClass() {
    Assert(isObjectClassNotAvail());
    return u.gnameClass;
  }

  OZ_Term getClass() { 
    Assert(isObjectClassAvail()); 
    Assert(u.aclass);
    return (u.aclass); 
  }

public:
  virtual void marshal(ByteBuffer *);
  //
  void transfer(Object *o, BorrowEntry *be);
};

//
TaggedRef newObjectProxy(int bi, GName *gnobj, GName *gnclass);
TaggedRef newObjectProxy(int bi, GName *gnobj, TaggedRef clas);

#endif
