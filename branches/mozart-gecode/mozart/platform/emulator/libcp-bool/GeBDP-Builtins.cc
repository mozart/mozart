/*
 *  Main authors:
 *     Diana Lorena Velasco <dlvelasco@puj.edu.co>
 *     Juan Gabriel Torres  <juantorres@puj.edu.co>
 *
 *  Contributing authors:
 *     Andres Felipe Barco <anfelbar@univalle.edu.co>   
 *
 *  Copyright:
 *     Diana Lorena Velasco, 2007
 *     Juan Gabriel Torres, 2007
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


#ifndef __GEOZ_BOOLVAR_PROP_BUILTINS_CC__
#define __GEOZ_BOOLVAR_PROP_BUILTINS_CC__

#include "BoolVarMacros.hh"


/**
   Relation Constraints
   Propagators to post a relation constraint on variables. This is a Gecode propagators interface. 
   For more information see http://www.gecode.org/gecode-doc-latest/group__TaskModelIntRelBool.html
 */

OZ_BI_define(gbd_rel_5,5,0){
  DeclareGSpace(home);

  //test whether input is ref or var
  for(int i=0; i<5; i++)
    SuspendPosting(OZ_in(i));
  
  IntConLevel __icl = getIntConLevel(OZ_in(3));
  PropKind __pk = getPropKind(OZ_in(4));

  if(OZ_isGeBoolVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
    /**
       void  Gecode::rel (Space *home, BoolVar x0, IntRelType r, BoolVar x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
       Post propagator for $ x_0 \sim_r x_1$. 
    */
    BoolVar *__x0 = boolOrBoolVar(OZ_in(0));
    IntRelType __r = getIntRelType(OZ_in(1));
    BoolVar *__x1 = boolOrBoolVar(OZ_in(2));
    try{
      Gecode::rel(home, *__x0, __r, *__x1, __icl, __pk);
      delete __x0, __x1;
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
    /**
       void  Gecode::rel (Space *home, const BoolVarArgs &x, IntRelType r, BoolVar y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
       Post propagator for $ x_i \sim_r y $ for all $0\leq i<|x|$.
    */
    BoolVarArgs __x = getBoolVarArgs(OZ_in(0));
    IntRelType __r = getIntRelType(OZ_in(1));
    BoolVar *__y = boolOrBoolVar(OZ_in(2));
    try{
      Gecode::rel(home, __x, __r, *__y, __icl, __pk);
      delete __y;
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeBoolVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
    /**
       void  Gecode::rel (Space *home, BoolVar x, IntRelType r, int n, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
       Propagates $ x \sim_r n$. 
    */
    BoolVar *__x0 = boolOrBoolVar(OZ_in(0));
    IntRelType __r = getIntRelType(OZ_in(1));
    DeclareInt2(2, __n);
    try{
      Gecode::rel(home, *__x0, __r, __n, __icl, __pk);
      delete __x0;
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
    /**
       void Gecode::rel (Space* home, const BoolVarArgs& x, IntRelType r, int n, IntConLevel icl, PropKind pk);
    */
    BoolVarArgs __x = getBoolVarArgs(OZ_in(0));
    IntRelType __r = getIntRelType(OZ_in(1));
    DeclareInt2(2, __n);
    try{
      Gecode::rel(home, __x, __r, __n, __icl, __pk);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isBoolVarArgs(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
    /**
       void Gecode::rel (Space* home, const BoolVarArgs& x, IntRelType r, const BoolVarArgs& y, IntConLevel icl, PropKind pk);
    */
    BoolVarArgs __x = getBoolVarArgs(OZ_in(0));
    IntRelType __r = getIntRelType(OZ_in(1));
    BoolVarArgs __y = getBoolVarArgs(OZ_in(2));
    try{
      Gecode::rel(home, __x, __r, __y, __icl, __pk);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isBoolOpType(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
    /**
       void Gecode::rel (Space* home, BoolOpType o, const BoolVarArgs& x, BoolVar y, IntConLevel icl, PropKind pk);
    */
    BoolOpType __o = getBoolOpType(OZ_in(0));
    BoolVarArgs __x = getBoolVarArgs(OZ_in(1));
    BoolVar *__y = boolOrBoolVar(OZ_in(2));
    try{
      Gecode::rel(home, __o, __x, *__y, __icl, __pk);
      delete __y;
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isBoolOpType(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
    /**
       void Gecode::rel (Space* home, BoolOpType o, const BoolVarArgs& x, int n, IntConLevel icl, PropKind pk);
    */
    BoolOpType __o = getBoolOpType(OZ_in(0));
    BoolVarArgs __x = getBoolVarArgs(OZ_in(1));
    DeclareInt2(2, __n);
    try{
      Gecode::rel(home, __o, __x, __n, __icl, __pk);

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

OZ_BI_define(gbd_rel_4,4,0){
  DeclareGSpace(home);

  //test whether input is ref or var
  for(int i=0; i<4; i++)
    SuspendPosting(OZ_in(i));

  BoolVarArgs __x = getBoolVarArgs(OZ_in(0));
  IntRelType __r = getIntRelType(OZ_in(1));
  IntConLevel __icl = getIntConLevel(OZ_in(2));
  PropKind __pk = getPropKind(OZ_in(3));

  /**
     void Gecode::rel (Space* home, const BoolVarArgs& x, IntRelType r, IntConLevel icl, PropKind pk);
  */
  try{
    Gecode::rel(home, __x, __r, __icl, __pk);
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }

  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gbd_rel_6,6,0){
  DeclareGSpace(home);
  
  //test whether input is ref or var
  for(int i=0; i<6; i++)
    SuspendPosting(OZ_in(i));
  
  BoolVar *__x0 = boolOrBoolVar(OZ_in(0));
  BoolOpType __o = getBoolOpType(OZ_in(1));
  BoolVar *__x1 = boolOrBoolVar(OZ_in(2));
  IntConLevel __icl = getIntConLevel(OZ_in(4));
  PropKind __pk = getPropKind(OZ_in(5));
  
  if(OZ_isGeBoolVar(OZ_in(0)) && OZ_isBoolOpType(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
    /**
       void Gecode::rel (Space* home, BoolVar x0, BoolOpType o, BoolVar x1, BoolVar x2, IntConLevel icl, PropKind pk);
    */
    BoolVar *__x2 = boolOrBoolVar(OZ_in(3));
    try{
      Gecode::rel(home, *__x0, __o, *__x1, *__x2, __icl, __pk);
      delete __x2;
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeBoolVar(OZ_in(0)) && OZ_isBoolOpType(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2)) && OZ_isInt(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
    /**
       void Gecode::rel (Space* home, BoolVar x0, BoolOpType o, BoolVar x1, int n, IntConLevel icl, PropKind pk);
    */
    DeclareInt2(3, __n);
    try{
      Gecode::rel(home, *__x0, __o, *__x1, __n, __icl, __pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }
  
  delete __x0, __x1;
  CHECK_POST(home);
}OZ_BI_end


/**
   Linear Constraints
   Propagators to post a linear constraint on variables. This is a Gecode propagators interface. 
   For more information see http://www.gecode.org/gecode-doc-latest/group__TaskModelIntLinearBool.html
 */

OZ_BI_define(gbd_linear_5,5,0){
  DeclareGSpace(home);

  //test whether input is ref or var
  for(int i=0; i<5; i++)
    SuspendPosting(OZ_in(i));

  BoolVarArgs __x = getBoolVarArgs(OZ_in(0));
  IntRelType __r = getIntRelType(OZ_in(1));
  IntConLevel __icl = getIntConLevel(OZ_in(3));
  PropKind __pk = getPropKind(OZ_in(4));
 
  if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
    /**
       void Gecode::linear (Space* home, const BoolVarArgs& x, IntRelType r, int c, IntConLevel icl, PropKind pk); 
    */
    DeclareInt2(2, __c);
    try{
      Gecode::linear(home, __x, __r, __c, __icl, __pk);
   
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
    /**
       void Gecode::linear (Space* home, const BoolVarArgs& x, IntRelType r, IntVar y, IntConLevel icl, PropKind pk); 
    */
    IntVar *__y = intOrIntVar(OZ_in(2));
    try{
      Gecode::linear(home, __x, __r, *__y, __icl, __pk);
      delete __y;
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

OZ_BI_define(gbd_linear_6,6,0){
  DeclareGSpace(home);

  //test whether input is ref or var
  for(int i=0; i<6; i++)
    SuspendPosting(OZ_in(i));

  IntConLevel __icl = getIntConLevel(OZ_in(4));
  PropKind __pk = getPropKind(OZ_in(5));
  
  
  if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
    /**
       void Gecode::linear (Space* home, const BoolVarArgs& x, IntRelType r, int c, BoolVar b, IntConLevel icl, PropKind pk); 
    */
    BoolVarArgs __x = getBoolVarArgs(OZ_in(0));
    IntRelType __r = getIntRelType(OZ_in(1));
    DeclareInt2(2, __c);
    BoolVar *__b = boolOrBoolVar(OZ_in(3));
    try{
      Gecode::linear(home, __x, __r, __c, *__b, __icl, __pk);
      delete __b;
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
    /**
       void Gecode::linear (Space* home, const BoolVarArgs& x, IntRelType r, IntVar y, BoolVar b, IntConLevel icl,  PropKind pk);
    */
    BoolVarArgs __x = getBoolVarArgs(OZ_in(0));
    IntRelType __r = getIntRelType(OZ_in(1));
    IntVar *__y = intOrIntVar(OZ_in(2));
    BoolVar *__b = boolOrBoolVar(OZ_in(3));
    try{
      Gecode::linear(home, __x, __r, *__y, *__b, __icl, __pk);
      delete __y, __b;
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
    /**
       void Gecode::linear (Space* home, const IntArgs& a, const BoolVarArgs& x, IntRelType r, int c, IntConLevel icl, PropKind pk); 
    */
    IntArgs __a = getIntArgs(OZ_in(0));
    BoolVarArgs __x = getBoolVarArgs(OZ_in(1));
    IntRelType __r = getIntRelType(OZ_in(2));
    DeclareInt2(3, __c);
    try{
      Gecode::linear(home, __a, __x, __r, __c, __icl, __pk);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
    /**
       void Gecode::linear (Space* home, const IntArgs& a, const BoolVarArgs& x, IntRelType r, IntVar y, IntConLevel icl, PropKind pk);  
    */
    IntArgs __a = getIntArgs(OZ_in(0));
    BoolVarArgs __x = getBoolVarArgs(OZ_in(1));
    IntRelType __r = getIntRelType(OZ_in(2));
    IntVar *__y = intOrIntVar(OZ_in(3));
    try{
      Gecode::linear(home, __a, __x, __r, *__y, __icl, __pk);
      delete __y;
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

OZ_BI_define(gbd_linear_7,7,0){
  DeclareGSpace(home);

  //test whether input is ref or var
  for(int i=0; i<7; i++)
    SuspendPosting(OZ_in(i));

  IntArgs __a = getIntArgs(OZ_in(0));
  BoolVarArgs __x = getBoolVarArgs(OZ_in(1));
  IntRelType __r = getIntRelType(OZ_in(2));
  BoolVar *__b = boolOrBoolVar(OZ_in(4));
  IntConLevel __icl = getIntConLevel(OZ_in(5));
  PropKind __pk = getPropKind(OZ_in(6));
 
  if(OZ_isIntArgs(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3)) && OZ_isGeBoolVar(OZ_in(4)) && OZ_isIntConLevel(OZ_in(5)) && OZ_isPropKind(OZ_in(6))){
    /**
       void Gecode::linear (Space* home, const IntArgs& a, const BoolVarArgs& x, IntRelType r, int c, BoolVar b, IntConLevel icl, PropKind pk);
    */ 
    DeclareInt2(3, __c);
    try{
      Gecode::linear(home, __a, __x, __r, __c,*__b, __icl, __pk);
   
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isBoolVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3)) && OZ_isGeBoolVar(OZ_in(4)) && OZ_isIntConLevel(OZ_in(5)) && OZ_isPropKind(OZ_in(6))){
    /**
       void Gecode::linear (Space* home, const IntArgs& a, const BoolVarArgs& x, IntRelType r, IntVar y, BoolVar b, IntConLevel icl, PropKind pk);
    */   
    IntVar *__y = intOrIntVar(OZ_in(3));
    try{
      Gecode::linear(home, __a, __x, __r, *__y, *__b, __icl, __pk);
      delete __y;
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }
  delete __b;
  CHECK_POST(home);
}OZ_BI_end

#endif
