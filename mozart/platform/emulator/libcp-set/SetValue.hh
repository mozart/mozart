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

/**
 * \brief Container for the values of a set variable.
 * This contains the lowerBound, upperBound and the
 * cardinality of a variable. This is needed to values of
 * the var can be visible in Mozart.
 */

class SetValueM: public OZ_Extension {

  //UpperBound of a variable
  const IntSet lbValue;
  //LowerBound of a variable
  const IntSet ubValue;
  //Cardinality of a variable
  const IntSet card;

public:
    
  /**
     \brief Constructor for SetValueM.
     @param x is an Gecode::IntSet representing the upperBound
     @param y is an Gecode::IntSet representing the lowerBound
     @param c is an Gecode::IntSet representing the cardinality
     TODO: anfelbar@ In Mozart-OZ the cardinality is a finite domain variable.
     should we change this?
   */
  SetValueM(IntSet x , IntSet y, IntSet c) : 
    lbValue(static_cast<const IntSet&>(x)),
    ubValue(static_cast<const IntSet&>(y)),
    card(static_cast<const IntSet&>(c)){}
  

  static SetValueM* tagged2SetVal(OZ_Term t)
  {
    Assert(OZ_isSetValueM(t));
    return (SetValueM*) OZ_getExtension(OZ_deref(t));
  }

  //
  // Extension 
  //
  //friend int oz_isSetValueM(OZ_Term);
  static int id;

  /**
     \brief Tests whether OZ_Term \a t is
     a SetValueM.
   */
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

  //Return Lower Bound of set
  IntSet getLBValue(){return lbValue;}
  //Return Upper Bound of set
  IntSet getUBValue(){return ubValue;}
  //Return Cardinality of set
  IntSet getCard(){return card;}
};




//
// Extension 
//



#endif
