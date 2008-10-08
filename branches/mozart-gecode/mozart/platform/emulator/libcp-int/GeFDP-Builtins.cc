/*
 *  Main authors:
 *     Diana Lorena Velasco <dlvelasco@puj.edu.co>
 *     Juan Gabriel Torres  <juantorres@puj.edu.co>
 *     Andres Felipe Barco <anfelbar@univalle.edu.co>
 *     Gustavo A. Gomez Farhat <gafarhat@univalle.edu.co>
 *
 *  Contributing authors:
 *		Victor Rivera Zuniga <varivera@javerianacali.edu.co>
 *
 *  Copyright:
 *     Diana Lorena Velasco, 2007
 *     Juan Gabriel Torres, 2007
 *     Andres Felipe Barco, 2008
 *     Gustavo A. Gomez, 2008
 *
 *  Last modified:
 *     $Date$
 *     $Revision$
 *
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 *
 *  See the file 'LICENSE' for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */
 

#ifndef __GEOZ_INTVAR_PROP_BUILTINS_CC__
#define __GEOZ_INTVAR_PROP_BUILTINS_CC__

#include "IntVarMacros.hh"


/**
   Macro for suspend the propagator posting if some his parametres (i.e. constraint variables, domains, integers, etc)
   are references or ...
   This not work like we want. The thread is acctually suspended, but never is waken up again.
*/
// #define SuspendPosting(Param)			
//   {						
//     printf("here it is...\n");fflush(stdout);	
//     TaggedRef tt = (TaggedRef) (Param);		
//     DEREF(tt, t_ptr);				
//     Assert(!oz_isRef(tt));			
//     if(oz_isVarOrRef(tt)){			
//       oz_suspendOn(makeTaggedRef(t_ptr));	
//     }						
//   }

/**
   Domain constraints:
   Propagators to post a domain constraint on variables. This is a Gecode propagators interface. 
   For more information see http://www.gecode.org/gecode-doc-latest/group__TaskModelIntDomain.html
*/
OZ_BI_define(gfd_dom_5,5,0){
  DeclareGSpace(home);

  Gecode::IntConLevel icl = getIntConLevel(OZ_in(3));
  Gecode::PropKind pk = getPropKind(OZ_in(4));

  if( OZ_isGeIntVar(OZ_in(0)) && OZ_isInt(OZ_in(1)) && OZ_isInt(OZ_in(2)) ){
    /**
       dom(Space* home, IntVar x, int l, int m, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVar &iv = intOrIntVar(OZ_in(0));
    int i = OZ_intToC(OZ_in(1));
    int j = OZ_intToC(OZ_in(2));
    
    try{
      Gecode::dom(home, iv, i, j, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isInt(OZ_in(1)) && OZ_isInt(OZ_in(2)) ){
    /** 
	dom(Space* home, const IntVarArgs& x, int l, int m, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVarArgs iva = getIntVarArgs(OZ_in(0));
    int i = OZ_intToC(OZ_in(1));
    int j = OZ_intToC(OZ_in(2));

    try{
      Gecode::dom(home, iva, i, j, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeIntVar(OZ_in(0)) && OZ_isIntSet(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2)) ){
    /**
       dom(Space* home, IntVar x, const IntSet& s, BoolVar b, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVar &iv = intOrIntVar(OZ_in(0));
    Gecode::IntSet is = getIntSet(OZ_in(1));
    DeclareGeBoolVar(2, __b, home);
    
    try{
      //Gecode::dom(home, __x, __s, __b, __ICL_DEF, __PK_DEF);
      Gecode::dom(home, iv, is, __b, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end


OZ_BI_define(gfd_dom_4,4,0){
  DeclareGSpace(home);

  Gecode::IntConLevel icl = getIntConLevel(OZ_in(2));
  Gecode::PropKind pk = getPropKind(OZ_in(3));  

  if(OZ_isGeIntVar(OZ_in(0)) && OZ_isIntSet(OZ_in(1)) ){
    /**
       dom(Space* home, IntVar x, const IntSet& s, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVar &iv = intOrIntVar(OZ_in(0));
    Gecode::IntSet is = getIntSet(OZ_in(1));
    
    try{
      Gecode::dom(home, iv, is, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntSet(OZ_in(1)) ){
    /**
       dom(Space* home, const IntVarArgs& x, const IntSet& s, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVarArgs iva = getIntVarArgs(OZ_in(0));
    Gecode::IntSet is = getIntSet(OZ_in(1));
    
    try{
      Gecode::dom(home, iva, is, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end



OZ_BI_define(gfd_dom_6,6,0){
  DeclareGSpace(home);

  //DeclareGeIntVar(0, __x, home);
  //DeclareInt2(1, __l);
  //DeclareInt2(2, __m);
  
  //DeclareIntConLevel(4, __ICL_DEF);
  //DeclarePropKind(5, __PK_DEF);

  if(OZ_isGeIntVar(OZ_in(0)) && OZ_isInt(OZ_in(1)) &&  OZ_isInt(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3)) && 
     OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){

    /**
       dom(Space* home, IntVar x, int l, int m, BoolVar b, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVar &iv = intOrIntVar(OZ_in(0));
    int i = OZ_intToC(OZ_in(1));
    int j = OZ_intToC(OZ_in(2));
    DeclareGeBoolVar(3, __b, home);
    Gecode::IntConLevel icl = getIntConLevel(OZ_in(4));
    Gecode::PropKind pk = getPropKind(OZ_in(5));  

    try{
      //Gecode::dom(home, __x, __l, __m, __b, __ICL_DEF, __PK_DEF);
      Gecode::dom(home, iv, i, j, __b, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  } else {
    return OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
 }OZ_BI_end



 /**
    Simple relation constraints over integer variables
    Propagators to post a relation constraint on variables. This is a Gecode propagators interface. 
    For more information see http://www.gecode.org/gecode-doc-latest/group__TaskModelIntRelInt.html
 */
OZ_BI_define(gfd_rel_5,5,0){
  DeclareGSpace(home);

  Gecode::IntRelType irt = getIntRelType(OZ_in(1));
  Gecode::IntConLevel icl = getIntConLevel(OZ_in(3));
  Gecode::PropKind pk = getPropKind(OZ_in(4));  

   if(OZ_isGeIntVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) ){
    /**
       rel(Space* home, IntVar x, IntRelType r, int c, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
     Gecode::IntVar &iv = intOrIntVar(OZ_in(0));
     int i = OZ_intToC(OZ_in(2));

    try{
      Gecode::rel(home, iv, irt, i, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2))){
    /**
       rel(Space* home, const IntVarArgs& x, IntRelType r, IntVar y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVarArgs iva = getIntVarArgs(OZ_in(0));
    Gecode::IntVar &iv = intOrIntVar(OZ_in(2));
    
    try{
      Gecode::rel(home, iva, irt, iv, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeIntVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) ){
    /**
       rel(Space* home, IntVar x0, IntRelType r, IntVar x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */    
    Gecode::IntVar &iv1 = intOrIntVar(OZ_in(0));
    Gecode::IntVar &iv2 = intOrIntVar(OZ_in(2));
    
    try{
      Gecode::rel(home, iv1, irt, iv2, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2))){
    /**
       rel(Space* home, const IntVarArgs& x, IntRelType r, int c, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVarArgs iva = getIntVarArgs(OZ_in(0));
    int i = OZ_intToC(OZ_in(2));

    try{
      Gecode::rel(home, iva, irt, i, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isIntVarArgs(OZ_in(2))){
    /**
       rel(Space* home, const IntVarArgs& x, IntRelType r, const IntVarArgs& y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVarArgs iva1 = getIntVarArgs(OZ_in(0));
    Gecode::IntVarArgs iva2 = getIntVarArgs(OZ_in(2));

    try{
      Gecode::rel(home, iva1, irt, iva2, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end


OZ_BI_define(gfd_rel_6,6,0){
  DeclareGSpace(home);
  DeclareGeBoolVar(3, __b, home);

  Gecode::IntVar &iv1 = intOrIntVar(OZ_in(0));
  Gecode::IntRelType irt = getIntRelType(OZ_in(1));
  Gecode::IntConLevel icl = getIntConLevel(OZ_in(4));
  Gecode::PropKind pk = getPropKind(OZ_in(5));  
  
  if(OZ_isGeIntVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3))){
    /**
       rel(Space* home, IntVar x0, IntRelType r, IntVar x1, BoolVar b, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVar &iv2 = intOrIntVar(OZ_in(2));
    
    try{
      Gecode::rel(home, iv1, irt, iv2, __b, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeIntVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3))){
    /**
       rel(Space* home, IntVar x, IntRelType r, int c, BoolVar b, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    int i = OZ_intToC(OZ_in(2));
    
    try{
      
      Gecode::rel(home, iv1, irt, i, __b, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end


OZ_BI_define(gfd_rel_4,4,0){
  DeclareGSpace(home);

  Gecode::IntRelType irt = getIntRelType(OZ_in(1));
  Gecode::IntConLevel icl = getIntConLevel(OZ_in(2));
  Gecode::PropKind pk = getPropKind(OZ_in(3));  

  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) ){
    /**
       rel(Space* home, const IntVarArgs& x, IntRelType r, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVarArgs iva = getIntVarArgs(OZ_in(0));
    
    try{
      Gecode::rel(home, iva, irt, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end

 
 /**
    Element constraints
    Propagators to post a element constraint on variables. This is a Gecode propagators interface. 
    For more information see http://www.gecode.org/gecode-doc-latest/group__TaskModelIntElement.html
 */
OZ_BI_define(gfd_element_5,5,0){
	DeclareGSpace(home);

  Gecode::IntVar &iv1 = intOrIntVar(OZ_in(1));
  Gecode::IntConLevel icl = getIntConLevel(OZ_in(3));
  Gecode::PropKind pk = getPropKind(OZ_in(4));  

  if(OZ_isIntArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) ){
		/**
       element(Space* home, const IntArgs& n, IntVar x0, IntVar x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntArgs iar = getIntArgs(OZ_in(0));
    Gecode::IntVar &iv2 = intOrIntVar(OZ_in(2));
    
    try{
      Gecode::element(home, iar, iv1, iv2, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2))){
		/**
       element(Space* home, const IntArgs& n, IntVar x0, BoolVar x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntArgs iar = getIntArgs(OZ_in(0));
    DeclareGeBoolVar(2, __x2, home);
    
    try{
      Gecode::element(home, iar, iv1, __x2, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isInt(OZ_in(2)) ){
		/**
       element(Space* home, const IntArgs& n, IntVar x0, int x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntArgs iar = getIntArgs(OZ_in(0));
    int i = OZ_intToC(OZ_in(2));

    try{
      Gecode::element(home, iar, iv1, i, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) ){
		/**
       element(Space* home, const IntVarArgs& x, IntVar y0, IntVar y1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVarArgs iva = getIntVarArgs(OZ_in(0));
    Gecode::IntVar &iv2 = intOrIntVar(OZ_in(2));

    try{
      Gecode::element(home, iva, iv1, iv2, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isInt(OZ_in(2)) ){
	  /**
       element(Space* home, const IntVarArgs& x, IntVar y0, int y1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVarArgs iva = getIntVarArgs(OZ_in(0));
    int i = OZ_intToC(OZ_in(2));
    
    try{
      Gecode::element(home, iva, iv1, i, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2))){
		/**
       element(Space* home, const BoolVarArgs& x, IntVar y0, BoolVar y1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    DECLARE_BOOLVARARGS(0, __x, home);
    DeclareGeBoolVar(2, __x2, home);
    try{
      //Gecode::element(home, __x, __x1, __x2, __ICL_DEF, __PK_DEF);
      Gecode::element(home, __x, iv1, __x2, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isInt(OZ_in(2))){
	  /**
       element(Space* home, const BoolVarArgs& x, IntVar y0, int y1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    DECLARE_BOOLVARARGS(0, __x, home);
    int i = OZ_intToC(OZ_in(2));

    try{
      //Gecode::element(home, __x, __x1, __x2, __ICL_DEF, __PK_DEF);
      Gecode::element(home, __x, iv1, i, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end



 /**
    Distinct constraints
    Propagators to post a distinct constraint on variables. This is a Gecode propagators interface. 
    For more information see http://www.gecode.org/gecode-doc-latest/group__TaskModelIntDistinct.html
 */
OZ_BI_define(gfd_distinct_3,3,0){
  DeclareGSpace(home);

  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntConLevel(OZ_in(1)) && OZ_isPropKind(OZ_in(2))){
    /**
       distinct(Space* home, const IntVarArgs& x, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVarArgs iva = getIntVarArgs(OZ_in(0));
    Gecode::IntConLevel icl = getIntConLevel(OZ_in(1));
    Gecode::PropKind pk = getPropKind(OZ_in(2));  
    
    try{
      Gecode::distinct(home, iva, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  } else {
    return OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end


OZ_BI_define(gfd_distinct_4,4,0){
  DeclareGSpace(home);
  
  if( OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntConLevel(OZ_in(2)) && OZ_isPropKind(OZ_in(3))){
    
    /**
       distinct(Space* home, const IntArgs& n, const IntVarArgs& x, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntArgs iar = getIntArgs(OZ_in(0));
    Gecode::IntVarArgs iva = getIntVarArgs(OZ_in(1));
    Gecode::IntConLevel icl = getIntConLevel(OZ_in(2));
    Gecode::PropKind pk = getPropKind(OZ_in(3));  
    
    try{
      Gecode::distinct(home, iar, iva, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  } else {
    return OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end


 /**
    Channel constraints
    Propagators to post a channel constraint on variables. This is a Gecode propagators interface. 
    For more information see http://www.gecode.org/gecode-doc-latest/group__TaskModelIntChannel.html
 */
OZ_BI_define(gfd_channel_4,4,0){
  DeclareGSpace(home);

  Gecode::IntConLevel icl = getIntConLevel(OZ_in(2));
  Gecode::PropKind pk = getPropKind(OZ_in(3));  


  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1))){
    /**
       channel(Space* home, const IntVarArgs& x, const IntVarArgs& y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVarArgs iva1 = getIntVarArgs(OZ_in(0));
    Gecode::IntVarArgs iva2 = getIntVarArgs(OZ_in(1));
    try{
      Gecode::channel(home, iva1, iva2, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeBoolVar(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) ){
    /**
       channel(Space* home, BoolVar x0, IntVar x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    DeclareGeBoolVar(0, __x0, home);
    Gecode::IntVar &iv = intOrIntVar(OZ_in(1));
    
    try{
      //Gecode::channel(home, __x0, __x1, __ICL_DEF, __PK_DEF);
      Gecode::channel(home, __x0, iv, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeIntVar(OZ_in(0)) && OZ_isGeBoolVar(OZ_in(1)) ) {
    /**
       channel(Space* home, IntVar x0, BoolVar x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    */
    Gecode::IntVar &iv = intOrIntVar(OZ_in(0));
    DeclareGeBoolVar(1, __x1, home);
    
    try{
      //Gecode::channel(home, __x0, __x1, __ICL_DEF, __PK_DEF);
      Gecode::channel(home, iv, __x1, icl, pk);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end


OZ_BI_define(gfd_channel_5,5,0){
  DeclareGSpace(home);

  DECLARE_BOOLVARARGS(0, __x, home);

  if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
    /**
       channel(Space* home, const BoolVarArgs& x, IntVar y, int o=0, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVar &iv = intOrIntVar(OZ_in(1));
    int i = OZ_intToC(OZ_in(2));
    Gecode::IntConLevel icl = getIntConLevel(OZ_in(3));
    Gecode::PropKind pk = getPropKind(OZ_in(4));

    try{
      //Gecode::channel(home, __x, __y, __o, __ICL_DEF, __PK_DEF);
      Gecode::channel(home, __x, iv, i, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  } else {
    return OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end

 /**
    Graph constraints
    Propagators to post a graph constraint on variables. This is a Gecode propagators interface. 
    For more information see http://www.gecode.org/gecode-doc-latest/group__TaskModelIntGraph.html
 */
OZ_BI_define(gfd_circuit_3,3,0){
  DeclareGSpace(home);

  if(OZ_isIntVarArgs(OZ_in(0)) &&  OZ_isIntConLevel(OZ_in(1)) && OZ_isPropKind(OZ_in(2))){
    /**
       circuit(Space* home, const IntVarArgs& x, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVarArgs iva = getIntVarArgs(OZ_in(0));
    Gecode::IntConLevel icl = getIntConLevel(OZ_in(1));
    Gecode::PropKind pk = getPropKind(OZ_in(2));  
    
    try{
      Gecode::circuit(home, iva, icl, pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  } else {
    return OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end


 /**
    Scheduling constraints
    Propagators to post a scheduling constraint on variables. This is a Gecode propagators interface. 
    For more information see http://www.gecode.org/gecode-doc-latest/group__TaskModelIntScheduling.html
 */
OZ_BI_define(gfd_cumulatives_9,9,0){
  DeclareGSpace(home);

  DeclareBool(6, __at_most);

  Gecode::IntVarArgs __start = getIntVarArgs(OZ_in(1));
  Gecode::IntVarArgs __end = getIntVarArgs(OZ_in(3));
  Gecode::IntArgs __limit = getIntArgs(OZ_in(5));
  Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(7));
  Gecode::PropKind __PK_DEF = getPropKind(OZ_in(8));  

  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntVarArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntVarArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6)) ){
    /*   cumulatives(Space* home, const IntVarArgs& machine, const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height, const IntArgs& limit, bool at_most, 
              IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVarArgs __machine = getIntVarArgs(OZ_in(0));
    Gecode::IntVarArgs __duration = getIntVarArgs(OZ_in(2));
    Gecode::IntVarArgs __height = getIntVarArgs(OZ_in(4));

    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntVarArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntVarArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6)) ){
    /*
      cumulatives(Space* home, const IntArgs& machine, const IntVarArgs& start, const IntVarArgs& duration,
      const IntVarArgs& end, const IntVarArgs& height, const IntArgs& limit, bool at_most, 
      IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntArgs __machine = getIntArgs(OZ_in(0));
    Gecode::IntVarArgs __duration = getIntVarArgs(OZ_in(2));
    Gecode::IntVarArgs __height = getIntVarArgs(OZ_in(4));

    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntVarArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6))){
    /*
      cumulatives(Space* home, const IntVarArgs& machine,  const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,  const IntArgs& limit, bool at_most, 
              IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVarArgs __machine = getIntVarArgs(OZ_in(0));
    Gecode::IntArgs __duration = getIntArgs(OZ_in(2));
    Gecode::IntVarArgs __height = getIntVarArgs(OZ_in(4));

    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntVarArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6)) ) {
    /*
        cumulatives(Space* home, const IntArgs& machine,  const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height, const IntArgs& limit, bool at_most, 
              IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */
    Gecode::IntArgs __machine = getIntArgs(OZ_in(0));
    Gecode::IntArgs __duration = getIntArgs(OZ_in(2));
    Gecode::IntVarArgs __height = getIntVarArgs(OZ_in(4));
    
    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntVarArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6)) ){
    /*
      cumulatives(Space* home, const IntVarArgs& machine,  const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntArgs& height,  const IntArgs& limit, bool at_most, 
              IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */
    Gecode::IntVarArgs __machine = getIntVarArgs(OZ_in(0));
    Gecode::IntVarArgs __duration = getIntVarArgs(OZ_in(2));
    Gecode::IntArgs __height = getIntArgs(OZ_in(4));

    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntVarArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6))){
    /*
        cumulatives(Space* home, const IntArgs& machine, const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntArgs& height, const IntArgs& limit, bool at_most, 
              IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */
    Gecode::IntArgs __machine = getIntArgs(OZ_in(0));
    Gecode::IntVarArgs __duration = getIntVarArgs(OZ_in(2));
    Gecode::IntArgs __height = getIntArgs(OZ_in(4));
    
    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6))){
    /*
        cumulatives(Space* home, const IntVarArgs& machine,  const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntArgs& height,  const IntArgs& limit, bool at_most, 
              IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */
    Gecode::IntVarArgs __machine = getIntVarArgs(OZ_in(0));
    Gecode::IntArgs __duration = getIntArgs(OZ_in(2));
    Gecode::IntArgs __height = getIntArgs(OZ_in(4));

    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6))){
    /*
        cumulatives(Space* home, const IntArgs& machine, const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntArgs& height,  const IntArgs& limit, bool at_most, 
              IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */
    Gecode::IntArgs __machine = getIntArgs(OZ_in(0));
    Gecode::IntArgs __duration = getIntArgs(OZ_in(2));
    Gecode::IntArgs __height = getIntArgs(OZ_in(4));
    
    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }

  CHECK_POST(home);
}OZ_BI_end


 /**
    Sorted constraints
    Propagators to post a sorted constraint on variables. This is a Gecode propagators interface. 
    For more information see http://www.gecode.org/gecode-doc-latest/group__TaskModelIntSorted.html
 */

OZ_BI_define(gfd_sorted_4,4,0){
  DeclareGSpace(home);

  Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(2));
  Gecode::PropKind __PK_DEF = getPropKind(OZ_in(3));  
  

  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1))){
    /*
      void Gecode::sorted (Space *home, const IntVarArgs &x, 
      const IntVarArgs &y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
      Post propagator that y is x sorted in increasing order. 
    */
    Gecode::IntVarArgs iva1 = getIntVarArgs(OZ_in(0));
    Gecode::IntVarArgs iva2 = getIntVarArgs(OZ_in(1));
    
    try{
      Gecode::sorted(home, iva1, iva2, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }

  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfd_sorted_5,5,0){
  DeclareGSpace(home);
  
  Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(3));
  Gecode::PropKind __PK_DEF = getPropKind(OZ_in(4));  

  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) &&  OZ_isIntVarArgs(OZ_in(2))){
    /*
      void Gecode::sorted (Space *, const IntVarArgs &x, const IntVarArgs &y, 
      const IntVarArgs &z, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
      Post propagator that y is x sorted in increasing order. 
    */
    Gecode::IntVarArgs iva1 = getIntVarArgs(OZ_in(0));
    Gecode::IntVarArgs iva2 = getIntVarArgs(OZ_in(1));
    Gecode::IntVarArgs iva3 = getIntVarArgs(OZ_in(2));

    try{
      Gecode::sorted(home, iva1, iva2, iva3, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }

  CHECK_POST(home);
}OZ_BI_end

 /**
    Cardinality constraints
    Propagators to post a cardinality constraint on variables. This is a Gecode propagators interface. 
    For more information see http://www.gecode.org/gecode-doc-latest/group__TaskModelIntCard.html
 */
OZ_BI_define(gfd_count_6,6,0){
  DeclareGSpace(home);
  
  Gecode::IntVarArgs __x = getIntVarArgs(OZ_in(0));
  Gecode::IntRelType irt = getIntRelType(OZ_in(2));
  Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(4));
  Gecode::PropKind __PK_DEF = getPropKind(OZ_in(5));  
  

  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isInt(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
    /**
       count(Space *home, const IntVarArgs &x, int n, IntRelType r, int m, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */
    int i = OZ_intToC(OZ_in(1));
    int j = OZ_intToC(OZ_in(3));
    
    try{
      Gecode::count(home, __x, i, irt, j, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
    /**
       count (Space *home, const IntVarArgs &x, IntVar y, IntRelType r, int m, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */
    Gecode::IntVar &iv = intOrIntVar(OZ_in(1));
    int j = OZ_intToC(OZ_in(3));
    
    try{
      Gecode::count(home, __x, iv, irt, j, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
    /**
       count (Space *home, const IntVarArgs &x, const IntArgs &y, IntRelType r, int m, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
     */
    Gecode::IntArgs iar = getIntArgs(OZ_in(1));
    int j = OZ_intToC(OZ_in(3));

    try{
      Gecode::count(home, __x, iar, irt, j, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isInt(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
    /**
       count (Space *home, const IntVarArgs &x, int n, IntRelType r, IntVar z, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
     */
    Gecode::IntVar &iv = intOrIntVar(OZ_in(3));
    int i = OZ_intToC(OZ_in(1));

    try{
      Gecode::count(home, __x, i, irt, iv, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
    /**
       count (Space *home, const IntVarArgs &x, IntVar y, IntRelType r, IntVar z, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
     */
    Gecode::IntVar &iv1 = intOrIntVar(OZ_in(1));
    Gecode::IntVar &iv2 = intOrIntVar(OZ_in(3));

    try{
      Gecode::count(home, __x, iv1, irt, iv2, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
    /**
       count (Space *home, const IntVarArgs &x, const IntArgs &y, IntRelType r, IntVar z, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
     */
    Gecode::IntArgs iar = getIntArgs(OZ_in(1));
    Gecode::IntVar &iv = intOrIntVar(OZ_in(3));
    
    try{
      Gecode::count(home, __x, iar, irt, iv, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }

  CHECK_POST(home);
}OZ_BI_end


OZ_BI_define(gfd_count_4,4,0){
  DeclareGSpace(home);
  
  Gecode::IntVarArgs __x = getIntVarArgs(OZ_in(0));
  Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(2));
  Gecode::PropKind __PK_DEF = getPropKind(OZ_in(3));  

  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntConLevel(OZ_in(2)) && OZ_isPropKind(OZ_in(3))){
    /**
       count(Space *home, const IntVarArgs &x, const IntVarArgs &c, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
     */
    Gecode::IntVarArgs __c = getIntVarArgs(OZ_in(1));
    try{
      Gecode::count(home, __x, __c, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntSetArgs(OZ_in(1)) && OZ_isIntConLevel(OZ_in(2)) && OZ_isPropKind(OZ_in(3))){
    /**
       count (Space *home, const IntVarArgs &x, const IntSetArgs &c, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
     */
    //DECLARE_INT_SET_ARGS(1, __c);
    Gecode::IntSetArgs __c = getIntSetArgs(OZ_in(1));
    
    try{
      Gecode::count(home, __x, __c, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }

  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfd_count_5,5,0){
  DeclareGSpace(home);
  
  Gecode::IntVarArgs __x = getIntVarArgs(OZ_in(0));
  Gecode::IntArgs __v = getIntArgs(OZ_in(2));
  Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(3));
  Gecode::PropKind __PK_DEF = getPropKind(OZ_in(4));  

  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntArgs(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
    /**
       count (Space *home, const IntVarArgs &x, const IntVarArgs &c, const IntArgs &v, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
     */
    Gecode::IntVarArgs __c = getIntVarArgs(OZ_in(0));
    
    try{
      Gecode::count(home, __x, __c, __v, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntSetArgs(OZ_in(1)) && OZ_isIntArgs(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
    /**
       count (Space *home, const IntVarArgs &x, const IntSetArgs &c, const IntArgs &v, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
     */
    //DECLARE_INT_SET_ARGS(1, __c);
    Gecode::IntSetArgs __c = getIntSetArgs(OZ_in(1));
    
    try{
      Gecode::count(home, __x, __c, __v, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntSet(OZ_in(1)) && OZ_isIntArgs(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
    /**
       count (Space *home, const IntVarArgs &x, const IntSet &c, const IntArgs &v, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
     */
    Gecode::IntSet is = getIntSet(OZ_in(1));

    try{
      //Gecode::count(home, __x, __c, __v, __ICL_DEF, __PK_DEF);
      Gecode::count(home, __x, is, __v, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }

  CHECK_POST(home);
}OZ_BI_end


 /**
    Extensional constraints
    Propagators to post a extensional constraint on variables. This is a Gecode propagators interface. 
    For more information see http://www.gecode.org/gecode-doc-latest/group__TaskModelIntExt.html
 */
OZ_BI_define(gfd_extensional_4,4,0){
  DeclareGSpace(home);

  Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(2));
  Gecode::PropKind __PK_DEF = getPropKind(OZ_in(3));  

  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isDFA(OZ_in(1))){
    /*
      void Gecode::extensional (Space *home, const IntVarArgs &x, DFA d, 
      IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
      Post propagator for extensional constraint described by a DFA. 
    */
    Gecode::IntVarArgs iva = getIntVarArgs(OZ_in(0));
    DeclareDFA(1, __d);
    
    try{
      Gecode::extensional(home, iva, __d, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isTupleSet(OZ_in(1))){
    /*
      void Gecode::extensional (Space *home, const IntVarArgs &x, 
      const TupleSet &t, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
      Post propagator for $x\in T$. 
    */
    Gecode::IntVarArgs iva = getIntVarArgs(OZ_in(0));
    DeclareTupleSet(1, __t);

    try{
      Gecode::extensional(home, iva, __t, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }

  CHECK_POST(home);
}OZ_BI_end


 /**
    Arithmetic constraints
    Propagators to post a arithmetic constraint on variables. This is a Gecode propagators interface. 
    For more information see http://www.gecode.org/gecode-doc-latest/group__TaskModelIntArith.html
 */
OZ_BI_define(gfd_max_5,5,0){
  DeclareGSpace(home);
  
  if(OZ_isGeIntVar(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) &&
     OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
    /**
       max (Space *home, IntVar x0, IntVar x1, IntVar x2, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
    */
    Gecode::IntVar &iv1 = intOrIntVar(OZ_in(0));
    Gecode::IntVar &iv2 = intOrIntVar(OZ_in(1));
    Gecode::IntVar &iv3 = intOrIntVar(OZ_in(2));
    Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(3));
    Gecode::PropKind __PK_DEF = getPropKind(OZ_in(4));  
    
    try{
      Gecode::max(home, iv1, iv2, iv3, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  } else {
    return OZ_typeError(0, "Malformed Propagator");
  }
    CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfd_max_4,4,0){
  DeclareGSpace(home);

  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1))){
    /**
       max (Space *home, const IntVarArgs &x, IntVar y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
    */
    Gecode::IntVarArgs iva = getIntVarArgs(OZ_in(0));
    Gecode::IntVar &iv = intOrIntVar(OZ_in(1));
    Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(2));
    Gecode::PropKind __PK_DEF = getPropKind(OZ_in(3));  

    try{
      Gecode::max(home, iva, iv, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  } else {
    return OZ_typeError(0, "Malformed Propagator");
  }

  CHECK_POST(home);
}OZ_BI_end



OZ_BI_define(gfd_mult_5,5,0){
  DeclareGSpace(home);

  if(OZ_isGeIntVar(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) ){
    
    /**
       mult (Space *home, IntVar x0, IntVar x1, IntVar x2, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
    */
    Gecode::IntVar &iv1 = intOrIntVar(OZ_in(0));
    Gecode::IntVar &iv2 = intOrIntVar(OZ_in(1));
    Gecode::IntVar &iv3 = intOrIntVar(OZ_in(2));
    Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(3));
    Gecode::PropKind __PK_DEF = getPropKind(OZ_in(4));  
    
    try{
      Gecode::mult(home, iv1, iv2, iv3, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  } else {
    return OZ_typeError(0, "Malformed Propagator");
  }

  CHECK_POST(home);
}OZ_BI_end

//Power constraint implemented by mult constraint
OZ_BI_define(gfd_power_5,5,0){
  DeclareGSpace(home);
  if(OZ_isGeIntVar(OZ_in(0)) && OZ_isInt(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) ){
    
    
    Gecode::IntVar &iv1 = intOrIntVar(OZ_in(0));
		int _I = OZ_intToC(OZ_in(1));
    Gecode::IntVar &iv2 = intOrIntVar(OZ_in(2));
    Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(3));
    Gecode::PropKind __PK_DEF = getPropKind(OZ_in(4));  
    
		IntVarArray tmpArray(home,_I,Int::Limits::min,Int::Limits::max);
		tmpArray[0] = iv1;
		for (int i = 0; i < _I-1; i++){
					/**
							mult (Space *home, IntVar x0, IntVar x1, IntVar x2, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
					*/
					 try{
							Gecode::mult(home,iv1,tmpArray[i],tmpArray[i+1],__ICL_DEF,__PK_DEF);
					}
					catch(Exception e){
							RAISE_GE_EXCEPTION(e);
					}
			}
		/**
			rel (Space *home,IntVar x0, IntRelType r, IntVar x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
		*/
		try{
			Gecode::rel(home,iv2,IRT_EQ,tmpArray[_I-1],__ICL_DEF,__PK_DEF);
		}
		catch(Exception e){
			RAISE_GE_EXCEPTION(e);
		}
  } else {
    return OZ_typeError(0, "Malformed Propagator");
  }

  CHECK_POST(home);
}OZ_BI_end


OZ_BI_define(gfd_div_5,5,0){
  DeclareGSpace(home);

  if(OZ_isGeIntVar(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) ){
    
    /**
       div (Space *home, IntVar x0, IntVar x1, IntVar x2, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
    */
    Gecode::IntVar &iv1 = intOrIntVar(OZ_in(0));
    Gecode::IntVar &iv2 = intOrIntVar(OZ_in(1));
    Gecode::IntVar &iv3 = intOrIntVar(OZ_in(2));
    Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(3));
    Gecode::PropKind __PK_DEF = getPropKind(OZ_in(4));  
    
    try{
      Gecode::div(home, iv1, iv2, iv3, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  } else {
    return OZ_typeError(0, "Malformed Propagator");
  }

  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfd_mod_5,5,0){
  DeclareGSpace(home);

  if(OZ_isGeIntVar(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) ){
    
    /**
       div (Space *home, IntVar x0, IntVar x1, IntVar x2, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
    */
    Gecode::IntVar &iv1 = intOrIntVar(OZ_in(0));
    Gecode::IntVar &iv2 = intOrIntVar(OZ_in(1));
    Gecode::IntVar &iv3 = intOrIntVar(OZ_in(2));
    Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(3));
    Gecode::PropKind __PK_DEF = getPropKind(OZ_in(4));  
    
    try{
      Gecode::mod(home, iv1, iv2, iv3, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  } else {
    return OZ_typeError(0, "Malformed Propagator");
  }

  CHECK_POST(home);
}OZ_BI_end


OZ_BI_define(gfd_min_5,5,0){
  DeclareGSpace(home);

  if(OZ_isGeIntVar(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) ){
    /**
       min (Space *home, IntVar x0, IntVar x1, IntVar x2, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
    */
    Gecode::IntVar &iv1 = intOrIntVar(OZ_in(0));
    Gecode::IntVar &iv2 = intOrIntVar(OZ_in(1));
    Gecode::IntVar &iv3 = intOrIntVar(OZ_in(2));
    Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(3));
    Gecode::PropKind __PK_DEF = getPropKind(OZ_in(4));  
    
    try{
      Gecode::min(home, iv1, iv2, iv3, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  } else {
    return OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfd_min_4,4,0){
  DeclareGSpace(home);
    
  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1))){
    /**
       min (Space *home, const IntVarArgs &x, IntVar y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
    */
    Gecode::IntVarArgs iva = getIntVarArgs(OZ_in(0));
    Gecode::IntVar &iv = intOrIntVar(OZ_in(1));
    Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(2));
    Gecode::PropKind __PK_DEF = getPropKind(OZ_in(3));  
    
    try{
      //Gecode::min(home, __x, __y, __ICL_DEF, __PK_DEF);
      Gecode::min(home, iva, iv, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  } else {
    return OZ_typeError(0, "Malformed Propagator");
  }
    
  CHECK_POST(home);
}OZ_BI_end


OZ_BI_define(gfd_abs_4,4,0){
  DeclareGSpace(home);
  
  if(OZ_isGeIntVar(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1))){
    /**
       abs (Space *home, IntVar x0, IntVar x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
    */
    Gecode::IntVar &iv1 = intOrIntVar(OZ_in(0));
    Gecode::IntVar &iv2 = intOrIntVar(OZ_in(1));
    Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(2));
    Gecode::PropKind __PK_DEF = getPropKind(OZ_in(3));  
    
    try{
      Gecode::abs(home, iv1, iv2, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  } else {
    return OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end


 /**
    Linear constraints
    Propagators to post a linear constraint on variables. This is a Gecode propagators interface. 
    For more information see http://www.gecode.org/gecode-doc-latest/group__TaskModelIntLinearInt.html
 */
OZ_BI_define(gfd_linear_5,5,0){
  DeclareGSpace(home);
  
  Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(3));
  Gecode::PropKind __PK_DEF = getPropKind(OZ_in(4));  

  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2))){
    /* linear(Space* home, const IntVarArgs& x, IntRelType r, int c,
              IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVarArgs  iva = getIntVarArgs(OZ_in(0));
    Gecode::IntRelType irt = getIntRelType(OZ_in(1));
    int i = OZ_intToC(OZ_in(2));

    try{
      Gecode::linear(home, iva, irt, i, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2))) {
    /*
      linear(Space* home, const IntVarArgs& x, IntRelType r, IntVar y,
             IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */
    Gecode::IntVarArgs  iva = getIntVarArgs(OZ_in(0));
    Gecode::IntRelType irt = getIntRelType(OZ_in(1));
    Gecode::IntVar &iv = intOrIntVar(OZ_in(2));
    try{
      Gecode::linear(home, iva, irt, iv, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end


OZ_BI_define(gfd_linear_6,6,0){
	DeclareGSpace(home);

  Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(4));
  Gecode::PropKind __PK_DEF = getPropKind(OZ_in(5));  

  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3)) ){
    /*  linear(Space* home, const IntVarArgs& x,  IntRelType r, int c, BoolVar b, 
         IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVarArgs iva = getIntVarArgs(OZ_in(0));
    Gecode::IntRelType irt = getIntRelType(OZ_in(1));
    int i = OZ_intToC(OZ_in(2));
    DeclareGeBoolVar(3, __b, home);
    try{
      Gecode::linear(home, iva, irt, i, __b, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3)) ){
		/*
      linear(Space* home, const IntVarArgs& x,  IntRelType r, IntVar y, BoolVar b, 
             IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    Gecode::IntVarArgs iva = getIntVarArgs(OZ_in(0));
    Gecode::IntRelType irt = getIntRelType(OZ_in(1));
    Gecode::IntVar &iv = intOrIntVar(OZ_in(2));
    DeclareGeBoolVar(3, __b, home);
    
    try{
      Gecode::linear(home, iva, irt, iv, __b, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
	else if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3)) ){
    /*
       linear(Space* home, const IntArgs& a, const IntVarArgs& x, IntRelType r, int c,
         IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */
    Gecode::IntArgs iar = getIntArgs(OZ_in(0));
    Gecode::IntVarArgs iva = getIntVarArgs(OZ_in(1));
    Gecode::IntRelType irt = getIntRelType(OZ_in(2));
    int i = OZ_intToC(OZ_in(3));
    try{
      Gecode::linear(home, iar, iva, irt, i, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3)) ){
    /*
       linear(Space* home, const IntArgs& a, const IntVarArgs& x,
         IntRelType r, IntVar y,
         IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */
    Gecode::IntArgs iar = getIntArgs(OZ_in(0));
    Gecode::IntVarArgs iva = getIntVarArgs(OZ_in(1));
    Gecode::IntRelType irt = getIntRelType(OZ_in(2));
    Gecode::IntVar &iv = intOrIntVar(OZ_in(3));

    try{
      Gecode::linear(home, iar, iva, irt, iv, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end


OZ_BI_define(gfd_linear_7,7,0){
  DeclareGSpace(home);
  Gecode::IntArgs  __a = getIntArgs(OZ_in(0));
  Gecode::IntVarArgs  __x = getIntVarArgs(OZ_in(1));
  Gecode::IntRelType __r = getIntRelType(OZ_in(2));
  DeclareGeBoolVar(4, __b, home);
  Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(5));
  Gecode::PropKind __PK_DEF = getPropKind(OZ_in(6));

  if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3)) && OZ_isGeBoolVar(OZ_in(4))){
    /*
       linear(Space* home, const IntArgs& a, const IntVarArgs& x, IntRelType r, int c, BoolVar b,
         IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */

    int i = OZ_intToC(OZ_in(3));
    
    try{
      Gecode::linear(home, __a, __x, __r, i, __b, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3)) && OZ_isGeBoolVar(OZ_in(4))){
    /*
      linear(Space* home, const IntArgs& a, const IntVarArgs& x, IntRelType r, IntVar y, BoolVar b,
         IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */
		 
    Gecode::IntVar &iv = intOrIntVar(OZ_in(3));
    
    try{
      Gecode::linear(home, __a, __x, __r, iv, __b, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }

  CHECK_POST(home);
}OZ_BI_end

#endif
