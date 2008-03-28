/*
 *  Main authors:
 *     Diana Lorena Velasco <dlvelasco@puj.edu.co>
 *     Juan Gabriel Torres  <juantorres@puj.edu.co>
 *
 *  Contributing authors:
 *     Andres Felipe Barco (anfelbar@univalle.edu.co)
 *     Gustavo A. Gomez Farhat (gafarhat@univalle.edu.co) 
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
 

#ifndef __GEOZ_INTVAR_PROP_BUILTINS_CC__
#define __GEOZ_INTVAR_PROP_BUILTINS_CC__

#include "IntVarMacros.hh"

/**
   Domain constraints
*/
OZ_BI_define(gfd_dom_5,5,0){
  DeclareGSpace(home);
  DeclareIntConLevel(3, __ICL_DEF);
  DeclarePropKind(4, __PK_DEF);

  Assert(OZ_isPropKind(OZ_in(4)));
  Assert( OZ_isIntConLevel(OZ_in(3)));

  if( OZ_isGeIntVar(OZ_in(0)) && OZ_isInt(OZ_in(1)) && OZ_isInt(OZ_in(2)) ){
    // dom(Space* home, IntVar x, int l, int m, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DeclareGeIntVar(0, __x, home);
    DeclareInt2(1, __l);
    DeclareInt2(2, __m);
    try{
      Gecode::dom(home, __x, __l, __m, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isInt(OZ_in(1)) && OZ_isInt(OZ_in(2)) ){
    // dom(Space* home, const IntVarArgs& x, int l, int m, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DECLARE_INTVARARGS(0, __x, home);
    DeclareInt2(1, __l);
    DeclareInt2(2, __m);
    try{
      Gecode::dom(home, __x, __l, __m, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeIntVar(OZ_in(0)) && OZ_isIntSet(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2)) ){
    // dom(Space* home, IntVar x, const IntSet& s, BoolVar b, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DeclareGeIntVar(0, __x, home);
    DECLARE_INT_SET2(1, __s);
    DeclareGeBoolVar(2, __b, home);
    try{
      Gecode::dom(home, __x, __s, __b, __ICL_DEF, __PK_DEF);
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
  Assert(OZ_isPropKind(OZ_in(4)));
  Assert( OZ_isIntConLevel(OZ_in(3)));

  if(OZ_isGeIntVar(OZ_in(0)) && OZ_isIntSet(OZ_in(1)) ){
    //  dom(Space* home, IntVar x, const IntSet& s, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DeclareGeIntVar(0, __x, home);
    DECLARE_INT_SET2(1, __s);
    DeclareIntConLevel(2, __ICL_DEF);
    DeclarePropKind(3, __PK_DEF);
    try{
      Gecode::dom(home, __x, __s, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntSet(OZ_in(1)) ){
    //  dom(Space* home, const IntVarArgs& x, const IntSet& s, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DECLARE_INTVARARGS(0, __x, home);
    DECLARE_INT_SET2(1, __s);
    DeclareIntConLevel(2, __ICL_DEF);
    DeclarePropKind(3, __PK_DEF);
    try{
      Gecode::dom(home, __x, __s, __ICL_DEF, __PK_DEF);
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
  DeclareGeIntVar(0, __x, home);
  DeclareInt2(1, __l);
  DeclareInt2(2, __m);
  DeclareGeBoolVar(3, __b, home);
  DeclareIntConLevel(4, __ICL_DEF);
  DeclarePropKind(5, __PK_DEF);
  // dom(Space* home, IntVar x, int l, int m, BoolVar b, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
  try{
    Gecode::dom(home, __x, __l, __m, __b, __ICL_DEF, __PK_DEF);
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(home);
}OZ_BI_end


 /**
    Simple relation constraints over integer variables
 */
OZ_BI_define(gfd_rel_5,5,0){
  DeclareGSpace(home);
  DeclareIntRelType(1, __r);
  DeclareIntConLevel(3, __ICL_DEF);
  DeclarePropKind(4, __PK_DEF);
  
  Assert( OZ_isIntConLevel(OZ_in(3)) &&  OZ_isPropKind(OZ_in(4)) && OZ_isIntRelType(OZ_in(1)));
  if(OZ_isGeIntVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) ){
    //   rel(Space* home, IntVar x0, IntRelType r, IntVar x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DeclareGeIntVar(0, __x, home);
    DeclareGeIntVar(2, __y, home);
    try{
      Gecode::rel(home, __x, __r, __y, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  } 
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) ){
    // rel(Space* home, const IntVarArgs& x, IntRelType r, IntVar y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DECLARE_INTVARARGS(0, __x, home);
    DeclareGeIntVar(2, __y, home);
    try{
      Gecode::rel(home, __x, __r, __y, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeIntVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) ){
    // rel(Space* home, IntVar x, IntRelType r, int c, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DeclareGeIntVar(0, __x, home);
    DeclareInt2(2, __c);
    try{
      Gecode::rel(home, __x, __r, __c, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) ){
    //  rel(Space* home, const IntVarArgs& x, IntRelType r, int c, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DECLARE_INTVARARGS(0, __x, home);
    DeclareInt2(2, __c);
    try{
      Gecode::rel(home, __x, __r, __c, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isIntVarArgs(OZ_in(2)) ){
    //  rel(Space* home, const IntVarArgs& x, IntRelType r, const IntVarArgs& y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DECLARE_INTVARARGS(0, __x, home);
    DECLARE_INTVARARGS(2, __y, home);
    try{
      Gecode::rel(home, __x, __r, __y, __ICL_DEF, __PK_DEF);
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
  DeclareGeIntVar(0, __x, home);
  DeclareIntRelType(1, __r);
  DeclareGeBoolVar(3, __b, home);

  Assert(OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5)));
  
  DeclareIntConLevel(4, __ICL_DEF);
  DeclarePropKind(5, __PK_DEF);

  if(OZ_isGeIntVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3)) ){
    //  rel(Space* home, IntVar x0, IntRelType r, IntVar x1, BoolVar b, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DeclareGeIntVar(2, __x1, home);
    try{
      Gecode::rel(home, __x, __r, __x1, __b, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeIntVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3)) ){
    // rel(Space* home, IntVar x, IntRelType r, int c, BoolVar b, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DeclareInt2(2, __c);
    try{
      Gecode::rel(home, __x, __r, __c, __b, __ICL_DEF, __PK_DEF);
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
  DeclareIntRelType(1, __r);

  Assert( OZ_isIntConLevel(OZ_in(2)) && OZ_isPropKind(OZ_in(3)) );
  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) ){
    // rel(Space* home, const IntVarArgs& x, IntRelType r, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DECLARE_INTVARARGS(0, __x, home);
    DeclareIntConLevel(2, __ICL_DEF);
    DeclarePropKind(3, __PK_DEF);
    try{
      Gecode::rel(home, __x, __r, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  /*
    // should go in rel_6
    else if(OZ_isGeIntVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3))){
    DeclareGeIntVar(0, __x, home);
    DeclareGeIntVar(2, __x1, home);
    DeclareGeBoolVar(3, __b, home);
    try{
      Gecode::rel(home, __x, __r, __x1, __b);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
    }*/
  /*
    // should go in rel_6
  else if(OZ_isGeIntVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3))){
    DeclareGeIntVar(0, __x, home);
    DeclareInt2(2, __c);
    DeclareGeBoolVar(3, __b, home);
    try{
      Gecode::rel(home, __x, __r, __c, __b);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
    }*/
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end
 
/*
  // replaced by rel_5
OZ_BI_define(rel_3,3,0){
  DeclareGSpace(home);
  DeclareIntRelType(1, __r);
  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2))){
    DECLARE_INTVARARGS(0, __x, home);
    DeclareGeIntVar(2, __y, home);
    try{
      Gecode::rel(home, __x, __r, __y);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeIntVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2))){
    DeclareGeIntVar(0, __x, home);
    DeclareInt2(2, __c);
    try{
      Gecode::rel(home, __x, __r, __c);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2))){
    DECLARE_INTVARARGS(0, __x, home);
    DeclareInt2(2, __c);
    try{
      Gecode::rel(home, __x, __r, __c);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isIntVarArgs(OZ_in(2))){
    DECLARE_INTVARARGS(0, __x, home);
    DECLARE_INTVARARGS(2, __y, home);
    try{
      Gecode::rel(home, __x, __r, __y);
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
*/
 /*
   // replaced by rel_4
OZ_BI_define(rel_2,2,0){
  DeclareGSpace(home);
  DECLARE_INTVARARGS(0, __x, home);
  DeclareIntRelType(1, __r);
  try{
    Gecode::rel(home, __x, __r);
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(home);
}OZ_BI_end
 */


 /**
    Element constraints
 */
OZ_BI_define(gfd_element_5,5,0){
  DeclareGSpace(home);
  DeclareGeIntVar(1, __x1, home);

  Assert(OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4)));
  DeclareIntConLevel(3, __ICL_DEF);
  DeclarePropKind(4, __PK_DEF);


  if(OZ_isIntArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) ){
    // element(Space* home, const IntArgs& n, IntVar x0, IntVar x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DECLARE_INTARGS(0, __x);
    DeclareGeIntVar(2, __x2, home);
    try{
      Gecode::element(home, __x, __x1, __x2, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2))){
    // element(Space* home, const IntArgs& n, IntVar x0, BoolVar x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DECLARE_INTARGS(0, __x);
    DeclareGeBoolVar(2, __x2, home);
    try{
      Gecode::element(home, __x, __x1, __x2, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isInt(OZ_in(2)) ){
    // element(Space* home, const IntArgs& n, IntVar x0, int x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DECLARE_INTARGS(0, __x);
    DeclareInt2(2, __x2);
    try{
      Gecode::element(home, __x, __x1, __x2, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) ){
    // element(Space* home, const IntVarArgs& x, IntVar y0, IntVar y1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DECLARE_INTVARARGS(0, __x, home);
    DeclareGeIntVar(2, __x2, home);
    try{
      Gecode::element(home, __x, __x1, __x2, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isInt(OZ_in(2)) ){
    //  element(Space* home, const IntVarArgs& x, IntVar y0, int y1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DECLARE_INTVARARGS(0, __x, home);
    DeclareInt2(2, __x2);
    try{
      Gecode::element(home, __x, __x1, __x2, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2))){
    // element(Space* home, const BoolVarArgs& x, IntVar y0, BoolVar y1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DECLARE_BOOLVARARGS(0, __x, home);
    DeclareGeBoolVar(2, __x2, home);
    try{
      Gecode::element(home, __x, __x1, __x2, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isInt(OZ_in(2))){
    // element(Space* home, const BoolVarArgs& x, IntVar y0, int y1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DECLARE_BOOLVARARGS(0, __x, home);
    DeclareGeBoolVar(2, __x2, home);
    try{
      Gecode::element(home, __x, __x1, __x2, __ICL_DEF, __PK_DEF);
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

 /*
   // not needed anymore
OZ_BI_define(element_3,3,0){
  DeclareGSpace(home);
  DeclareGeIntVar(1, __x1, home);
  if(OZ_isIntArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2))){
    DECLARE_INTARGS(0, __x);
    DeclareGeIntVar(2, __x2, home);
    try{
      Gecode::element(home, __x, __x1, __x2);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2))){
    DECLARE_INTARGS(0, __x);
    DeclareGeBoolVar(2, __x2, home);
    try{
      Gecode::element(home, __x, __x1, __x2);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isInt(OZ_in(2))){
    DECLARE_INTARGS(0, __x);
    DeclareInt2(2, __x2);
    try{
      Gecode::element(home, __x, __x1, __x2);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2))){
    DECLARE_INTVARARGS(0, __x, home);
    DeclareGeIntVar(2, __x2, home);
    try{
      Gecode::element(home, __x, __x1, __x2);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isInt(OZ_in(2))){
    DECLARE_INTVARARGS(0, __x, home);
    DeclareInt2(2, __x2);
    try{
      Gecode::element(home, __x, __x1, __x2);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2))){
    DECLARE_BOOLVARARGS(0, __x, home);
    DeclareGeBoolVar(2, __x2, home);
    try{
      Gecode::element(home, __x, __x1, __x2);
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
 */

 /**
    Distinct constraints
 */
OZ_BI_define(gfd_distinct_3,3,0){
  DeclareGSpace(home);
  DECLARE_INTVARARGS(0, __x, home);
  DeclareIntConLevel(1, __ICL_DEF);
  DeclarePropKind(2, __PK_DEF);
  // distinct(Space* home, const IntVarArgs& x, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
  try{
    Gecode::distinct(home, __x, __ICL_DEF, __PK_DEF);
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfd_distinct_4,4,0){
  DeclareGSpace(home);
  DECLARE_INTARGS(0, __n);
  DECLARE_INTVARARGS(1, __x, home);
  DeclareIntConLevel(2, __ICL_DEF);
  DeclarePropKind(3, __PK_DEF);
  // distinct(Space* home, const IntArgs& n, const IntVarArgs& x, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
  try{
    Gecode::distinct(home, __n, __x, __ICL_DEF, __PK_DEF);
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(home);
}OZ_BI_end

 /*
OZ_BI_define(distinct_1,1,0){
  DeclareGSpace(home);
  DECLARE_INTVARARGS(0, __x, home);
  try{
    Gecode::distinct(home, __x);
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(distinct_2,2,0){
  DeclareGSpace(home);
  DECLARE_INTARGS(0, __n);
  DECLARE_INTVARARGS(1, __x, home);
  try{
    Gecode::distinct(home, __n, __x);
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(home);
}OZ_BI_end
 */

 /**
    Channel constraints
 */
OZ_BI_define(gfd_channel_4,4,0){
  DeclareGSpace(home);

  Assert(OZ_isIntConLevel(OZ_in(2)) && OZ_isPropKind(OZ_in(3)));

  DeclareIntConLevel(2, __ICL_DEF);
  DeclarePropKind(3, __PK_DEF);


  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1))){
    // channel(Space* home, const IntVarArgs& x, const IntVarArgs& y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DECLARE_INTVARARGS(0, __x, home);
    DECLARE_INTVARARGS(1, __y, home);
    try{
      Gecode::channel(home, __x, __y, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeBoolVar(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) ){
    //   channel(Space* home, BoolVar x0, IntVar x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    DeclareGeBoolVar(0, __x0, home);
    DeclareGeIntVar(1, __x1, home);
    try{
      Gecode::channel(home, __x0, __x1, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeIntVar(OZ_in(0)) && OZ_isGeBoolVar(OZ_in(1)) ) {
    //   channel(Space* home, IntVar x0, BoolVar x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    DeclareGeIntVar(0, __x0, home);
    DeclareGeBoolVar(1, __x1, home);
    try{
      Gecode::channel(home, __x0, __x1, __ICL_DEF, __PK_DEF);
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
  DeclareGeIntVar(1, __y, home);
  DeclareInt2(2, __o);
  DeclareIntConLevel(3, __ICL_DEF);
  DeclarePropKind(4, __PK_DEF);
  //  channel(Space* home, const BoolVarArgs& x, IntVar y, int o=0, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
  try{
    Gecode::channel(home, __x, __y, __o, __ICL_DEF, __PK_DEF);
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(home);
}OZ_BI_end
 /*
OZ_BI_define(channel_2,2,0){
  DeclareGSpace(home);
  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1))){
    DECLARE_INTVARARGS(0, __x, home);
    DECLARE_INTVARARGS(1, __y, home);
    try{
      Gecode::channel(home, __x, __y);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeBoolVar(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1))){
    DeclareGeBoolVar(0, __x0, home);
    DeclareGeIntVar(1, __x1, home);
    try{
      Gecode::channel(home, __x0, __x1);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeIntVar(OZ_in(0)) && OZ_isGeBoolVar(OZ_in(1))){
    DeclareGeIntVar(0, __x0, home);
    DeclareGeBoolVar(1, __x1, home);
    try{
      Gecode::channel(home, __x0, __x1);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1))){
    DECLARE_BOOLVARARGS(0, __x, home);
    DeclareGeIntVar(1, __y, home);
    try{
      Gecode::channel(home, __x, __y);
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
 */

 /**
    Graph constraints
 */
OZ_BI_define(gfd_circuit_3,3,0){
  DeclareGSpace(home);
  DECLARE_INTVARARGS(0, __x, home);
  DeclareIntConLevel(1, __ICL_DEF);
  DeclarePropKind(2, __PK_DEF);
  //   circuit(Space* home, const IntVarArgs& x, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);

  try{
    Gecode::circuit(home, __x, __ICL_DEF, __PK_DEF);
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(home);
}OZ_BI_end

 /*
OZ_BI_define(circuit_1,1,0){
  DeclareGSpace(home);
  DECLARE_INTVARARGS(0, __x, home);
  try{
    Gecode::circuit(home, __x);
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(home);
}OZ_BI_end
 */

 /**
    Scheduling constraints
 */
OZ_BI_define(gfd_cumulatives_9,9,0){
  DeclareGSpace(home);
  DECLARE_INTVARARGS(1, __start, home);
  DECLARE_INTVARARGS(3, __end, home);
  DECLARE_INTARGS(5, __limit);
  DeclareBool(6, __at_most);
  
  Assert(OZ_isIntConLevel(OZ_in(7)) && OZ_isPropKind(OZ_in(8)));

  DeclareIntConLevel(7, __ICL_DEF);
  DeclarePropKind(8, __PK_DEF);



  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntVarArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntVarArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6)) ){
    /*   cumulatives(Space* home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most, 
              IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    DECLARE_INTVARARGS(0, __machine, home);
    DECLARE_INTVARARGS(2, __duration, home);
    DECLARE_INTVARARGS(4, __height, home);
    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntVarArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntVarArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6)) ){
    /*
      cumulatives(Space* home, const IntArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most, 
              IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */
    DECLARE_INTARGS(0, __machine);
    DECLARE_INTVARARGS(2, __duration, home);
    DECLARE_INTVARARGS(4, __height, home);
    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most, __ICL_DEF, __PK_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntVarArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6))){
    /*
      cumulatives(Space* home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most, 
              IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    DECLARE_INTVARARGS(0, __machine, home);
    DECLARE_INTARGS(2, __duration);
    DECLARE_INTVARARGS(4, __height, home);
    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntVarArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6)) ) {
    /*
        cumulatives(Space* home, const IntArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most, 
              IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);

     */
    DECLARE_INTARGS(0, __machine);
    DECLARE_INTARGS(2, __duration);
    DECLARE_INTVARARGS(4, __height, home);
    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntVarArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6)) ){
    /*
      cumulatives(Space* home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most, 
              IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */
    
    DECLARE_INTVARARGS(0, __machine, home);
    DECLARE_INTVARARGS(2, __duration, home);
    DECLARE_INTARGS(4, __height);
    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntVarArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6))){
    /*
        cumulatives(Space* home, const IntArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most, 
              IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */
    DECLARE_INTARGS(0, __machine);
    DECLARE_INTVARARGS(2, __duration, home);
    DECLARE_INTARGS(4, __height);
    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6))){
    /*
        cumulatives(Space* home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most, 
              IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);

     */
    DECLARE_INTVARARGS(0, __machine, home);
    DECLARE_INTARGS(2, __duration);
    DECLARE_INTARGS(4, __height);
    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6))){
    /*
        cumulatives(Space* home, const IntArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most, 
              IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */
    DECLARE_INTARGS(0, __machine);
    DECLARE_INTARGS(2, __duration);
    DECLARE_INTARGS(4, __height);
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

 /*
OZ_BI_define(cumulatives_7,7,0){
  DeclareGSpace(home);
  DECLARE_INTVARARGS(1, __start, home);
  DECLARE_INTVARARGS(3, __end, home);
  DECLARE_INTARGS(5, __limit);
  DeclareBool(6, __at_most);
  
  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntVarArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntVarArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6))){
  DECLARE_INTVARARGS(0, __machine, home);
  DECLARE_INTVARARGS(2, __duration, home);
  DECLARE_INTVARARGS(4, __height, home);
  try{
  Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most);
  }
  catch(Exception e){
  RAISE_GE_EXCEPTION(e);
  }
  }
  else 
  if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntVarArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntVarArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6))){
    DECLARE_INTARGS(0, __machine);
    DECLARE_INTVARARGS(2, __duration, home);
    DECLARE_INTVARARGS(4, __height, home);
    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntVarArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6))){
    DECLARE_INTVARARGS(0, __machine, home);
    DECLARE_INTARGS(2, __duration);
    DECLARE_INTVARARGS(4, __height, home);
    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntVarArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6))){
    DECLARE_INTARGS(0, __machine);
    DECLARE_INTARGS(2, __duration);
    DECLARE_INTVARARGS(4, __height, home);
    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntVarArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6))){
    DECLARE_INTVARARGS(0, __machine, home);
    DECLARE_INTVARARGS(2, __duration, home);
    DECLARE_INTARGS(4, __height);
    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntVarArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6))){
    DECLARE_INTARGS(0, __machine);
    DECLARE_INTVARARGS(2, __duration, home);
    DECLARE_INTARGS(4, __height);
    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6))){
    DECLARE_INTVARARGS(0, __machine, home);
    DECLARE_INTARGS(2, __duration);
    DECLARE_INTARGS(4, __height);
    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntArgs(OZ_in(2)) && OZ_isIntVarArgs(OZ_in(3)) && OZ_isIntArgs(OZ_in(4)) && OZ_isIntArgs(OZ_in(5)) && OZ_isBool(OZ_in(6))){
    DECLARE_INTARGS(0, __machine);
    DECLARE_INTARGS(2, __duration);
    DECLARE_INTARGS(4, __height);
    try{
      Gecode::cumulatives(home, __machine, __start, __duration, __end, __height, __limit, __at_most);
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
 */

 /**
    Sorted constraints
 */

OZ_BI_define(gfd_sorted_4,4,0){
  DeclareGSpace(home);
  
  Assert(OZ_isIntConLevel(OZ_in(2)) && OZ_isPropKind(OZ_in(3)));
  
  DeclareIntConLevel(2, __ICL_DEF);
  DeclarePropKind(3, __PK_DEF);



  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1))){
    /*
      void Gecode::sorted (Space *home, const IntVarArgs &x, 
      const IntVarArgs &y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
      Post propagator that y is x sorted in increasing order. 
    */
    
    DECLARE_INTVARARGS(0, __x, home);
    DECLARE_INTVARARGS(1, __y, home);
    
    try{
      Gecode::sorted(home, __x, __y, __ICL_DEF, __PK_DEF);

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

  Assert(OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4)));

  DeclareIntConLevel(3, __ICL_DEF);
  DeclarePropKind(4, __PK_DEF);



  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && 
     OZ_isIntVarArgs(OZ_in(2))){
    /*
      void Gecode::sorted (Space *, const IntVarArgs &x, const IntVarArgs &y, 
      const IntVarArgs &z, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
      Post propagator that y is x sorted in increasing order. 
    */
    
    DECLARE_INTVARARGS(0, __x, home);
    DECLARE_INTVARARGS(1, __y, home);
    DECLARE_INTVARARGS(2, __z, home);

    try{
      Gecode::sorted(home, __x, __y, __z, __ICL_DEF, __PK_DEF);

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
 */
OZ_BI_define(gfd_count_6,6,0){
  DeclareGSpace(home);
  DECLARE_INTVARARGS(0, __x, home);
  DeclareIntRelType(2, __r);
  DeclareIntConLevel(4, __ICL_DEF);
  DeclarePropKind(5, __PK_DEF);

  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isInt(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
    DeclareInt2(1, __n);
    DeclareInt2(3, __m);
    try{
      Gecode::count(home, __x, __n, __r, __m, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
    DeclareGeIntVar(1, __y, home);
    DeclareInt2(3, __m);
    try{
      Gecode::count(home, __x, __y, __r, __m, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
    DECLARE_INTARGS(1, __y);
    DeclareInt2(3, __m);
    try{
      Gecode::count(home, __x, __y, __r, __m, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isInt(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
    DeclareInt2(1, __n);
    DeclareGeIntVar(3, __z, home);
    try{
      Gecode::count(home, __x, __n, __r, __z, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
    DeclareGeIntVar(1, __y, home);
    DeclareGeIntVar(3, __z, home);
    try{
      Gecode::count(home, __x, __y, __r, __z, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3)) && OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5))){
    DECLARE_INTARGS(1, __y);
    DeclareGeIntVar(3, __z, home);
    try{
      Gecode::count(home, __x, __y, __r, __z, __ICL_DEF, __PK_DEF);

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
  DECLARE_INTVARARGS(0, __x, home);

  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntConLevel(OZ_in(2)) && OZ_isPropKind(OZ_in(3))){
    DECLARE_INTVARARGS(1, __c, home);
    DeclareIntConLevel(2, __ICL_DEF);
    DeclarePropKind(3, __PK_DEF);
    try{
      Gecode::count(home, __x, __c, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntSetArgs(OZ_in(1)) && OZ_isIntConLevel(OZ_in(2)) && OZ_isPropKind(OZ_in(3))){
    DECLARE_INT_SET_ARGS(1, __c);
    DeclareIntConLevel(2, __ICL_DEF);
    DeclarePropKind(3, __PK_DEF);
    try{
      Gecode::count(home, __x, __c, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isInt(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3))){
    DeclareInt2(1, __n);
    DeclareIntRelType(2, __r);
    DeclareInt2(3, __m);
    try{
      Gecode::count(home, __x, __n, __r, __m);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3))){
    DeclareGeIntVar(1, __y, home);
    DeclareIntRelType(2, __r);
    DeclareInt2(3, __m);
    try{
      Gecode::count(home, __x, __y, __r, __m);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3))){
    DECLARE_INTARGS(1, __y);
    DeclareIntRelType(2, __r);
    DeclareInt2(3, __m);
    try{
      Gecode::count(home, __x, __y, __r, __m);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isInt(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3))){
    DeclareInt2(1, __n);
    DeclareIntRelType(2, __r);
    DeclareGeIntVar(3, __z, home);
    try{
      Gecode::count(home, __x, __n, __r, __z);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3))){
    DeclareGeIntVar(1, __y, home);
    DeclareIntRelType(2, __r);
    DeclareGeIntVar(3, __z, home);
    try{
      Gecode::count(home, __x, __y, __r, __z);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3))){
    DECLARE_INTARGS(1, __y);
    DeclareIntRelType(2, __r);
    DeclareGeIntVar(3, __z, home);
    try{
      Gecode::count(home, __x, __y, __r, __z);

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
  DECLARE_INTVARARGS(0, __x, home);
  DECLARE_INTARGS(2, __v);
  DeclareIntConLevel(3, __ICL_DEF);
  DeclarePropKind(4, __PK_DEF);

  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntArgs(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
    DECLARE_INTVARARGS(1, __c, home);
    try{
      Gecode::count(home, __x, __c, __v, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntSetArgs(OZ_in(1)) && OZ_isIntArgs(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
    DECLARE_INT_SET_ARGS(1, __c);
    try{
      Gecode::count(home, __x, __c, __v, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntSet(OZ_in(1)) && OZ_isIntArgs(OZ_in(2)) && OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4))){
    DECLARE_INT_SET2(1, __c);
    try{
      Gecode::count(home, __x, __c, __v, __ICL_DEF, __PK_DEF);

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

OZ_BI_define(gfd_count_2,2,0){
  DeclareGSpace(home);
  DECLARE_INTVARARGS(0, __x, home);

  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1))){
    DECLARE_INTVARARGS(1, __c, home);
    try{
      Gecode::count(home, __x, __c);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntSetArgs(OZ_in(1))){
    DECLARE_INT_SET_ARGS(1, __c);
    try{
      Gecode::count(home, __x, __c);

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

OZ_BI_define(gfd_count_3,3,0){
  DeclareGSpace(home);
  DECLARE_INTVARARGS(0, __x, home);
  DECLARE_INTARGS(2, __v);

  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntArgs(OZ_in(2))){
    DECLARE_INTVARARGS(1, __c, home);
    try{
      Gecode::count(home, __x, __c, __v);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntSetArgs(OZ_in(1)) && OZ_isIntArgs(OZ_in(2))){
    DECLARE_INT_SET_ARGS(1, __c);
    try{
      Gecode::count(home, __x, __c, __v);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntSet(OZ_in(1)) && OZ_isIntArgs(OZ_in(2))){
    DECLARE_INT_SET2(1, __c);
    try{
      Gecode::count(home, __x, __c, __v);

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
 */
OZ_BI_define(gfd_extensional_4,4,0){
  DeclareGSpace(home);

  Assert(OZ_isIntConLevel(OZ_in(2)) && OZ_isPropKind(OZ_in(3)));

  DeclareIntConLevel(2, __ICL_DEF);
  DeclarePropKind(3, __PK_DEF);



  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isDFA(OZ_in(1))){
    /*
      void Gecode::extensional (Space *home, const IntVarArgs &x, DFA d, 
      IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
      Post propagator for extensional constraint described by a DFA. 
    */
    
    DECLARE_INTVARARGS(0, __x, home);
    DeclareDFA(1, __d);
    
    try{
      Gecode::extensional(home, __x, __d, __ICL_DEF, __PK_DEF);

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
    DECLARE_INTVARARGS(0, __x, home);
    DeclareTupleSet(1, __t);
    try{
      Gecode::extensional(home, __x, __t, __ICL_DEF, __PK_DEF);

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
 */
OZ_BI_define(gfd_max_5,5,0){
  DeclareGSpace(home);
  DeclareGeIntVar(0, __x0, home);
  DeclareGeIntVar(1, __x1, home);
  DeclareGeIntVar(2, __x2, home);
  DeclareIntConLevel(3, __ICL_DEF);
  DeclarePropKind(4, __PK_DEF);

  try{
    Gecode::max(home, __x0, __x1, __x2, __ICL_DEF, __PK_DEF);
    
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }

  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfd_max_4,4,0){
  DeclareGSpace(home);
  DECLARE_INTVARARGS(0, __x, home);
  DeclareGeIntVar(1, __y, home);
  DeclareIntConLevel(2, __ICL_DEF);
  DeclarePropKind(3, __PK_DEF);

  try{
    Gecode::max(home, __x, __y, __ICL_DEF, __PK_DEF);
    
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }

  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfd_max_3,3,0){
  DeclareGSpace(home);
  DeclareGeIntVar(0, __x0, home);
  DeclareGeIntVar(1, __x1, home);
  DeclareGeIntVar(2, __x2, home);

  try{
    Gecode::max(home, __x0, __x1, __x2);
    
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }

  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfd_max_2,2,0){
  DeclareGSpace(home);
  DECLARE_INTVARARGS(0, __x, home);
  DeclareGeIntVar(1, __y, home);

  try{
    Gecode::max(home, __x, __y);
    
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }

  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfd_mult_5,5,0){
  DeclareGSpace(home);
  DeclareGeIntVar(0, __x0, home);
  DeclareGeIntVar(1, __x1, home);
  DeclareGeIntVar(2, __x2, home);
  DeclareIntConLevel(3, __ICL_DEF);
  DeclarePropKind(4, __PK_DEF);

  try{
    Gecode::mult(home, __x0, __x1, __x2, __ICL_DEF, __PK_DEF);
    
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }

  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfd_mult_3,3,0){
  DeclareGSpace(home);
  DeclareGeIntVar(0, __x0, home);
  DeclareGeIntVar(1, __x1, home);
  DeclareGeIntVar(2, __x2, home);

  try{
    Gecode::mult(home, __x0, __x1, __x2);
    
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }

  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfd_min_5,5,0){
  DeclareGSpace(home);
  DeclareGeIntVar(0, __x0, home);
  DeclareGeIntVar(1, __x1, home);
  DeclareGeIntVar(2, __x2, home);
  DeclareIntConLevel(3, __ICL_DEF);
  DeclarePropKind(4, __PK_DEF);

  try{
    Gecode::min(home, __x0, __x1, __x2, __ICL_DEF, __PK_DEF);
    
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }

  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfd_min_4,4,0){
  DeclareGSpace(home);
  DECLARE_INTVARARGS(0, __x, home);
  DeclareGeIntVar(1, __y, home);
  DeclareIntConLevel(2, __ICL_DEF);
  DeclarePropKind(3, __PK_DEF);

  try{
    Gecode::min(home, __x, __y, __ICL_DEF, __PK_DEF);
    
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }

  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfd_min_3,3,0){
  DeclareGSpace(home);
  DeclareGeIntVar(0, __x0, home);
  DeclareGeIntVar(1, __x1, home);
  DeclareGeIntVar(2, __x2, home);

  try{
    Gecode::min(home, __x0, __x1, __x2);
    
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }

  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfd_min_2,2,0){
  DeclareGSpace(home);
  DECLARE_INTVARARGS(0, __x, home);
  DeclareGeIntVar(1, __y, home);

  try{
    Gecode::min(home, __x, __y);
    
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }

  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfd_abs_4,4,0){
  DeclareGSpace(home);
  DeclareGeIntVar(0, __x0, home);
  DeclareGeIntVar(1, __x1, home);
  DeclareIntConLevel(2, __ICL_DEF);
  DeclarePropKind(3, __PK_DEF);

  try{
    Gecode::abs(home, __x0, __x1, __ICL_DEF, __PK_DEF);
    
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }

  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfd_abs_2,2,0){
  DeclareGSpace(home);
  DeclareGeIntVar(0, __x0, home);
  DeclareGeIntVar(1, __x1, home);

  try{
    Gecode::abs(home, __x0, __x1);
    
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }

  CHECK_POST(home);
}OZ_BI_end

 /**
    Linear constraints over integer variables
 */
OZ_BI_define(gfd_linear_5,5,0){
  DeclareGSpace(home);
  
  Assert( OZ_isIntConLevel(OZ_in(3)) && OZ_isPropKind(OZ_in(4)));
  DeclareIntConLevel(3, __ICL_DEF);
  DeclarePropKind(4, __PK_DEF);
  


  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2))){
    /* linear(Space* home, const IntVarArgs& x, 
         IntRelType r, int c,
         IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    DECLARE_INTVARARGS(0, __x, home);
    DeclareIntRelType(1, __r);
    DeclareInt2(2, __c);
    try{
      Gecode::linear(home, __x, __r, __c, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2))) {
    /*
      linear(Space* home, const IntVarArgs& x,
         IntRelType r, IntVar y,
         IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */
    DECLARE_INTVARARGS(0, __x, home);
    DeclareIntRelType(1, __r);
    DeclareGeIntVar(2, __y, home);
    try{
      Gecode::linear(home, __x, __r, __y, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  /*
    else if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3)) && OZ_isGeBoolVar(OZ_in(4))){
    DECLARE_INTARGS(0, __a);
    DECLARE_INTVARARGS(1, __x, home);
    DeclareIntRelType(2, __r);
    DeclareInt2(3, __c);
    DeclareGeBoolVar(4, __b, home);
    try{
      Gecode::linear(home, __a, __x, __r, __c, __b);
      called = true;
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
    }*/
  /* else if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3)) && OZ_isGeBoolVar(OZ_in(4))){
    DECLARE_INTARGS(0, __a);
    DECLARE_INTVARARGS(1, __x, home);
    DeclareIntRelType(2, __r);
    DeclareGeIntVar(3, __y, home);
    DeclareGeBoolVar(4, __b, home);
    try{
      Gecode::linear(home, __a, __x, __r, __y, __b);
      called = true;
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }*/
  else{
    return OZ_typeError(0, "Malformed Propagator");
  }

  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfd_linear_6,6,0){
  DeclareGSpace(home);

  Assert( OZ_isIntConLevel(OZ_in(4)) && OZ_isPropKind(OZ_in(5)));
  DeclareIntConLevel(4, __ICL_DEF);
  DeclarePropKind(5, __PK_DEF);



  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3)) ){
    /*  linear(Space* home, const IntVarArgs& x,
         IntRelType r, int c, BoolVar b, 
         IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
    */
    DECLARE_INTVARARGS(0, __x, home);
    DeclareIntRelType(1, __r);
    DeclareInt2(2, __c);
    DeclareGeBoolVar(3, __b, home);
    try{
      Gecode::linear(home, __x, __r, __c, __b, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3)) ){
    /*
       linear(Space* home, const IntVarArgs& x,
         IntRelType r, IntVar y, BoolVar b, 
         IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */
    DECLARE_INTVARARGS(0, __x, home);
    DeclareIntRelType(1, __r);
    DeclareGeIntVar(2, __y, home);
    DeclareGeBoolVar(3, __b, home);
    try{
      Gecode::linear(home, __x, __r, __y, __b, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3)) ){
    /*
       linear(Space* home, const IntArgs& a, const IntVarArgs& x,
         IntRelType r, int c,
         IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */
    DECLARE_INTARGS(0, __a);
    DECLARE_INTVARARGS(1, __x, home);
    DeclareIntRelType(2, __r);
    DeclareInt2(3, __c);
    try{
      Gecode::linear(home, __a, __x, __r, __c, __ICL_DEF, __PK_DEF);

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
    DECLARE_INTARGS(0, __a);
    DECLARE_INTVARARGS(1, __x, home);
    DeclareIntRelType(2, __r);
    DeclareGeIntVar(3, __y, home);
    try{
      Gecode::linear(home, __a, __x, __r, __y, __ICL_DEF, __PK_DEF);

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
  DECLARE_INTARGS(0, __a);
  DECLARE_INTVARARGS(1, __x, home);
  DeclareIntRelType(2, __r);
  DeclareGeBoolVar(4, __b, home);
  
  Assert(OZ_isIntConLevel(OZ_in(5)) && OZ_isPropKind(OZ_in(6)));

  DeclareIntConLevel(5, __ICL_DEF);
  DeclarePropKind(6, __PK_DEF);



  if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3)) && OZ_isGeBoolVar(OZ_in(4))){
    /*
       linear(Space* home, const IntArgs& a, const IntVarArgs& x,
         IntRelType r, int c, BoolVar b,
         IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */
    DeclareInt2(3, __c);
    try{
      Gecode::linear(home, __a, __x, __r, __c, __b, __ICL_DEF, __PK_DEF);

    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3)) && OZ_isGeBoolVar(OZ_in(4))){
    /*
      linear(Space* home, const IntArgs& a, const IntVarArgs& x,
         IntRelType r, IntVar y, BoolVar b,
         IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
     */
    DeclareGeIntVar(3, __y, home);
    try{
      Gecode::linear(home, __a, __x, __r, __y, __b, __ICL_DEF, __PK_DEF);

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

 /*
OZ_BI_define(gfd_linear_3,3,0){
  DeclareGSpace(home);
  DECLARE_INTVARARGS(0, __x, home);
  DeclareIntRelType(1, __r);
  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2))){
    DeclareInt2(2, __c);
    try{
      Gecode::linear(home, __x, __r, __c);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2))){
    DeclareGeIntVar(2, __y, home);
    try{
      Gecode::linear(home, __x, __r, __y);
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
 */
 /*
OZ_BI_define(gfd_linear_4,4,0){
  DeclareGSpace(home);
  if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3))){
    DECLARE_INTVARARGS(0, __x, home);
    DeclareIntRelType(1, __r);
    DeclareInt2(2, __c);
    DeclareGeBoolVar(3, __b, home);
    try{
      Gecode::linear(home, __x, __r, __c, __b);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3))){
    DECLARE_INTVARARGS(0, __x, home);
    DeclareIntRelType(1, __r);
    DeclareGeIntVar(2, __y, home);
    DeclareGeBoolVar(3, __b, home);
    try{
      Gecode::linear(home, __x, __r, __y, __b);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isInt(OZ_in(3))){
    DECLARE_INTARGS(0, __a);
    DECLARE_INTVARARGS(1, __x, home);
    DeclareIntRelType(2, __r);
    DeclareInt2(3, __c);
    try{
      Gecode::linear(home, __a, __x, __r, __c);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3))){
    DECLARE_INTARGS(0, __a);
    DECLARE_INTVARARGS(1, __x, home);
    DeclareIntRelType(2, __r);
    DeclareGeIntVar(3, __y, home);
    try{
      Gecode::linear(home, __a, __x, __r, __y);
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
 */
#endif
