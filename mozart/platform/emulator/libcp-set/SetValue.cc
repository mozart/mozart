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


#include "SetValue.hh"


int SetValueM::id;

//
// Member Functions
//


OZ_Term SetValueM::printV(int n)
{

  //printf("SetValueM printV\n");fflush(stdout);
  IntSet w = getCard();



  IntSet LB = getLBValue();
  OZ_Term tupLB = OZ_tupleC("#",LB.size());
  for(int i=0;i<LB.size();i++)
    OZ_putArg(tupLB,i,OZ_mkTupleC("#",3,OZ_int(LB.min(i)),OZ_atom(".."),OZ_int(LB.max(i))));

  IntSet UB = getUBValue();
  OZ_Term tupUB = OZ_tupleC("#",UB.size());

  for(int i=0;i<UB.size();i++)
    OZ_putArg(tupUB,i,OZ_mkTupleC("#",3,OZ_int(UB.min(i)),OZ_atom(".."),OZ_int(UB.max(i))));

  
  
  
  if(w.width(0)==1){
    return OZ_mkTupleC("#",7,
		       OZ_atom("cardinality: "),
		       OZ_int(w.min(0)),
		       OZ_atom("  LB {"),
		       tupLB,		     
		       OZ_atom("} UB {"),
		       tupUB,
		       OZ_atom("}"));
  }
  else{
    return OZ_mkTupleC("#",9,
		       OZ_atom("cardinality: {"),
		       OZ_int(w.min(0)),
		       OZ_atom(" .. "),
		       OZ_int(w.max(0)),
		       OZ_atom("}  LB {"),
		       tupLB,		     
		       OZ_atom("} UB {"),
		       tupUB,
		       OZ_atom("}"));
  }
}
/*
  void SetValueM::toStream(ostream &out) {
  std::stringstream oss, oss1, oss2;
  oss << getLBValue();
  oss1 << getUBValue();
  oss2 << getCard();
  
  out << "<SetVal " << oss.str().c_str() <<", "<< oss1.str().c_str() <<", "<< oss2.str().c_str() << ">"; 
  }
*/

OZ_Extension* SetValueM::gCollectV(void)
{
  return(new SetValueM(this->getLBValue(),this->getUBValue(), this->getCard()));
}

OZ_Extension* SetValueM::sCloneV(void)
{
  return(new SetValueM(this->getLBValue(),this->getUBValue(), this->getCard()));
}



/*
  OZ_BI_define(setValueM_is,1,1)
  {
  OZ_declareDetTerm(0,t);
  OZ_RETURN((oz_isSetValueM(t))?OZ_true():OZ_false());
  } OZ_BI_end

  OZ_BI_define(setValue_new,1,1)
  {
  OZ_RETURN(OZ_extension(new SetValueM(OZ_in(0))));
  } OZ_BI_end 


  void module_init_geselval(void){ 
  }

  char oz_module_name[] = "SetValueM";

  extern "C" 
  {
  OZ_C_proc_interface * oz_init_module(void) 
  {
  static OZ_C_proc_interface i_table[] = {
  {"is"		,1,1,setValueM_is},
  //{"new"		,1,1,setValueaaa_new},
  {0,0,0,0}
  };
  SetValueM::id = oz_newUniqueId();
  module_init_geselval();
  return i_table;
  }
  
  
  #ifndef STATICALLY_INCLUDED
  OZ_C_proc_interface * oz_init_module(void) {
  return oz_init_module(); }
  #endif
  
  }
*/



