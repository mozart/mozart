/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __dvar__hh__
#define __dvar__hh__


#if defined(INTERFACE)
#pragma interface
#endif

#include "genvar.hh"
#include "oz.h"

enum PV_TYPES {
  PV_MANAGER,
  PV_PROXY,
  PV_OBJECTURL,   // class available maybe only as URL
  PV_OBJECTGNAME, // only the class's gname known
  PV_URL
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
    TaggedRef aclass;
    TaggedRef url;
    GName *gnameClass;
  } u;
public:
  void setpvType(PV_TYPES t) { pvtype = t; }
  PV_TYPES getpvType()       { return pvtype; }

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
    setpvType(PV_OBJECTURL);
    ptr = o;
  }

  void setClass(TaggedRef cl) {
    Assert(isObjectURL());
    u.aclass=cl;
  }

  void setGNameClass(GName *gn) {
    setpvType(PV_OBJECTGNAME);
    u.gnameClass=gn;
  }
  PerdioVar(GName *gname, TaggedRef url) : GenCVariable(PerdioVariable) {
    u.url = url;
    setpvType(PV_URL);
    ptr = gname;
  }

  void globalize(int i) { setpvType(PV_MANAGER); ptr = ToPointer(i); }

  Bool isManager()   { return getpvType()==PV_MANAGER; }
  Bool isProxy()     { return getpvType()==PV_PROXY; }
  Bool isObjectURL() { return getpvType()==PV_OBJECTURL; }
  Bool isObjectGName() { return getpvType()==PV_OBJECTGNAME; }
  Bool isURL()       { return getpvType()==PV_URL; }

  int getIndex() { return ToInt32(ptr); }

  GName *getGName() { return isURL() ? (GName *) ptr : 0; }
  GName *getGNameClass() { Assert(isObjectGName()); return u.gnameClass; }
  TaggedRef getURL() { Assert(isURL()); return u.url; }
  void setIndex(int i) {
    Assert(!isURL() && !isObjectURL() && !isObjectGName());
    ptr = ToPointer(i);
  }

  Bool valid(TaggedRef *varPtr, TaggedRef v);
  
  size_t getSize(void) { return sizeof(PerdioVar); }
  
  Object *getObject() { Assert(isObjectURL() || isObjectGName()); return (Object*)ptr; }
  TaggedRef getClass() { Assert(isObjectURL()); return u.aclass; }

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
  Bool unifyPerdioVar(TaggedRef * vptr, TaggedRef * tptr, Bool prop);

  int hasVal() { Assert(isProxy()); return u.bindings!=0; }
  void setVal(OZ_Term t) {
    Assert(isProxy());
    Assert(u.bindings==0);
    oz_stop(oz_currentThread);
    PD((THREAD_D,"stop thread setVal %x",oz_currentThread));
    u.bindings=new PendBinding(t,oz_currentThread,0);
  }
  void pushVal(OZ_Term t) {
    Assert(isProxy());
    Assert(u.bindings!=0);
    oz_stop(oz_currentThread);
    PD((THREAD_D,"stop thread pushVal %x",oz_currentThread));
    u.bindings->next=new PendBinding(t,oz_currentThread,u.bindings->next);
  }
  void redirect(OZ_Term val);
  void acknowledge(OZ_Term *ptr);

  ProxyList *getProxies() { Assert(isManager()); return u.proxies; }

  void addSuspPerdioVar(Thread *el);

  void gcPerdioVar(void);
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

inline
Bool isURL(TaggedRef term)
{
  return isPerdioVar(term) && tagged2PerdioVar(term)->isURL();
}

#endif
