/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
  */

#ifndef __FOREIGNH
#define __FOREIGNH

/* ------------------------------------------------------------------------ *
 * 0. intro
 * ------------------------------------------------------------------------ */

#ifdef __cplusplus
#define _PROTOTYPE(argl) argl
extern "C" {
#else
#define _PROTOTYPE(ignore) ()
#endif

/* Tell me whether this version of Oz supports dynamic linking */

#if defined(sun) || defined(linux) || defined(sgi)
#define OZDYNLINKING
#endif

/* ------------------------------------------------------------------------ *
 * I. type declarations
 * ------------------------------------------------------------------------ */

typedef unsigned int OZ_Term;
typedef double OZ_Float;

typedef enum {
  FAILED,
  PROCEED,
  SUSPEND
} OZ_Bool;
  
typedef void *OZ_Suspension;

typedef OZ_Bool (*OZ_CFun) _PROTOTYPE((int, OZ_Term *));

/* ------------------------------------------------------------------------ *
 * II. function prototypes
 * ------------------------------------------------------------------------ */

/* tests */
extern int OZ_isAtom       _PROTOTYPE((OZ_Term));
extern int OZ_isCell       _PROTOTYPE((OZ_Term));
extern int OZ_isCons       _PROTOTYPE((OZ_Term));
extern int OZ_isFloat      _PROTOTYPE((OZ_Term));
extern int OZ_isInt        _PROTOTYPE((OZ_Term));
extern int OZ_isLiteral    _PROTOTYPE((OZ_Term));
extern int OZ_isName       _PROTOTYPE((OZ_Term));
extern int OZ_isNil        _PROTOTYPE((OZ_Term));
extern int OZ_isNoNumber   _PROTOTYPE((OZ_Term));
extern int OZ_isProcedure  _PROTOTYPE((OZ_Term));
extern int OZ_isRecord     _PROTOTYPE((OZ_Term));
extern int OZ_isTuple      _PROTOTYPE((OZ_Term));
extern int OZ_isValue      _PROTOTYPE((OZ_Term));
extern int OZ_isVariable   _PROTOTYPE((OZ_Term));

extern OZ_Term OZ_termType _PROTOTYPE((OZ_Term));

/* convert: C from/to Oz datastructure */

extern char *   OZ_atomToC   _PROTOTYPE((OZ_Term));
extern OZ_Term  OZ_CToAtom   _PROTOTYPE((char *));

extern OZ_Term  OZ_CToInt        _PROTOTYPE((int));
extern int      OZ_intToC        _PROTOTYPE((OZ_Term));
extern OZ_Term  OZ_CStringToInt  _PROTOTYPE((char *str));
extern char *   OZ_intToCString  _PROTOTYPE((OZ_Term term));
extern char *   OZ_intFloat      _PROTOTYPE((char *s));
extern char *   OZ_normInt       _PROTOTYPE((char *s));
extern char *   OZ_parseInt      _PROTOTYPE((char *s));

extern OZ_Term  OZ_CToFloat        _PROTOTYPE((OZ_Float));
extern OZ_Float OZ_floatToC        _PROTOTYPE((OZ_Term));
extern OZ_Term  OZ_CStringToFloat  _PROTOTYPE((char *s));
#define OZ_floatToCString(f) OZ_floatToCStringPretty(f)
extern char *   OZ_floatToCStringLong    _PROTOTYPE((OZ_Term term));
extern char *   OZ_floatToCStringInt     _PROTOTYPE((OZ_Term term));
extern char *   OZ_floatToCStringPretty  _PROTOTYPE((OZ_Term term));
extern char *   OZ_normFloat       _PROTOTYPE((char *s));
extern char *   OZ_parseFloat      _PROTOTYPE((char *s));

extern OZ_Term  OZ_CStringToNumber _PROTOTYPE((char *));

extern char *   OZ_toC       _PROTOTYPE((OZ_Term));

extern OZ_Term  OZ_CToString _PROTOTYPE((char *));
extern char *   OZ_stringToC _PROTOTYPE((OZ_Term t));

/* tuples */
extern OZ_Term OZ_label     _PROTOTYPE((OZ_Term));
extern int     OZ_width     _PROTOTYPE((OZ_Term));
extern OZ_Term OZ_tuple     _PROTOTYPE((OZ_Term, int));
#define OZ_tupleC(s,n) OZ_tuple(OZ_CToAtom(s),n)

extern int     OZ_putArg    _PROTOTYPE((OZ_Term, int, OZ_Term));
extern OZ_Term OZ_getArg    _PROTOTYPE((OZ_Term , int));
extern OZ_Term OZ_nil       _PROTOTYPE(());
extern OZ_Term OZ_cons      _PROTOTYPE((OZ_Term ,OZ_Term));
extern OZ_Term OZ_head      _PROTOTYPE((OZ_Term));
extern OZ_Term OZ_tail      _PROTOTYPE((OZ_Term));
extern int     OZ_length    _PROTOTYPE((OZ_Term list));

/* records */
extern OZ_Term OZ_record       _PROTOTYPE((OZ_Term, OZ_Term));
extern OZ_Term OZ_recordProp   _PROTOTYPE((OZ_Term, OZ_Term));
extern void OZ_putRecordArg    _PROTOTYPE((OZ_Term, OZ_Term, OZ_Term));
extern OZ_Term OZ_getRecordArg _PROTOTYPE((OZ_Term, OZ_Term));
#define OZ_getRecordArgC(t,s) OZ_getRecordArg(t,OZ_CToAtom(s))

/* unification */
extern OZ_Bool OZ_unify       _PROTOTYPE((OZ_Term, OZ_Term));

#define OZ_unifyFloat(t1,f)      OZ_unify(t1, OZ_CToFloat(f))
#define OZ_unifyInt(t1,i)        OZ_unify(t1, OZ_CToInt(i))
#define OZ_unifyString(t1,s)     OZ_unify(t1, OZ_CToAtom(s))

/* create a new oz variable */
extern OZ_Term OZ_newVariable();

/* cell */
extern OZ_Term OZ_newCell ();
/* exchangeCell, deepFeed */

/* name */
extern OZ_Term OZ_newName ();

/* string storage */
extern void OZ_free _PROTOTYPE((char *));

/* print warning */
extern void OZ_warning _PROTOTYPE((char * ...));

/* check for toplevel */
extern int OZ_onToplevel();

/* replace new builtins */
extern int OZ_addBuiltin _PROTOTYPE((char *, int, OZ_CFun));

/* IO */

extern int OZ_select  _PROTOTYPE((int));
extern int OZ_openIO  _PROTOTYPE((int));
extern int OZ_closeIO _PROTOTYPE((int));

/* garbage collection */
extern int OZ_protect         _PROTOTYPE((OZ_Term *));
extern int OZ_unprotect       _PROTOTYPE((OZ_Term *));

/* Suspending builtins */

OZ_Suspension OZ_makeSuspension _PROTOTYPE((OZ_CFun, OZ_Term *, int));

void OZ_addSuspension _PROTOTYPE((OZ_Term, OZ_Suspension));

/* for example
   OZ_Suspension s = OZ_makeSuspension(BIplus,OZ_args,OZ_arity);
   OZ_addSuspension(t1,s);
   OZ_addSuspension(t2,s);
   */

/* suspend self */
#define OZ_makeSelfSuspension()   OZ_makeSuspension(OZ_self,OZ_args,OZ_arity)

/* ------------------------------------------------------------------------ *
 * III. macros
 * ------------------------------------------------------------------------ */

/* variable arity is marked as follows: */
#define VarArity -1

#if defined(__GNUC__) || defined(__cplusplus)
#define OZStringify(Name) #Name
#else
#define OZStringify(Name) "Name"
#endif


#ifdef __cplusplus

#define OZ_C_proc_proto(Name)						      \
    extern "C" OZ_Bool Name(int OZ_arity, OZ_Term OZ_args[]);

#define OZ_C_proc_header(Name)						      \
    OZ_Bool Name(int OZ_arity, OZ_Term OZ_args[]) {

#else

#define OZ_C_proc_proto(Name) 						      \
  OZ_Bool Name(OZ_arity, OZ_args)

#define OZ_C_proc_header(Name)						      \
  int OZ_arity; OZ_Term OZ_args[]; {

#endif

#define OZ_C_proc_begin(Name,Arity) 					      \
    OZ_C_proc_proto(Name) 						      \
    OZ_C_proc_header(Name) 						      \
       OZ_CFun OZ_self = Name; 						      \
       if (OZ_arity != Arity && Arity != VarArity) {			      \
	 OZ_warning("Wrong arity in C proc. '%s' Expected: %d, got %d",       \
		    OZStringify(Name),Arity, OZ_arity);			      \
           return FAILED;						      \
         }


#define OZ_C_ioproc_begin(Name,Arity) 					          \
        OZ_C_proc_begin(Name,Arity) 					          \
      if (!OZ_onToplevel()) {						          \
         OZ_warning("Procedure '%s' only allowed on toplevel",OZStringify(Name)); \
           return FAILED;						          \
         }

#define OZ_C_proc_end }
#define OZ_C_ioproc_end }


						    
/* access arguments */
#define OZ_getCArg(N) OZ_args[N]

/* useful macros and functions (mm 9.2.93) */

#define OZ_nonvarArg(ARG) 						      \
{ if (OZ_isVariable(OZ_getCArg(ARG))) 					      \
      {   OZ_addSuspension(OZ_getCArg(ARG),OZ_makeSelfSuspension());          \
        return PROCEED; }                                                     \
}

#define OZ_declareIntArg(FUN,ARG,VAR) 					      \
 int VAR; 								      \
 OZ_nonvarArg(ARG); 							      \
 if (! OZ_isInt(OZ_getCArg(ARG))) {					      \
   OZ_warning("%s : arg %d must be int",FUN,ARG+1);			      \
   return FAILED;							      \
 } else {								      \
   VAR = OZ_intToC(OZ_getCArg(ARG));					      \
 }

#define OZ_declareFloatArg(FUN,ARG,VAR) 				      \
 OZ_float VAR; 								      \
 OZ_nonvarArg(ARG); 							      \
 if (! OZ_isFloat(OZ_getCArg(ARG))) {					      \
   OZ_warning("%s : arg %d must be float",FUN,ARG+1);			      \
   return FAILED;							      \
 } else {								      \
   VAR = OZ_floatToC(OZ_getCArg(ARG));					      \
 }

#define OZ_declareStringArg(FUN,ARG,VAR) 				      \
 char *VAR; 								      \
 OZ_nonvarArg(ARG); 							      \
 if (! OZ_isAtom(OZ_getCArg(ARG))) {					      \
   OZ_warning("%s: arg %d must be string",FUN,ARG+1);			      \
   return FAILED;							      \
 } else {								      \
   VAR = OZ_atomToC(OZ_getCArg(ARG));					      \
 }

/* ------------------------------------------------------------------------ *
 * end
 * ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif


#endif // __FOREIGN_H
