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

#ifndef __VAR__HH__
#define __VAR__HH__

#if defined(INTERFACE)
#pragma interface
#endif

#include "base.hh"
#include "debug.hh"
#include "var_ext.hh"
#include "table.hh"
#include "controlvar.hh"

enum PV_TYPES {
  PV_MANAGER,
  PV_PROXY,
  PV_OBJECTCLASSAVAIL,     // class available
  PV_OBJECTCLASSNOTAVAIL   // only the class's gname known
};

class ProxyList {
public:
  DSite* sd;
  ProxyList *next;
public:
  ProxyList(DSite* s,ProxyList *nxt) :sd(s),next(nxt) {}

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

class PerdioVar: public ExtVar {
public:
  PerdioVar(Board *bb) : ExtVar(bb) {}

  int getIdV() { return OZ_EVAR_DIST; }
  virtual VariableStatus statusV() = 0;
  virtual OZ_Term isDetV() = 0;

  virtual PerdioVar *gcV(void) = 0;
  virtual void gcRecurseV(void) = 0;
  virtual Bool validV(TaggedRef v) = 0;
  virtual OZ_Return unifyV(TaggedRef *vptr, TaggedRef t, ByteCode *scp) = 0;
  virtual void addSuspV(TaggedRef *v, Suspension susp, int unstable) = 0;
  virtual void printStreamV(ostream &out,int depth = 10) = 0;

  // check if entry in borrow table is still alive
  virtual Bool gcIsAliveV() = 0;
  virtual void registerSiteV(DSite* sd) = 0;
  virtual void proxyBindV(TaggedRef *vPtr,TaggedRef val, BorrowEntry *be) = 0;
  virtual void managerBindV(TaggedRef *vPtr, TaggedRef val,
                            OwnerEntry *oe, DSite *rsite, int OTI) = 0;
  virtual void proxyAckV(TaggedRef *vPtr, BorrowEntry *be) = 0;
  virtual void marshalV(MsgBuffer *bs) = 0;
};

inline
int oz_isPerdioVar(TaggedRef r)
{
  return oz_isExtVar(r) && oz_getExtVar(r)->getIdV()==OZ_EVAR_DIST;
}

class DistributedVar : public PerdioVar {
public:
  DistributedVar() : PerdioVar(0) {}
};

class LazyObjVar : public PerdioVar {
public:
  LazyObjVar() : PerdioVar(0) {}
};

class OldPerdioVar : public PerdioVar {
protected:
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

  NO_DEFAULT_CONSTRUCTORS2(OldPerdioVar);
  OldPerdioVar(Board *bb) : PerdioVar(bb) {
    u.proxies=0;
    setpvType(PV_MANAGER);
  }

  OldPerdioVar(int i,Board *bb) : PerdioVar(bb) {
    u.bindings=0;
    setpvType(PV_PROXY);
    setIndex(i);
  }

  OldPerdioVar(Object *o,Board *bb) : PerdioVar(bb) {
    setpvType(PV_OBJECTCLASSAVAIL);
    ptr   = o;
  }

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

  Bool gcIsAliveV() {
    return getSuspList() || (isProxy() && hasVal());
  }

  // mm2: OZ_DISTRIBUTED! and OZ_LAZY!
  VariableStatus statusV() { return isObject()?OZ_OTHER:OZ_FREE; }

  OZ_Term isDetV();

  int getIndex() { return ToInt32(ptr); }

  GName *getGNameClass() { Assert(isObjectClassNotAvail()); return u.gnameClass; }
  void setIndex(int i) {
    Assert(!isObject());
    ptr = ToPointer(i);
  }

  Object *getObject() { Assert(isObject()); return (Object*)ptr; }
  ObjectClass *getClass() { Assert(isObjectClassAvail()); return u.aclass; }

  void registerSiteV(DSite* sd) {
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
  void proxyAckV(TaggedRef *vPtr, BorrowEntry *be);

  ProxyList *getProxies() { Assert(isManager()); return u.proxies; }

  void addSuspV(TaggedRef *v, Suspension susp, int unstable);
  Bool validV(TaggedRef v) {
    Assert(!oz_isRef(v) && !oz_isVariable(v));
    return (isObject()) ? FALSE : TRUE;
  }

  void primBind(TaggedRef *lPtr,TaggedRef v);

  void dispose(void) {}

  OldPerdioVar *gcV() { return new OldPerdioVar(*this); }
  void gcRecurseV(void);

  OZ_Return unifyV(TaggedRef *vptr, TaggedRef t, ByteCode *scp);
  void printStreamV(ostream &out,int depth = 10) {
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
    printStreamV(out,depth); out << endl;
  }

  void proxyBindV(TaggedRef *vPtr,TaggedRef val, BorrowEntry *be);
  void managerBindV(TaggedRef *vPtr, TaggedRef val,
                    OwnerEntry *oe, DSite *rsite, int OTI);
  void marshalV(MsgBuffer *bs);
};

Bool checkExportable(TaggedRef var);
OldPerdioVar* var2PerdioVar(TaggedRef*);

inline
OldPerdioVar *tagged2PerdioVar(TaggedRef t) {
  Assert(oz_isPerdioVar(t));
  return (OldPerdioVar *) oz_getExtVar(t);
}

TaggedRef newObjectProxy(Object *o, GName *gnobj,
                         GName *gnclass, TaggedRef clas);

#endif
