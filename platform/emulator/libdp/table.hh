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

typedef long Credit;

#ifdef DEBUG_CHECK
Bool withinBorrowTable(int i);
#endif

class NetAddress {
public:
  /* DummyClassConstruction(NetAddress) */
  DSite* site;
  int index;

  NetAddress(DSite* s, int i) : site(s), index(i) {}

  void set(DSite *s,int i) {site=s,index=i;}

  Bool same(NetAddress *na) { return na->site==site && na->index==index; }

  Bool isLocal() { return site==myDSite; }
};

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
  GenCast(ghn->getBaseKey(),GenHashBaseKey*,na,NetAddress*);
  return na;}

inline int GenHashNode2BorrowIndex(GenHashNode *ghn){
  int i;
  GenCast(ghn->getEntry(),GenHashEntry*,i,int);
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


/* possibilities
 *     PO_NONE
 *     PO_EXTENDED | PO_BIGCREDIT                              (owner)
 *     PO_EXTENDED | PO_MASTER                                 (borrow)
 *     PO_EXTENDED | PO_MASTER | PO_BIGCREDIT                  (borrow)
 *     PO_EXTENDED | PO_SLAVE  | PO_MASTER | PO_BIGCREDIT      (borrow)
 */

enum PO_FLAGS{
  PO_NONE=0,
  PO_EXTENDED=1,
  PO_BIGCREDIT=2,
  PO_MASTER=4,
  PO_SLAVE=8,
  PO_GC_MARK=16,
  PO_PERSISTENT=32
};

class ProtocolObject {
  short type;
  unsigned short flags;
protected:

  // all should be TaggedRefs to simplify gc and access
  union {
    TaggedRef ref;
    Tertiary *tert;
  } u;
public:
  ProtocolObject()            { DebugCode(type=(PO_TYPE)4711; u.ref=0x5b5b5b5b;)}
  Bool isTertiary()           { return type==PO_Tert; }
  Bool isRef()                { return type==PO_Ref; }
  Bool isVar()                { return type==PO_Var; }
  Bool isFree()               { return type==PO_Free; }
  void setFree()              { type = PO_Free; DebugCode(u.tert=(Tertiary*)0xfebc5d4d); }
  void unsetFree()            { DebugCode(type=(PO_TYPE)4712); }
  Bool initialized()          { DebugCode(return type!=(PO_TYPE)4712);return TRUE;}

  void mkTertiary(Tertiary *t,unsigned short f){
    type = PO_Tert; u.tert=t; flags=f; }

  void mkTertiary(Tertiary *t){
    type = PO_Tert; u.tert=t; flags=PO_NONE; }

  void mkRef(TaggedRef v,unsigned short f){
    type=PO_Ref; u.ref=v; flags=f; }

  void mkRef(TaggedRef v){
    type=PO_Ref; u.ref=v; flags=PO_NONE; }

  void mkVar(TaggedRef v,unsigned short f){
    type=PO_Var; u.ref=v; flags=f; }

  void mkVar(TaggedRef v){
    type=PO_Var; u.ref=v; flags=PO_NONE;}

  void changeToRef(){
    Assert(isVar()); type=PO_Ref; }

  void changeToVar(TaggedRef v){
    type=PO_Var; u.ref=v;}

  void changeToTertiary(Tertiary* t){
    type=PO_Tert; u.tert=t;}

  void updateTertiaryGC(Tertiary *t){
    u.tert=t; }

  unsigned short getFlags()         {return flags;}
  void setFlags(unsigned short f)   {flags=f;}
  void removeFlags(unsigned short f) {flags = flags & (~f);}
  void addFlags(unsigned short f)    {flags = flags | f;}

  Tertiary *getTertiary() { Assert(isTertiary()); return u.tert; }
  TaggedRef getRef()      { Assert(isRef()||isVar()); return u.ref; }
  TaggedRef *getPtr()     { Assert(isVar()); return tagged2Ref(getRef()); }
  TaggedRef *getAnyPtr()  { return tagged2Ref(getRef()); }

  TaggedRef getValue() {
    if (isTertiary())
      return makeTaggedConst(getTertiary());
    else
      return getRef();
  }
};

/* -------------------------------------------------------------------- */

#define INFINITE_CREDIT            (0-1)
#define PERSISTENT_CRED            (0-1)

#ifdef DEBUG_CREDIT

#define START_CREDIT_SIZE        (256)
#define OWNER_GIVE_CREDIT_SIZE   (16)
#define BORROW_MIN               (2)
#define BORROW_GIVE_CREDIT_SIZE  (4)
#define BORROW_LOW_THRESHOLD     (8)
#define BORROW_HIGH_THRESHOLD    (64)


#else

#define START_CREDIT_SIZE        ((1<<30)-1)
#define OWNER_GIVE_CREDIT_SIZE   ((1<<14))
#define BORROW_MIN               (2)
#define BORROW_GIVE_CREDIT_SIZE  ((1<<7))
#define BORROW_LOW_THRESHOLD     ((1<<4))
#define BORROW_HIGH_THRESHOLD    ((1<<19))
#endif

extern DSite* creditSiteIn;
extern DSite* creditSiteOut;

extern void sendSecondaryCredit(DSite* s,DSite* site,int index,Credit c);

/* -------------------------------------------------------------------- */

class OwnerCreditExtension;
class BorrowCreditExtension;

class OB_Entry : public ProtocolObject {
protected:
  union {
    Credit credit;
    int nextfree;
    OwnerCreditExtension *oExt;
    BorrowCreditExtension *bExt;
  } uOB;

  void makeFree(int next) {
    setFree();
    uOB.nextfree=next;}

  int getNextFree(){
    Assert(isFree());
    return uOB.nextfree;}

  Bool isExtended(){
    if(getFlags() & PO_EXTENDED) return OK;
    return NO;}

  void initCreditOB(Credit c) {
    uOB.credit=START_CREDIT_SIZE;}

  void addCreditOB(Credit c) {
    Assert(!isExtended());
    uOB.credit += c;
    Assert(uOB.credit<=(START_CREDIT_SIZE));}

  void subCreditOB(Credit c) {
    Assert(uOB.credit>c);
    Assert(!isExtended());
    uOB.credit -=c;}

  void setCreditOB(Credit c){
    uOB.credit=c;}


public:

  void makeGCMark(){addFlags(PO_GC_MARK);}
  Bool isGCMarked(){ return (getFlags() & PO_GC_MARK); }

  void gcPO(Tertiary *newval) {
    if (isGCMarked())
      return;
    makeGCMark();

#ifdef DEBUG_GC
    Assert(isTertiary() && !inToSpace(u.tert) && inToSpace(newval));
#endif
    u.tert = newval;
  }

  void gcPO();

  Credit getCreditOB(){
    Assert(!isExtended());
    Assert(!isFree());
    return uOB.credit;}

  void print();
  };

enum ReduceCode{
 CANNOT_REDUCE,
   CAN_REDUCE_LAST,
   CAN_REDUCE_SINGLE};

class OwnerCreditExtension{
friend class OB_Entry;
  Credit credit[2];
  OwnerCreditExtension *next;

protected:

public:
  OwnerCreditExtension(){}
  void init(Credit);
  void requestCreditE(Credit);
  ReduceCode addCreditE(Credit);
  ReduceCode isReducible();
  OwnerCreditExtension* getNext(){return next;}
  void reduceLast();
  Credit reduceSingle(){
    Assert(credit[1]==START_CREDIT_SIZE);
    Assert(next==NULL);
    return credit[0];}
  Credit getCredit(int Index){
    return credit[Index];}
  void expand();

};

/* ********************************************************************** */
/*   SECTION 10:: OwnerEntry                                               */
/* ********************************************************************** */

class OwnerEntry: public OB_Entry {
friend class OwnerTable;
private:

  OwnerCreditExtension* getOwnerCreditExtension(){
    Assert(isExtended());
    return uOB.oExt;}

  void setOwnerCreditExtension(OwnerCreditExtension* oce){
    Assert(isExtended());
    uOB.oExt=oce;}

  void extend();

  void requestCredit(Credit req){
    if(isExtended()){
      getOwnerCreditExtension()->requestCreditE(req);
      return;}
    Credit credit=getCreditOB();
    if(credit<=req){
      extend();
      requestCredit(req);
      return;}
    setCreditOB(credit-req);
    return;}

  void addCreditExtended(Credit);

  void addCredit(Credit back){
    if(isExtended()){
      addCreditExtended(back);
      return;}
    addCreditOB(back);
    return;}

public:

  void localize(int index);
  void returnCreditOwner(Credit c, int index) {
    addCredit(c);
    if (hasFullCredit())
      localize(index);
  }

  Bool hasFullCredit(){
    if(isPersistent()) return NO;
    if(isExtended()) return NO;
    Credit c=getCreditOB();
    Assert(c<=START_CREDIT_SIZE);
    if(c<START_CREDIT_SIZE) return NO;
    return OK;}

  Credit getSendCredit() {

    requestCredit(OWNER_GIVE_CREDIT_SIZE);
    return OWNER_GIVE_CREDIT_SIZE;}

  void getOneCreditOwner() {
    requestCredit(1);
    return;}

  Credit giveMoreCredit() {
    requestCredit(OWNER_GIVE_CREDIT_SIZE);
    return OWNER_GIVE_CREDIT_SIZE;}

  void removeExtension();
  void receiveCredit(int i){
    if(creditSiteIn==NULL){
      addCredit(1);
      return;}
    sendSecondaryCredit(creditSiteIn,myDSite,i,1);
    creditSiteIn=NULL;}

  void removeGCMark(){ removeFlags(PO_GC_MARK); }

  Bool isPersistent(){
    return (getFlags() & PO_PERSISTENT);}

  void makePersistent(){
    Assert(!isPersistent());
    addFlags(PO_PERSISTENT);
    uOB.credit = PERSISTENT_CRED;}
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

/* ********************************************************************** */
/*   SECTION 12:: BorrowCreditExtension                                   */
/* ********************************************************************** */

class BorrowCreditExtension{
friend class BorrowEntry;
  union{
  Credit secCredit;          // SecSlave || (SecMaster && ~BigCredit)
  OwnerCreditExtension *oce; // SecMaster && BigCredit
  BorrowCreditExtension *bce; // SecSlave
}uSOB;
  Credit primCredit;
  DSite* site;                // non-NULL SecSlave SecMaster



protected:

  BorrowCreditExtension* getMaster(){return uSOB.bce;}
  OwnerCreditExtension* getBig(){return uSOB.oce;}
  DSite *getSite(){return site;}

  void initMaster(Credit cred){
    uSOB.secCredit=START_CREDIT_SIZE;
    primCredit=cred;
    site=NULL;}

  void initSlave(Credit pc,Credit sc,DSite *s){
    uSOB.secCredit=sc;
    primCredit=pc;
    site=s;}

  Bool getSecCredit_Master(Credit); // called from Master - can expand to Big
  Bool getSmall_Slave(Credit &);    // called from Slave - can expand to Master
  Bool getOne_Slave();              // called from Slave - can expand to Master

  Credit reduceSlave(Credit,DSite* &,Credit &); // called from Slave - before removal
  Credit addPrimaryCredit_Master(Credit); // called from Master
  Credit addPrimaryCredit_SlaveMaster(Credit); // called from Master

  Credit msGetPrimCredit(){return primCredit;}
  Credit slaveGetSecCredit(){return uSOB.secCredit;}
  Credit masterGetSecCredit(){return uSOB.secCredit;}

  void msSetPrimCredit(Credit c){primCredit=c;}
  void slaveSetSecCredit(Credit c){uSOB.secCredit=c;}
  void masterSetSecCredit(Credit c){uSOB.secCredit=c;}

  Bool isReducibleSlave() {return (primCredit>BORROW_MIN)||(primCredit==0);}
  Bool isReducibleMaster(){return uSOB.secCredit==START_CREDIT_SIZE;}
  Bool isReducibleBig(){return uSOB.oce->isReducible();}

  void expandSlave();    // called from Slave
  void expandMaster();   // called from Master

public:
  BorrowCreditExtension(){}
  void print_entry(int nr);
};

/* ********************************************************************** */
/*   SECTION 13:: BorrowCreditExtension methods                           */
/* ********************************************************************** */

inline OwnerCreditExtension* newOwnerCreditExtension(){
  return (OwnerCreditExtension*) genFreeListManager->getOne_3();}

inline BorrowCreditExtension* newBorrowCreditExtension(){
  return (BorrowCreditExtension*) genFreeListManager->getOne_3();}

inline void freeOwnerCreditExtension(OwnerCreditExtension* oce){
  genFreeListManager->putOne_3((FreeListEntry*)oce);}

inline void freeBorrowCreditExtension(BorrowCreditExtension* bce){
  genFreeListManager->putOne_3((FreeListEntry*)bce);}

/* **********************************************************************  */
/*   SECTION 14:: BorrowEntry                                              */
/* **********************************************************************  */

// START_CREDIT_SIZE
// BORROW_HIGH_THRESHOLD
// BORROW_GIVE_CREDIT_SIZE
// BORROW_LOW_THRESHOLD

class BorrowEntry: public OB_Entry {
friend class BorrowTable;
friend class BorrowCreditExtension;
private:
  NetAddress netaddr;

  BorrowCreditExtension* getSlave(){
    Assert(getFlags() & PO_SLAVE);
    return uOB.bExt;}

  BorrowCreditExtension* getMaster(){
    Assert(getFlags() & PO_MASTER);
    Assert(!(getFlags() & PO_SLAVE));
    return uOB.bExt;}


  void setSlave(BorrowCreditExtension* bce){
    Assert(getFlags() & PO_SLAVE);
    uOB.bExt = bce;}

  void setMaster(BorrowCreditExtension* bce){
    Assert(getFlags() & PO_MASTER);
    Assert(!(getFlags() & PO_SLAVE));
    uOB.bExt = bce;}


  Bool getOnePrimaryCredit_E();
  Credit getSmallPrimaryCredit_E();

  void thresholdCheck(Credit c){
    if((getCreditOB()+c>BORROW_LOW_THRESHOLD) &&
       (getCreditOB()<=BORROW_LOW_THRESHOLD)){
      Assert(!isExtended());
      moreCredit();}}

  void removeSoleExtension(Credit);

  void createSecMaster();
  void removeSlave();
  void createSecSlave(Credit,DSite *);

  Credit extendGetPrimCredit(){
    if(getFlags() & PO_MASTER)
      return getMaster()->msGetPrimCredit();
    return getSlave()->msGetPrimCredit();}

  void extendSetPrimCredit(Credit c){
    if(getFlags() & PO_MASTER)
      getMaster()->msSetPrimCredit(c);
    else
      getSlave()->msSetPrimCredit(c);}

  void generalTryToReduce();
  void giveBackSecCredit(DSite *,Credit);
  void removeBig(BorrowCreditExtension*);
  void removeMaster_SM(BorrowCreditExtension*);
  void removeMaster_M(BorrowCreditExtension*);

  void addSecCredit_MasterBig(Credit,BorrowCreditExtension *);
  void addSecCredit_Master(Credit,BorrowCreditExtension *);
  Bool addSecCredit_Slave(Credit,BorrowCreditExtension *);

public:

  int getExtendFlags(){
    return getFlags() & (~PO_GC_MARK|PO_EXTENDED);}

  void print_entry(int);
  OZ_Term extract_info(int);

  void gcBorrowRoot(int);
  void gcBorrowUnusedFrame(Tertiary*);

  void copyBorrow(BorrowEntry* from,int i);

  void initBorrow(Credit c,DSite* s,int i){
    Assert(isFree());
    setCreditOB(c);
    unsetFree();
    setFlags(PO_NONE);
    netaddr.set(s,i);
    return;}

  void initSecBorrow(DSite*,Credit,DSite*,int);

  NetAddress* getNetAddress() {
    Assert(!isFree());
    return &netaddr;}

  DSite *getSite(){return netaddr.site;}

  int getOTI(){return netaddr.index;}

  void addPrimaryCreditExtended(Credit c);
  void addSecondaryCredit(Credit c,DSite *s);

  void addPrimaryCredit(Credit c){
    if(isExtended()) {
      addPrimaryCreditExtended(c);
      return;}
    Credit cur=getCreditOB();
    PD((CREDIT,"borrow add s:%s o:%d add:%d to:%d",
        oz_site2String(getNetAddress()->site),
        getNetAddress()->index,c,cur));
    if(cur>BORROW_HIGH_THRESHOLD){
      giveBackCredit(c+cur-BORROW_HIGH_THRESHOLD);
      setCreditOB(BORROW_HIGH_THRESHOLD);
      return;}
    setCreditOB(cur+c);}

  Bool getOnePrimaryCredit(){
    PD((CREDIT,"Trying to get one primary credit"));
    if(isPersistent()) {
      PD((CREDIT,"Persistent, no credit needed"));
      PD((CREDIT,"%d credit left", getCreditOB()));
      return OK;}
    if(isExtended()){
      PD((CREDIT,"Structure extended, no primary"));
      return NO;}
    Credit tmp=getCreditOB();
    Assert(tmp>0);
    if(tmp-1<BORROW_MIN) {
      PD((CREDIT,"gopc low credit %d",tmp));
      PD((CREDIT,"got no credit"));
      return NO;}
    setCreditOB(tmp-1);
    thresholdCheck(1);
    PD((CREDIT,"Got one credit, %d credit left", tmp-1));
    return OK;}

  Credit getSmallPrimaryCredit(){
    PD((CREDIT,"Trying to get %d primary credit",
        BORROW_GIVE_CREDIT_SIZE));
    if(isPersistent())
      return PERSISTENT_CRED;
    if(isExtended()){
      PD((CREDIT,"Structure extended, no primary"));
      return NO;}
    Credit tmp=getCreditOB();
    Assert(tmp>0);
    if(tmp-BORROW_GIVE_CREDIT_SIZE >= BORROW_MIN){
      setCreditOB(tmp-BORROW_GIVE_CREDIT_SIZE);
      PD((CREDIT,"Got %d credit, %d credit left",
          BORROW_GIVE_CREDIT_SIZE,
          tmp-BORROW_GIVE_CREDIT_SIZE));
      thresholdCheck(BORROW_GIVE_CREDIT_SIZE);
      return BORROW_GIVE_CREDIT_SIZE;}
    PD((CREDIT,"gspc low credit %d",tmp));
    if(tmp-2>=BORROW_MIN){
      setCreditOB(tmp-2);
      PD((CREDIT,"Got 2 credit, %d credit left", tmp-2));
      thresholdCheck(2);
      return 2;}
    PD((CREDIT,"got no credit"));
    return 0;}

  DSite *getOneSecondaryCredit();
  DSite *getSmallSecondaryCredit(Credit &);

  void freeBorrowEntry();
  void giveBackCredit(Credit c);
  void moreCredit();

  int getCredit(){
    return uOB.credit;}

  void receiveCredit(){
    if(creditSiteIn==NULL){
      addPrimaryCredit(1);
      return;}
    addSecondaryCredit(1,creditSiteIn);
    creditSiteIn=NULL;}

  void getOneMsgCredit(){
    if(getOnePrimaryCredit()){
      PD((CREDIT,"Got one primary"));
      Assert(creditSiteOut==NULL);
      return;}
    creditSiteOut=getOneSecondaryCredit();
    PD((CREDIT,"Got one secondary %s",oz_site2String(creditSiteOut)));
    Assert(creditSiteOut);
    return;}

  void removeGCMark(){
    removeFlags(PO_GC_MARK);
  }

  void makePersistent(){
    Assert(!isPersistent());
    addFlags(PO_PERSISTENT);}

  Bool isPersistent(){
    return (getFlags() & PO_PERSISTENT);}
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
  int newSecBorrow(DSite*,Credit,DSite*,int);

  Bool maybeFreeBorrowEntry(int);

  void freeSecBorrow(int);

  DSite* getOriginSite(int bi){
    return getBorrow(bi)->getNetAddress()->site;}

  int getOriginIndex(int bi){
    return getBorrow(bi)->getNetAddress()->index;}

  void copyBorrowTable(BorrowEntry *,int);

  void closeFrameToProxy(unsigned int ms);
  int closeProxyToFree(unsigned int ms);
};

extern BorrowTable *borrowTable;
#define BT borrowTable

Bool withinBorrowTable(int i); // for assertion

#endif
