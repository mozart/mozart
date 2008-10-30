/*
 *  Main authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co>
 *
 *  Contributing authors:
 *   Andres Felipe Barco <anfelbar@univalle.edu.co>   
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


#include "SetValue.hh"


int SetValueM::id;

//
// Member Functions
//

/**
   PrintV: Prints the upperBound and the cardinality of this SetValueM.
   param@ n  is the deep of something...
 */
OZ_Term SetValueM::printV(int n)
{

  //Set Cardinality
  IntSet w = getCard();
  //Lower Bound
  IntSet LB = getLBValue();
  //Upper Bound
  IntSet UB = getUBValue();

  OZ_Term tupLB = OZ_tupleC("#",LB.size());
  OZ_Term tupUB = OZ_tupleC("#",UB.size());

  //TODO: anfelbar@ this code block is ugly, can we do it better?
  for(int i=0; i<LB.size(); ){
    int min = LB.min(i);
    int max = LB.max(i);
    if (min == max){
      if(i+1 < LB.size()){
	//printf("still more values in LB, put %d and atom (,) \n", min);fflush(stdout);
	OZ_putArg(tupLB, i, OZ_mkTupleC("#", 2, OZ_int(min), OZ_atom(",")));
      } else OZ_putArg(tupLB, i, OZ_mkTupleC("#", 1, OZ_int(min)));
    } else {
      if(i+1 < LB.size()){
	//printf("2 still more values in LB, put %d atom(..)  %d and atom(,) \n", min, max);fflush(stdout);
	OZ_putArg(tupLB, i, OZ_mkTupleC("#", 4, OZ_int(min), OZ_atom(".."), OZ_int(max), OZ_atom(",")));
      } else  OZ_putArg(tupLB, i, OZ_mkTupleC("#", 3, OZ_int(min), OZ_atom(".."), OZ_int(max)));
    }
    i++;
  }

  /**
     TODO: The upper bound is determined in the same fashion as the lower bound. 
     But is really neccessary get the upperBound?
     Now we only show lowerBound ever!!!
  */
  //for(int i=0;i<UB.size();i++){
    //printf("minUB of %d is %d and maxUB is %d\n", i, UB.min(i), UB.max(i));fflush(stdout);
    //OZ_putArg(tupUB,i,OZ_mkTupleC("#",3,OZ_int(UB.min(i)),OZ_atom(".."),OZ_int(UB.max(i))));
  //}
  
  //if(w.width(0)==1){
    return OZ_mkTupleC("#",5,
		       OZ_atom("{"),
		       tupLB,
		       OZ_atom("}"),
		       OZ_atom("#"), // (anfelbar@) this does not work, don't known why
		       OZ_int(w.min(0)));
}

/**
   Garbage Collection for this SetValueM
 */
OZ_Extension* SetValueM::gCollectV(void)
{
  return(new SetValueM(this->getLBValue(),this->getUBValue(), this->getCard()));
}

/**
   Cloning for this SetValueM
 */
OZ_Extension* SetValueM::sCloneV(void)
{
  return(new SetValueM(this->getLBValue(),this->getUBValue(), this->getCard()));
}

