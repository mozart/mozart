/*
 *  Main authors:
 *     Alejandro Arbelaez <aarbelaez@cic.puj.edu.co>
 *
 *  Contributing authors:
 *		Andres Felipe Barco <anfelbar@univalle.edu.co>
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

#ifndef __GEOZ_MOZ_PROP_BUILTINS_CC__
#define __GEOZ_MOZ_PROP_BUILTINS_CC__

#include "IntVarMacros.hh"
#include "MozProp/MozProp.cc"




using namespace Gecode;
using namespace Gecode::Int;


/**
 * \brief the same that FD.disjoint in mozart
 * @param 0 A reference to the variable
 * @param 1 Integer
 * @param 2 A reference to the variable
 * @param 3 Integer
 */

OZ_BI_define(int_disjoint,4,0)
{
  OZ_declareInt(1,i2);
  OZ_declareInt(3,i4);

  DeclareGSpace(sp);
  DeclareGeIntVar(0,v1,sp);
  DeclareGeIntVar(2,v3,sp);

  try{
    Disjoint(sp,v1,i2,v3,i4);
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  return PROCEED;

} OZ_BI_end

/**
 * \brief the same that FD.disjointC
 * @param 0 A reference to the variable
 * @param 1 Integer
 * @param 2 A reference to the variable
 * @param 3 Integer
 * @param 4 A reference to the variable (BoolVar)
 */

OZ_BI_define(int_disjointC,5,0)
{

  OZ_declareInt(1,i2);
  OZ_declareInt(3,i4);
  GenericSpace *sp;
  DeclareGeIntVar(0,v1,sp);
  DeclareGeIntVar(2,v3,sp);

  DeclareGeIntVar(4,v5,sp);

  /*
  try{
    DisjointC(sp,v1,i2,v3,i4,v5);    
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);

    }*/

  return PROCEED;

} OZ_BI_end

//    {FD.reified.int +Spec *D1 D2}
/*OZ_BI_define(int_reified_int,2,1)
{
  GenericSpace *sp;
  //  cout<<"AQUI SI "<<endl;  fflush(stdout);
  if(OZ_isGeIntVar(OZ_in(1))) {
    sp = get_Space(OZ_in(1));
  }
  else RAISE_EXCEPTION("The second variable have to be GeInt");
  //  cout<<"Antes"<<endl; fflush(stdout);
  DECLARE_INT_SET(dom,0);
  //  cout<<"AQUI Si tiene que entrar"<<endl; fflush(stdout);
  //  cout<<"DomSize: "<<dom.size()<<endl; fflush(stdout);
  DeclareGeIntVar(1,v1,sp);
  IntVar v0(sp,dom);
  //  cout<<"antes del AAA"<<endl; fflush(stdout);
  //Esto esta mal
  OZ_Term Res = new_GeIntVar(OZ_in(0),dom);
  
  //  cout<<"despues de AAA"<<endl; fflush(stdout);
  IntVar v2 = get_IntVar(OZ_deref(Res));
  
  try {
  //    if(sp==NULL) cout<<"ESTA NULL"<<endl; fflush(stdout);
    //    cout<<"v0: "<<v0.min()<<" v1: "<<v1.min()<<" v2: "<<v2.min()<<endl; fflush(stdout);
    //    cout<<"Antes de imponer el propagador"<<endl; fflush(stdout);
    ReifiedInt(sp,v2,static_cast<BoolVar>(v0),v1);
    //    cout<<"Despues de imponer el propagador"<<endl; fflush(stdout);
  }
  catch(Exception e) {
  RAISE_GE_EXCEPTION(e);
  //return OZ_raiseC("int_reified_int: ",0);
  }
  OZ_RETURN(Res);
  
} OZ_BI_end
*/
/*This is a first approach of the sumCN, here we propose
  to simule the behavior of sumCN propagator(mozart-FD), using linear and mult propagators of gecode
*/

//#include "libfd/fdaux.hh"

OZ_BI_define(int_sumCN,4,0)
{
  DECLARE_INTARGS(0,x0);
  DeclareIntRelType(2,relType);
  //  IntVar Res;

  DeclareGSpace(sp);

  OZ_Term D = OZ_deref(OZ_in(1));
  int tamD;

  OZ_Term *Vec = vectorToOzTerms2(D,tamD);

  DeclareGeIntVar(3,Res,sp);

  IntVarArray Arreglo(sp,tamD, Int::Limits::min,Int::Limits::max);

  linear(sp,x0,Arreglo,relType,Res,ICL_VAL);

  for(int i=0;i<tamD;i++) {
    int tamD2;
    OZ_Term *Vec2 = vectorToOzTerms2(Vec[i],tamD2);
    IntVarArray ArregloTmp(sp,tamD2,Int::Limits::min,Int::Limits::max);
    rel(sp,Arreglo[i],IRT_EQ,ArregloTmp[tamD2-1],ICL_VAL);
    IntVar ValVec2;
    OZ_Term val = Vec2[0];

    if(OZ_isInt(val)) {
      int domain = OZ_intToC(val);
      ValVec2.init(sp,domain,domain);
    }
    else ValVec2 = get_IntVar(OZ_deref(val));

    rel(sp,ArregloTmp[0],IRT_EQ,ValVec2,ICL_VAL);

    rel(sp,ArregloTmp[0],IRT_EQ,ValVec2,ICL_VAL);

    for(int j=1;j<tamD2;j++) {
      IntVar Tmpj;
      if(OZ_isGeIntVar(Vec2[j])) {
	Tmpj = get_IntVar(Vec2[j]);
      }
      else if(OZ_isInt(Vec2[j])) {
	int domain = OZ_intToC(Vec2[j]);
	Tmpj.init(sp,domain,domain);
      }
      else OZ_typeError(0,"Elements inside of OZ_vector must be GeIntVar or Int: sumCN");
      mult(sp,ArregloTmp[j-1],Tmpj,ArregloTmp[j],ICL_VAL);
    }
  }
  return PROCEED;

} OZ_BI_end


// 0/1 propagators
OZ_BI_define(bool_Gand,4,0)
{
  DeclareGSpace(sp);
  DeclareGeIntVar(0,v0,sp);
  DeclareGeIntVar(1,v1,sp);
  DeclareGeIntVar(2,v2,sp);

  DeclareIntConLevel(3,conLevel);
  /*
  try {
    bool_and(sp,static_cast<BoolVar>(v0),static_cast<BoolVar>(v1),static_cast<BoolVar>(v2),conLevel);
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
    }*/

  return PROCEED;  
} OZ_BI_end

OZ_BI_define(bool_Gor,4,0)
{
  DeclareGSpace(sp);
  DeclareGeIntVar(0,v0,sp);
  DeclareGeIntVar(1,v1,sp);
  DeclareGeIntVar(2,v2,sp);
  DeclareIntConLevel(3,conLevel);
  /*
  try {
    bool_or(sp,static_cast<BoolVar>(v0),static_cast<BoolVar>(v1),static_cast<BoolVar>(v2),conLevel);
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
 }*/

  return PROCEED;    
} OZ_BI_end


OZ_BI_define(bool_Gxor,4,0)
{
  DeclareGSpace(sp);
  DeclareGeIntVar(0,v0,sp);
  DeclareGeIntVar(1,v1,sp);
  DeclareGeIntVar(2,v2,sp);
  DeclareIntConLevel(3,conLevel);
  /*
  try {
    bool_xor(sp,static_cast<BoolVar>(v0),static_cast<BoolVar>(v1),static_cast<BoolVar>(v2),conLevel);
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
    }*/

  return PROCEED;

} OZ_BI_end
 

OZ_BI_define(bool_Gnot,3,0)
{
  DeclareGSpace(sp);
  DeclareGeIntVar(0,v0,sp);
  DeclareGeIntVar(1,v1,sp);
  DeclareIntConLevel(2,conLevel);
  /*
  try {
    bool_not(sp,static_cast<BoolVar>(v0),static_cast<BoolVar>(v1),conLevel);
  }
  catch(Exception e) {
    //    return OZ_raiseC("prop: bool_not",0);
    RAISE_GE_EXCEPTION(e);
    }*/

  return PROCEED;
} OZ_BI_end

OZ_BI_define(bool_Gimp,4,0)
{
  DeclareGSpace(sp);
  DeclareGeIntVar(0,v0,sp);
  DeclareGeIntVar(1,v1,sp);
  DeclareGeIntVar(2,v2,sp);
  DeclareIntConLevel(3,conLevel);
/*

  try {
    bool_imp(sp,static_cast<BoolVar>(v0),static_cast<BoolVar>(v1),static_cast<BoolVar>(v2),conLevel);
    return PROCEED;
  }
  catch(Exception e) {
    return OZ_raiseC("prop: bool_imp",0);
    }*/
  return PROCEED;
} OZ_BI_end


OZ_BI_define(bool_Geqv,4,0)
{
  DeclareGSpace(sp);
  DeclareGeIntVar(0,v0,sp);
  DeclareGeIntVar(1,v1,sp);
  DeclareGeIntVar(2,v2,sp);
  DeclareIntConLevel(3,conLevel);
  /*
  try {
    bool_eqv(sp,static_cast<BoolVar>(v0),static_cast<BoolVar>(v1),static_cast<BoolVar>(v2),conLevel);
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
    }*/
  return PROCEED;

} OZ_BI_end

#endif
