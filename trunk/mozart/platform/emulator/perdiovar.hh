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
  PV_OBJECT,      // class available
  PV_OBJECTGNAME  // only the class's gname known
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
  Thread *thread;
  PendBinding *next;
public:
  PendBinding() { DebugCheckT(val=4711; thread=0; next=this;) }
  PendBinding(TaggedRef v,Thread *th,PendBinding *nxt)
    : val(v), thread(th), next(nxt) {}
  USEFREELISTMEMORY;

  void dispose() 
  {
    freeListDispose(this,sizeof(PendBinding));
  }
  PendBinding *gcPendBinding();
};

class PerdioVar: public GenCVariable {
  PV_TYPES pvtype;
  void *ptr;
  union {
    PendBinding *bindings;
    ProxyList *proxies;
    ObjectClass *aclass;
    GName *gnameClass;
  } u;
public:
  void setpvType(PV_TYPES t) { pvtype = t; }
  PV_TYPES getpvType()       { return pvtype; }

  NO_DEFAULT_CONSTRUCTORS2(PerdioVar);
  PerdioVar() : GenCVariable(PerdioVariable) {
    u.proxies=0;
    setpvType(PV_MANAGER);
  }

  PerdioVar(int i) : GenCVariable(PerdioVariable) {
    u.bindings=0;
    setpvType(PV_PROXY);
    setIndex(i);
  }

  PerdioVar(Object *o) : GenCVariable(PerdioVariable) {
    setpvType(PV_OBJECT);
    ptr = o;
  }

  void setClass(ObjectClass *cl) {
    Assert(isObject());
    u.aclass=cl;
  }

  void setGNameClass(GName *gn) {
    setpvType(PV_OBJECTGNAME);
    u.gnameClass=gn;
  }

  void globalize(int i) { setpvType(PV_MANAGER); ptr = ToPointer(i); }

  Bool isManager()   { return getpvType()==PV_MANAGER; }
  Bool isProxy()     { return getpvType()==PV_PROXY; }
  Bool isObject()    { return getpvType()==PV_OBJECT; }
  Bool isObjectGName() { return getpvType()==PV_OBJECTGNAME; }

  int getIndex() { return ToInt32(ptr); }

  GName *getGName() { return 0; }
  GName *getGNameClass() { Assert(isObjectGName()); return u.gnameClass; }
  void setIndex(int i) {
    Assert(!isObject() && !isObjectGName());
    ptr = ToPointer(i);
  }

  Bool valid(TaggedRef *varPtr, TaggedRef v);
  
  Object *getObject() { Assert(isObject() || isObjectGName()); return (Object*)ptr; }
  ObjectClass *getClass() { Assert(isObject()); return u.aclass; }

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

  void primBind(TaggedRef *lPtr,TaggedRef v);
  Bool unifyPerdioVar(TaggedRef * vptr, TaggedRef * tptr, ByteCode *);

  int hasVal() { Assert(isProxy()); return u.bindings!=0; }
  void setVal(OZ_Term t) {
    Assert(isProxy());
    Assert(u.bindings==0);
    oz_suspendOnNet(am.currentThread());
    PD((THREAD_D,"stop thread setVal %x",am.currentThread()));
    u.bindings=new PendBinding(t,am.currentThread(),0);
  }
  void pushVal(OZ_Term t) {
    Assert(isProxy());
    Assert(u.bindings!=0);
    oz_suspendOnNet(am.currentThread());
    PD((THREAD_D,"stop thread pushVal %x",am.currentThread()));
    u.bindings->next=new PendBinding(t,am.currentThread(),u.bindings->next);
  }
  void redirect(OZ_Term val);
  void acknowledge(OZ_Term *ptr);

  ProxyList *getProxies() { Assert(isManager()); return u.proxies; }

  void addSuspPerdioVar(TaggedRef *v,Thread *el, int unstable);

  void gc(void);
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
