/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Contributors:
 *    Michael Mehl <mehl@ps.uni-sb.de>
 *    Ralf Scheidhauer <scheidhr@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Michael Mehl, 1999
 *    Ralf Scheidhauer, 1999
 *    Christian Schulte, 2000
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

#ifndef __INDEXINGH
#define __INDEXINGH

#ifdef INTERFACE
#pragma interface
#endif

#include "value.hh"

class IHashTableEntry {
public:
  TaggedRef    val;
  SRecordArity sra;
  int          lbl;
};

class IHashTable {
public:
  int elseLbl;
  int listLbl;
  int hashMsk;
  IHashTableEntry entries[1];

  static IHashTable * allocate(int, int);
  IHashTable * clone(void);
  void gCollect(void);

  void deallocate(void) {
    free(this);
  }

  int getSize(void) {
    return hashMsk+1;
  }

  int getEntries(void);

  void addRecord(TaggedRef, SRecordArity, int);
  void addScalar(TaggedRef, int);
  void addLTuple(int lbl) {
    listLbl = lbl;
  }

  int lookupElse(void) {
    return elseLbl;
  }

  int lookupLTuple(void) {
    /* If there is no alternative list, listLabel equals elseLabel */
    return listLbl;
  }
  
  int lookupLiteral(TaggedRef t) {
    register int hm = hashMsk;
    register int i  = tagged2Literal(t)->hash();
    while (OK) {
      i &= hm;
      if (!entries[i].val)
	break;
      if (oz_eq(entries[i].val,t) && 
	  sameSRecordArity(entries[i].sra,mkTupleWidth(0)))
	break;
      i++;
    }
    return entries[i].lbl;
  }
  
  int lookupSmallInt(TaggedRef t) {
    register int hm = hashMsk;
    register int i  = smallIntHash(t);
    while (OK) {
      i &= hm;
      if (!entries[i].val)
	break;
      if (oz_eq(entries[i].val,t))
	break;
      i++;
    }
    return entries[i].lbl;
  }
  
  int lookupBigInt(TaggedRef t) {
    BigInt * b = tagged2BigInt(t);
    int i      = b->hash();
    while (OK) {
      i &= hashMsk;
      if (!entries[i].val)
	break;
      if (oz_isConst(entries[i].val) && 
	  tagged2BigInt(entries[i].val)->equal(b))
	break;
      i++;
    }
    return entries[i].lbl;
  }
  
  int lookupFloat(TaggedRef t) {
    Float * f = tagged2Float(t);
    double d  = f->getValue();
    int i     = f->hash();
    while (OK) {
      i &= hashMsk;
      if (!entries[i].val)
	break;
      if (oz_isFloat(entries[i].val) && 
	  tagged2Float(entries[i].val)->getValue()==d)
	break;
      i++;
    }
    return entries[i].lbl;
  }
  
  int lookupSRecord(TaggedRef t) {
    int hm           = hashMsk;
    SRecord * r      = tagged2SRecord(t);
    TaggedRef l      = r->getLabel();
    SRecordArity sra = r->getSRecordArity();
    int i            = tagged2Literal(l)->hash();
    while (OK) {
      i &= hm;
      if (!entries[i].val)
	break;
      if (oz_eq(entries[i].val,l) && sameSRecordArity(entries[i].sra,sra))
	break;
      i++;
    }
    return entries[i].lbl;
  }

  Bool disentailed(OzVariable * var);
  
};

#endif
