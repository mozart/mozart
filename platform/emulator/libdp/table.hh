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
#include "perdio.hh"
#include "referenceConsistency.hh"
#include "hashtbl.hh"

class HomeReference;
class RemoteReference;

//
enum PO_TYPE {
  PO_Tert = 0,
  PO_Var,
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
  ProtocolObject()   { DebugCode(type = (PO_TYPE) 4711;); }
  Bool isTertiary()  { return type==PO_Tert; }
  Bool isRef()       { return type==PO_Ref; }
  Bool isVar()       { return type==PO_Var; }
  Bool initialized() {
    DebugCode(return (type!=(PO_TYPE)4712));
    return (TRUE);
  }

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

  void mkVar(TaggedRef v, unsigned short f) {
    Assert(oz_isRef(v) && oz_isVar(*(tagged2Ref(v))));
    type=PO_Var; u.ref=v; flags=f;
  }

  void mkVar(TaggedRef v){
    Assert(oz_isRef(v) && oz_isVar(*(tagged2Ref(v))));
    type=PO_Var; u.ref=v;
  }

  void changeToRef(){
    Assert(isVar()); type=PO_Ref; }

  void changeToVar(TaggedRef v){
    Assert(oz_isRef(v) && oz_isVar(*(tagged2Ref(v))));
    type=PO_Var; u.ref=v;
  }

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
public:
  OB_Entry() {}

  void makeGCMark(){addFlags(PO_GC_MARK);}
  Bool isGCMarked(){ return (getFlags() & PO_GC_MARK); }
  void removeGCMark(){ removeFlags(PO_GC_MARK); }

  //
  void gcPO(Tertiary *newval) {
    if (isGCMarked())
      return;
    makeGCMark();

    setTert(newval);
  }
  void gcPO();

  //
  void print();
};


/* ********************************************************************** */
/*   SECTION 10:: OwnerEntry                                              */
/* ********************************************************************** */

class OwnerEntry : public OB_Entry, public CppObjMemory {
public:
  // 'homeRef' keeps the 'extOTI' as is;
  HomeReference homeRef;

  //
public:
  // when allocating the owner table:
  OwnerEntry(Ext_OB_TIndex extOTI) {
    setFlags(0);
    homeRef.setUp(extOTI);
  }

  //
  void localize();
  ~OwnerEntry() { DebugCode(setFlags((unsigned short) -1);); }

  // credits;
  inline RRinstance *getCreditBig() {
    return homeRef.getBigReference();
  }
  inline RRinstance *getCreditSmall() {
    return homeRef.getSmallReference();
  }
  inline void mergeReference(RRinstance *r) {
    if (homeRef.mergeReference(r))
      localize();
  }

  // kost@ : what is that ?
  void updateReference(DSite *rsite);

  //
  Bool isPersistent() {
    return homeRef.isPersistent();
  }
  void makePersistent(){
    Assert(!isPersistent());
    homeRef.makePersistent();
    Assert(isPersistent());
  }

  Ext_OB_TIndex getExtOTI() { return (homeRef.getExtOTI()); }
};

//
inline OwnerEntry* ownerIndex2ownerEntry(OB_TIndex oe)
{
  return ((OwnerEntry *) oe);
}
inline Ext_OB_TIndex ownerEntry2extOTI(OB_TIndex oe)
{
  return (ownerIndex2ownerEntry(oe)->getExtOTI());
}

/* ********************************************************************** */
/*   SECTION 11:: OwnerTable                                              */
/* ********************************************************************** */

// kost@ : used to be a hash table at some point in time (that mapped
// external indices to internal ones), but now the following scheme is
// adapted:
// . internal indices are pointers to owner entries;
// . external (network) indices are 'OwnerTable' array indices.
//   . in order to implement "unforgeable" references, a random
//     number can be included in external indices;
// Note that this way the table cannot be in general compactified (but
// never was anyway). Should we want that, we can still re-introduce
// the hash table back;

//
class OwnerTableSlot {
private:
  OwnerEntry *oe;
  int next;

public:
  void setFree(int indexIn) {
    oe = (OwnerEntry *) 0;
    next = indexIn;
  }
  int unsetFree() {
    int index = next;
    DebugCode(next = -1;);
    return (index);
  }
  int isFree() { return (!oe); }

  OwnerTableSlot() : oe((OwnerEntry *) 0) { DebugCode(setFree(-1);); }
  ~OwnerTableSlot() { DebugCode(setFree(-1);); }

  void setOE(OwnerEntry *oeIn) { oe = oeIn; }
  OwnerEntry *getOE() { return (oe); }
};

//
class OwnerTable {
private:
  OwnerTableSlot *table;
  int tableSize;
  DebugCode(int counter;);      // number of allocated entries;
  int nextfree;                 // -1 if none;
  int localized;                // Used by the distpane;

private:
  void resize();
  void compactify() {}

  // External indices are just table indices.  Otherwise, hashing
  // would be involved - thus, generally this is a method of the owner
  // table;
  int extOTI2ownerTableIndex(Ext_OB_TIndex extOTI) { return (extOTI); }

  //
public:
  OwnerTable(int sizeIn);
  ~OwnerTable();

  //
  OwnerEntry* extOTI2ownerEntry(Ext_OB_TIndex extOTI) {
    Assert(!table[extOTI2ownerTableIndex(extOTI)].isFree());
    return (table[extOTI2ownerTableIndex(extOTI)].getOE());
  }

  void gcOwnerTableRoots();
  void gcOwnerTableFinal();
  DebugCode(Bool notGCMarked(););

  // 'newOwner()' does not assign the type: this is supposed to be
  // done later with one of the 'mkVar()', 'mkTertiary()' etc.;
  OB_TIndex newOwner(OwnerEntry *&oe) {
    if (nextfree < 0) resize();

    //
    int index = nextfree;
    nextfree = table[index].unsetFree();
    DebugCode(counter++;);

    //
    Ext_OB_TIndex extOTI = MakeExt_OB_TIndex(index);
    oe = new OwnerEntry(extOTI);
    table[index].setOE(oe);
    return (MakeOB_TIndex(oe));
  }

  //
  void freeOwnerEntry(Ext_OB_TIndex extOTI) {
    int index = extOTI2ownerTableIndex(extOTI);
    DebugCode(counter--;);
    table[index].setFree(nextfree);
    nextfree = index;
    localized++;
  }

  //
  int getSize() { return (tableSize); }
  OwnerEntry *getOE(int i) { return (table[i].getOE()); }

  // kost@ : statistics ?
  int getLocalized() { int ret=localized; localized = 0; return (ret); }
  OZ_Term extract_info();
  void print();

#if defined(DEBUG_CHECK)
  void checkEntries();
#endif
};

extern OwnerTable *ownerTable;
#define OT ownerTable

/* **********************************************************************  */
/*   SECTION 14:: BorrowEntry                                              */
/* **********************************************************************  */

class BorrowEntry : public OB_Entry,
                    public GenDistEntryNode<BorrowEntry>,
                    public CppObjMemory {
  friend class BorrowTable;
public:
  RemoteReference remoteRef;

  //
protected:
  Bool canBeFreed();

  //
public:
  BorrowEntry() {
    DebugCode(setFlags((unsigned short) -1););
  }
  BorrowEntry(DSite* s, Ext_OB_TIndex extOTI, RRinstance *r) {
    setFlags(0);
    remoteRef.setUp(r, s, extOTI);
  }
  //
  void freeBorrowEntry();
  ~BorrowEntry() { Assert(getFlags() == (unsigned short) -1); }

  //
  NetAddress* getNetAddress() { return (remoteRef.getNetAddress()); }
  DSite *getSite(){ return ((remoteRef.getNetAddress())->site); }
  Ext_OB_TIndex getExtOTI() { return ((remoteRef.getNetAddress())->index); }

  // Support for GenDistEntryNode;
  // Exploit the fact the DSite"s are unique;
  unsigned int value4hash() {
    NetAddress *netAddr = remoteRef.getNetAddress();
    unsigned int v = (unsigned int) ToInt32(netAddr->site);
    v = ((v<<13)^(v>>19)) ^ ((unsigned int) netAddr->index);
    return (v);
  }
  int compare(BorrowEntry *be) {
    NetAddress *myAddr = remoteRef.getNetAddress();
    NetAddress *beAddr = (be->remoteRef).getNetAddress();
    int cmpSite = ToInt32(myAddr->site) - ToInt32(beAddr->site);
    if (cmpSite == 0) {
      return (myAddr->index - beAddr->index);
    } else {
      return (cmpSite);
    }
  }

  Bool isPersistent() {
    return (remoteRef.isPersistent());
  }

  void gcBorrowRoot();
  void gcBorrowUnusedFrame();

  // credits;
  inline RRinstance *getBigReference() {
    return remoteRef.getBigReference();
  }
  inline RRinstance *getSmallReference() {
    return remoteRef.getSmallReference();
  }
  inline void mergeReference(RRinstance *r) {
    remoteRef.mergeReference(r);
  }

  void removeGCMark(){
    removeFlags(PO_GC_MARK);
  }

  // kost@ : statistics ?
  OZ_Term extract_info();
};

//
inline BorrowEntry* borrowIndex2borrowEntry(OB_TIndex bi)
{
  return ((BorrowEntry *) bi);
}

//
class BorrowTable : public GenDistEntryTable<BorrowEntry> {
public:
  BorrowTable(int sizeAsPowerOf2)
    : GenDistEntryTable<BorrowEntry>(sizeAsPowerOf2) {}

  //
  OB_TIndex newBorrow(RRinstance *c, DSite *sd, Ext_OB_TIndex extOTI) {
    BorrowEntry* be = new BorrowEntry(sd, extOTI, c);
    htAdd(be);
    return (MakeOB_TIndex(be));
  }

  //
  BorrowEntry* find(Ext_OB_TIndex extOTI, DSite *site) {
    BorrowEntry cmpBE;
    cmpBE.remoteRef.netaddr.site = site;
    cmpBE.remoteRef.netaddr.index = extOTI;
    return (htFind(&cmpBE));
  }

  //
  Bool maybeFreeBorrowEntry(OB_TIndex index) {
    BorrowEntry *be = borrowIndex2borrowEntry(index);
    if (!be->remoteRef.canBeReclaimed()) {
      if (be->isVar())
        be->changeToRef();
      return (FALSE);
    } else {
      htDel(be);
      be->freeBorrowEntry();
      delete be;
      return (TRUE);
    }
  }

  // GC;
  void gcBorrowTableRoots();
  void gcBorrowTableUnusedFrames();
  void gcBorrowTableFinal();
  void gcFrameToProxy();
  DebugCode(Bool notGCMarked(););

  //
  DSite* getOriginSite(OB_TIndex bi){
    return ((BorrowEntry *) OB_TIndex2Ptr(bi))->getNetAddress()->site;
  }
  Ext_OB_TIndex getOriginIndex(OB_TIndex bi) {
    return (((BorrowEntry*) OB_TIndex2Ptr(bi))->getNetAddress()->index);
  }

  // process termination;
  int dumpFrames();
  void dumpProxies();

  // kost@ : statistics ?
  OZ_Term extract_info();

  void print();

private:
  void compactify() {
    printf("New Borrow table not compactified\n");
  }
};

extern BorrowTable *borrowTable;
#define BT borrowTable

#endif
