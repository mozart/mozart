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
 *    Organization or Person (Year(s))
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

#include "base.hh"
#include "gname.hh"
#include "site.hh"
#include "am.hh"
#include "board.hh"

//
GNameTable gnameTable;
FatInt *idCounter;

int GNameTable::hash(GName *gname)
{
 int ret = gname->site->hash();
  for(int i=0; i<fatIntDigits; i++) {
    ret += gname->id.number[i];
  }
  return ret<0?-ret:ret;
}

inline void GNameTable::add(GName *name)
{
  int hvalue=hash(name);
  GenHashTable::htAdd(hvalue,(GenHashBaseKey*)name,0);
}

TaggedRef GNameTable::find(GName *name)
{
  int hvalue = hash(name);
  GenHashNode *aux = htFindFirst(hvalue);
  while(aux) {
    GName *gn = (GName*)aux->getBaseKey();
    if (name->same(gn)) {
      return gn->getValue();
    }
    aux = htFindNext(aux,hvalue); }
  return makeTaggedNULL();
}

void GNameTable::remove(GName *name)
{
  int hvalue = hash(name);
  GenHashNode *aux = htFindFirst(hvalue);
  while(aux) {
    GName *gn = (GName *) aux->getBaseKey();
    if (name->same(gn)) {
      htSub(hvalue, aux);
      break;
    }
    aux = htFindNext(aux, hvalue);
  }
}

inline TaggedRef findGName(GName *gn) {
  return GT.find(gn);
}

TaggedRef oz_findGName(GName *gn) 
{
  return findGName(gn);
}

inline void addGName(GName *gn) {
  Assert(!findGName(gn));
  GT.add(gn);
}

void addGName(GName *gn, TaggedRef t)
{
  gn->setValue(t);
  addGName(gn);
}

//
// The distribution's lazy protocols involve (atomic) changing a
// gname's binding from a proxy to a proper entity. So, we have:
void overwriteGName(GName *gn, TaggedRef t)
{
  gn->setValue(t);
  if (!findGName(gn))
    addGName(gn);
}

GName *newGName(TaggedRef t, GNameType gt)
{
  GName* ret = new GName(mySite,gt,t);
  addGName(ret);
  return ret;
}

static
Bool checkGName(GName *gn)
{
  if (gn->getGCMark()) {
    gn->resetGCMark();
    gn->site->setGCFlag();
    return OK;
  }
  if (gn->getGNameType()==GNT_NAME &&
      tagged2Literal(gn->getValue())->isNamedName()) {
    return OK;
  }    
  delete gn;
  return NO;
}

/* OBSERVE - this must be done at the end of other gc */
void GNameTable::gCollectGNameTable()
{
  int i=0;
  GenHashNode *ghn1,*ghn=getFirst(i);
  while(ghn!=NULL){
    GName *gn;
    gn = (GName*) (ghn->getBaseKey());
    if (checkGName(gn)==NO) {
      deleteFirst(ghn);
      ghn=getByIndex(i);
      continue;
    }
    ghn1=ghn->getNext();
    while(ghn1!=NULL) {
      gn = (GName*) (ghn1->getBaseKey());
      if (checkGName(gn)==NO) {
	deleteNonFirst(ghn,ghn1);
	ghn1=ghn->getNext();
	continue;
      }
      ghn=ghn1;
      ghn1=ghn1->getNext();
    }
    i++;
    ghn=getByIndex(i);
  }
  compactify();
}

//
void GName::gcMaybeOff()
{
  if (!value) {
    Assert(!findGName(this));
    gcMark = 1;
  } else {
    Assert(findGName(this));
  }
}

/**********************************************************************/
/*   SECTION 19 :: Globalizing       stateless BASIC otherwise entity */
/**********************************************************************/

GName *Name::globalize()
{
  if (!hasGName()) {
    Assert(oz_isRootBoard(GETBOARD(this)));
    homeOrGName = ToInt32(newGName(makeTaggedLiteral(this),GNT_NAME));
    setFlag(Lit_hasGName);
  }
  return getGName1();
}

GName *Abstraction::globalize(){
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_PROC));}
  return getGName1();
}

GName *SChunk::globalize() {
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_CHUNK));}
  return getGName1();
}

GName *ObjectClass::globalize() {
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_CLASS));}
  return getGName1();
}
