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

#include "dp_gname.hh"
#include "comm.hh"

GNameTable gnameTable;

FatInt *idCounter;

int GNameTable::hash(GName *gname)
{
 int ret = gname->site->hashSecondary();
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
