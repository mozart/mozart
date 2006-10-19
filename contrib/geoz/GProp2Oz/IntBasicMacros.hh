/*
 *  Main authors:
 *     Javier A. Mena <javimena@univalle.edu.co>    
 *
 *  Contributing authors:
 *     
 *
 *  Copyright:
 *     Javier A. Mena, 2006    
 *
 *  Last modified:
 *     $Date: 2006-07-19 15:02:55 -0500 (mié, 19 jul 2006) $
 *     $Revision: 156 $
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

#if !defined(__INT_BASIC_MACROS__INCLUDE__)
#define __INT_BASIC_MACROS__INCLUDE__

#include <GeIntVar-builtins.hh>
#include <int.hh>

#define DECLARE_GE_INT_VAR(VAR_NAME,CURR_SPACE,OZ_PARAM_NUMBER)   \
   GeIntVar* __giv_##VAR_NAME = get_GeIntVar(OZ_deref(OZ_in(OZ_PARAM_NUMBER))); \
   Gecode::IntVar& VAR_NAME = __giv_##VAR_NAME->getIntVar();      \
   if (CURR_SPACE==NULL) {                                        \
     CURR_SPACE = __giv_##VAR_NAME->getGeSpace();                 \
   }                                                              \
   assert(CURR_SPACE==__giv_##VAR_NAME->getGeSpace());


#define DECLARE_GE_INT_VAR_UPCAST(VAR_NAME,CURR_SPACE,OZ_PARAM_NUMBER)   \
  OZ_Term __##VAR_NAME = OZ_deref(OZ_in(OZ_PARAM_NUMBER));  \
  Gecode::IntVar VAR_NAME;                                  \
  if (OZ_isInt(__##VAR_NAME)) {                             \
    if (CURR_SPACE == NULL) {                               \
      RAISE_EXCEPTION("Cannot initialize Space");           \
    }                                                       \
    int value = OZ_intToC(OZ_in(OZ_PARAM_NUMBER));          \
    VAR_NAME = Gecode::IntVar(CURR_SPACE->getSpace(),value,value); \
  }                                                         \
  else {                                                    \
   GeIntVar* __giv_##VAR_NAME = get_GeIntVar(OZ_deref(OZ_in(OZ_PARAM_NUMBER))); \
   if (CURR_SPACE==NULL) {                                  \
     CURR_SPACE = __giv_##VAR_NAME->getGeSpace();           \
   }                                                        \
   assert(CURR_SPACE==__giv_##VAR_NAME->getGeSpace());      \
   VAR_NAME = __giv_##VAR_NAME->getIntVar();                \
  }


#define DECLARE_INT_CON_LEVEL(OZ_PARAM_NUMBER,VAR_NAME)     \
  OZ_declareInt(OZ_PARAM_NUMBER,__##VAR_NAME);              \
  Gecode::IntConLevel VAR_NAME = static_cast<Gecode::IntConLevel>(__##VAR_NAME);


#define DECLARE_INT_ARGS(VAR_NAME,OZ_PARAM_NUMBER)                   \
  OZ_declareTerm(OZ_PARAM_NUMBER,__##VAR_NAME)                       \
  bool isList_##VAR_NAME = OZ_isList(__##VAR_NAME,NULL);             \
  if (!(OZ_isTuple(__##VAR_NAME) || OZ_isList(__##VAR_NAME,NULL))) { \
    RAISE_EXCEPTION("Error D1: var " #OZ_PARAM_NUMBER " not tuple nor list");   \
  }                                                                  \
  Gecode::IntArgs VAR_NAME(isList_##VAR_NAME ? OZ_length(__##VAR_NAME) : OZ_width(__##VAR_NAME));   \
  if (isList_##VAR_NAME) {                                           \
    int i=0;                                                         \
    for (OZ_Term ozt = __##VAR_NAME; !OZ_isNil(ozt); ozt = OZ_tail(ozt), i++) {    \
      VAR_NAME[i] = OZ_intToC(OZ_head(ozt));                         \
    }                                                                \
  }                                                                  \
  else if (OZ_isTuple(__##VAR_NAME)) {                               \
    int oz_width = OZ_width(__##VAR_NAME);                           \
    for (int i=0; i < oz_width; i++) {                               \
      VAR_NAME[i] = OZ_getArg(__##VAR_NAME,i);                       \
    }                                                                \
  }

#define OBTAIN_VEC_VAR_AND_CURR_SPACE(VAR_NAME,LOZT,CS,I) \
      if (OZ_isInt(LOZT)) {                                          \
        int vt = OZ_intToC(LOZT);                                    \
        assert(CS != NULL);                                  \
        VAR_NAME[I].init(CS->getSpace(),vt,vt);              \
      }                                                              \
      else {                                                         \
        assert(OZ_isGeIntVar(LOZT));                                 \
        GeIntVar* __giv_##VAR_NAME = get_GeIntVar(LOZT);             \
        if (CS==NULL) {                                      \
          CS = __giv_##VAR_NAME->getGeSpace();               \
        }                                                            \
        assert(CS == __giv_##VAR_NAME->getGeSpace());        \
        VAR_NAME[I] = __giv_##VAR_NAME->getIntVar();                 \
      }

/* This MACRO is OK. Don't change it */
#define DECLARE_INTVAR_ARGS_UPCAST(CS,VAR_NAME,OZ_PARAM_NUMBER)         \
  OZ_declareTerm(OZ_PARAM_NUMBER,__##VAR_NAME);                      \
  if (!(OZ_isTuple(__##VAR_NAME) || OZ_isList(__##VAR_NAME,NULL))) { \
    RAISE_EXCEPTION("Error D2: var " #OZ_PARAM_NUMBER " not tuple nor list"); \
  }                                                                  \
  int arr_w##OZ_PARAM_NUMBER;                                        \
  arr_w##OZ_PARAM_NUMBER = OZ_isList(__##VAR_NAME,NULL)? OZ_length(__##VAR_NAME) : OZ_width(__##VAR_NAME); \
  Gecode::IntVarArgs VAR_NAME(arr_w##OZ_PARAM_NUMBER);               \
  if (OZ_isList(__##VAR_NAME,NULL)) {                                \
    int i=0;                                                         \
    for (OZ_Term ozt = __##VAR_NAME; !OZ_isNil(ozt); ozt = OZ_tail(ozt), i++) {    \
      OZ_Term ozv = OZ_head(ozt); \
      OBTAIN_VEC_VAR_AND_CURR_SPACE(VAR_NAME,ozv,i);\
    }                                                                \
  }                                                                  \
  else if (OZ_isTuple(__##VAR_NAME)) {                               \
    int oz_width = OZ_width(__##VAR_NAME);                           \
    for (int i=0; i < oz_width; i++) {                               \
      OZ_Term ozarg = OZ_getArg(__##VAR_NAME,i);                     \
      OBTAIN_VEC_VAR_AND_CURR_SPACE(VAR_NAME,ozarg,i);               \
    }                                                                \
  }                                                                  \
  else {                                                             \
    RAISE_EXCEPTION("The argument must be a list or a tuple");       \
  }


#define DECLARE_INTVAR_ARGS(VAR_NAME,OZ_PARAM_NUMBER)                \
  OZ_declareTerm(OZ_PARAM_NUMBER,__##VAR_NAME);                      \
  if (!(OZ_isTuple(__##VAR_NAME) || OZ_isList(__##VAR_NAME,NULL))) { \
    RAISE_EXCEPTION("Error D3: var " #OZ_PARAM_NUMBER "not tuple nor list");     \
  }                                                                  \
  int arr_w##OZ_PARAM_NUMBER;                                        \
  arr_w##OZ_PARAM_NUMBER = OZ_isList(__##VAR_NAME,NULL)? OZ_length(__##VAR_NAME) : OZ_width(__##VAR_NAME); \
  Gecode::IntVarArgs VAR_NAME(arr_w##OZ_PARAM_NUMBER);               \
  if (OZ_isList(__##VAR_NAME,NULL)) {                                \
    int i=0;                                                         \
    for (OZ_Term ozt = __##VAR_NAME; !OZ_isNil(ozt); ozt = OZ_tail(ozt), i++) {    \
      OZ_Term ozv = OZ_head(ozt);                                    \
      assert(OZ_isGeIntVar(ozv));                                    \
      OBTAIN_VEC_VAR_AND_CURR_SPACE(VAR_NAME,ozv,i);                 \
    }                                                                \
  }                                                                  \
  else if (OZ_isTuple(__##VAR_NAME)) {                               \
    int oz_width = OZ_width(__##VAR_NAME);                           \
    for (int i=0; i < oz_width; i++) {                               \
      OZ_Term ozarg = OZ_getArg(__##VAR_NAME,i);                     \
      OBTAIN_VEC_VAR_AND_CURR_SPACE(VAR_NAME,ozarg,i);               \
    }                                                                \
  }                                                                  \
  else { \
    RAISE_EXCEPTION("Nor list nor tuple"); \
  }


#define CHECK_AND_SET_SPACE(CURR_SPACE,V)          \
      if (OZ_isGeIntVar(V)) {                      \
	GeIntVar* giv = get_GeIntVar(V);           \
	if (CURR_SPACE != NULL) {                  \
	  if (CURR_SPACE != giv->getGeSpace()) {   \
            /*printf("CS != giv->getGeSpace()\n");*/ \
	      /*printf("CS: %p", CURR_SPACE); */   \
	      /*printf("giv->getGeSpace(): %p\n", giv->getGeSpace()); */ \
	    CURR_SPACE = NULL;                     \
	    break;                                 \
	  }                                        \
	  /* ok. do nothing. we have checked that the variable is in the same space of the others */ \
	}                                          \
	else {                                     \
	  /* there was no currently space set. */  \
	  CURR_SPACE = giv->getGeSpace();          \
	}                                          \
      }


#define DECLARE_AVAL_SEL(VAR_NAME,OZ_PARAM_NUMBER)             \
  Gecode::AvalSel VAR_NAME;                                    \
  OZ_declareAtom(OZ_PARAM_NUMBER,__##VAR_NAME);                \
  if (std::strcmp(__##VAR_NAME,"aval_min")==0)                 \
    VAR_NAME = Gecode::AVAL_MIN;                               \
  else if (std::strcmp(__##VAR_NAME,"aval_med")==0)            \
    VAR_NAME = Gecode::AVAL_MED;                               \
  else if (std::strcmp(__##VAR_NAME,"aval_max")==0)            \
    VAR_NAME = Gecode::AVAL_MAX;

#define DECLARE_INT_REL(VAR_NAME,OZ_PARAM_NUMBER)              \
  Gecode::IntRelType VAR_NAME;                                 \
  OZ_declareAtom(OZ_PARAM_NUMBER,__##VAR_NAME);                \
  if (std::strcmp(__##VAR_NAME,"\\=:")==0)                     \
    VAR_NAME = Gecode::IRT_NQ;                                 \
  else if (std::strcmp(__##VAR_NAME,"=:")==0)                  \
    VAR_NAME = Gecode::IRT_EQ;                                 \
  else if (std::strcmp(__##VAR_NAME,"<:")==0)                  \
    VAR_NAME = Gecode::IRT_LE;                                 \
  else if (std::strcmp(__##VAR_NAME,"=<:")==0)                 \
    VAR_NAME = Gecode::IRT_LQ;                                 \
  else if (std::strcmp(__##VAR_NAME,">:")==0)                  \
    VAR_NAME = Gecode::IRT_GR;                                 \
  else if (std::strcmp(__##VAR_NAME,">=:")==0)                 \
    VAR_NAME = Gecode::IRT_GQ;



GeSpace* get_GeIntVarArgsAnyGeSpace(OZ_Term vec);
bool vectorChecker(OZ_Term v, bool checkingIsIntVar);

/* Returns true if there is at least one element of type GeIntVar,
 * and all other elements are IntVar or ints.
 */
bool isGeIntVarArgs(OZ_Term v);
bool isIntArgsTerm(OZ_Term v);

  

#endif  // __INT_BASIC_MACROS__INCLUDE__
