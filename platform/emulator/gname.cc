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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
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
#include "marshaler.hh"

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

void addGName(GName *gn, TaggedRef t) {
  gn->setValue(t);
  addGName(gn);
}

GName *newGName(TaggedRef t, GNameType gt)
{
  GName* ret = new GName(mySite,gt,t);
  addGName(ret);
  return ret;
}

void deleteGName(GName *gn)
{
  delete gn;
}

GName *newGName(PrTabEntry *pr)
{
  GName *ret = newGName(ToInt32(pr),GNT_CODE);
  return ret;
}


PrTabEntry *findCodeGName(GName *gn)
{
  TaggedRef aux = findGName(gn);
  if (aux) {
    return (PrTabEntry*) ToPointer(aux);
  }
  return NULL;
}


/* OBSERVE - this must be done at the end of other gc */
void GNameTable::gcGNameTable()
{
  int index;
  GenHashNode *aux = getFirst(index);
  DebugCode(int used = getUsed());
  while (aux!=NULL) {
    GName *gn = (GName*) aux->getBaseKey();

    DebugCode(used--);

    /* code is never garbage collected */
    if (gn->getGNameType()==GNT_CODE){
      gn->site->setGCFlag();
      goto next_one;}

    if (gn->getGCMark()) {
      gn->resetGCMark();
      gn->site->setGCFlag();
    } else {
      if (gn->getGNameType()==GNT_NAME &&
	  tagged2Literal(gn->getValue())->isNamedName()) {
	goto next_one;
      }
      delete gn;
      if (!htSub(index,aux)) 
	continue;}
  next_one:
    aux = getNext(aux,index);
  }

  Assert(used==0);
  compactify();
}


/**********************************************************************/
/*   SECTION 19 :: Globalizing       stateless BASIC otherwise entity                         */
/**********************************************************************/

GName *Name::globalize()
{
  if (!hasGName()) {
    Assert(oz_isRootBoard(GETBOARD(this)));
    homeOrGName = ToInt32(newGName(makeTaggedLiteral(this),GNT_NAME));
    setFlag(Lit_hasGName);
  }
  return getGName();
}

GName *Abstraction::globalize(){
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_PROC));}
  return getGName();
}

GName *SChunk::globalize() {
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_CHUNK));}
  return getGName();
}

void ObjectClass::globalize() {
  if (!hasGName()) {
    setGName(newGName(makeTaggedConst(this),GNT_CLASS));}
}
