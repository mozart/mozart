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

#include "var_ext.hh"
#include "dpBase.hh"
#include "table.hh"

class ProxyManagerVar : public ExtVar {
protected:
  int index;
public:
  ProxyManagerVar(Board *bb,int i)
    : ExtVar(bb), index(i) { }

  OZ_Term statusV() = 0;
  VarStatus checkStatusV() = 0;
  Bool validV(TaggedRef v) { return TRUE; }
  virtual int getIdV() = 0;
  virtual ExtVar *gcV(void) = 0;
  virtual void gcRecurseV(void) = 0;
  virtual void disposeV(void) = 0;
  virtual void printStreamV(ostream &out,int depth = 10) = 0;
  virtual OZ_Return bindV(TaggedRef *vptr, TaggedRef t) = 0;
  OZ_Return unifyV(TaggedRef *vptr, TaggedRef *tPtr);
  OZ_Return addSuspV(TaggedRef *, Suspension susp, int unstable = TRUE) = 0;

  int getIndex() { return index; }
  void gcSetIndex(int i);
};

class ProxyVar : public ProxyManagerVar {
private:
  TaggedRef binding;
public:
  ProxyVar(Board *bb, int i) : ProxyManagerVar(bb,i), binding(0) { }

  int getIdV() { return OZ_EVAR_PROXY; }
  OZ_Term statusV();
  VarStatus checkStatusV();
  ExtVar *gcV() { return new ProxyVar(*this); }
  void gcRecurseV(void);
  void disposeV(void) {
    disposeS();
    freeListDispose(this,sizeof(ProxyVar));
  }

  void printStreamV(ostream &out,int depth = 10) { out << "<dist:pxy>"; }
  OZ_Return bindV(TaggedRef *vptr, TaggedRef t);
  OZ_Return addSuspV(TaggedRef *, Suspension susp, int unstable = TRUE);

  void redirect(TaggedRef *vPtr,TaggedRef val, BorrowEntry *be);
  void acknowledge(TaggedRef *vPtr, BorrowEntry *be);
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

class ProxyList {
public:
  DSite* sd;
  ProxyList *next;
public:
  ProxyList(DSite* s,ProxyList *nxt) :sd(s),next(nxt) {}

  USEFREELISTMEMORY;

  ProxyList *dispose()
  {
    ProxyList *n=next;
    freeListDispose(this,sizeof(ProxyList));
    return n;
  }
  ProxyList *gcProxyList();
};

class ManagerVar : public ProxyManagerVar {
private:
  ProxyList *proxies;
  OzVariable *origVar;
protected:
  OZ_Return sendRedirectToProxies(OZ_Term val, DSite* ackSite);
public:
  ManagerVar(OzVariable *ov, int index)
    :  ProxyManagerVar(ov->getHome1(),index), proxies(0),origVar(ov) {}
  int getIdV() { return OZ_EVAR_MANAGER; }
  OZ_Term statusV();
  VarStatus checkStatusV();
  ExtVar *gcV() { return new ManagerVar(*this); }
  void gcRecurseV(void);
  void printStreamV(ostream &out,int depth = 10) { out << "<dist:mgr>"; }
  OZ_Return bindV(TaggedRef *vptr, TaggedRef t);
  OZ_Return forceBindV(TaggedRef*p, TaggedRef v);
  OZ_Return addSuspV(TaggedRef *, Suspension susp, int unstable = TRUE);
  void disposeV(void) {
    disposeS();
    ProxyList *pl = proxies;
    while (pl) {
      pl=pl->dispose();
    }
    DebugCode(proxies=0);
    if (origVar) {
      oz_var_dispose(origVar);
      DebugCode(origVar=0);
    }
    freeListDispose(this,sizeof(ManagerVar));
  }

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

  void surrender(TaggedRef*, TaggedRef);
  void requested(TaggedRef*);
  void marshal(MsgBuffer*);

  inline
  void localize(TaggedRef *vPtr);
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

/* ---------------------------------------------------------------------- */

OZ_Return sendRedirect(DSite*, int, TaggedRef);
OZ_Term unmarshalVarImpl(MsgBuffer*);
Bool marshalVariableImpl(TaggedRef *tPtr, MsgBuffer *bs);

#endif
