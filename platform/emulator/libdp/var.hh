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

  virtual int getIdV() = 0;
  virtual VariableStatus statusV() = 0;
  virtual OZ_Term isDetV() = 0;
  virtual PerdioVar *gcV(void) = 0;
  virtual void gcRecurseV(void) = 0;
  virtual Bool validV(TaggedRef v) = 0;
  virtual void printStreamV(ostream &out,int depth = 10) = 0;

  virtual Bool gcIsAliveV() = 0;
  virtual void marshalV(MsgBuffer *bs) = 0;
  virtual OZ_Return doBindPV(TaggedRef *lPtr, TaggedRef v) = 0;
  virtual void primBind(TaggedRef *lPtr,TaggedRef v);

  OZ_Return unifyV(TaggedRef *vptr, TaggedRef *tPtr, ByteCode *scp);
  OZ_Return bindV(TaggedRef *vptr, TaggedRef t, ByteCode *scp);
};

class ProxyManagerVar : public PerdioVar {
protected:
  int index;
public:
  ProxyManagerVar(Board *bb,int i) : PerdioVar(bb), index(i) { }

  VariableStatus statusV() { return OZ_FREE; }
  Bool validV(TaggedRef v) { return TRUE; }
  virtual int getIdV() = 0;
  virtual OZ_Term isDetV() = 0;
  virtual PerdioVar *gcV(void) = 0;
  virtual void gcRecurseV(void) = 0;
  virtual void printStreamV(ostream &out,int depth = 10) = 0;

  virtual void marshalV(MsgBuffer *bs) = 0;
  virtual Bool gcIsAliveV() = 0;
  virtual OZ_Return doBindPV(TaggedRef *lPtr, TaggedRef v) = 0;

  int getIndex() { return index; }
  void gcSetIndex(int i) { index =  i; }
};

class ProxyVar : public ProxyManagerVar {
protected:
  PendBinding *bindings;
protected:
  void redirect(OZ_Term val);
public:
  ProxyVar(Board *bb, int i) : ProxyManagerVar(bb,i), bindings(0) { }

  int getIdV() { return OZ_EVAR_PROXY; }
  VariableStatus statusV() { return OZ_FREE; }
  OZ_Term isDetV();
  PerdioVar *gcV() { return new ProxyVar(*this); }
  void gcRecurseV(void);
  void printStreamV(ostream &out,int depth = 10) { out << "<dist:pxy>"; }

  Bool gcIsAliveV() { return getSuspList()!=0 || hasVal(); }
  virtual void marshalV(MsgBuffer *bs);
  virtual OZ_Return doBindPV(TaggedRef *lPtr, TaggedRef v);

  int hasVal() { return bindings!=0; }

  OZ_Return setVal(OZ_Term t) {
    Assert(bindings==0);
    ControlVarNew(controlvar,GETBOARD(this));
    PD((THREAD_D,"stop thread setVal %x",oz_currentThread()));
    bindings=new PendBinding(t,controlvar,0);
    SuspendOnControlVar;
  }

  OZ_Return pushVal(OZ_Term t) {
    Assert(bindings!=0);
    ControlVarNew(controlvar,GETBOARD(this));
    PD((THREAD_D,"stop thread pushVal %x",oz_currentThread()));
    bindings->next=new PendBinding(t,controlvar,bindings->next);
    SuspendOnControlVar;
  }

  void proxyBind(TaggedRef *vPtr,TaggedRef val, BorrowEntry *be);

  void proxyAck(TaggedRef *vPtr, BorrowEntry *be);
};

inline
Bool oz_isProxyVar(TaggedRef v) {
  return oz_isExtVar(v) && oz_getExtVar(v)->getIdV()==OZ_EVAR_PROXY;
}

class ManagerVar : public ProxyManagerVar {
private:
  ProxyList *proxies;
#ifdef ORIG
  OzVariable *origVar;
#endif
protected:
  OZ_Return sendRedirectToProxies(OZ_Term val, DSite* ackSite);
public:
  ManagerVar(Board *bb, int index) :  ProxyManagerVar(bb,index), proxies(0) {}

  int getIdV() { return OZ_EVAR_MANAGER; }
  OZ_Term isDetV() { return OZ_false(); }
  PerdioVar *gcV() { return new ManagerVar(*this); }
  void gcRecurseV(void);
  void printStreamV(ostream &out,int depth = 10) { out << "<dist:mgr>"; }

  Bool gcIsAliveV() { return getSuspList()!=0; }
  virtual void marshalV(MsgBuffer *bs);
  virtual OZ_Return doBindPV(TaggedRef *lPtr, TaggedRef v);

  void registerSite(DSite* sd) {
    // test if already registered
    for (ProxyList *pl = proxies; pl != 0; pl=pl->next) {
      if (pl->sd==sd) {
	PD((WEIRD,"REGISTER o:%d s:%s already registered",
	    getIndex(),
	    oz_site2String(sd)));
	return;
      }
    }
    proxies = new ProxyList(sd,proxies);
  }
  ProxyList *getProxies() { return proxies; }
  void managerBind(TaggedRef *vPtr, TaggedRef val,
		   OwnerEntry *oe, DSite *rsite);
};

inline
Bool oz_isManagerVar(TaggedRef v) {
  return oz_isExtVar(v) && oz_getExtVar(v)->getIdV()==OZ_EVAR_MANAGER;
}

enum PV_TYPES {
  PV_OBJECTCLASSAVAIL,     // class available
  PV_OBJECTCLASSNOTAVAIL   // only the class's gname known
};

class ObjectVar : public PerdioVar {
protected:
  short pvtype;
  Object *obj;
  union {
    ObjectClass *aclass;
    GName *gnameClass;
  } u;

protected:
  void setpvType(PV_TYPES t) { pvtype = (short) t; }
  PV_TYPES getpvType()       { return (PV_TYPES) pvtype; }

public:
  ObjectVar(Object *o,Board *bb) : PerdioVar(bb) {
    setpvType(PV_OBJECTCLASSAVAIL);
    obj   = o;
  }

  int getIdV() { return OZ_EVAR_OBJECT; }
  VariableStatus statusV() { return OZ_OTHER; } // mm2: OZ_LAZY!
  OZ_Term isDetV() { return OZ_true(); }
  void addSuspV(TaggedRef *v, Suspension susp, int unstable);
  Bool validV(TaggedRef v) { return FALSE; }
  PerdioVar *gcV() { return new ObjectVar(*this); }
  void gcRecurseV(void);
  void printStreamV(ostream &out,int depth = 10) { out << "<dist:oprxy>"; }

  Bool gcIsAliveV() { return getSuspList()!=0; }
  void marshalV(MsgBuffer *bs);
  virtual OZ_Return doBindPV(TaggedRef *lPtr, TaggedRef v);
  virtual void primBind(TaggedRef *lPtr,TaggedRef v);

  Bool isObjectClassAvail()    { return getpvType()==PV_OBJECTCLASSAVAIL; }
  Bool isObjectClassNotAvail() { return getpvType()==PV_OBJECTCLASSNOTAVAIL; }

  void setClass(ObjectClass *cl) {
    Assert(isObjectClassAvail());
    u.aclass=cl;
  }

  void setGNameClass(GName *gn) {
    setpvType(PV_OBJECTCLASSNOTAVAIL);
    u.gnameClass=gn;
  }

  GName *getGNameClass() {
    Assert(isObjectClassNotAvail());
    return u.gnameClass;
  }

  Object *getObject() { return obj; }
  ObjectClass *getClass() { Assert(isObjectClassAvail()); return u.aclass; }
};

inline
Bool oz_isObjectVar(TaggedRef v) {
  return oz_isExtVar(v) && oz_getExtVar(v)->getIdV()==OZ_EVAR_OBJECT;
}

Bool checkExportable(TaggedRef);
TaggedRef newObjectProxy(Object*, GName*, GName*, TaggedRef);
OZ_Return sendRedirect(DSite*, int, TaggedRef);
OZ_Term unmarshalVar(MsgBuffer*);
Bool marshalVariable(TaggedRef *tPtr, MsgBuffer *bs);

#endif
