/*
 *  Main authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co>
 *
 *  Contributing authors:
 *     Alejandro Arbelaez <aarbelaez@puj.edu.co>
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


#ifndef __GEOZ_INT_VAR_BUILTINS_CC__
#define __GEOZ_INT_VAR_BUILTINS_CC__

#include "GeIntVar-builtins.hh"


using namespace Gecode;
using namespace Gecode::Int;


/** 
 * \brief Creates a new IntVar variable 
 * 
 * @param 0 The space
 * @param 1 Domain description
 * @param 2 The new variable
 */
OZ_BI_define(new_intvar,1,1)
{
  //  DECLARE_SPACE(sp,0);      // the GenericSpace of the var
  //printf("antes de declaración\n");fflush(stdout);
  DECLARE_INT_SET(dom,0);   // the domain of the IntVar
  //printf("despues de declaración new_\n");fflush(stdout);
  OZ_RETURN(new_GeIntVar(dom));
}
OZ_BI_end

/** 
 * \brief Test whether \a OZ_in(0) is a GeIntVar
 * 
 */
OZ_BI_define(intvar_is,1,1)
{
  OZ_RETURN_BOOL(OZ_isGeIntVar(OZ_in(0)));
}
OZ_BI_end

/** 
 * \brief Returns the GeSpace where OZ_in(0) belongs to
 * 
 */
/*
OZ_BI_define(intvar_space,1,1)
  {
    Board *CBB = extVar2Var(get_GeIntVar(OZ_in(0)))->getBoardInternal();
    Board *sd = new Board(CBB);

    //OZ_result(makeTaggedConst(new Space(CBB,sd)));
    OZ_result(makeTaggedConst(NULL));
    return BI_PREEMPT;
    //OZ_RETURN(OZ_extension(get_GeIntVar(OZ_in(0))->getGeSpace()));
  }
OZ_BI_end
*/

/** 
 * \brief Returns the minimum elemen in the domain
 * 
 * @param intvar_getMin 
 * @param 0 A reference to the variable 
 * @param 1 The minimum of the domain 
 */
OZ_BI_define(intvar_getMin,1,1)
{
  OZ_RETURN_INT(get_IntVar(OZ_in(0)).min());
}
OZ_BI_end

/** 
 * \brief Returns the maximum elemen in the domain
 * 
 * @param intvar_getMin 
 * @param 0 A reference to the variable 
 * @param 1 The maximum of the domain 
 */
OZ_BI_define(intvar_getMax,1,1)
{
  OZ_RETURN_INT(get_IntVar(OZ_in(0)).max());
}
OZ_BI_end

/** 
 * \brief Returns the size of the domain
 * 
 * @param OZ_in(0) A reference to the variable 
 * @param OZ_out(0) The domain size
 */
OZ_BI_define(intvar_getSize,1,1)
{
  OZ_RETURN_INT(get_IntVar(OZ_in(0)).size());
}
OZ_BI_end


/** 
 * \brief Returns the median of the domain
 * 
 * @param OZ_in(0) A reference to the variable 
 * @param OZ_out(0) The median
 */
OZ_BI_define(intvar_getMed,1,1)
{
  OZ_RETURN_INT(get_IntVar(OZ_in(0)).med());
}
OZ_BI_end


/** 
 * \brief Returns the width of the domain
 * 
 * @param OZ_in(0) A reference to the variable 
 * @param OZ_out(0) The width
 */
OZ_BI_define(intvar_getWidth,1,1)
{
  OZ_RETURN_INT(get_IntVar(OZ_in(0)).width());
}
OZ_BI_end

/** 
 * \brief Returns the Regret Min of the domain
 * 
 * @param OZ_in(0) A reference to the variable 
 * @param OZ_out(0) The Regret Min
 */
OZ_BI_define(intvar_getRegretMin,1,1)
{
  OZ_RETURN_INT(IntView(get_IntVar(OZ_in(0))).regret_min());
}
OZ_BI_end

/** 
 * \brief Returns the Regret Max of the domain
 * 
 * @param OZ_in(0) A reference to the variable 
 * @param OZ_out(0) The Regret Max
 */
OZ_BI_define(intvar_getRegretMax,1,1)
{
  OZ_RETURN_INT(IntView(get_IntVar(OZ_in(0))).regret_max());
}
OZ_BI_end

/** 
 * \brief Creates a branching strategy 
 * 
 * @param 0 List of variables
 * @param 1 Variable selection strategy
 * @param 2 Value selection strategy
 */
OZ_BI_define(intvar_branch,3,0)
{ 
  OZ_declareInt(1,var_sel);
  OZ_declareInt(2,val_sel);
  DeclareGSpace(sp);
  DECLARE_INTVARARRAY(sp,vars,0);
  try{
    branch(sp,vars, (BvarSel) var_sel,(BvalSel) val_sel);
  }catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
   }
  return PROCEED;
}
OZ_BI_end


#endif
