/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Per Brand, 1998
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
 
#if defined(INTERFACE)
#pragma implementation "table.hh"
#endif

#include "base.hh"
#include "table.hh"
#include "value.hh"
#include "var.hh"
#include "var_obj.hh"
#include "msgContainer.hh"
#include "state.hh"
#include "fail.hh"
#include "protocolState.hh"
#include "dpResource.hh"
#include "os.hh" 


#define PO_getValue(po) \
((po)->isTertiary() ? makeTaggedConst((po)->getTertiary()) : (po)->getRef())

//
template class GenDistEntryTable<BorrowEntry>;
#include "hashtblDefs.cc"


OwnerTable *ownerTable;
BorrowTable *borrowTable;


// from var.cc
void oz_dpvar_localize(TaggedRef *);

void OwnerEntry::localize()
{
  if (isTertiary()) {
    if (!localizeTertiary(getTertiary()))
      return;
  } else if (isVar()) {
    if (GET_VAR(this,Manager)->getInfo() == NULL)
      oz_dpvar_localize(getPtr());
    else 
      return;
  }
  // entity localization complete - get rid of 'this';
  OT->freeOwnerEntry(getExtOTI());
  delete this;
}

void OwnerEntry::updateReference(DSite* site)
{
  if (isVar() || isTertiary()) {
    MsgContainer *msgC = msgContainerManager->newMsgContainer(site);
    msgC->put_M_BORROW_REF(getValue());
    send(msgC);
  } else
    printf("Warning: Updating bound variable!\n");
}

OZ_Term BorrowEntry::extract_info()
{
  OZ_Term primCred, secCred;
  OZ_Term na=
    OZ_recordInit(oz_atom("netAddress"),
		  oz_cons(oz_pairA("site", oz_atom(remoteRef.netaddr.site->stringrep_notype())),
		  oz_cons(oz_pairAI("index",(int)remoteRef.netaddr.index), oz_nil())));

  return OZ_recordInit(oz_atom("be"),
		       oz_cons(oz_pairAA("type", toC(PO_getValue(this))),
		       oz_cons(oz_pairA("na", na),
		       oz_cons(oz_pairA("dist_gc",remoteRef.extract_info()),
			       oz_nil()))));
}

void BorrowEntry::freeBorrowEntry()
{
  Assert(remoteRef.canBeReclaimed());
  if (isVar() && typeOfBorrowVar(this) == VAR_PROXY) {
    GET_VAR(this,Proxy)->nowGarbage(this);
  }
  if (!isPersistent())
    remoteRef.dropReference();
  DebugCode(setFlags((unsigned short) -1););
}

void BorrowEntry::gcBorrowRoot()
{
  if (isTertiary()) {
    if(getTertiary()->cacIsMarked() || 
       !remoteRef.canBeReclaimed() ||
       isTertiaryPending(getTertiary())) {
      gcPO();
    }
  } else if (isRef()) {
    gcPO(); 
  } else {
    Assert(isVar());
    if (oz_isMark(getRef())) {
      gcPO();
    } else if (tagged2Var(*getPtr())->getSuspList() != 0) {
      gcPO();
    } else if (!remoteRef.canBeReclaimed()) {
      gcPO();
    }
  }
}

void OB_Entry::gcPO()
{
  if (isGCMarked()) return;
  makeGCMark();
  Assert(isTertiary() || isRef() || isVar());
  PD((GC,"var/ref/tert found"));
  oz_gCollectTerm(u.tert, u.tert);
}

void BorrowTable::gcBorrowTableRoots()
{
  for (int i = getSize(); i--; ) {
    BorrowEntry *be = getFirstNode(i); 
    while (be) {
      if (!be->isGCMarked())
	be->gcBorrowRoot();
      be = be->getNext();
    }
  }
}

void BorrowEntry::gcBorrowUnusedFrame()
{
  Tertiary* t = getTertiary();
  Assert(!(isTertiaryPending(t)));
  if (t->getType() == Co_Cell) {
    CellFrame *cf = (CellFrame*) t;
    if (cf->dumpCandidate())
      cellLockSendDump(this);
  } else {
    Assert(t->getType() == Co_Lock);
    LockFrame *lf = (LockFrame*) t;
    if (lf->dumpCandidate())
      cellLockSendDump(this);
  }
  gcPO();
}

void BorrowTable::gcBorrowTableUnusedFrames()
{
  for (int i = getSize(); i--; ) {
    BorrowEntry *be = getFirstNode(i); 
    while (be) {
      if ((!be->isGCMarked()) &&
	  be->isTertiary() && 
	  be->getTertiary()->isFrame())
	be->gcBorrowUnusedFrame();
      be = be->getNext();
    }
  }
}

void BorrowTable::gcFrameToProxy()
{
  for (int i = getSize(); i--; ) {
    BorrowEntry *be = getFirstNode(i); 
    while (be) {
      if ((be->isTertiary())) {
	Tertiary *t= be->getTertiary();
	if (t->isFrame()) {
	  if ((t->getType()==Co_Cell)
	      && ((CellFrame*)t)->getState()==Cell_Lock_Invalid
	      && ((CellSec*)((CellFrame*)t)->getSec())->getPending()==NULL) {
	    ((CellFrame*)t)->convertToProxy();
	  } else {
	    if ((t->getType()==Co_Lock)
		&& ((LockFrame*)t)->getState()==Cell_Lock_Invalid)
	      ((LockFrame*)t)->convertToProxy();
	  }
	}
      }
      be = be->getNext();
    }
  }
}


/* OBSERVE - this must done at the end of other gc */
void BorrowTable::gcBorrowTableFinal()
{
  for (int i = getSize(); i--; ) {
    BorrowEntry *be = getFirstNode(i); 
    while (be) {
      BorrowEntry *nbe = be->getNext();

      //
      if (be->isTertiary()) {
	Tertiary *t = be->getTertiary();
	if (be->isGCMarked()) {
	  be->removeGCMark();
	  be->getSite()->makeGCMarkSite();
	} else {
	  if (!errorIgnore(t)) maybeUnask(t);
	  Assert(t->isProxy());
	  borrowTable->maybeFreeBorrowEntry(MakeOB_TIndex(be));
	}

	//
      } else if (be->isRef()) {
	Assert(be->isGCMarked());
	be->removeGCMark();
	be->getSite()->makeGCMarkSite();

	//
      } else {
	Assert(be->isVar());
	if (be->isGCMarked()) {
	  be->removeGCMark();
	  be->getSite()->makeGCMarkSite();
	} else {
	  if(!errorIgnoreVar(be)) maybeUnaskVar(be);
	  borrowTable->maybeFreeBorrowEntry(MakeOB_TIndex(be));
	}
      }

      //
      be = nbe;
    }
  }
}

int BorrowTable::dumpFrames()
{
  int notReady = 0;

  //
  for (int i = getSize(); i--; ) {
    BorrowEntry *be = getFirstNode(i); 
    while (be) {
      if (be->isTertiary()){
	Tertiary *t = be->getTertiary();
	if (t->isFrame()) {
	  int type = t->getType();
	  int state;

	  if (type == Co_Cell) {
	    state = ((CellFrame *) t)->getState();
	    ((CellFrame *) t)->getCellSec()->dumpPending();
	  } else if (type == Co_Lock) {
	    state = ((LockFrame *) t)->getState();
	    ((LockFrame *) t)->getLockSec()->dumpPending();
	  } else {
	    be = be->getNext();
	    continue;
	  }

	  switch (state){
	  case Cell_Lock_Invalid:
	    if (type == Co_Lock)
	      ((CellFrame *) t)->convertToProxy();
	    else
	      ((LockFrame *) t)->convertToProxy();
	    break;

	  case Cell_Lock_Requested:
	  case Cell_Lock_Valid:
	    cellLockSendDump(be);
	    if (type == Co_Lock)
	      ((CellFrame *) t)->getCellSec()->markDumpAsk();
	    else
	      ((LockFrame *) t)->getLockSec()->markDumpAsk();
	    notReady++;		// dumping just begun;
	    break;

	  case Cell_Lock_Valid|Cell_Lock_Dump_Asked:
	  case Cell_Lock_Requested|Cell_Lock_Next:
	  case Cell_Lock_Requested|Cell_Lock_Next|Cell_Lock_Dump_Asked:
	  case Cell_Lock_Requested|Cell_Lock_Dump_Asked:
	    notReady++;		// dumping still in progress;
	    break;

	    // kost@ : optimization: don't do 'ask dump' protocol, but
	    // just break the lock locally and send it away:
	  case Cell_Lock_Valid|Cell_Lock_Next:
	    if(type == Co_Lock) {
	      NetAddress *na = be->getNetAddress();
	      LockSec *sec = ((LockFrame *) t)->getLockSec();
	      sec->markInvalid();
	      DSite *toS = sec->getNext();
	      lockSendToken(na->site, na->index, toS);
	      ((LockFrame *) t)->convertToProxy();
	    } else {
	      Assert(0);
	    }
	    break;

	  default:
	    Assert(0);
	  }
	}
      }

      //
      be = be->getNext();
    }
  }
  return (notReady);
}

//
void BorrowTable::dumpProxies()
{
  DebugCode(int proxies = 0;);
  DebugCode(int frames = 0;);

  //
  for (int i = getSize(); i--; ) {
    BorrowEntry *be = getFirstNode(i); 
    while (be) {
      BorrowEntry *nbe = be->getNext();

      //
      if (be->isTertiary()) {
	Tertiary *t = be->getTertiary();

	if (t->isProxy()) {
	  maybeFreeBorrowEntry(MakeOB_TIndex(be));
	  DebugCode(proxies++;);
	} else {
	  // kost@ : any frames left over are stuck here: it does not
	  // make any sense to reclaim the credit 'cause some credit
	  // will be lost anyway;
	  if (t->isFrame()) {
	    DebugCode(frames++;);
	  }
	}
      } else {
	if (be->isVar() && oz_isProxyVar(oz_deref(be->getRef()))) {
	  maybeFreeBorrowEntry(MakeOB_TIndex(be));
	  DebugCode(proxies++;);
	}
      }

      //
      be = nbe;
    }
  }
}

#if defined(DEBUG_CHECK)
int BorrowTable::notGCMarked()
{
  for (int i = getSize(); i--; ) {
    BorrowEntry *be = getFirstNode(i); 
    while (be) {
      if (be->isGCMarked())
	return (FALSE);
      if (be->isTertiary()) {
	if (be->getTertiary()->cacIsMarked())
	  return (FALSE);
	Assert(borrowIndex2borrowEntry(MakeOB_TIndex(be->getTertiary()->getTertPointer())) == be);
      }
      be = be->getNext();
    }
  }
  return (TRUE);
}
#endif

//
// Owner Table
//

OwnerTable::OwnerTable(int sizeIn)
  : tableSize(sizeIn), localized(0) DebugArg(counter(0))
{
  table = (OwnerTableSlot *) malloc(tableSize * sizeof(OwnerTableSlot));
  if (table == (OwnerTableSlot *) 0)
    OZ_error("Memory allocation: OwnerTable (re)allocation not possible");
  //
  table[tableSize-1].setFree(-1);
  for (int i = tableSize-1; i--; )
    table[i].setFree(i+1);
  nextfree = 0;
}

OwnerTable::~OwnerTable()
{
  DebugCode(tableSize = counter = localized = nextfree = -1;);
  delete table;
  DebugCode(table = (OwnerTableSlot *) -1;);
}

void OwnerTable::resize()
{
  int newsize = (ozconf.dpTableExpandFactor * tableSize) / 100;
  Assert(newsize > tableSize);
  table =
    (OwnerTableSlot *) realloc(table, newsize * sizeof(OwnerTableSlot));
  if (table == (OwnerTableSlot *) 0)
    OZ_error("Memory allocation: OwnerTable (re)allocation not possible");
  //
  table[newsize-1].setFree(-1);
  for (int i = tableSize; i < newsize-1; i++)
    table[i].setFree(i+1);
  nextfree = tableSize;
  //
  tableSize = newsize;
}

OZ_Term OwnerTable::extract_info()
{
  OZ_Term list=oz_nil();
  OZ_Term credit;
  int size = getSize();

  //
  for (int i = 0; i<size; i++) {
    if (!table[i].isFree()) {
      OwnerEntry *oe = table[i].getOE();
      credit = oe->homeRef.extract_info();
      list=
	oz_cons(OZ_recordInit(oz_atom("oe"),
			      oz_cons(oz_pairAI("extOTI",oe->getExtOTI()),
			      oz_cons(oz_pairAA("type",toC(PO_getValue(oe))),
			      oz_cons(oz_pairA("dist_gc", credit), 
				      oz_nil())))), list);
    }
  }

  //
  return OZ_recordInit(oz_atom("ot"),
		       oz_cons(oz_pairAI("size", size),
		       oz_cons(oz_pairAI("localized", getLocalized()),
		       oz_cons(oz_pairA("list", list), oz_nil()))));  
}
 
void OwnerTable::print() { printf("OwnerTable::print\n"); }

void OwnerTable::gcOwnerTableRoots()
{
  for (int i = tableSize; i--; )
    if (!table[i].isFree()) {
      OwnerEntry *oe = table[i].getOE();
      oe->gcPO();
    }
} 

void OwnerTable::gcOwnerTableFinal()
{
  for (int i = tableSize; i--; ) {
    if (!table[i].isFree()) {
      OwnerEntry *oe = table[i].getOE();
      oe->removeGCMark();
      if (oe->isVar()) {
	TaggedRef *ptr = oe->getPtr();
	DEREFPTR(v,ptr);
	Assert(oz_isManagerVar(v));
	oe->mkVar(makeTaggedRef(ptr), oe->getFlags());
      }
    }
  }
  // compactify?
}

#if defined(DEBUG_CHECK)
int OwnerTable::notGCMarked()
{
  int size = getSize();
  for (int i=0; i<size; i++) {
    if (!table[i].isFree()) {
      OwnerEntry *oe = table[i].getOE();
      if (oe->isGCMarked() ||
	  (oe->isTertiary() && oe->getTertiary()->cacIsMarked()))
	return (FALSE);
    }
  }
  return (TRUE);
}
#endif

#if defined(DEBUG_CHECK)
void OwnerTable::checkEntries()
{
//    for (int i = tableSize; i--; ) {
//      if (!table[i].isFree()) {
//        OwnerEntry *oe = table[i].getOE();
//        if (oe->isVar()) {
//  	TaggedRef *ptr = oe->getPtr();
//  	if (!oz_isVar(*ptr))
//  	  OZ_error("bang!");
//        }
//      }
//    }
}
#endif

OZ_Term BorrowTable::extract_info()
{
  OZ_Term ans = oz_nil();
  int size = getSize();
  for (int i=0; i<size; i++) {
    BorrowEntry *be = getFirstNode(i); 
    while (be) {
      ans = oz_cons(be->extract_info(), ans);
      be = be->getNext();
    }
  }
  return OZ_recordInit(oz_atom("ot"),
		       oz_cons(oz_pairAI("size", size),
		       oz_cons(oz_pairA("list",ans),oz_nil())));

}

void BorrowTable::print()
{
  int size = getSize();
  for (int i=0;i<size;i++) {
    BorrowEntry *be = getFirstNode(i); 
    while (be) {
      printf("E %d", i);
      while (be){
	printf(" %d#%d", be->getExtOTI(), ToInt32(be));
	be = be->getNext();
      }
    }
  }
  printf("\n"); 
}
