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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __perdiovar__hh__
#define __perdiovar__hh__


#if defined(INTERFACE)
#pragma interface
#endif

#include "controlvar.hh"
#include "genvar.hh"
#include "oz.h"
#include "perdio_debug.hh"
#include "am.hh"

char *oz_site2String(Site *s);

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

#define PV_EXPORTED 0x1 /* non-exported futures look like PerdioVars,
                           but can be bound to non-exportables */

class PerdioVar: public GenCVariable {
protected:
  short flags;
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

  PerdioVar(Board *bb) : GenCVariable(PerdioVariable,bb) {
    u.proxies=0;
    flags = 0;
    setpvType(PV_MANAGER);
  }

  PerdioVar(int i,Board *bb) : GenCVariable(PerdioVariable,bb) {
    u.bindings=0;
    flags = 0;
    setpvType(PV_PROXY);
    setIndex(i);
  }

  PerdioVar(Object *o,Board *bb) : GenCVariable(PerdioVariable,bb) {
    setpvType(PV_OBJECTCLASSAVAIL);
    ptr   = o;
    flags = 0;
  }

  int isExported()    { return (flags&PV_EXPORTED); }
  void markExported() { flags |= PV_EXPORTED; }

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
    // test if already registered
    for (ProxyList *pl = u.proxies; pl != 0; pl=pl->next) {
      if (pl->sd==sd) {
        PD((WEIRD,"REGISTER o:%d s:%s already registered",
            getIndex(),
            oz_site2String(sd)));
        return;
      }
    }
    u.proxies = new ProxyList(sd,u.proxies);
  }

  int hasVal() { Assert(isProxy()); return u.bindings!=0; }
  OZ_Return setVal(OZ_Term t) {
    Assert(isProxy());
    Assert(u.bindings==0);
    ControlVarNew(controlvar,GETBOARD(this));
    PD((THREAD_D,"stop thread setVal %x",oz_currentThread()));
    u.bindings=new PendBinding(t,controlvar,0);
    SuspendOnControlVar;
  }
  OZ_Return pushVal(OZ_Term t) {
    Assert(isProxy());
    Assert(u.bindings!=0);
    ControlVarNew(controlvar,GETBOARD(this));
    PD((THREAD_D,"stop thread pushVal %x",oz_currentThread()));
    u.bindings->next=new PendBinding(t,controlvar,u.bindings->next);
    SuspendOnControlVar;
  }
  void redirect(OZ_Term val);
  void acknowledge(OZ_Term *ptr);

  ProxyList *getProxies() { Assert(isManager()); return u.proxies; }

  void addSusp(TaggedRef *v, Suspension susp, int unstable);
  Bool valid(TaggedRef v) {
    Assert(!oz_isRef(v) && !oz_isVariable(v));
    return (isObject()) ? FALSE : TRUE;
  }

  void primBind(TaggedRef *lPtr,TaggedRef v);

  void dispose(void) {}

  void gcRecurse(void);

  OZ_Return unify(TaggedRef *vptr, TaggedRef t, ByteCode *scp);
  void printStream(ostream &out,int depth = 10) {
    out << "<dist:";
    char *type = "";
    if (isManager()) {
      type = "mgr";
    } else if (isProxy()) {
      type = "pxy";
    } else {
      type = "oprxy";
    }
    out << type << ">";
  }
  void printLongStream(ostream &out,int depth = 10,
                        int offset = 0) {
    printStream(out,depth); out << endl;
  }
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
