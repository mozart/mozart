/*
 *  Main authors:
 *     Alejandro Arbelaez (aarbelaez@cic.puj.edu.co)
 *
 *  Contributing authors:
 *
 *  Copyright:
 *    Alejandro Arbelaez, 2006-2007
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

#ifndef __MOZ_PROP_HH__
#define __MOZ_PROP_HH__

#include "gecode/kernel.hh"
#include "gecode/int.hh"
#include "gecode/set.hh"
#include "gecode/int/linear.hh"
#include "gecode/int/rel.hh"


using namespace Gecode;
using namespace Gecode::Int;

//-------------------------------------------------------------------------------------
class WatchMaxProp: public Gecode::BinaryPropagator<Gecode::Int::IntView, PC_INT_BND>
{
private:
  int x2;
public:
  WatchMaxProp(Gecode::Space *s, Gecode::Int::IntView x0, Gecode::Int::IntView x1,int x2):
    Gecode::BinaryPropagator<Gecode::Int::IntView,PC_INT_BND>(s,x0,x1),x2(x2) {}
  WatchMaxProp(Gecode::Space *s, bool share, WatchMaxProp &p):
    Gecode::BinaryPropagator<Gecode::Int::IntView,PC_INT_BND>(s,share,p),x2(x2) {}

  int getX2() {return x2;}

  Gecode::Actor* copy(Gecode::Space *s, bool share) {
    return new(s) WatchMaxProp(s,share,*this);
  }
  
  Gecode::ExecStatus propagate(Gecode::Space *s, Gecode::ModEventDelta);
};

class WatchMinProp: public WatchMaxProp
{
public:
  WatchMinProp(Gecode::Space *s, Gecode::Int::IntView x0, Gecode::Int::IntView x1, int x2):
    WatchMaxProp(s,x0,x1,x2) {}
  WatchMinProp(Gecode::Space *s, bool share, WatchMinProp &p):
    WatchMaxProp(s,share,p) {}
  Gecode::Actor* copy(Gecode::Space *s,bool share) {
    return new(s) WatchMinProp(s,share,*this);
  }
  Gecode::ExecStatus propagate(Gecode::Space *s, Gecode::ModEventDelta);
};

class WatchSizeProp: public WatchMaxProp
{
public:
  WatchSizeProp(Gecode::Space *s, Gecode::Int::IntView x0, Gecode::Int::IntView x1, int x2):
    WatchMaxProp(s,x0,x1,x2) {}
  WatchSizeProp(Gecode::Space *s, bool share, WatchSizeProp &p):
    WatchMaxProp(s,share,p) {}
  Gecode::Actor* copy(Gecode::Space *s, bool share) {
    return new(s) WatchSizeProp(s,share,*this);
  }
  Gecode::ExecStatus propagate(Gecode::Space *s, Gecode::ModEventDelta);
};

//-------------------------------------------------------------------------------------

class DisjointProp: public Gecode::BinaryPropagator<Gecode::Int::IntView, PC_INT_DOM>
{
private:
  int xd, yd;
public:
  DisjointProp(Gecode::Space *s, Gecode::Int::IntView x0, Gecode::Int::IntView x1, int xd,int yd):
    Gecode::BinaryPropagator<Gecode::Int::IntView, PC_INT_DOM>(s,x0,x1),xd(xd), yd(yd) {}
  DisjointProp(Gecode::Space *s, bool share, DisjointProp &p):
    Gecode::BinaryPropagator<Gecode::Int::IntView, PC_INT_DOM>(s,share,p),xd(xd),yd(yd) {}
  
  Gecode::Actor* copy(Gecode::Space *s, bool share) {
    return new(s) DisjointProp(s,share,*this);
  }

  Gecode::ExecStatus propagate(Gecode::Space *s, Gecode::ModEventDelta);
};


//------------- Reified version of Disjoint propagator

class DisjointCProp: public Gecode::MixTernaryPropagator<Gecode::Int::IntView, PC_INT_DOM, Gecode::Int::IntView, PC_INT_DOM, Gecode::Int::BoolView, 
PC_BOOL_NONE >
{
private:
  int xd, yd;
  
public:
  DisjointCProp(Gecode::Space *s, Gecode::Int::IntView x0, Gecode::Int::IntView x1, Gecode::Int::BoolView x2, int xd, int yd):
    Gecode::MixTernaryPropagator<Gecode::Int::IntView, PC_INT_DOM, Gecode::Int::IntView, PC_INT_DOM, Gecode::Int::BoolView, 
PC_BOOL_NONE >(s,x0,x1,x2), xd(xd), yd(yd) {}
  DisjointCProp(Gecode::Space *s, bool share, DisjointCProp &p):
    Gecode::MixTernaryPropagator<Gecode::Int::IntView, PC_INT_DOM, Gecode::Int::IntView, PC_INT_DOM, Gecode::Int::BoolView, 
PC_BOOL_NONE >(s,share,p), xd(xd), yd(yd) {}

  Gecode::Actor* copy(Gecode::Space *s, bool share) {
    return new(s) DisjointCProp(s,share,*this);
  }
  
  void LessEqY(Gecode::Space *s);
  
  void LessEqX(Gecode::Space *s);
  
  Gecode::ExecStatus propagate(Gecode::Space *s, Gecode::ModEventDelta);
};

//-------------------------------------------------------------------------------------

class ReifiedIntProp: public Gecode::ReUnaryPropagator<Gecode::Int::IntView, PC_INT_DOM, Gecode::Int::BoolView>
{
private:
  //  IntView d2;
  //  const Gecode::IntSet d;
  IntVar d;
public:
  ReifiedIntProp(Gecode::Space *s, Gecode::Int::IntView x0, Gecode::Int::BoolView b, Gecode::IntVar d):
    Gecode::ReUnaryPropagator<Gecode::Int::IntView, PC_INT_DOM, Gecode::Int::BoolView>(s,x0,b), d(d) {}
  ReifiedIntProp(Gecode::Space *s, bool share, ReifiedIntProp &p):
    Gecode::ReUnaryPropagator<Gecode::Int::IntView, PC_INT_DOM, Gecode::Int::BoolView>(s,share,*this), d(d) {}

  Gecode::Actor* copy(Gecode::Space *s, bool share) {
    return new(s) ReifiedIntProp(s, share, *this);
  }

  Gecode::ExecStatus propagate(Gecode::Space *s, Gecode::ModEventDelta);  
};

//-------------------------------------------------------------------------------------

void WatchMin(Gecode::Space *s, IntVar a, IntVar b, int Num);
void WatchMax(Gecode::Space *s, Gecode::IntVar a, Gecode::IntVar b, int Num);
void WatchSize(Gecode::Space *s, IntVar a, IntVar b, int Num);
void Disjoint(Gecode::Space *s, IntVar D1, int I1, IntVar D2, int I2);
void DisjointC(Gecode::Space *s, IntVar D1, int I1, IntVar D2, int I2, BoolVar D3);
void ReifiedInt(Gecode::Space *s, Gecode::IntVar x, Gecode::IntVar b, const Gecode::IntSet d);

#endif
