/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *
 *  Contributors:
 *    Denys Duchier (duchier@ps.uni-sb.de)
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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __OZ_H__
#define __OZ_H__

#ifndef NOFINALIZATION
#define FINALIZATION
#endif

#undef OZ_DEBUG_INTERFACE // don't use 'extern "C"' (may cause trouble!!)

/* ------------------------------------------------------------------------ *
 * 0. intro
 * ------------------------------------------------------------------------ */

/* calling convention "cdecl" under win32 */
#ifdef __WATCOMC__
#define ozcdecl __cdecl
#define OZWIN
#else
#ifdef __BORLANDC__
#define ozcdecl __export __cdecl
#define OZWIN
#else
#ifdef _MSC_VER
#define ozcdecl cdecl
#define OZWIN
#else
#define ozcdecl
#endif
#endif
#endif

#if defined(__STDC__)
#define OZStringify(Name) #Name
#define CONST const
#else
#define OZStringify(Name) "Name"
#define CONST
#endif

#if defined(OZWIN) || defined(OZC)
#define OzFun(fun) (ozcdecl *fun)
#else
#define OzFun(fun) fun
#endif

#if defined(__STDC__) || defined(__cplusplus) || __BORLANDC__ || _MSC_VER
#define _FUNDECL(fun,arglist) OzFun(fun) arglist
#define _FUNTYPEDECL(fun,arglist) (ozcdecl *fun) arglist
#if defined(__cplusplus) && !defined(OZ_DEBUG_INTERFACE)
extern "C" {
#endif
#else
#define _FUNDECL(fun,ignore) OzFun(fun) ()
#define _FUNTYPEDECL(fun,ignore) (ozcdecl *fun) ()
#endif


/* Tell me whether this version of Oz supports dynamic linking */

#if defined(sun) || defined(linux) || defined(sgi)
#define OZDYNLINKING
#endif

/* ------------------------------------------------------------------------ *
 * I. type declarations
 * ------------------------------------------------------------------------ */

typedef unsigned int OZ_Term;

typedef unsigned int OZ_Return;

#define OZ_FAILED      0
#define FAILED         OZ_FAILED
#define PROCEED        1
#define OZ_ENTAILED    PROCEED
#define SUSPEND     2
#define SLEEP       3
#define OZ_SLEEP    SLEEP
#define SCHEDULED   4
#define RAISE       5


typedef void *OZ_Thread;
typedef void *OZ_Arity;

typedef OZ_Return _FUNTYPEDECL(OZ_CFun,(OZ_Term *,int *));

/* for tobias */
typedef int OZ_Boolean;
#define OZ_FALSE 0
#define OZ_TRUE 1


/* ------------------------------------------------------------------------ *
 * II. function prototypes
 * ------------------------------------------------------------------------ */

extern void     _FUNDECL(OZ_main, (int argc,char **argv));


extern OZ_Term  _FUNDECL(OZ_deref,(OZ_Term term));

/* tests */
extern int _FUNDECL(OZ_isAtom,(OZ_Term));
extern int _FUNDECL(OZ_isBigInt,(OZ_Term));
extern int _FUNDECL(OZ_isCell,(OZ_Term));
extern int _FUNDECL(OZ_isThread,(OZ_Term));
extern int _FUNDECL(OZ_isPort,(OZ_Term));
extern int _FUNDECL(OZ_isChunk,(OZ_Term));
extern int _FUNDECL(OZ_isCons,(OZ_Term));
extern int _FUNDECL(OZ_isFalse,(OZ_Term));
extern int _FUNDECL(OZ_isFeature,(OZ_Term));
extern int _FUNDECL(OZ_isFloat,(OZ_Term));
extern int _FUNDECL(OZ_isInt,(OZ_Term));
extern int _FUNDECL(OZ_isNumber,(OZ_Term));
extern int _FUNDECL(OZ_isLiteral,(OZ_Term));
extern int _FUNDECL(OZ_isName,(OZ_Term));
extern int _FUNDECL(OZ_isNil,(OZ_Term));
extern int _FUNDECL(OZ_isObject,(OZ_Term));
extern int _FUNDECL(OZ_isPair,(OZ_Term));
extern int _FUNDECL(OZ_isPair2,(OZ_Term));
extern int _FUNDECL(OZ_isProcedure,(OZ_Term));
extern int _FUNDECL(OZ_isRecord,(OZ_Term));
extern int _FUNDECL(OZ_isSmallInt,(OZ_Term));
extern int _FUNDECL(OZ_isTrue,(OZ_Term));
extern int _FUNDECL(OZ_isTuple,(OZ_Term));
extern int _FUNDECL(OZ_isUnit,(OZ_Term));
extern int _FUNDECL(OZ_isValue,(OZ_Term));
extern int _FUNDECL(OZ_isVariable,(OZ_Term));

extern int _FUNDECL(OZ_isList,(OZ_Term, OZ_Term *));
extern int _FUNDECL(OZ_isString,(OZ_Term, OZ_Term *));
extern int _FUNDECL(OZ_isProperString,(OZ_Term, OZ_Term *));
extern int _FUNDECL(OZ_isVirtualString,(OZ_Term, OZ_Term *));

/* heap chunks */

extern OZ_Term   _FUNDECL(OZ_makeHeapChunk,(int s));
extern void *    _FUNDECL(OZ_getHeapChunkData,(OZ_Term t));
extern int       _FUNDECL(OZ_getHeapChunkSize,(OZ_Term t));
extern int       _FUNDECL(OZ_isHeapChunk,(OZ_Term t));


#define OZ_assertList(t)                        \
  {                                             \
    OZ_Term var;                                \
    if (!OZ_isList(t,&var)) {                   \
      if (var == 0) return FAILED;              \
      OZ_suspendOn(var);                        \
    }                                           \
  }

#define OZ_assertProperString(t)                \
  {                                             \
    OZ_Term var;                                \
    if (!OZ_isProperString(t,&var)) {           \
      if (var == 0) return FAILED;              \
      OZ_suspendOn(var);                        \
    }                                           \
  }
#define OZ_assertVirtualString(t)               \
  {                                             \
    OZ_Term var;                                \
    if (!OZ_isVirtualString(t,&var)) {          \
      if (var == 0) return FAILED;              \
      OZ_suspendOn(var);                        \
    }                                           \
  }

/*
 * mm2: should we support this ?
 */
extern OZ_Term _FUNDECL(OZ_termType,(OZ_Term));

/* convert: C from/to Oz datastructure */

extern CONST char* _FUNDECL(OZ_atomToC,(OZ_Term));
extern OZ_Term _FUNDECL(OZ_atom,(CONST char *));
extern int     _FUNDECL(OZ_featureCmp,(OZ_Term,OZ_Term));

extern int     _FUNDECL(OZ_smallIntMin,(void));
extern int     _FUNDECL(OZ_smallIntMax,(void));
extern OZ_Term _FUNDECL(OZ_false,(void));
extern OZ_Term _FUNDECL(OZ_true,(void));
extern OZ_Term _FUNDECL(OZ_unit,(void));
extern OZ_Term _FUNDECL(OZ_int,(int));
extern int     _FUNDECL(OZ_getLowPrio,(void));
extern int     _FUNDECL(OZ_getMediumPrio,(void));
extern int     _FUNDECL(OZ_getHighPrio,(void));
extern int     _FUNDECL(OZ_intToC,(OZ_Term));
extern OZ_Term _FUNDECL(OZ_CStringToInt,(char *str));
extern char *  _FUNDECL(OZ_parseInt,(char *s));

extern OZ_Term _FUNDECL(OZ_float,(double));
extern double  _FUNDECL(OZ_floatToC,(OZ_Term));

extern OZ_Term _FUNDECL(OZ_CStringToFloat,(char *s));
extern char *  _FUNDECL(OZ_parseFloat,(char *s));

extern OZ_Term _FUNDECL(OZ_CStringToNumber,(char *));

extern char *  _FUNDECL(OZ_toC,(OZ_Term, int, int));
extern int     _FUNDECL(OZ_termGetSize,(OZ_Term, int, int));

extern OZ_Term _FUNDECL(OZ_string,(CONST char *));
extern char *  _FUNDECL(OZ_stringToC,(OZ_Term t));

extern void    _FUNDECL(OZ_printVirtualString,(OZ_Term t));
#define OZ_printVS(t) OZ_printVirtualString(t)
extern char *  _FUNDECL(OZ_virtualStringToC,(OZ_Term t));


/* tuples */
extern OZ_Term  _FUNDECL(OZ_label,(OZ_Term));
extern int      _FUNDECL(OZ_width,(OZ_Term));
extern OZ_Term _FUNDECL(OZ_tuple,(OZ_Term, int));
#define OZ_tupleC(s,n) OZ_tuple(OZ_atom(s),n)
extern OZ_Term  _FUNDECL(OZ_mkTuple,(OZ_Term label,int arity,...));
extern OZ_Term  _FUNDECL(OZ_mkTupleC,(char *label,int arity,...));

extern void     _FUNDECL(OZ_putArg,(OZ_Term, int, OZ_Term));
extern OZ_Term  _FUNDECL(OZ_getArg,(OZ_Term, int));
extern OZ_Term  _FUNDECL(OZ_nil,());
extern OZ_Term  _FUNDECL(OZ_cons,(OZ_Term ,OZ_Term));
extern OZ_Term  _FUNDECL(OZ_head,(OZ_Term));
extern OZ_Term  _FUNDECL(OZ_tail,(OZ_Term));
extern int      _FUNDECL(OZ_length,(OZ_Term list));
extern OZ_Term  _FUNDECL(OZ_toList,(int, OZ_Term *));


extern OZ_Term  _FUNDECL(OZ_pair,(int));
extern OZ_Term  _FUNDECL(OZ_pair2,(OZ_Term t1,OZ_Term t2));

#define OZ_pairA(s1,t)      OZ_pair2(OZ_atom(s1),t)
#define OZ_pairAI(s1,i)     OZ_pair2(OZ_atom(s1),OZ_int(i))
#define OZ_pairAA(s1,s2)    OZ_pair2(OZ_atom(s1),OZ_atom(s2))
#define OZ_pairAS(s1,s2)    OZ_pair2(OZ_atom(s1),OZ_string(s2))


/* records */
extern OZ_Arity _FUNDECL(OZ_makeArity,(OZ_Term list));
extern OZ_Term  _FUNDECL(OZ_record,(OZ_Term, OZ_Term));
extern OZ_Term  _FUNDECL(OZ_recordInit,(OZ_Term, OZ_Term));
extern void     _FUNDECL(OZ_putSubtree,(OZ_Term, OZ_Term, OZ_Term));
extern OZ_Term  _FUNDECL(OZ_subtree,(OZ_Term, OZ_Term));
extern OZ_Term  _FUNDECL(OZ_arityList,(OZ_Term));
extern OZ_Term  _FUNDECL(OZ_adjoinAt,(OZ_Term, OZ_Term, OZ_Term));

/* unification */
extern OZ_Return _FUNDECL(OZ_unify,(OZ_Term, OZ_Term));
extern void      _FUNDECL(OZ_unifyInThread,(OZ_Term,OZ_Term));
extern int       _FUNDECL(OZ_eq,(OZ_Term, OZ_Term));

#define OZ_unifyFloat(t1,f)      OZ_unify(t1, OZ_float(f))
#define OZ_unifyInt(t1,i)        OZ_unify(t1, OZ_int(i))
#define OZ_unifyAtom(t1,s)       OZ_unify(t1, OZ_atom(s))

#define OZ_eqAtom(t1,s)       OZ_eq(t1, OZ_atom(s))
#define OZ_eqInt(t1,s)        OZ_eq(t1, OZ_int(s))
#define OZ_eqFloat(t1,s)      OZ_eq(t1, OZ_float(s))

/* create a new oz variable */
extern OZ_Term _FUNDECL(OZ_newVariable,());

extern OZ_Term _FUNDECL(OZ_newChunk,(OZ_Term));

/* cell */
extern OZ_Term _FUNDECL(OZ_newCell,(OZ_Term));
/* exchangeCell, deepFeed */

/* port */
extern OZ_Term _FUNDECL(OZ_newPort,(OZ_Term));
extern void _FUNDECL(OZ_send,(OZ_Term,OZ_Term));

/* name */
extern OZ_Term _FUNDECL(OZ_newName,());

/* print warning */
#ifdef __cplusplus
extern void _FUNDECL(OZ_warning,(CONST char * ...));
#else
extern void _FUNDECL(OZ_warning,(CONST char *, ...));
#endif

/* generate the unix error string from an errno (see perror(3)) */
extern char * _FUNDECL(OZ_unixError,(int err));

/* check for toplevel */
extern int _FUNDECL(OZ_onToplevel,());

extern int _FUNDECL(OZ_addBuiltin,(CONST char *, int, int, OZ_CFun));

/* replace new builtins */
typedef struct {
  char *name;
  int inArity;
  int outArity;
  OZ_CFun fun;
} OZ_BIspec;

/* add specification to builtin table */
extern void _FUNDECL(OZ_addBISpec,(OZ_BIspec *spec));

/* IO */

extern OZ_Return _FUNDECL(OZ_readSelect,(int, OZ_Term, OZ_Term));
extern OZ_Return _FUNDECL(OZ_writeSelect,(int, OZ_Term, OZ_Term));
extern OZ_Return _FUNDECL(OZ_acceptSelect,(int, OZ_Term, OZ_Term));
extern void      _FUNDECL(OZ_deSelect,(int));

/*
 * OZ_IOHandler is called with fd + static pointer given when registered
 *   if it returns TRUE, it is unregistered
 *   else (return FALSE) its called again, when something is available
 */

typedef int _FUNTYPEDECL(OZ_IOHandler,(int,void *));

extern void _FUNDECL(OZ_registerReadHandler,(int,OZ_IOHandler,void *));
extern void _FUNDECL(OZ_unregisterRead,(int));

extern void _FUNDECL(OZ_registerWriteHandler,(int,OZ_IOHandler,void *));
extern void _FUNDECL(OZ_unregisterWrite,(int));

extern void _FUNDECL(OZ_registerAcceptHandler,(int,OZ_IOHandler,void *));

/* garbage collection */
extern int _FUNDECL(OZ_protect,(OZ_Term *));
extern int _FUNDECL(OZ_unprotect,(OZ_Term *));

/* raise exception */
extern OZ_Return _FUNDECL(OZ_typeError,(int pos,char *type));
extern OZ_Return _FUNDECL(OZ_raise,(OZ_Term));
extern OZ_Return _FUNDECL(OZ_raiseDebug,(OZ_Term));
extern OZ_Return _FUNDECL(OZ_raiseC,(char *label,int arity,...));
extern OZ_Return _FUNDECL(OZ_raiseError,(OZ_Term));
extern OZ_Return _FUNDECL(OZ_raiseErrorC,(char *label,int arity,...));
extern OZ_Term   _FUNDECL(OZ_makeException,(OZ_Term kind,OZ_Term key,char*label,int arity,...));

/* Suspending builtins */

extern OZ_Thread _FUNDECL(OZ_newRunnableThread,());
extern void      _FUNDECL(OZ_makeRunnableThread,(OZ_CFun, OZ_Term *, int));
extern OZ_Thread _FUNDECL(OZ_newSuspendedThread,());
extern OZ_Thread _FUNDECL(OZ_makeSuspendedThread,(OZ_CFun, OZ_Term *, int));
extern void      _FUNDECL(OZ_addThread,(OZ_Term, OZ_Thread));
extern void      _FUNDECL(OZ_pushCFun,(OZ_Thread,OZ_CFun,OZ_Term *,int));
extern void      _FUNDECL(OZ_pushCall,(OZ_Thread,OZ_Term,OZ_Term *,int));

#define OZ_makeSelfSuspendedThread() \
  OZ_makeSuspendedThread(OZ_self, OZ_args,OZ_arity)

/* for example
   OZ_Thread s = OZ_makeSuspendedThread(BIplus,OZ_args,OZ_arity);
   OZ_addThread(t1,s);
   OZ_addThread(t2,s);
   */

extern OZ_Return _FUNDECL(OZ_suspendOnInternal,(OZ_Term));
extern OZ_Return _FUNDECL(OZ_suspendOnInternal2,(OZ_Term,OZ_Term));
extern OZ_Return _FUNDECL(OZ_suspendOnInternal3,(OZ_Term,OZ_Term,OZ_Term));

#define OZ_suspendOn(t1) \
   { return OZ_suspendOnInternal(t1); }
#define OZ_suspendOn2(t1,t2) \
   { return OZ_suspendOnInternal2(t1,t2); }
#define OZ_suspendOn3(t1,t2,t3) \
   { return OZ_suspendOnInternal3(t1,t2,t3); }

/* ------------------------------------------------------------------------ *
 * III. macros
 * ------------------------------------------------------------------------ */

/* Transitional Interface for the new regime for builtins */
/* OZ_BI_define(Name,InArity,OutArity){ ... }
        defines Name to be a builtin with arity=InArity+OutArity
        define BI__Name to be the new style primitive with
                InArity input registers
                OutArity output register
        Name calls BI__Name and performs necessary unifications
        when it returns

   OZ_in(n) accesses input register n
   OZ_out(n) accesses output register n
   OZ_result(v) assigns v to output register 0 (the usual case)
   */

#ifdef __cplusplus
#define OZ_BI_proto(Name) \
  extern "C" OZ_Return _FUNDECL(Name,(OZ_Term [],int []));
#else
#define OZ_BI_proto(Name) \
  OZ_Return _FUNDECL(Name,(OZ_Term [],int []));
#endif

#define OZ_ID_MAP 0
#define OZ_in(N) _OZ_ARGS[_OZ_LOC==OZ_ID_MAP?N:_OZ_LOC[N]]
#define OZ_out(N) _OZ_ARGS[_OZ_LOC==OZ_ID_MAP?_OZ_arity+N:_OZ_LOC[_OZ_arity+N]]
#define OZ_result(V) OZ_out(0)=V

#define OZ_BI_define(Name,Arity_IN,Arity_OUT)                   \
OZ_BI_proto(Name);                                              \
OZ_Return _FUNDECL(Name,(OZ_Term _OZ_ARGS[],int _OZ_LOC[])) {   \
    const int _OZ_arity = Arity_IN;

#define OZ_BI_end }

#define OZ_declareIN(ARG,VAR) \
     OZ_Term VAR = OZ_in(ARG);

#define OZ_nonvarIN(ARG)                        \
{                                               \
  if (OZ_isVariable(OZ_in(ARG))) {              \
    OZ_suspendOn(OZ_in(ARG));                   \
  }                                             \
}

#define OZ_declareNonvarIN(ARG,VAR)             \
OZ_Term VAR = OZ_in(ARG);                       \
{                                               \
  if (OZ_isVariable(VAR)) {                     \
    OZ_suspendOn(VAR);                          \
  }                                             \
}

#define OZ_declareIntIN(ARG,VAR)                \
 int VAR;                                       \
 OZ_nonvarIN(ARG);                              \
 if (! OZ_isInt(OZ_in(ARG))) {                  \
   return OZ_typeError(ARG,"Int");              \
 } else {                                       \
   VAR = OZ_intToC(OZ_in(ARG));                 \
 }

#define OZ_declareFloatIN(ARG,VAR)              \
 double VAR;                                    \
 OZ_nonvarIN(ARG);                              \
 if (! OZ_isFloat(OZ_in(ARG))) {                \
   return OZ_typeError(ARG,"Float");            \
   return FAILED;                               \
 } else {                                       \
   VAR = OZ_floatToC(OZ_in(ARG));               \
 }


#define OZ_declareAtomIN(ARG,VAR)               \
 CONST char *VAR;                               \
 OZ_nonvarIN(ARG);                              \
 if (! OZ_isAtom(OZ_in(ARG))) {                 \
   return OZ_typeError(ARG,"Atom");             \
 } else {                                       \
   VAR = OZ_atomToC(OZ_in(ARG));                \
 }

#define OZ_declareProperStringIN(ARG,VAR)               \
 char *VAR;                                             \
 {                                                      \
   OZ_Term OZ_avar;                                     \
   if (!OZ_isProperString(OZ_in(ARG),&OZ_avar)) {       \
     if (OZ_avar == 0) {                                \
       return OZ_typeError(ARG,"ProperString");         \
     } else {                                           \
       OZ_suspendOn(OZ_avar);                           \
     }                                                  \
   }                                                    \
   VAR = OZ_stringToC(OZ_in(ARG));                      \
 }

#define OZ_declareVirtualStringIN(ARG,VAR)              \
 char *VAR;                                             \
 {                                                      \
   OZ_Term OZ_avar;                                     \
   if (!OZ_isVirtualString(OZ_in(ARG),&OZ_avar)) {      \
     if (OZ_avar == 0) {                                \
       return OZ_typeError(ARG,"VirtualString");        \
     } else {                                           \
       OZ_suspendOn(OZ_avar);                           \
     }                                                  \
   }                                                    \
   VAR = OZ_virtualStringToC(OZ_in(ARG));               \
 }

#define OZ_declareForeignPointerIN(ARG,VAR)     \
void *VAR;                                      \
{                                               \
  OZ_declareNonvarIN(ARG,_VAR);         \
  if (!OZ_isForeignPointer(_VAR)) {             \
    return OZ_typeError(ARG,"ForeignPointer");  \
  } else {                                      \
    VAR = OZ_getForeignPointer(_VAR);           \
  }                                             \
}

#define OZ_RETURN(V) return ((OZ_result(V)),PROCEED)
#define OZ_RETURN_INT(I) OZ_RETURN(OZ_int(I))
#define OZ_RETURN_ATOM(S) OZ_RETURN(OZ_atom(S))
#define OZ_RETURN_STRING(S) OZ_RETURN(OZ_string(S))


/* the old interface */

#define OZ_C_proc_proto(Name)   OZ_BI_proto(Name)

#define OZ_C_proc_begin(Name,arity)                                       \
OZ_BI_proto(Name);                                                        \
OZ_Return _FUNDECL(Name,(OZ_Term _OZ_NEW_ARGS[],int _OZ_NEW_LOC[])) {     \
    const OZ_CFun OZ_self = Name;                                         \
    OZ_Term OZ_args[arity];                                               \
    const int OZ_arity = arity;                                           \
    { int i;                                                              \
      for (i = arity; i--; ) {                                            \
        OZ_args[i]=_OZ_NEW_ARGS[_OZ_NEW_LOC==OZ_ID_MAP?i:_OZ_NEW_LOC[i]]; \
    }                                                                     \
}

#define OZ_C_proc_end                   OZ_BI_end

/* access arguments */
#define OZ_getCArg(N) OZ_args[N]

/* useful macros and functions (mm 9.2.93) */

#define OZ_declareArg(ARG,VAR) \
     OZ_Term VAR = OZ_getCArg(ARG);

#define OZ_nonvarArg(ARG)                       \
{                                               \
  if (OZ_isVariable(OZ_getCArg(ARG))) {         \
    OZ_suspendOn(OZ_getCArg(ARG));              \
  }                                             \
}

#define OZ_declareNonvarArg(ARG,VAR)            \
OZ_Term VAR = OZ_getCArg(ARG);                  \
{                                               \
  if (OZ_isVariable(VAR)) {                     \
    OZ_suspendOn(VAR);                          \
  }                                             \
}

#define OZ_declareIntArg(ARG,VAR)               \
 int VAR;                                       \
 OZ_nonvarArg(ARG);                             \
 if (! OZ_isInt(OZ_getCArg(ARG))) {             \
   return OZ_typeError(ARG,"Int");              \
 } else {                                       \
   VAR = OZ_intToC(OZ_getCArg(ARG));            \
 }

#define OZ_declareFloatArg(ARG,VAR)             \
 double VAR;                                    \
 OZ_nonvarArg(ARG);                             \
 if (! OZ_isFloat(OZ_getCArg(ARG))) {           \
   return OZ_typeError(ARG,"Float");            \
 } else {                                       \
   VAR = OZ_floatToC(OZ_getCArg(ARG));          \
 }


#define OZ_declareAtomArg(ARG,VAR)              \
 CONST char *VAR;                               \
 OZ_nonvarArg(ARG);                             \
 if (! OZ_isAtom(OZ_getCArg(ARG))) {            \
   return OZ_typeError(ARG,"Atom");             \
 } else {                                       \
   VAR = OZ_atomToC(OZ_getCArg(ARG));           \
 }

#define OZ_declareProperStringArg(ARG,VAR)              \
 char *VAR;                                             \
 {                                                      \
   OZ_Term OZ_avar;                                     \
   if (!OZ_isProperString(OZ_getCArg(ARG),&OZ_avar)) {  \
     if (OZ_avar == 0) {                                \
       return OZ_typeError(ARG,"ProperString");         \
     } else {                                           \
       OZ_suspendOn(OZ_avar);                           \
     }                                                  \
   }                                                    \
   VAR = OZ_stringToC(OZ_getCArg(ARG));                 \
 }

#define OZ_declareVirtualStringArg(ARG,VAR)             \
 char *VAR;                                             \
 {                                                      \
   OZ_Term OZ_avar;                                     \
   if (!OZ_isVirtualString(OZ_getCArg(ARG),&OZ_avar)) { \
     if (OZ_avar == 0) {                                \
       return OZ_typeError(ARG,"VirtualString");        \
     } else {                                           \
       OZ_suspendOn(OZ_avar);                           \
     }                                                  \
   }                                                    \
   VAR = OZ_virtualStringToC(OZ_getCArg(ARG));          \
 }

#define OZ_declareForeignPointerArg(ARG,VAR)    \
void *VAR;                                      \
{                                               \
  OZ_declareNonvarArg(ARG,_VAR);                \
  if (!OZ_isForeignPointer(_VAR)) {             \
    return OZ_typeError(ARG,"ForeignPointer");  \
  } else {                                      \
    VAR = OZ_getForeignPointer(_VAR);           \
  }                                             \
}

/* ------------------------------------------------------------------------ *
 * end
 * ------------------------------------------------------------------------ */

#if defined(__cplusplus) && !defined(OZ_DEBUG_INTERFACE)
}
#endif

#if defined(__cplusplus) && !defined(OZ_DEBUG_INTERFACE)
extern "C" {
#endif


extern OZ_Term  _FUNDECL(OZ_makeForeignPointer,(void*));
extern void*    _FUNDECL(OZ_getForeignPointer,(OZ_Term));
extern int      _FUNDECL(OZ_isForeignPointer,(OZ_Term));


struct OZ_C_proc_interface {
  const char* name;
  int         inArity;
  int         outArity;
  OZ_CFun     func;
};

/* Perdio related things */

typedef struct {
  char *data;  /* NULL on error */
  int size;    /* contain error code */
} OZ_Datum;


#define OZ_DATUM_UNKNOWNERROR -1
#define OZ_DATUM_OUTOFMEMORY  -2

extern OZ_Return _FUNDECL(OZ_valueToDatum,(OZ_Term  t, OZ_Datum* d));
extern OZ_Return _FUNDECL(OZ_datumToValue,(OZ_Datum d, OZ_Term   t));

#if defined(__cplusplus) && !defined(OZ_DEBUG_INTERFACE)
}
#endif

#endif /* __OZ_H__ */
