/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#include "builtins.hh"
#include "space.hh"
#include "cpi.hh"
#include "var_fs.hh"

//*****************************************************************************

#define FSETDESCR_SYNTAX						\
"The syntax of a " OZ_EM_FSETDESCR " is:\n"				\
"   set_descr   ::= simpl_descr | compl(simpl_descr)\n"			\
"   simpl_descr ::= range_descr | nil | [range_descr+]\n"		\
"   range_descr ::= integer | integer#integer\n"			\
"   integer     ::= {" _OZ_EM_FSETINF ",...," _OZ_EM_FSETSUP "}"

//*****************************************************************************

OZ_BI_define(BIfsValueToString, 1,1)
{
  OZ_declareDetTerm(0,in);

  if (oz_isFSetValue(in)) {
    char *s = OZ_toC(in,100,100); // mm2
    OZ_RETURN_STRING(s);
  }
  oz_typeError(0,"FSetValue");
} OZ_BI_end

OZ_BI_define(BIfsIsVarB, 1,1)
{
  OZ_RETURN(oz_bool(isGenFSetVar(oz_deref(OZ_in(0)))));
} OZ_BI_end

OZ_C_proc_begin(BIfsIsValueB, 2)
{
  OZ_Term term = OZ_args[0];
  DEREF(term, term_ptr, term_tag);
  if (isVariableTag(term_tag)) 
    return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, term_ptr);

  return OZ_unify(OZ_getCArg (1),
		  oz_bool(oz_isFSetValue(oz_deref(OZ_getCArg(0)))));
}
OZ_C_proc_end

void
makeFSetValue(OZ_Term desc,OZ_Term*fs)
{
  *fs = makeTaggedFSetValue(new FSetValue(desc));
}

OZ_C_proc_begin(BIfsSetValue, 2) 
{
  ExpectedTypes(OZ_EM_FSETDESCR "," OZ_EM_FSETVAL);

  ExpectOnly pe;
  
  EXPECT_BLOCK(pe, 0, expectFSetDescr, FSETDESCR_SYNTAX);

  return OZ_unify(OZ_getCArg(1),
		  makeTaggedFSetValue(new FSetValue(OZ_getCArg(0))));
}
OZ_C_proc_end

OZ_C_proc_begin(BIfsSet, 3) 
{
  ExpectedTypes(OZ_EM_FSETDESCR "," OZ_EM_FSETDESCR "," OZ_EM_FSET);
  
  ExpectOnly pe;
  
  EXPECT_BLOCK(pe, 0, expectFSetDescr, FSETDESCR_SYNTAX);
  EXPECT_BLOCK(pe, 1, expectFSetDescr, FSETDESCR_SYNTAX);
  
  FSetConstraint fset(OZ_getCArg(0), OZ_getCArg(1));

  if (! fset.isValid()) {
    TypeError(2, "Invalid set description");
    return FAILED;
  }

  return tellBasicConstraint(OZ_getCArg(2), &fset);
}
OZ_C_proc_end

OZ_BI_define(BIfsSup, 0,1)
{
  OZ_RETURN_INT(fset_sup);
} OZ_BI_end

OZ_C_proc_begin(BIfsClone, 2) 
{
  ExpectedTypes(OZ_EM_FSET "," OZ_EM_FSET);
  
  ExpectOnly pe;
  
  EXPECT_BLOCK(pe, 0, expectFSetVar, "");
  
  DEREF(OZ_getCArg(0), arg0ptr, arg0tag);

  return arg0tag == FSETVALUE ? OZ_unify(OZ_getCArg(0), OZ_getCArg(1))
    : tellBasicConstraint(OZ_getCArg(1), 
			  (FSetConstraint *) &tagged2GenFSetVar(oz_deref(OZ_getCArg(0)))->getSet());
}
OZ_C_proc_end

//-----------------------------------------------------------------------------

OZ_C_proc_begin(BIfsGetKnownIn, 2) 
{
  ExpectedTypes(OZ_EM_FSET ","  OZ_EM_FSETDESCR);
  
  OZ_Term v = OZ_getCArg(0);
  DEREF(v, vptr, vtag);

  if (isFSetValueTag(vtag)) {
    OZ_FSetValue * fsetval = tagged2FSetValue(v);
    return OZ_unify(OZ_getCArg(1), fsetval->getKnownInList());
  } else if (isGenFSetVar(v, vtag)) {
    OZ_FSetConstraint * fsetconstr = &tagged2GenFSetVar(v)->getSet();
    return OZ_unify(OZ_getCArg(1), fsetconstr->getKnownInList());
  } else if (oz_isNonKinded(v)) {
    return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  } 
  TypeError(0, "");
}
OZ_C_proc_end

OZ_C_proc_begin(BIfsGetNumOfKnownIn, 2) 
{
  ExpectedTypes(OZ_EM_FSET "," OZ_EM_INT);
  
  OZ_Term v = OZ_getCArg(0);
  DEREF(v, vptr, vtag);

  if (isFSetValueTag(vtag)) {
    OZ_FSetValue * fsetval = tagged2FSetValue(v);
    return OZ_unify(OZ_getCArg(1), OZ_int(fsetval->getCard()));
  } else if (isGenFSetVar(v, vtag)) {
    OZ_FSetConstraint * fsetconstr = &tagged2GenFSetVar(v)->getSet();
    return OZ_unify(OZ_getCArg(1), OZ_int(fsetconstr->getKnownIn()));
  } else if (oz_isNonKinded(v)) {
    return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  } 
  TypeError(0, "");
}
OZ_C_proc_end

//-----------------------------------------------------------------------------

OZ_C_proc_begin(BIfsGetKnownNotIn, 2) 
{
  ExpectedTypes(OZ_EM_FSET ","  OZ_EM_FSETDESCR);
  
  OZ_Term v = OZ_getCArg(0);
  DEREF(v, vptr, vtag);

  if (isFSetValueTag(vtag)) {
    OZ_FSetValue * fsetval = tagged2FSetValue(v);
    return OZ_unify(OZ_getCArg(1), fsetval->getKnownNotInList());
  } else if (isGenFSetVar(v, vtag)) {
    OZ_FSetConstraint * fsetconstr = &tagged2GenFSetVar(v)->getSet();
    return OZ_unify(OZ_getCArg(1), fsetconstr->getKnownNotInList());
  } else if (oz_isNonKinded(v)) {
    return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  } 
  TypeError(0, "");
}
OZ_C_proc_end

OZ_C_proc_begin(BIfsGetNumOfKnownNotIn, 2) 
{
  ExpectedTypes(OZ_EM_FSET "," OZ_EM_INT);
  
  OZ_Term v = OZ_getCArg(0);
  DEREF(v, vptr, vtag);

  if (isFSetValueTag(vtag)) {
    OZ_FSetValue * fsetval = tagged2FSetValue(v);
    return OZ_unify(OZ_getCArg(1), OZ_int(fsetval->getKnownNotIn()));
  } else if (isGenFSetVar(v, vtag)) {
    OZ_FSetConstraint * fsetconstr = &tagged2GenFSetVar(v)->getSet();
    return OZ_unify(OZ_getCArg(1), OZ_int(fsetconstr->getKnownNotIn()));
  } else if (oz_isNonKinded(v)) {
    return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  } 
  TypeError(0, "");
}
OZ_C_proc_end

//-----------------------------------------------------------------------------

OZ_C_proc_begin(BIfsGetUnknown, 2) 
{
  ExpectedTypes(OZ_EM_FSET "," OZ_EM_FSETDESCR);
  
  OZ_Term v = OZ_getCArg(0);
  DEREF(v, vptr, vtag);

  if (isFSetValueTag(vtag)) {
    return OZ_unify(OZ_getCArg(1), OZ_nil());
  } else if (isGenFSetVar(v, vtag)) {
    OZ_FSetConstraint * fsetconstr = &tagged2GenFSetVar(v)->getSet();
    return OZ_unify(OZ_getCArg(1), fsetconstr->getUnknownList());
  } else if (oz_isNonKinded(v)) {
    return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  } 
  TypeError(0, "");
}
OZ_C_proc_end

OZ_C_proc_begin(BIfsGetNumOfUnknown, 2) 
{
  ExpectedTypes(OZ_EM_FSET "," OZ_EM_INT);
  
  OZ_Term v = OZ_getCArg(0);
  DEREF(v, vptr, vtag);

  if (isFSetValueTag(vtag)) {
    return OZ_unify(OZ_getCArg(1), OZ_int(0));
  } else if (isGenFSetVar(v, vtag)) {
    OZ_FSetConstraint * fsetconstr = &tagged2GenFSetVar(v)->getSet();
    return OZ_unify(OZ_getCArg(1), OZ_int(fsetconstr->getUnknown()));
  } else if (oz_isNonKinded(v)) {
    return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  } 
  TypeError(0, "");
}
OZ_C_proc_end

//-----------------------------------------------------------------------------

OZ_C_proc_begin(BIfsGetLub, 2) 
{
  ExpectedTypes(OZ_EM_FSET "," OZ_EM_FSETDESCR);
  
  OZ_Term v = OZ_getCArg(0);
  DEREF(v, vptr, vtag);

  if (isFSetValueTag(vtag)) {
    OZ_FSetValue * fsetval = tagged2FSetValue(v);
    return OZ_unify(OZ_getCArg(1), fsetval->getKnownInList());
  } else if (isGenFSetVar(v, vtag)) {
    OZ_FSetConstraint * fsetconstr = &tagged2GenFSetVar(v)->getSet();
    return OZ_unify(OZ_getCArg(1), fsetconstr->getLubList());
  } else if (oz_isNonKinded(v)) {
    return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  } 
  TypeError(0, "");
}
OZ_C_proc_end

OZ_C_proc_begin(BIfsGetCard, 2) 
{
  ExpectedTypes(OZ_EM_FSET "," OZ_EM_INT);
  
  OZ_Term v = OZ_getCArg(0);
  DEREF(v, vptr, vtag);

  if (isFSetValueTag(vtag)) {
    OZ_FSetValue * fsetval = tagged2FSetValue(v);
    return OZ_unify(OZ_getCArg(1), OZ_int(fsetval->getCard()));
  } else if (isGenFSetVar(v, vtag)) {
    OZ_FSetConstraint * fsetconstr = &tagged2GenFSetVar(v)->getSet();
    return OZ_unify(OZ_getCArg(1), fsetconstr->getCardTuple());
  } else if (oz_isNonKinded(v)) {
    return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  } 
  TypeError(0, "");
}
OZ_C_proc_end

OZ_C_proc_begin(BIfsCardRange, 3) 
{
  ExpectedTypes(OZ_EM_INT "," OZ_EM_INT "," OZ_EM_FSET);
  
  int l = -1;
  {
    OZ_Term lt = OZ_getCArg(0);
    DEREF(lt, ltptr, lttag);
    
    if (isSmallIntTag(lttag)) {
      l = smallIntValue(lt);
    } else if (isVariableTag(lttag)) {
      return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, ltptr);
    } else {
      TypeError(0, "");
    }
  }

  int u = -1;
  {
    OZ_Term ut = OZ_getCArg(1);
    DEREF(ut, utptr, uttag);
    
    if (isSmallIntTag(uttag)) {
      u = smallIntValue(ut);
    } else if (isVariableTag(uttag)) {
      return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, utptr);
    } else {
      TypeError(1, "");
    }
  }

  if (l > u) 
    return FAILED;

  {
    OZ_Term v = OZ_getCArg(2);
    DEREF(v, vptr, vtag);
    
    if (isFSetValueTag(vtag)) {
      int card = tagged2FSetValue(v)->getCard();
      return ((l <= card) && (card <= u)) ? PROCEED : FAILED;
    } else if (isGenFSetVar(v, vtag)) {
      OZ_FSetConstraint * fsetconstr = &tagged2GenFSetVar(v)->getSet();
      if (!fsetconstr->putCard(l, u)) 
	return FAILED;
      /* a variable might have become a fset value because of 
	 imposing a card constraints */
      if (fsetconstr->isValue())
	tagged2GenFSetVar(v)->becomesFSetValueAndPropagate(vptr);
      return PROCEED; 
    } else if (oz_isNonKinded(v)) {
      return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
    } 
  }
  TypeError(2, "");
}
OZ_C_proc_end

