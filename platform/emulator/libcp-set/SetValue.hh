/*
 *  Main authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co>
 *
 *  Contributing authors:
 *     
 *
 *  Copyright:
 *     Gustavo Gutierrez, 2006
 *     Alberto Delgado, 2006
 *
 *  Last modified:
 *     $Date$
 *     $Revision$
 *
 *  This file is part of GeOz, a module for integrating gecode 
 *  constraint system to Mozart: 
 *     http://home.gna.org/geoz
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef __GEOZ_SET_VALM_HH__
#define __GEOZ_SET_VALM_HH__

#include "value.hh"
#include "GeSpace-builtins.hh"

#include "gecode/kernel.hh"
#include "gecode/int.hh"
#include "gecode/set.hh"

using namespace Gecode;
using namespace Gecode::Int;



class SetValueM;

class SetValueM: public OZ_Extension {

  const IntSet lbValue;
  const IntSet ubValue;
  const IntSet card;

public:


  static SetValueM* tagged2SetVal(OZ_Term t)
  {
    Assert(OZ_isSetValueM(t));
    return (SetValueM*) OZ_getExtension(OZ_deref(t));
  }
  
  //  SetValueM(OZ_Term x):lbValue(tagged2SetVal(x)->getLBValue()), ubValue(tagged2SetVal(x)->getUBValue()), card(tagged2SetVal(x)->getCard()){}
  
  
  SetValueM(IntSet x , IntSet y, IntSet c) : 
    lbValue(static_cast<const IntSet&>(x)),
    ubValue(static_cast<const IntSet&>(y)),
    card(static_cast<const IntSet&>(c)){}

  
  //
  // Extension 
  //
  //friend int oz_isSetValueM(OZ_Term);
  static int id;

  static int OZ_isSetValueM(OZ_Term t){
    t = OZ_deref(t);
    return OZ_isExtension(t) &&
      OZ_getExtension(t)->getIdV()==SetValueM::id;
  }

  virtual int getIdV() { return id; }
  virtual OZ_Term typeV() { return OZ_atom("SetValueM"); }
  virtual OZ_Extension* gCollectV(void);
  virtual OZ_Extension* sCloneV(void);
  virtual void gCollectRecurseV(void) {}
  virtual void sCloneRecurseV(void) {}
  //void toStream(ostream &out);
  virtual OZ_Term printV(int depth = 10);

  IntSet getLBValue(){return lbValue;}
  IntSet getUBValue(){return ubValue;}
  IntSet getCard(){return card;}
};




//
// Extension 
//



#endif
