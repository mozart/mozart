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
#include "referenceConsistency.hh"
#include "bucketHashTable.hh"

class HomeReference;
class RemoteReference;

#ifdef DEBUG_CHECK
Bool withinBorrowTable(int i);
#endif

#define END_FREE -1

enum PO_TYPE {
  PO_Var,
  PO_Tert,
  PO_Ref
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
  } u;
public:
  ProtocolObject()            { type=(PO_TYPE)4711;}
  Bool isTertiary()           { return type==PO_Tert; }
  Bool isRef()                { return type==PO_Ref; }
  Bool isVar()                { return type==PO_Var; }
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

};


/* ********************************************************************** */
/*   SECTION 10:: OwnerEntry                                               */
/* ********************************************************************** */

class OwnerEntry: public OB_Entry,public BucketHashNode {
friend class NewOwnerTable;
public:
  HomeReference homeRef;

  OwnerEntry(int odi, int algs):BucketHashNode((unsigned int) odi, (unsigned int) odi)
  {
    setFlags(0);
    homeRef.setUp(odi, algs);
  }

  void localize(int index);

  inline RRinstance *getCreditBig() {
    return homeRef.getBigReference();
  }

  inline RRinstance *getCreditSmall() {
    return homeRef.getSmallReference();
  }

  inline void mergeReference(RRinstance *r) {
    if(homeRef.mergeReference(r)){
      localize(homeRef.oti);
    }
  }

  void updateReference(DSite *rsite);

  Bool isPersistent(){
    return homeRef.isPersistent();}

  void makePersistent(){
    Assert(!isPersistent());
    homeRef.makePersistent();
    Assert(isPersistent());
  }

  int getOdi(){ return homeRef.oti;}
};

/* ********************************************************************** */
/*   SECTION 11:: OwnerTable                                               */
/* ********************************************************************** */

class NewOwnerTable:public BucketHashTable {
  int localized;  /* Used by the distpane */
  int nxtId;

  void init(int,int);
  void compactify();
unsigned int hash(int Oid){return (unsigned int)Oid;}

public:
  void print();

  OZ_Term extract_info();


  NewOwnerTable(int sz):BucketHashTable(sz) {
    localized = 0;
    nxtId = 0;
  }

  OwnerEntry* index2entry(int oe)
  {
    Assert(oe > 10000);
    return (OwnerEntry*) oe;
  }

  OwnerEntry* odi2entry(int odi);

  int entry2odi(int oe)
  {
    Assert(oe > 10000);
    return ((OwnerEntry*)oe)->getOdi();
  }

  void gcOwnerTableRoots();
  void gcOwnerTableFinal();

  Bool notGCMarked();

  void resize();

  int newOwner(OwnerEntry *&, int);
  int newOwner(OwnerEntry *&oe){
    int algs = (ozconf.dpUseTimeLease?GC_ALG_TL:0)|(ozconf.dpUseFracWRC?GC_ALG_WRC:0);
    return newOwner(oe,algs);
  }

  void freeOwnerEntry(int);
  int  getLocalized(){int ret=localized; localized = 0; return ret;}
  int getNxtId() {return nxtId;}
};


extern NewOwnerTable *ownerTable;
#define OT ownerTable

/* **********************************************************************  */
/*   SECTION 14:: BorrowEntry                                              */
/* **********************************************************************  */

class BorrowEntry: public OB_Entry, public BucketHashNode {
  friend class BorrowTable;

protected:
  Bool canBeFreed();

public:
  RemoteReference remoteRef;

  BorrowEntry(DSite* s, int i,RRinstance *r):BucketHashNode((unsigned int)i, (unsigned int)s)
  {
    setFlags(0);
    remoteRef.setUp(r,s,i);
  }

  OZ_Term extract_info();

  Bool isPersistent(){
    return remoteRef.isPersistent();}

  void gcBorrowRoot(int);
  void gcBorrowUnusedFrame(Tertiary*);

  inline NetAddress* getNetAddress() {
    return &(remoteRef.netaddr);
  }

  DSite *getSite(){return remoteRef.netaddr.site;}

  int getOTI(){return remoteRef.netaddr.index;}

  inline RRinstance *getBigReference() {
    return remoteRef.getBigReference();
  }

  inline RRinstance *getSmallReference() {
    return remoteRef.getSmallReference();
  }

  inline void mergeReference(RRinstance *r) {
    remoteRef.mergeReference(r);
  }

  void freeBorrowEntry();

  void removeGCMark(){
    removeFlags(PO_GC_MARK);
  }
};


class BorrowTable:public BucketHashTable {
private:

  void compactify(){
    printf("New Borrow table not compactified\n");
  }

public:
  void print();

  BorrowTable(int sz):BucketHashTable(sz)  {}

  void gcBorrowTableRoots();
  void gcBorrowTableUnusedFrames();
  void gcBorrowTableFinal();
  void gcFrameToProxy();

  Bool notGCMarked();

  BorrowEntry* find(NetAddress *na);
  BorrowEntry* find(int,DSite*);


  void resize(){printf("resize\n");}

  int newBorrow(RRinstance *,DSite*,int);

  Bool maybeFreeBorrowEntry(int);

  DSite* getOriginSite(int bi){
    return ((BorrowEntry *) bi)->getNetAddress()->site;}

  int getOriginIndex(int bi){
    return ((BorrowEntry*)bi)->getNetAddress()->index;}

  BorrowEntry *bi2borrow(int bi) {return (BorrowEntry*) bi;}

  int dumpFrames();
  void dumpProxies();

  OZ_Term extract_info();
};

extern BorrowTable *borrowTable;
#define BT borrowTable

Bool withinBorrowTable(int i); // for assertion

#endif
