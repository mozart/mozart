/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
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
 *     http://www.mozart-oz.org/
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __PEL_FNCTS_HH__
#define __PEL_FNCTS_HH__

#include "std.hh"
#include "pel_engine.hh"

//-----------------------------------------------------------------------------

// X + C <= Y
class PEL_LessEqOffset : public PEL_Propagator {
protected:
  int _x, _y, _c;
public:
  PEL_LessEqOffset(PEL_PersistentFDIntVar &x, int c, 
		   PEL_PersistentFDIntVar &y) 
    :  _c(-c) {
    _x = x.newId(*_pe);
    _y = y.newId(*_pe);
    CDM(("constr lesseqoff _x=%d _y=%d _c=%d\n", _x, _y, _c));
  }
  //
  void print(PEL_Engine &e) {
    printf("LessEqOffset x(%s,%d) + c(%d) <= ", 
	   (*(PEL_FDIntVar *) e[_x])->toString(), _x, _c);
    printf("y(%s,%d)\n", (*(PEL_FDIntVar *) e[_y])->toString(), _y);
  }
  //
  virtual pf_return_t propagate(PEL_Engine &e);
};

inline 
void make_PEL_LessEqOffset(PEL_PersistentEngine &engine,
			   PEL_PersistentFDIntVar &x, int c,
			   PEL_PersistentFDIntVar &y)
{
  engine.expectIntVarBounds(x);
  engine.expectIntVarBounds(y);
  engine.impose(new PEL_LessEqOffset(x, c, y));
}

// X + C > Y == Y - C - 1 <= X
class PEL_GreaterOffset : public PEL_LessEqOffset {
public:
  PEL_GreaterOffset(PEL_PersistentFDIntVar &x, int c,
		    PEL_PersistentFDIntVar &y) 
    : PEL_LessEqOffset(y, -c+1, x) {}
  //
  void print(PEL_Engine &e) {
    printf("GreaterOffset x(%s,%d) + c(%d) > ", 
	   (*(PEL_FDIntVar *) e[_y])->toString(), _y, -_c+1);
    printf("y(%s,%d)\n", (*(PEL_FDIntVar *) e[_x])->toString(), _x);
  }
};

inline 
void make_PEL_GreaterOffset(PEL_PersistentEngine &engine,
			   PEL_PersistentFDIntVar &x, int c,
			   PEL_PersistentFDIntVar &y)
{
  engine.expectIntVarBounds(x);
  engine.expectIntVarBounds(y);
  engine.impose(new PEL_GreaterOffset(x, c, y));
}

#endif /* __PEL_FNCTS_HH__ */


