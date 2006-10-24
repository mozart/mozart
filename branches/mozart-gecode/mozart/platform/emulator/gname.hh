/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Kostja Popov <kost@sics.se>
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
#include "site.hh"
#include "hashtbl.hh"

//
const unsigned int maxDigit = 0xffffffff;
const int fatIntDigits = 2;
const int lastFatIntDigit = 1;

//
class FatIntBody {
private:
  unsigned int number[fatIntDigits];

  //
public:
  FatIntBody() {}
  void init() {			// only for 'gnameID';
    for (int i = fatIntDigits; i--; )
      number[i] = 0;
  }
#if defined(DEBUG_CHECK)
  void cInit() {		// only for 'noGNameID';
    for (int i = fatIntDigits; i--; )
      number[i] = (unsigned int) -1;
  }
#endif

  // hash only on the last (least important) byte;
  unsigned int fib4hash() {
    return (number[lastFatIntDigit]);
  }
  // Note: this is NOT the natural order on integers - but who cares?
  // Note: this should be optimized for the case "equal", in which
  // case all digits are inspected, and thus the loop has to be
  // efficient;
  int compare(FatIntBody &other) {
    for (int i = fatIntDigits; i--; ) {
      int cmpDigit = number[i] - other.number[i];
      if (cmpDigit)
	return (cmpDigit);
    }
    return (0);
  }

  void inc() {
    // so to say, a high-endian representation;
    unsigned int *dp = &number[lastFatIntDigit];
    while (*dp == maxDigit)
	 *(dp--) = 0;
    Assert(dp >= &number[0]);
    (*dp)++;
  }

  unsigned int getNumber(int i) { return (number[i]); }
  void setNumber(int i, unsigned int d) { number[i] = d; }
};

extern FatIntBody gnameID;

//
class FatInt {
private:
  FatIntBody fib;

  //
public:
  FatInt() {}
#if defined(DEBUG_CHECK)
  void cInit() { fib.cInit(); }	// only for 'noGNameID';
#endif

  //
  void generate() {
    fib = gnameID;
    gnameID.inc();
  }
  unsigned int fi4hash()  { return (fib.fib4hash()); }
  int compare(FatInt &other) { return (fib.compare(other.fib)); }
  unsigned int getNumber(int i) { return (fib.getNumber(i)); }
  void setNumber(int i, unsigned int d) { fib.setNumber(i, d); }
};

DebugCode(extern FatInt noGNameID;)

/*
 * The restricted version is *longer* that the generic one (!)
class FatInt {
public:
  unsigned int d0, d1;

  //
public:
  FatInt() { DebugCode(d0 = d1 = maxDigit;); }

  //
  void inc() {
    if (d0 == maxDigit) {
      d0 = 0;
      d1++;
    } else {
      d0++;
    }
  }

  //
  int same(FatInt &other) {
    if (d0 != other.d0)
      return (NO);
    else
      return (d1 == other.d1);
  }
};
 */

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

#if defined(DEBUG_CHECK)
class GName;
TaggedRef findGNameDEBUG(GName *gn);
#endif

//
class GName : public GenDistEntryNode<GName>,
	      public CppObjMemory {
private:
  TaggedRef value;
  char gcMark;

public:
  char gnameType;
  Site* site;
  FatInt id;
  TaggedRef url;

  GName() { 
    gcMark = 0;
    url = 0;
    value = 0;
    DebugCode(site = (Site *) -1;);
    DebugCode(gnameType = (char) -1;);
    DebugCode(id = noGNameID;);
  }
  // GName(GName &) // this implicit constructor is used!
  GName(Site *s, GNameType gt, TaggedRef val) {
    gcMark = 0;
    url = 0;
    site = s;
    gnameType = (char) gt;
    value = val;
    id.generate();
  }
  ~GName() {
    DebugCode(gcMark = gnameType = (char) -1;);
    DebugCode(value = url = (TaggedRef) -1;);
    DebugCode(site = (Site *) -1;);
  }

  // Support for GenDistEntryNode;
  // Exploit the fact the Site"s are unique;
  unsigned int value4hash() {
    return (site->value4hash() ^ id.fi4hash());
  }
  int compare(GName *other) {
    int cmpSite = ToInt32(site) - ToInt32(other->site);
    if (cmpSite == 0) {
      return (id.compare(other->id));
    } else {
      return (cmpSite);
    }
  }

  GNameType getGNameType() { return (GNameType) gnameType; }

  TaggedRef getValue()       { return value; }
  void setValue(TaggedRef v) { value = v; }

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
  void gcMaybeOff() {
    if (!value) {
      Assert(!findGNameDEBUG(this));
      gcMark = 1;
    } else {
      Assert(findGNameDEBUG(this));
    }
  }
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

  TaggedRef getURL() { return url; }
  void markURL(TaggedRef u) { 
    if (u && !oz_eq(u,NameUnit))
      url = u; 
  }
};

#define GNAME_HASH_TABLE_DEFAULT_SIZE 10

class GNameTable: public GenDistEntryTable<GName> {
private:
  int hash(GName *);

public:
  GNameTable()
    : GenDistEntryTable<GName>(GNAME_HASH_TABLE_DEFAULT_SIZE) {}

  void add(GName *name) {
    Assert(!htFind(name));
    htAdd(name);
  }
  TaggedRef find(GName *name) {
    GName *gn = htFind(name);
    return (gn ? gn->getValue() : makeTaggedNULL());
  }
  // nothing is actually removed like this:
  void remove(GName *name) { 
    Assert(htFind(name));
    htDel(name);
  }
  
  void gCollectGNameTable();
};

//
extern GNameTable gnameTable;

inline
TaggedRef oz_findGName(GName *gn) {
  return (gnameTable.find(gn));
}

inline
void addGName(GName *gn, TaggedRef t) {
  Assert(!oz_findGName(gn));
  gn->setValue(t);
  gnameTable.add(gn);
}

//
// The distribution's lazy protocols involve (atomic) changing a
// gname's binding from a proxy to a proper entity. So, we have:
inline
void overwriteGName(GName *gn, TaggedRef t)
{
  gn->setValue(t);
  if (!oz_findGName(gn))
    gnameTable.add(gn);
}

inline
GName *newGName(TaggedRef t, GNameType gt) {
  GName* gn = new GName(mySite,gt,t);
  Assert(!oz_findGName(gn));
  gnameTable.add(gn);
  return (gn);
}

inline
void gCollectGName(GName* name) {
  if (name)
    name->gCollectGName();
}

void initGNameTable();

#endif
