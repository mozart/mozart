/*
 *  Authors:
 *    Kostja Popov <kost@sics.se>
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Konstantin Popov, 2001
 * 
 *  Last change:
 *    $Date$
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

#ifndef __MARSHALERDICT_H
#define __MARSHALERDICT_H

#if defined(INTERFACE)
#pragma interface
#endif

#include "base.hh"
#include "tagged.hh"

//
// A node of a MarshalerDict contains:
// . a unique node, which is either
//   . an Oz valriable: a ref to a variable OZ_Term, or
//   . an Oz value: a dereferenced OZ_Term;
// . an index, which is:
//   . 0 when not assigned
//   . a *signed* integer otherwise. 
//
// In our three-step marshaling, a node:
// . in the first "marshaling" phase is recorded and assigned
//   an index if not seen before;
// . in the "scanning" phase:
//   . is recorded without an assigned index when seen 
//     for the first time;
//   . is assigned an index when found for the first time
//     (the index recorded is negative);
// . in the second "marshaling" phase:
//   . is marshaled with a DEF if a negative index is discovered.
//     The index is then negated in the table;
//   . a REF is marshaled if a positive index is discovered;
//   . if a node is not found, or it is unassigned, then nothing
//     special is done;
//
// 'MarshalerDict' is a clone (specialization) of
// 'AddressHashTableO1Reset';
//

//
class MarshalerDict_Node {
  friend class MarshalerDict;
private:
  OZ_Term node;
  int index;
  //
  unsigned int cnt;		// allocation phase for which it's busy;

  //
private:
  void reset() { cnt = 0; }
  MarshalerDict_Node() {
    DebugCode(node = (OZ_Term) -1;);
    DebugCode(index = 0;);
    reset();
  }

  //
  void set(OZ_Term nIn, unsigned int cntIn) {
    Assert(cnt < cntIn);
    node = nIn;
    index = 0;
    cnt = cntIn;
  }
  void setInd(OZ_Term nIn, int ind, unsigned int cntIn) {
    Assert(cnt < cntIn);
    node = nIn;
    index = ind;
    cnt = cntIn;
  }

  //
  unsigned int getCnt() { return (cnt); }
  int getAnyIndex() { return (index); }

  //
public:
  Bool firstTime() { return (index == 0); }

  //
  OZ_Term getNode() { 
    Assert(node != (OZ_Term) -1);
    return (node);
  }
  int getIndex() {
    Assert(index != 0);
    return (index);
  }

  //
  void setIndex(int iIn) {
    Assert(index == 0);
    Assert(iIn != 0);
    index = iIn;
  }
  void resetIndex(int iIn) {
    Assert(index != 0);
    Assert(iIn != 0);
    index = iIn;
  }
};

//
class MarshalerDict {
private:
  MarshalerDict_Node *table;
  int tableSize;		// 
  int counter;			// number of entries;
  int percent;			// reallocate when percent > counter;
  //
  int bits;			// ipkey & skey;
  int rsBits;			// right shifht;
  int slsBits;			// skey left shift;
  unsigned int pass;		// current pass;
  int lastIndex;		// findTerm()==0 .. rememberTerm()
  DebugCode(intlong lastKey;);	// consistency check for lastIndex;
  DebugCode(int nsearch;);	// number of searches since last 'mkEmpty()';
  DebugCode(int tries;);	// accumulated over 'nsearch';
  DebugCode(int maxtries;);
  DebugCode(int nsearchAcc;);	// .. since object construction;
  DebugCode(int triesAcc;);	// accumulated over 'nsearchAcc';

  // a node index is assigned once it is found for the first time;
  int index;

  //
private:
  unsigned int primeHashFunc(OZ_Term);
  unsigned int incHashFunc(OZ_Term);

  //
  void mkTable();
  void resize();
  //
  int getSize() { return (counter); }

  // The "full" version of 'rememberNode()', without the constraint on
  // when it can be used;
  void recordNode(OZ_Term v, int ind);

  //
public:
  MarshalerDict(int sz);
  ~MarshalerDict();
  //
  void mkEmpty();
  void gCollect();

  // Sequentially allocated indexes, starting from 1:
  int getNextIndex() { return (index++); }

  // if not found it returns 0 and prepares for 'rememberNode()';
  MarshalerDict_Node* findNode(OZ_Term n);
  // 'rememberNode()' must follow an unsuccessful 'findTerm()'.
  // An index is not assigned;
  MarshalerDict_Node* rememberNode(OZ_Term n) {
    Assert(lastIndex != -1);
    Assert(n == lastKey);
    MarshalerDict_Node *mdn = &table[lastIndex];
    Assert(mdn->getCnt() < pass);	// not there;

    //
    if (counter > percent) {
      resize();
      // Prepare for recording the node in the *new* table:
      MarshalerDict_Node *zeroMDN = findNode(n);
      Assert(zeroMDN == (MarshalerDict_Node *) 0);
      mdn = &table[lastIndex];
    }

    //
    mdn->set(n, pass);
    DebugCode(lastIndex = -1;);
    DebugCode(lastKey = (OZ_Term) -1;);
    counter++;
    return (mdn);
  }

  // as 'findNode()' but does not expect a subsequent 'rememverNode()';
  MarshalerDict_Node* locateNode(OZ_Term n);

  //
  MarshalerDict_Node *getNext(MarshalerDict_Node *hn) {
    for (hn--; hn >= table; hn--) {
      if (hn->getCnt() == pass)
  	return (hn);
    }
    return ((MarshalerDict_Node *) 0);
  }
  MarshalerDict_Node *getFirst() { return (getNext(table+tableSize)); }

  //
  DebugCode(void print(););
  DebugCode(void printStatistics(int th = 0););
};


//
// The first phase marshaling ("M1stP") records all previously unseen
// nodes, and assigns indexes to all of them:
#define VisitNodeM1stP(n, md, mb, index, Return)			\
{									\
  MarshalerDict_Node *mdn = md->findNode(n);				\
  if (mdn) {								\
    /* all the nodes that were seen have a DEF; */			\
    Assert(!mdn->firstTime());						\
    marshalDIF(mb, DIF_REF);						\
    marshalTermRef(mb, mdn->getIndex());				\
    Return;								\
  } else { 								\
    /* seen for the first time: record it & assign an index; */		\
    index = md->getNextIndex();						\
    mdn = md->rememberNode(n);						\
    mdn->setIndex(index);						\
    /* .. and proceed with marshaling; */				\
  }									\
}

//
// Traverser also records all previously unseen nodes, but assigns
// indexes only to those nodes that were seen before but still do not
// have one. Note that indexes are entered negated: when such a
// negated index is seen, a DEF is put and the index is negated again;
#define VisitNodeTrav(n, md, Return)					\
{									\
  MarshalerDict_Node *mdn = md->findNode(n);				\
  if (mdn) {								\
    /* check whether we still have to assign an index; */		\
    if (mdn->firstTime()) {						\
      int index = md->getNextIndex();					\
      mdn->setIndex(-index);						\
    }									\
    Return;								\
  } else { 								\
    /* seen for the first time: just record it in the table; */		\
    (void) md->rememberNode(n);						\
    /* proceed with marshaling; */					\
  }									\
}

//
// During marshaling-after-traversing, or the second phase marshaling
// ("M2ndP") it is known which nodes need a DEF.
//
// A marshaler dictionary can (should?) be reduced (and actually is
// reduced during GC) between phases: only the to-be- and already
// DEFed nodes remain in the table. The current system does not
// perform that reduction eagerly (that is, when traversing is
// finished), so there can still be "firstTime()" nodes;
#define VisitNodeM2ndP(n, md, mb, index, Return)			\
{									\
  MarshalerDict_Node *mdn = md->locateNode(n);				\
  if (mdn == (MarshalerDict_Node *) 0 || mdn->firstTime()) {		\
    /* was never seen again in phase 1; */				\
    index = 0;								\
  } else {								\
    index = mdn->getIndex();						\
    if (index > 0) {							\
      /* have already put a DEF - put a REF and bail out; */		\
      marshalDIF(mb, DIF_REF);						\
      marshalTermRef(mb, index);					\
      Return;								\
    } else {								\
      /* not only put a DEF, but also prepare for future REFs; */	\
      index = -index;							\
      mdn->resetIndex(index);						\
    }									\
  }									\
}

#endif
