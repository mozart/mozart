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
  EntityInfo *info;
public:
  ProxyManagerVar(Board *bb,int i)
    : ExtVar(bb), info(NULL), index(i){}
  OZ_Term statusV() = 0;
  VarStatus checkStatusV() = 0;
  Bool validV(TaggedRef v) { return TRUE; }
  virtual int getIdV() = 0;
  virtual OzVariable *gcV(void) = 0;
  virtual void gcRecurseV(void) = 0;
  virtual void disposeV(void) = 0;
  virtual void printStreamV(ostream &out,int depth = 10) = 0;
  virtual OZ_Return bindV(TaggedRef *vptr, TaggedRef t) = 0;
  OZ_Return unifyV(TaggedRef *vptr, TaggedRef *tPtr);
  OZ_Return addSuspV(TaggedRef *, Suspension susp, int unstable = TRUE) = 0;

  int getIndex() { return index; }
  void gcSetIndex(int i);

  // for failure
  EntityInfo *getInfo(){return info;}
  EntityCond *getEntityCond(); 
  void setInfo(EntityInfo* ei){info=ei;}
  Bool errorIgnore(){
    if(info==NULL) return TRUE;
    if(info->getEntityCond()==ENTITY_NORMAL) return TRUE;
    return FALSE;}
};

class ProxyVar : public ProxyManagerVar {
private:
  TaggedRef binding;
  TaggedRef status;
  short is_future;
  short is_auto;
public:
  ProxyVar(Board *bb, int i,Bool isF) : 
    ProxyManagerVar(bb,i), binding(0),status(0), is_future(isF),is_auto(FALSE){ }

  void makeAuto(){is_auto=TRUE;}
  Bool isAuto(){return is_auto==1;}

  int getIdV() { return OZ_EVAR_PROXY; }
  OZ_Term statusV();
  VarStatus checkStatusV();
  OzVariable *gcV() { return new ProxyVar(*this); }
  void gcRecurseV(void);
  void disposeV(void) { // PER-LOOK when is this used
    disposeS();
    freeListDispose(this,sizeof(ProxyVar));
  }

  void printStreamV(ostream &out,int depth = 10) { out << "<dist:pxy>"; }
  OZ_Return bindV(TaggedRef *vptr, TaggedRef t);
  void receiveStatus(TaggedRef);
  OZ_Return addSuspV(TaggedRef *, Suspension susp, int unstable = TRUE);

  void redirect(TaggedRef *vPtr,TaggedRef val, BorrowEntry *be);
  void acknowledge(TaggedRef *vPtr, BorrowEntry *be);
  void marshal(MsgBuffer*);

  Bool isFuture(){ return is_future;}
  void addEntityCond(EntityCond);
  void subEntityCond(EntityCond);
  void probeFault(int);
  void newWatcher(Bool);


  Bool failurePreemption(TaggedRef);
  void wakeAll();

  TaggedRef getTaggedRef();

  void nowGarbage(BorrowEntry*); 
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

enum ProxyListKind {
  EXP_REG=0,
  AUT_REG=1};

class ProxyList {
friend class ManagerVar;
public:
  DSite* sd;
  ProxyListKind kind; 
  ProxyList *next;
public: 
  // pb2
  // ProxyList(DSite* s,ProxyList *nxt) :sd(s),next(nxt),kind(EXP_REG){}
  ProxyList(DSite* s,ProxyList *nxt) :sd(s),next(nxt),kind(AUT_REG){}

  USEFREELISTMEMORY;

  ProxyList *dispose() 
  {
    ProxyList *n=next;
    freeListDispose(this,sizeof(ProxyList));
    return n;
  }
  ProxyList *gcProxyList();

  void unAuto(){kind=EXP_REG;}
};

class ManagerVar : public ProxyManagerVar {
private:
  ProxyList *proxies;
  OzVariable *origVar;
  InformElem *inform; // for failure
protected:
  void sendRedirectToProxies(OZ_Term val, DSite* ackSite);
public:
  ManagerVar(OzVariable *ov, int index)
    :  ProxyManagerVar(ov->getHome1(),index), inform(NULL), 
       proxies(0),origVar(ov) {}

  int getIdV() { return OZ_EVAR_MANAGER; }
  OZ_Term statusV();
  VarStatus checkStatusV();
  OzVariable *gcV() { return new ManagerVar(*this); }
  void gcRecurseV(void);
  void printStreamV(ostream &out,int depth = 10) { out << "<dist:mgr>"; }
  OZ_Return bindV(TaggedRef *vptr, TaggedRef t);
  OZ_Return bindVInternal(TaggedRef *vptr, TaggedRef t,DSite* );
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

  Bool siteInProxyList(DSite*);

  void deregisterSite(DSite* sd);
  void deAutoSite(DSite*);
  void surrender(TaggedRef*, TaggedRef);
  void requested(TaggedRef*);
  void marshal(MsgBuffer*);
  void getStatus(DSite*, int, TaggedRef);

  inline
  void localize(TaggedRef *vPtr);
  Bool isFuture(){ // mm3
    if(origVar->getType()==OZ_VAR_FUTURE) return TRUE;
    return FALSE;}

  // for failure
  void newInform(DSite*, EntityCond);
  void probeFault(DSite *,int);
  void addEntityCond(EntityCond);

  EntityCond getEntityCond(){
    if(info==NULL) return ENTITY_NORMAL;
    return info->getEntityCond();}

  void subEntityCond(EntityCond);
  void newWatcher(Bool);
  TaggedRef getTaggedRef();
  Bool failurePreemption(TaggedRef);
  void wakeAll();
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

void sendRedirect(DSite*, int, TaggedRef);
OZ_Term unmarshalVarImpl(MsgBuffer*,Bool,Bool);
Bool marshalVariableImpl(TaggedRef *tPtr, MsgBuffer *bs, GenTraverser *);

/* ---------------------------------------------------------------------- */

enum VarKind{
  VAR_PROXY,
  VAR_MANAGER,
  VAR_OBJECT,
  VAR_FREE,
  VAR_FUTURE,    
  VAR_KINDED,
};

VarKind classifyVar(TaggedRef *);
VarKind classifyVarLim(TaggedRef);

inline ManagerVar* getManagerVar(TaggedRef *tPtr){
  Assert(classifyVar(tPtr)==VAR_MANAGER);
  return oz_getManagerVar(*tPtr);}

inline ProxyVar* getProxyVar(TaggedRef *tPtr){
  Assert(classifyVar(tPtr)==VAR_PROXY);
  return oz_getProxyVar(*tPtr);}

Watcher *varGetWatchersIfExist(TaggedRef* tPtr);

#define GET_VAR(po,T) oz_get##T##Var(*((po)->getPtr()))
#define GET_TERM(po,T) oz_get##T##Var(*((po)->getAnyPtr()))

VarKind typeOfBorrowVar(BorrowEntry*);

ManagerVar* globalizeFreeVariable(TaggedRef*);
EntityInfo *varMakeOrGetEntityInfo(TaggedRef*);
EntityInfo *varGetEntityInfo(TaggedRef*);
EntityCond varGetEntityCond(TaggedRef*);

void maybeUnaskVar(BorrowEntry*);
Bool errorIgnoreVar(BorrowEntry*);
Bool varFailurePreemption(TaggedRef t,EntityInfo*, Bool&,TaggedRef);
void varPOAdjustForFailure(int,EntityCond,EntityCond);

void recDeregister(TaggedRef,DSite*);

Bool varCanSend(DSite*);

#define BAD_BORROW_INDEX (0-1)

#endif













