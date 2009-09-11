
/*
 *  Main authors:
 *     Alejandro Arbelaez <aarbelaez@cic.puj.edu.co>
 *
 *  Contributing authors:
 *		Andres Felipe Barco <anfelbar@univalle.edu.co>
 *		Victor Rivera Zuniga <varivera@javerianacali.edu.co>
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
  IntView v1 = intOrIntView(OZ_in(0));
  IntView v3 = intOrIntView(OZ_in(2));
  
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
  DeclareGSpace(sp);
  OZ_declareInt(1,i2);
  OZ_declareInt(3,i4);
  IntView v1 = intOrIntView(OZ_in(0));
  IntView v3 = intOrIntView(OZ_in(2));
  BoolView v5 = boolOrBoolView(OZ_in(4));
    
  try{
    DisjointC(sp,v1,i2,v3,i4,v5);    
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }

  return PROCEED;

} OZ_BI_end

OZ_BI_define(int_watch_min,3,0)
{
  DeclareGSpace(sp);
  IntView v1 = intOrIntView(OZ_in(0));
  IntView v2 = intOrIntView(OZ_in(1));

  OZ_declareInt(2,v3);
  try {

    WatchMin(sp,v1,v2,v3);
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);    
  }
  return PROCEED;

} 
OZ_BI_end


/**
 * \brief the same that FD.watch.max in mozart
 * @param 0 A reference to the variable
 * @param 1 A reference to the variable (BoolVar)
 * @param 2 Integer
 */

OZ_BI_define(int_watch_max,3,0)
{
  DeclareGSpace(sp);
  IntView v1 = intOrIntView(OZ_in(0));
  IntView v2 = intOrIntView(OZ_in(1));

  OZ_declareInt(2,v3);
  try {

    WatchMax(sp,v1,v2,v3);
  }
  catch(Exception e) {
    return OZ_raiseC("prop: watch size",0);
  }
  return PROCEED;
  
} OZ_BI_end

/**
 * \brief the same that FD.watch.size in mozart
 * @param 0 A reference to the variable
 * @param 1 A reference to the variable (BoolVar)
 * @param 2 Integer
 */

OZ_BI_define(int_watch_size,3,0)
{

  DeclareGSpace(sp);
  IntView v1 = intOrIntView(OZ_in(0));
  IntView v2 = intOrIntView(OZ_in(1));

  OZ_declareInt(2,v3);
  try {
    WatchSize(sp,v1,v2,v3);
  }
  catch(Exception e) {
    return OZ_raiseC("prop: watch size",0);
  }
  return PROCEED;

} OZ_BI_end

//#include "libfd/fdaux.hh"

OZ_BI_define(int_sumCN,4,0)
{
  IntArgs x0 = getIntArgs(OZ_in(0));
  IntRelType relType = getIntRelType(OZ_in(2));
  DeclareGSpace(sp);

  OZ_Term D = OZ_deref(OZ_in(1));
  int tamD;
  
  OZ_Term *Vec = vectorToOzTerms2(D,tamD);

  IntView Res = intOrIntView(OZ_in(3));
  IntVarArray Arreglo(sp,tamD, Int::Limits::min,Int::Limits::max);

  linear(sp,x0,Arreglo,relType,Res,ICL_VAL);

  for(int i=0;i<tamD;i++) {
    int tamD2;
    OZ_Term *Vec2 = vectorToOzTerms2(Vec[i],tamD2);
    IntVarArray ArregloTmp(sp,tamD2,Int::Limits::min,Int::Limits::max);
    rel(sp,Arreglo[i],IRT_EQ,ArregloTmp[tamD2-1],ICL_VAL);
    //IntVar *ValVec2 = new IntVar();
    IntView ValVec2;
    OZ_Term val = Vec2[0];

    if(OZ_isInt(val)) {
      int domain = OZ_intToC(val);
      IntVar *tmp = new IntVar(sp,domain,domain);
      IntView vwTmp(*tmp);
      delete tmp;
      ValVec2 = vwTmp;
    }
    else {
      IntView vwTmp = get_IntView(OZ_deref(val));
      ValVec2 = vwTmp;
    }
    
    rel(sp,ArregloTmp[0],IRT_EQ,ValVec2,ICL_VAL);

    rel(sp,ArregloTmp[0],IRT_EQ,ValVec2,ICL_VAL);

    for(int j=1;j<tamD2;j++) {
      IntView Tmpj;
      if(OZ_isGeIntVar(Vec2[j])) {
	Tmpj = get_IntView(Vec2[j]);
      }
      else if(OZ_isInt(Vec2[j])) {
	int domain = OZ_intToC(Vec2[j]);
	IntVar *tmp = new IntVar(sp,domain,domain);
	IntView vwTmp(*tmp);
	delete tmp;
	Tmpj = vwTmp;
      }
      else OZ_typeError(0,"Elements inside of OZ_vector must be GeIntVar or Int: sumCN");
      mult(sp,ArregloTmp[j-1],Tmpj,ArregloTmp[j],ICL_VAL);
    }
  }
  return PROCEED;
} OZ_BI_end


/*##################### Reified Implementation ############################*/

/**
	* This method makes a scalar produt between two list or a list and matrix.
	* This method will be use to these reified constraints, they are not implemented by Gecode.
	*/
IntVar scalarProduct(GenericSpace *sp, IntArgs ia, IntVarArgs iva, IntConLevel __ICL_DEF){
  IntVarArray arrayTmp(sp,iva.size(),Int::Limits::min,Int::Limits::max);
  for (int i = 0; i < iva.size();i++){
    IntVar valVar;
    int domain = ia[i];
    valVar.init(sp,domain,domain);
    Gecode::mult(sp,valVar,iva[i],arrayTmp[i],__ICL_DEF);
  }
  IntVar ScalarProduct;
  ScalarProduct.init(sp,Int::Limits::min,Int::Limits::max);
  Gecode::linear(sp,arrayTmp,IRT_EQ,ScalarProduct,__ICL_DEF);
  return ScalarProduct;
}

IntVar scalarProduct(GenericSpace *sp, IntArgs ia, OZ_Term Dvv, IntConLevel __ICL_DEF){
  int lenD;
  OZ_Term *Mat = vectorToOzTerms2(Dvv,lenD);
  
  //if (lenD != ia.size()){
  //IntVar tmp;
  //return (IntVar)NULL;
  //}
  
  /**
   *arrayScaPro -> Each position of this vector contains ia[i]*Dv[i]; 0<= i <= ia.size()
   *Add each position of arrayScaPro will get the scalar product 
   */
  IntVarArray arrayScaPro(sp,ia.size(),Int::Limits::min,Int::Limits::max);
  
  for (int i=0; i<ia.size();i++){
    int lenV;
    OZ_Term *Vec = vectorToOzTerms2(Mat[i],lenV); //Vec -> Represents Dv[i]
    
    IntVar accValue(sp,Int::Limits::min,Int::Limits::max);
    //accValue.init(sp,Int::Limits::min,Int::Limits::max);
    
    /**
     * arrayTmp -> In its last position constains multiplication of each value of Dv
     */
    IntVarArray arrayTmp(sp,lenV+1,Int::Limits::min,Int::Limits::max);
    IntVar tmp(sp,1,1);
    //tmp.init(sp,1,1);
    arrayTmp[0] = tmp;
    
    for (int j=0; j<lenV;j++){
      IntView valVec = get_IntView(OZ_deref(Vec[j]));
      Gecode::mult (sp, valVec, arrayTmp[j], arrayTmp[j+1]	, __ICL_DEF);
    }
    tmp.init(sp,ia[i],ia[i]);
    Gecode::mult (sp, arrayTmp[lenV], tmp, arrayScaPro[i], __ICL_DEF);
  }
  IntVar ScalarProduct;
  ScalarProduct.init(sp,Int::Limits::min,Int::Limits::max);
  Gecode::linear(sp,arrayScaPro,IRT_EQ,ScalarProduct,__ICL_DEF);
  return ScalarProduct;
}


/**
	*  Implementation of reified.sumAC constraint using mult, abs, linear and rel constraint of gecode
*/
OZ_BI_define(reified_sumAC,6,0)
{
  DeclareGSpace(sp);
  
  if(OZ_isIntArgs(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntRelType(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3)) && OZ_isGeBoolVar(OZ_in(4))){
    IntArgs ia = getIntArgs(OZ_in(0));
    IntVarArgs iva = getIntVarArgs(OZ_in(1));
    IntRelType relType = getIntRelType(OZ_in(2));
    IntView D1 = intOrIntView(OZ_in(3));
    BoolView D2 = boolOrBoolView(OZ_in(4));
    Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(5));
    
    if (ia.size() != iva.size())
      return OZ_typeError(0, "Vectors must have same size: reified.sumAC");
    
    IntVar ScalarProduct = scalarProduct(sp,ia,iva, __ICL_DEF);
    IntVar tmpD;
    tmpD.init(sp,Int::Limits::min,Int::Limits::max);
    try {
      /**
	 abs (Space *home, IntVar x0, IntVar x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
      */
      Gecode::abs(sp,ScalarProduct,tmpD,ICL_VAL);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
    try {
      /**
	 rel (Space *home, IntVar x0, IntRelType r, IntVar x1, BoolVar b, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
      */
      Gecode::rel (sp, tmpD, relType, D1, D2, __ICL_DEF);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    return OZ_typeError(0, "Malformed Propagator: reified.sumAC");
  }
  
  CHECK_POST(sp);
} OZ_BI_end

/**
	*  Implementation of reified.sumCN constraint using mult, abs, linear and rel constraint of gecode
*/
OZ_BI_define(reified_sumCN,6,0)
{
  DeclareGSpace(sp);
  IntArgs ia = getIntArgs(OZ_in(0));
  OZ_Term Dvv = OZ_deref(OZ_in(1));
  IntRelType relType = getIntRelType(OZ_in(2));
  IntView D1 = intOrIntView(OZ_in(3));
  BoolView D2 = boolOrBoolView(OZ_in(4));
  Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(5));

  IntVar ScalarProduct = scalarProduct(sp, ia, Dvv, __ICL_DEF);
  if (ScalarProduct.size() == 0){
    return OZ_typeError(0, "Vectors must have same size: reified.sumCN");
  }
  
  Gecode::rel (sp, ScalarProduct, relType, D1, D2, __ICL_DEF);
  
  CHECK_POST(sp);
} OZ_BI_end

/**
	*  Implementation of reified.sumACN constraint using mult, abs, linear and rel constraint of gecode
*/
OZ_BI_define(reified_sumACN,6,0)
{
  DeclareGSpace(sp);
  IntArgs ia = getIntArgs(OZ_in(0));
  OZ_Term Dvv = OZ_deref(OZ_in(1));
  IntRelType relType = getIntRelType(OZ_in(2));
  IntVar D1 = intOrIntView(OZ_in(3));
  BoolView D2 = boolOrBoolView(OZ_in(4));
  Gecode::IntConLevel __ICL_DEF = getIntConLevel(OZ_in(5));
  
  IntVar ScalarProduct = scalarProduct(sp, ia, Dvv, __ICL_DEF);
  //if (ScalarProduct )
  //		return OZ_typeError(0, "Vectors must have same size: reified.sumACN");
  
  IntVar ABSsp;
  ABSsp.init(sp,Int::Limits::min,Int::Limits::max);
  Gecode::abs(sp,ScalarProduct,ABSsp,ICL_VAL);
  
  Gecode::rel (sp, ABSsp, relType, D1, D2, __ICL_DEF);

  CHECK_POST(sp);
} OZ_BI_end


OZ_BI_define(reified_dom,3,0){
  /*
   *
   *  This contraint is not implemented yet
   *
   */
  
  DeclareGSpace(sp);
  //if (OZ_isPair(OZ_in(0))){
  //	printf("PAIR\n");fflush(stdout);
  //	}
  if (OZ_isInt(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2))){
    int Spec = OZ_intToC(OZ_in(0));
    IntVarArgs iva = getIntVarArgs(OZ_in(1));
    BoolView D1 = boolOrBoolView(OZ_in(2));
    
    BoolVarArray tmpArray(sp, iva.size(), 0,1);
    for (int i = 0; i < iva.size(); i++){
      IntSet set(iva[i].min(),iva[i].max());
      IntVar valVar;
      valVar.init(sp,Spec,Spec);
      /**
	 dom (Space *home, IntVar x, const IntSet &s, BoolVar b, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
      */
      try{
	Gecode::dom(sp,valVar,set,tmpArray[i],ICL_VAL);
      }
      catch (Exception e){
	RAISE_GE_EXCEPTION(e);
      }
    }
    BoolVar res;
    res.init(sp,0,1);
    /**
       rel (Space *home, const BoolVarArgs &x, IntRelType r, BoolVar y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF)
    */
    try{
      Gecode::rel(sp,tmpArray,IRT_EQ,res,ICL_VAL);
    }
    catch (Exception e){
      RAISE_GE_EXCEPTION(e);
    }
    Gecode::rel(sp,res,IRT_EQ,D1,ICL_VAL);
  }
  else{
    return OZ_typeError(0, "Malformed Propagator: reified.dom");
  }
  CHECK_POST(sp);
} OZ_BI_end


OZ_BI_define(gfd_reifiedCard, 4, 0){
  
  
  DeclareGSpace(home);
  IntView ew1 = intOrIntView(OZ_in(0));
  BoolVarArgs bva = getBoolVarArgs(OZ_in(1));
  IntView ew2 = intOrIntView(OZ_in(2));
  BoolView bv = boolOrBoolView(OZ_in(3));
  
  ViewArray<BoolView> varr(home, bva);
  
  try {
    ReifiedCard(home, ew1, varr, ew2, bv);
  } catch (Exception e){
      RAISE_GE_EXCEPTION(e);
  }


  return PROCEED;

} OZ_BI_end


#endif
