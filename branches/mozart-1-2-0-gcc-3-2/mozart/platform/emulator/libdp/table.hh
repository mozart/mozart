/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Per Brand,1998
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

#ifndef __TABLE_HH
#define __TABLE_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "base.hh"
#include "tagged.hh"
#include "dsite.hh"
#include "comm.hh"
#include "dpDebug.hh"
#include "genhashtbl.hh"
#include "perdio.hh"
#include "creditHandler.hh"
#include "protocolCredit.hh"

#ifdef DEBUG_CHECK
Bool withinBorrowTable(int i);
#endif

#define NET_HASH_TABLE_DEFAULT_SIZE 100

class NetHashTable: public GenHashTable{
  int hashFunc(NetAddress *);
  inline Bool findPlace(int ,NetAddress *, GenHashNode *&);
public:
  NetHashTable():GenHashTable(NET_HASH_TABLE_DEFAULT_SIZE){}
  int findNA(NetAddress *);
  void add(NetAddress *,int);
  void sub(NetAddress *);
};

inline NetAddress * GenHashNode2NetAddr(GenHashNode *ghn){
  NetAddress *na;
  na = (NetAddress*)(void*) ghn->getBaseKey();
  return na;}

inline int GenHashNode2BorrowIndex(GenHashNode *ghn){
  int i;
  i = (int) ghn->getEntry();
  Assert(i>=0);
  Assert(withinBorrowTable(i));
  return i;}

inline Bool NetHashTable::findPlace(int hvalue,NetAddress *na,GenHashNode *&ghn){
  PD((HASH,"find Place hvalue=%d, net%d:%d",hvalue,
	       na->site,na->index));
  ghn=htFindFirst(hvalue);
  NetAddress *na2;
  while(ghn!=NULL){
    na2=GenHashNode2NetAddr(ghn);
    if(na->same(na2)){
      PD((HASH,"compare success hvalue=%d bk=%x net%d:%d",
		    ghn->getKey(),ghn->getBaseKey(),na2->site,na2->index));
      return TRUE;}
    PD((HASH,"compare fail hvalue=%d bk=%x net%d:%d",
		  ghn->getKey(),ghn->getBaseKey(),na2->site,na2->index));
    ghn=htFindNext(ghn,hvalue);}
  return FALSE;}

/* -------------------------------------------------------------------- */

enum PO_TYPE {
  PO_Var,
  PO_Tert,
  PO_Ref,
  PO_Free
};

enum PO_FLAGS{      
  PO_GC_MARK=1
};

class ProtocolObject {
  short type;
  unsigned short flags;
protected:

  // all should be TaggedRefs to simplify gc and access
  union {
    TaggedRef ref;
    TaggedRef tert;
    int nextfree;
  } u;
public:
  ProtocolObject()            { DebugCode(type=(PO_TYPE)4711; )}
  Bool isTertiary()           { return type==PO_Tert; }
  Bool isRef()                { return type==PO_Ref; }
  Bool isVar()                { return type==PO_Var; }
  Bool isFree()               { return type==PO_Free; }
  void setFree()              { type = PO_Free; }
  void unsetFree()            { DebugCode(type=(PO_TYPE)4712); }
  Bool initialized()          { DebugCode(return type!=(PO_TYPE)4712);return TRUE;}

  void setTert(Tertiary * t) {
    u.tert = t ? makeTaggedConst(t) : makeTaggedNULL();
  }

  void mkTertiary(Tertiary *t,unsigned short f){ 
    type = PO_Tert; setTert(t); flags=f; }

  void mkTertiary(Tertiary *t){ 
    type = PO_Tert; setTert(t); }

  //
  // 'v' is an immediate value, or a reference to a variable.
  // However, a variable can be bound later (so a ref chain can still
  // emerge);
  void mkRef(TaggedRef v, unsigned short f) {
    Assert((!oz_isRef(v) && !oz_isVar(v)) ||
	   (oz_isRef(v) && oz_isVar(*tagged2Ref(v))));
    type=PO_Ref; u.ref=v; flags=f;
  }
  void mkRef(TaggedRef v) {
    Assert((!oz_isRef(v) && !oz_isVar(v)) ||
	   (oz_isRef(v) && oz_isVar(*tagged2Ref(v))));
    type=PO_Ref; u.ref=v;
  }

  void mkVar(TaggedRef v,unsigned short f){
    type=PO_Var; u.ref=v; flags=f; }

  void mkVar(TaggedRef v){
    type=PO_Var; u.ref=v; }

  void changeToRef(){ 
    Assert(isVar()); type=PO_Ref; }

  void changeToVar(TaggedRef v){
    type=PO_Var; u.ref=v;}

  void changeToTertiary(Tertiary* t){
    type=PO_Tert; setTert(t);}

  void updateTertiaryGC(Tertiary *t){ 
    setTert(t); }

  unsigned short getFlags()         {return flags;}
  void setFlags(unsigned short f)   {flags=f;}
  void removeFlags(unsigned short f) {flags = flags & (~f);}
  void addFlags(unsigned short f)    {flags = flags | f;}

  Tertiary *getTertiary() {
    Assert(isTertiary()); 
    return ((Tertiary *) (u.tert ? tagged2Const(u.tert) : 0));
  }
  TaggedRef getTertTerm(void) {
    Assert(isTertiary()); 
    return (u.tert);
  }
  TaggedRef getRef() {
    Assert(isRef()||isVar());
    return (u.ref);
  }
  TaggedRef *getPtr() {
    Assert(isVar());
    return (tagged2Ref(getRef()));
  }
  TaggedRef *getAnyPtr() {
    return tagged2Ref(getRef());
  }

  TaggedRef getValue() {
    if (isTertiary()) 
      return (u.tert);
    else
      return (getRef());
  }
};

class OB_Entry : public ProtocolObject {
protected:
//    union {
//      int nextfree; 
//    } uOB;

  void makeFree(int next) {
    setFree(); 
    u.nextfree=next;}

  int getNextFree(){
    Assert(isFree());
    return u.nextfree;}

public:
  void makeGCMark(){addFlags(PO_GC_MARK);}
  Bool isGCMarked(){ return (getFlags() & PO_GC_MARK); }
  void removeGCMark(){ removeFlags(PO_GC_MARK); }
  
  void gcPO(Tertiary *newval) {
    if (isGCMarked())
      return;
    makeGCMark();

    setTert(newval);
  }

  void gcPO();

  void print();

//    virtual Bool isPersistent()=0;
//    virtual void makePersistent()=0;
};

  
/* ********************************************************************** */
/*   SECTION 10:: OwnerEntry                                               */  
/* ********************************************************************** */

class OwnerEntry: public OB_Entry {
friend class OwnerTable;
private:
  OwnerCreditHandler ocreditHandler;

public:
  void setUp(int index);

  void localize(int index);

  inline Credit getCreditBig() {
    Assert(!isFree());
    return ocreditHandler.getCreditBig();
  }

  inline Credit getCreditSmall() {
    Assert(!isFree());
    return ocreditHandler.getCreditSmall();
  }

  inline void addCredit(Credit c) {
    Assert(!isFree());
    ocreditHandler.addCredit(c);
  }

  Bool isPersistent(){
    Assert(!isFree());
    return ocreditHandler.isPersistent();}

  void makePersistent(){
    Assert(!isFree());
    Assert(!isPersistent());
    ocreditHandler.makePersistent();
    Assert(isPersistent());
  }
}; 

/* ********************************************************************** */
/*   SECTION 11:: OwnerTable                                               */  
/* ********************************************************************** */

#define END_FREE -1

class OwnerTable {
  OwnerEntry* array;
  int size;
  int no_used;
  int nextfree;
  int localized;  /* Used by the distpane */
    
  
  void init(int,int);
  void compactify();
public:
  void print();

  OZ_Term extract_info();

  OwnerEntry *getOwner(int i)  { Assert(i>=0 && i<size); return &array[i];}
  
  int getSize() {return size;}

  OwnerTable(int sz) {
    size = sz;
    array = (OwnerEntry*) malloc(size*sizeof(OwnerEntry));
    Assert(array!=NULL);
    nextfree = END_FREE;
    no_used=0;
    localized = 0;
    init(0,sz);
  }

  OwnerEntry* getEntry(int i){
    Assert(i<=size);
    if(array[i].isFree()) return NULL;
    return &array[i];}

  void gcOwnerTableRoots();
  void gcOwnerTableFinal();

  Bool notGCMarked();

  void resize();

  int newOwner(OwnerEntry *&);

  void freeOwnerEntry(int);
  void localizing(){localized = (localized + 1) % 100000;}
  int  getLocalized(){int ret=localized; localized = 0; return ret;}
  
};

extern OwnerTable *ownerTable;
#define OT ownerTable

/* **********************************************************************  */
/*   SECTION 14:: BorrowEntry                                              */
/* **********************************************************************  */

class BorrowEntry: public OB_Entry {
friend class BorrowTable;
friend class BorrowCreditHandler;
private:
  BorrowCreditHandler bcreditHandler;

protected:
  Bool canBeFreed();

public:
  void print_entry(int);
  OZ_Term extract_info(int);

  Bool isPersistent(){
    Assert(!isFree());
    return bcreditHandler.isPersistent();}

  void makePersistent(){
    Assert(!isFree());
    Assert(!isPersistent());
    bcreditHandler.makePersistent();
    Assert(isPersistent());
  }

  void gcBorrowRoot(int);
  void gcBorrowUnusedFrame(Tertiary*);

  void copyBorrow(BorrowEntry* from,int i);

  void initBorrowPersistent(DSite* s,int i){
    Assert(isFree());
    unsetFree();
    setFlags(0);
    bcreditHandler.setUpPersistent(s,i);
    return;
  }

  void initBorrow(Credit c,DSite* s,int i){
    Assert(isFree());
    unsetFree();
    setFlags(0);
    bcreditHandler.setUp(c,s,i);
    return;
  }

  inline NetAddress* getNetAddress() {
    Assert(!isFree());
    return &(bcreditHandler.netaddr);
  }

  DSite *getSite(){return bcreditHandler.netaddr.site;}

  int getOTI(){return bcreditHandler.netaddr.index;}

  inline Credit getCreditBig() {
    Assert(!isFree());
    return bcreditHandler.getCreditBig();
  }

  inline Credit getCreditSmall() {
    Assert(!isFree());
    return bcreditHandler.getCreditSmall();
  }

  inline void addCredit(Credit c) {
    Assert(!isFree());
    bcreditHandler.addCredit(c);
  }

  void freeBorrowEntry();    

  void removeGCMark(){
    removeFlags(PO_GC_MARK);
  }
};

/* ********************************************************************** */
/*   SECTION 15:: BorrowTable                                              */
/* ********************************************************************** */

class BorrowTable {           
private:
  int no_used;
  BorrowEntry* array;
  int size;
  int nextfree;

  void init(int,int);
  void compactify();

public:
  NetHashTable *hshtbl;
  void print();

  BorrowEntry *getBorrow(int i)  { Assert(i>=0 && i<size); return &array[i];}

  int ptr2Index(BorrowEntry *a) { return(a-array);}

  int getSize() {return size;}   

  BorrowEntry* getEntry(int i){
    Assert(i<=size);
    if(array[i].isFree()) return NULL;
    return &array[i];}

  BorrowTable(int sz)  {
    size= sz;
    array = (BorrowEntry*) malloc(size *sizeof(BorrowEntry));
    Assert(array!=NULL);
    nextfree = END_FREE;
    init(0,sz);
    no_used=0;
    hshtbl = new NetHashTable();  }

  void gcBorrowTableRoots();
  void gcBorrowTableUnusedFrames();
  void gcBorrowTableFinal();
  void gcFrameToProxy();

  Bool notGCMarked();

  BorrowEntry* find(NetAddress *na)  {
    int i = hshtbl->findNA(na);
    if(i<0) {
      PD((LOOKUP,"borrow not found"));
      return 0;
    } else {
      PD((LOOKUP,"borrow found b:%d",i));
      return getBorrow(i);
      // mm2 was: return borrowTable->getBorrow(i);
    }
  }

  void resize();

  int newBorrow(Credit,DSite*,int);
  int newBorrowPersistent(DSite*,int);

  Bool maybeFreeBorrowEntry(int);

  void freeSecBorrow(int);

  DSite* getOriginSite(int bi){
    return getBorrow(bi)->getNetAddress()->site;}

  int getOriginIndex(int bi){
    return getBorrow(bi)->getNetAddress()->index;}

  void copyBorrowTable(BorrowEntry *,int);

  // 'dumpFrames()' returns the number remaining frames;
  int dumpFrames();
  void dumpProxies();
};

extern BorrowTable *borrowTable;
#define BT borrowTable

Bool withinBorrowTable(int i); // for assertion

#endif
