/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __BUILTINSH
#define __BUILTINSH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"
#include "oz.h"
#include "oz_cpi.hh"
#include "gc.hh"

BuiltinTabEntry *BIinit();

void threadRaise(Thread *th,OZ_Term E,int debug=0);

// -----------------------------------------------------------------------
// tables

extern OZ_Return dotInline(TaggedRef term, TaggedRef fea, TaggedRef &out);
extern OZ_Return uparrowInlineBlocking(TaggedRef term, TaggedRef fea,
                                       TaggedRef &out);

OZ_Return BIarityInline(TaggedRef, TaggedRef &);
OZ_Return adjoinPropList(TaggedRef t0, TaggedRef list, TaggedRef &out,
                             Bool recordFlag);

OZ_C_proc_proto(BIatWithState)
OZ_C_proc_proto(BIassignWithState)

// -----------------------------------------------------------------------
// propagators

class WidthPropagator : public OZ_Propagator {
private:
  static OZ_CFunHeader spawner;
protected:
  OZ_Term rawrec, rawwid;
public:
  WidthPropagator(OZ_Term r, OZ_Term w)
    : rawrec(r), rawwid(w) {}

  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_collectHeapTerm(rawrec,rawrec);
    OZ_collectHeapTerm(rawwid,rawwid);
  }
  virtual size_t sizeOf(void) { return sizeof(WidthPropagator); }
  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const {return &spawner; }
  virtual OZ_Term getParameters(void) const { return OZ_nil(); }
};

class MonitorArityPropagator : public OZ_Propagator {
private:
  static OZ_CFunHeader spawner;
protected:
  OZ_Term X, K, L, FH, FT;
public:
  MonitorArityPropagator(OZ_Term X1, OZ_Term K1, OZ_Term L1,
                         OZ_Term FH1, OZ_Term FT1)
    : X(X1), K(K1), L(L1), FH(FH1), FT(FT1) {}

  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_collectHeapTerm(X,X);
    OZ_collectHeapTerm(K,K);
    OZ_collectHeapTerm(L,L);
    OZ_collectHeapTerm(FH,FH);
    OZ_collectHeapTerm(FT,FT);
  }
  virtual size_t sizeOf(void) { return sizeof(MonitorArityPropagator); }
  virtual OZ_Return propagate(void);
  virtual OZ_CFunHeader * getHeader(void) const {return &spawner; }
  virtual OZ_Term getParameters(void) const { return OZ_nil(); }

  TaggedRef getX(void) { return X; }
  TaggedRef getK(void) { return K; }
  TaggedRef getFH(void) { return FH; }
  TaggedRef getFT(void) { return FT; }
  void setFH(TaggedRef FH1) { FH=FH1; }
};


// -----------------------------------------------------------------------
// arithmetics

OZ_Return BIminusOrPlus(Bool callPlus,TaggedRef A, TaggedRef B, TaggedRef &out);
OZ_Return BILessOrLessEq(Bool callLess, TaggedRef A, TaggedRef B);

#endif
