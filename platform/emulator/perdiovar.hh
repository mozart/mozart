/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Michael Mehl (1997)
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __dvar__hh__
#define __dvar__hh__


#if defined(INTERFACE)
#pragma interface
#endif

#include "am.hh"
#include "genvar.hh"
#include "oz.h"
#include "perdio_debug.hh"

enum PV_TYPES {
  PV_MANAGER,
  PV_PROXY,
  PV_OBJECTCLASSAVAIL,     // class available
  PV_OBJECTCLASSNOTAVAIL   // only the class's gname known
};

class ProxyList {
public:
  Site* sd;
  ProxyList *next;
public:
  ProxyList(Site* s,ProxyList *nxt) :sd(s),next(nxt) {}

  USEFREELISTMEMORY;

  void dispose() 
  {
    freeListDispose(this,sizeof(ProxyList));
  }
  ProxyList *gcProxyList();

};

class PendBinding {
public:
  TaggedRef val;
  TaggedRef controlvar;
  PendBinding *next;
public:
  PendBinding() { DebugCheckT(val=4711; controlvar=0; next=this;) }
  PendBinding(TaggedRef v,TaggedRef cv,PendBinding *nxt)
    : val(v), controlvar(cv), next(nxt) {}
  USEFREELISTMEMORY;

  void dispose() 
  {
    freeListDispose(this,sizeof(PendBinding));
  }
  PendBinding *gcPendBinding();
};

class PerdioVar: public GenCVariable {
protected:
  short isfuture;
private:
  short pvtype;
  void *ptr;
  union {
    PendBinding *bindings;
    ProxyList *proxies;
    ObjectClass *aclass;
    GName *gnameClass;
  } u;
public:
  void setpvType(PV_TYPES t) { pvtype = (short) t; }
  PV_TYPES getpvType()       { return (PV_TYPES) pvtype; }

  NO_DEFAULT_CONSTRUCTORS2(PerdioVar);
  PerdioVar(Bool isf) : GenCVariable(PerdioVariable) {
    u.proxies=0;
    isfuture = (short) isf;
    setpvType(PV_MANAGER);
  }

  PerdioVar(int i, Bool isf) : GenCVariable(PerdioVariable) {
    u.bindings=0;
    isfuture = (short) isf;
    setpvType(PV_PROXY);
    setIndex(i);
  }

  PerdioVar(Object *o) : GenCVariable(PerdioVariable) {
    setpvType(PV_OBJECTCLASSAVAIL);
    ptr = o;
    isfuture = 0;
  }

  int isFuture() { return (int) isfuture; }

  void setClass(ObjectClass *cl) {
    Assert(isObjectClassAvail());
    u.aclass=cl;
  }

  void setGNameClass(GName *gn) {
    setpvType(PV_OBJECTCLASSNOTAVAIL);
    u.gnameClass=gn;
  }

  void globalize(int i) { setpvType(PV_MANAGER); ptr = ToPointer(i); }

  Bool isManager()             { return getpvType()==PV_MANAGER; }
  Bool isProxy()               { return getpvType()==PV_PROXY; }
  Bool isObjectClassAvail()    { return getpvType()==PV_OBJECTCLASSAVAIL; }
  Bool isObjectClassNotAvail() { return getpvType()==PV_OBJECTCLASSNOTAVAIL; }
  Bool isObject() { return isObjectClassAvail() || isObjectClassNotAvail();}

  int getIndex() { return ToInt32(ptr); }

  GName *getGNameClass() { Assert(isObjectClassNotAvail()); return u.gnameClass; }
  void setIndex(int i) {
    Assert(!isObject());
    ptr = ToPointer(i);
  }

  Object *getObject() { Assert(isObject()); return (Object*)ptr; }
  ObjectClass *getClass() { Assert(isObjectClassAvail()); return u.aclass; }

  void registerSite(Site* sd) {
    Assert(isManager());
    u.proxies = new ProxyList(sd,u.proxies);
  }

  Bool isRegistered(Site* sd) {
    Assert(isManager());
    for (ProxyList *pl = u.proxies; pl != 0; pl=pl->next) {
      if (pl->sd==sd) return OK;
    }
    return NO;
  }

  int hasVal() { Assert(isProxy()); return u.bindings!=0; }
  OZ_Return setVal(OZ_Term t) {
    Assert(isProxy());
    Assert(u.bindings==0);
    ControlVarNew(controlvar,GETBOARD(this));
    PD((THREAD_D,"stop thread setVal %x",am.currentThread()));
    u.bindings=new PendBinding(t,controlvar,0);
    SuspendOnControlVar;
  }
  OZ_Return pushVal(OZ_Term t) {
    Assert(isProxy());
    Assert(u.bindings!=0);
    ControlVarNew(controlvar,GETBOARD(this));
    PD((THREAD_D,"stop thread pushVal %x",am.currentThread()));
    u.bindings->next=new PendBinding(t,controlvar,u.bindings->next);
    SuspendOnControlVar;
  }
  void redirect(OZ_Term val);
  void acknowledge(OZ_Term *ptr);

  ProxyList *getProxies() { Assert(isManager()); return u.proxies; }

  void addSuspPerdioVar(TaggedRef *v,Thread *el, int unstable);
  Bool valid(TaggedRef *varPtr, TaggedRef v);
  void primBind(TaggedRef *lPtr,TaggedRef v);
  OZ_Return unifyPerdioVar(TaggedRef * vptr, TaggedRef * tptr, ByteCode *);

  void dispose(void);

  void gcRecurse(void);
};

inline
Bool isPerdioVar(TaggedRef term)
{
  GCDEBUG(term);
  return isCVar(term) && (tagged2CVar(term)->getType() == PerdioVariable);
}

inline
PerdioVar *tagged2PerdioVar(TaggedRef t) {
  Assert(isPerdioVar(t));
  return (PerdioVar *) tagged2CVar(t);
}


#endif
