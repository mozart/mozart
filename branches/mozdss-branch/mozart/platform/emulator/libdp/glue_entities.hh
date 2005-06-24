/*
 *  Authors:
 *    Zacharias El Banna (zeb@sics.se)
 *    Erik Klintskog (erik@sics.se)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Per Brand, 1998
 *    Erik Klintskog, 1998
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

#ifndef __ENTITIES_HH
#define __ENTITIES_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"
#include "value.hh"
#include "var_readonly.hh"
#include "var_ext.hh"
#include "glue_tables.hh"

const int NOT_USED_INDEX = 0xbedda;
const Mediator * const NOT_USED_TERT_INDEX = reinterpret_cast<Mediator *>(0xbedda);
const Mediator * const NOT_USED_VAR_INDEX  = reinterpret_cast<Mediator *>(0xbedda);


extern TaggedRef BI_portWait;

/************************ A Port ************************/

class PortProxy: public PortWithStream  {
public:
  NO_DEFAULT_CONSTRUCTORS(PortProxy);
  PortProxy(): PortWithStream(oz_currentBoard(),oz_newReadOnly(oz_currentBoard())) 
  {
    setTertType(Te_Proxy);
    setTertIndex(NOT_USED_INDEX);
  }
};

/************************ A Cell ************************/

class CellProxy : public Tertiary {
private:
  int holder; // mm2: on alpha sizeof(int) != sizeof(void *)
  void *dummy; // mm2
public:
  NO_DEFAULT_CONSTRUCTORS(CellProxy)

    CellProxy():Tertiary(NOT_USED_INDEX,Co_Cell,Te_Proxy){  // on import
    holder = 0;}
};

/************************ Lock Proxy **********************/

class LockProxy : public LockLocal{
public:
  LockProxy():LockLocal(oz_currentBoard())
  {
    setTertType(Te_Proxy); setTertIndex(NOT_USED_INDEX);
  } 
  
};


class ArrayProxy :public OzArray{
public:
  ArrayProxy(int low, int high):OzArray(oz_currentBoard(), low, high, oz_nil()){;}
};

/***************** The Variable ******************/
#define GET_VAR(po,T) oz_get##T##Var(*((po)->getPtr()))
#define GET_TERM(po,T) oz_get##T##Var(*((po)->getAnyPtr()))

typedef enum {
  VAR_PROXY,
  VAR_MANAGER,
  VAR_LAZY,
  VAR_FREE,
  VAR_READONLY,    
  VAR_KINDED
} VarKind;

VarKind classifyVar(TaggedRef *);

/************************ A LazyVar **********************/
typedef enum {
  LT_OBJECT,
  LT_CLASS
} LazyType;


class LazyVar : public ExtVar {
protected:
  Mediator *e_name; 
  short requested;		// flag - whether in transition;
  GName *gname;			// how it is known;

public:
  LazyVar(Board *bb, GName *gIn)
    : ExtVar(bb), e_name(const_cast<Mediator *>(NOT_USED_VAR_INDEX)), requested(0), gname(gIn){ }

  Mediator *getMediator(){ return e_name;}
  void setMediator(Mediator *e){ e_name = e;}

  virtual OZ_Term statusV();
  virtual VarStatus checkStatusV();
  virtual OZ_Return addSuspV(TaggedRef *v, Suspendable * susp);
  virtual LazyType getLazyType() = 0;
  virtual void sendRequest(TaggedRef *) = 0;
  virtual Bool validV(TaggedRef v) { return (TRUE); }
  virtual ExtVar* gCollectV() { Assert(0); return NULL; }
  virtual ExtVar* sCloneV() { Assert(0); return NULL; }
  virtual void gCollectRecurseV(void);
  virtual void sCloneRecurseV(void) { Assert(0); }
  virtual void printStreamV(ostream &out,int depth = 10) {
    out << "<dist:lazy>";
  }
  virtual OZ_Return bindV(TaggedRef *vptr, TaggedRef t);
  virtual OZ_Return unifyV(TaggedRef *vptr, TaggedRef *tPtr);
  virtual void disposeV(void);
  virtual ExtVarType getIdV() { return (OZ_EVAR_LAZY); }
  
  virtual void marshal(ByteBuffer *bs, Bool hasIndex);
  virtual void transfer(TaggedRef val, TaggedRef* selfPtr)=0;
  GName *getGName() { return (gname); }

  TaggedRef getTaggedRef();
};

//
inline
Bool oz_isLazyVar(TaggedRef v) {
  return (oz_isExtVar(v) && oz_getExtVar(v)->getIdV() == OZ_EVAR_LAZY);
}

inline
LazyVar* oz_getLazyVar(TaggedRef v) { Assert(oz_isLazyVar(v));
  return ((LazyVar *) oz_getExtVar(v));
}

inline
LazyVar* getLazyVar(TaggedRef *tPtr) { Assert(classifyVar(tPtr) == VAR_LAZY);
  return (oz_getLazyVar(*tPtr));
}



/************************ A ProxyVar **********************/

class ProxyVar : public ExtVar {
private:
  Mediator *e_name; 
  TaggedRef status;
  short is_readOnly;

public: 
  ProxyVar(Board *bb, Bool isF) : 
    ExtVar(bb), e_name(const_cast<Mediator *>(NOT_USED_VAR_INDEX)), status(0), is_readOnly(isF){}
  
  Mediator *getMediator(){ return e_name;}
  void setMediator(Mediator *e){ e_name = e;}
  
  TaggedRef getTaggedRef();
  OZ_Return addSuspV(TaggedRef *, Suspendable * susp);
  void gCollectRecurseV(void);
  OZ_Return bindV(TaggedRef *lPtr, TaggedRef r);
  void redoStatus(TaggedRef val, TaggedRef status);
  
  ExtVarType getIdV() { return (OZ_EVAR_PROXY); }
  ExtVar *gCollectV(); 
  ExtVar *sCloneV() { Assert(0); return NULL; }
  void sCloneRecurseV(void) { Assert(0); }
  
  Bool isReadOnly(){ return is_readOnly;}

  OZ_Term statusV();
  VarStatus checkStatusV();
  
  Bool validV(TaggedRef v) { return TRUE; }
  void disposeV()
  {
    disposeS();
    freeListDispose(sizeof(ProxyVar));
  }
  
  /* From ProxyManagerVar */
  
  OZ_Return unifyV(TaggedRef *lPtr, TaggedRef *rPtr);
  
  
  void marshal(ByteBuffer*, Bool, TaggedRef*, Bool push);

};


inline
ProxyVar *oz_getProxyVar(TaggedRef v) {
  return (ProxyVar*) oz_getExtVar(v);
}

Bool triggerVariable(TaggedRef *);



/************************ ObjectVar **********************/

class ObjectVar : public LazyVar {
protected:
  GName *gClass; 
  
public:
  ObjectVar(Board *bb, GName *gobjIn, GName *gclass)
    : LazyVar(bb, gobjIn), gClass(gclass){}
  
  virtual LazyType getLazyType();
  // 'sendRequest' defines what is to be done for a particular lazy
  // var type:
  virtual void sendRequest(TaggedRef *);
  // New (extended) format;
  virtual ExtVar * gCollectV(); 
  virtual void gCollectRecurseV(void);
  virtual void disposeV(void);

  Bool isObjectClassAvail(void); 
  GName *getGNameClass();
  virtual void marshal(ByteBuffer *, Bool hasCoRefsIndex);

  void transfer(TaggedRef, TaggedRef*);
};

//TaggedRef newObjectProxy(int bi, GName *gnobj, TaggedRef clas);

#endif







