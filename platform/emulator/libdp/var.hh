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
  virtual OZ_Return bindV(TaggedRef *vptr, TaggedRef t) = 0;

  OZ_Return unifyV(TaggedRef *vptr, TaggedRef *tPtr);
};

class ProxyManagerVar : public PerdioVar {
protected:
  int index;
  TaggedRef binding;
public:
  ProxyManagerVar(Board *bb,int i)
    : PerdioVar(bb), index(i), binding(0) { }

  VariableStatus statusV() { return OZ_FREE; }
  Bool validV(TaggedRef v) { return TRUE; }
  virtual int getIdV() = 0;
  virtual OZ_Term isDetV() = 0;
  virtual PerdioVar *gcV(void) = 0;
  virtual void gcRecurseV(void) = 0;
  virtual void printStreamV(ostream &out,int depth = 10) = 0;
  virtual OZ_Return bindV(TaggedRef *vptr, TaggedRef t) = 0;

  int getIndex() { return index; }
  void gcSetIndex(int i) { index =  i; }
};

class ProxyVar : public ProxyManagerVar {
public:
  ProxyVar(Board *bb, int i) : ProxyManagerVar(bb,i) { }

  int getIdV() { return OZ_EVAR_PROXY; }
  VariableStatus statusV() { return OZ_FREE; }
  OZ_Term isDetV();
  PerdioVar *gcV() { return new ProxyVar(*this); }
  void gcRecurseV(void);
  void printStreamV(ostream &out,int depth = 10) { out << "<dist:pxy>"; }
  OZ_Return bindV(TaggedRef *vptr, TaggedRef t);

  void proxyBind(TaggedRef *vPtr,TaggedRef val, BorrowEntry *be);
  void proxyAck(TaggedRef *vPtr, BorrowEntry *be);
  void marshal(MsgBuffer*);
};

inline
Bool oz_isProxyVar(TaggedRef v) {
  return oz_isExtVar(v) && oz_getExtVar(v)->getIdV()==OZ_EVAR_PROXY;
}

inline
ProxyVar *oz_getProxyVar(TaggedRef v) {
  Assert(oz_isProxyVar(v));
  return (ProxyVar*) oz_getExtVar(v);
}

// #define ORIG
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
  OZ_Return bindV(TaggedRef *vptr, TaggedRef t);

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
  void marshal(MsgBuffer*);
};

inline
Bool oz_isManagerVar(TaggedRef v) {
  return oz_isExtVar(v) && oz_getExtVar(v)->getIdV()==OZ_EVAR_MANAGER;
}

inline
ManagerVar *oz_getManagerVar(TaggedRef v) {
  Assert(oz_isManagerVar(v));
  return (ManagerVar*) oz_getExtVar(v);
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
  ObjectVar(Board *bb,Object *o,ObjectClass *cl) : PerdioVar(bb) {
    setpvType(PV_OBJECTCLASSAVAIL);
    obj   = o;
    u.aclass=cl;
  }
  ObjectVar(Board *bb,Object *o,GName *gn) : PerdioVar(bb) {
    setpvType(PV_OBJECTCLASSNOTAVAIL);
    obj   = o;
    u.gnameClass=gn;
  }

  int getIdV() { return OZ_EVAR_OBJECT; }
  VariableStatus statusV() { return OZ_OTHER; } // mm2: OZ_LAZY!
  OZ_Term isDetV() { return OZ_true(); }
  void addSuspV(TaggedRef *v, Suspension susp, int unstable);
  Bool validV(TaggedRef v) { return FALSE; }
  PerdioVar *gcV() { return new ObjectVar(*this); }
  void gcRecurseV(void);
  void printStreamV(ostream &out,int depth = 10) { out << "<dist:oprxy>"; }
  OZ_Return bindV(TaggedRef *vptr, TaggedRef t);

  void disposeV();

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


/* ---------------------------------------------------------------------- */

OZ_Return sendRedirect(DSite*, int, TaggedRef);
TaggedRef newObjectProxy(Object*, GName*, GName*, TaggedRef);
OZ_Term unmarshalVar(MsgBuffer*);
Bool marshalVariable(TaggedRef *tPtr, MsgBuffer *bs);

#endif
