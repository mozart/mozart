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

#ifndef __dp_gname_hh
#define __dp_gname_hh

#include "value.hh"
#include "genhashtbl.hh"
#include "site.hh"

const int fatIntDigits = 2;
const unsigned int maxDigit = 0xffffffff;  

class FatInt {
public:
  unsigned int number[fatIntDigits];

  FatInt() { for(int i=0; i<fatIntDigits; i++) number[i]=0; }
  void inc()
  {
    int i=0;
    while(number[i]==maxDigit) {
      number[i]=0;
      i++;
    }
    Assert(i<fatIntDigits);
    number[i]++;
  }

  Bool same(FatInt &other)
  {
    for (int i=0; i<fatIntDigits; i++) {
      if(number[i]!=other.number[i])
	return NO;
    }
    return OK;
  }
};

extern FatInt *idCounter;

enum GNameType {
  GNT_NAME,
  GNT_PROC,
  GNT_CODE_UNUSED,
  GNT_CHUNK,
  GNT_OBJECT,
  GNT_CLASS,
  GNT_PROMISE
};

const int MAX_GNT = GNT_PROMISE;

class GName {
  TaggedRef value;
  char gcMark;

public:
  char gnameType;
  Site* site;
  FatInt id;
  TaggedRef url;

  TaggedRef getURL() { return url; }
  void markURL(TaggedRef u) { 
    if (u && !oz_eq(u,NameUnit))
      url = u; 
  }

  TaggedRef getValue()       { return value; }
  void setValue(TaggedRef v) { value = v; }

  Bool same(GName *other) {
    return (site==other->site && id.same(other->id));
  }

  GName() { gcMark = 0; url=0; value = 0; }
  // GName(GName &) // this implicit constructor is used!
  GName(Site *s, GNameType gt, TaggedRef val) 
  {
    gcMark = 0;
    url = 0;
    site=s;
    idCounter->inc();
    id = *idCounter;
    gnameType = (char) gt;
    value = val;
  }
  
  GNameType getGNameType() { return (GNameType) gnameType; }

  void setGCMark()   { gcMark = 1; }
  Bool getGCMark()   { return gcMark; }
  void resetGCMark() { gcMark = 0;}

  //
  // Disable GC"ing of names that are not in the table
  // (e.g. distribution marshaler keeps certain GNames outside the
  // table for some time). A gname with a value must be in the table,
  // and, respectively, a gname without a value must stay outside. The
  // limitation of the current implementation is that value-free names
  // cannot contain GC"able stuff ('cause the 'gcMark' is (ab)used);
  void gcMaybeOff();
  void gcOn() { gcMark = 0; }

  void gcMarkSite() {
    site->setGCFlag();
  }
  void gCollectGName() {
    if (!getGCMark()) {
      setGCMark();
      gcMarkSite();
      oz_gCollectTerm(value,value);
    }
  }
};

#define GNAME_HASH_TABLE_DEFAULT_SIZE 500

class GNameTable: public GenHashTable{
  friend TaggedRef findGName(GName *gn);
private:
  int hash(GName *);
  TaggedRef find(GName *name);
public:
  void add(GName *name);
  void remove(GName *name);
  GNameTable():GenHashTable(GNAME_HASH_TABLE_DEFAULT_SIZE) {}
  
  void gCollectGNameTable();
};

extern GNameTable gnameTable;
#define GT gnameTable

TaggedRef oz_findGName(GName *gn);
void addGName(GName *gn, TaggedRef t);
void overwriteGName(GName *gn, TaggedRef t);
GName *newGName(TaggedRef t, GNameType gt);

inline void gCollectGName(GName* name) { if (name) name->gCollectGName(); }

#endif
