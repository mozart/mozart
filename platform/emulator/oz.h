/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
  */

#ifndef __OZ_H__
#define __OZ_H__

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

typedef enum {
  FAILED,
  PROCEED,
  SUSPEND,
  SLEEP,
  SCHEDULED,
  RAISE
} OZ_Return;

typedef void *OZ_Thread;
typedef void *OZ_Arity;

typedef OZ_Return (*OZ_CFun) _PROTOTYPE((int, OZ_Term *));


/* for tobias */
typedef int OZ_Boolean;
#define OZ_FALSE 0
#define OZ_TRUE 1


/* ------------------------------------------------------------------------ *
 * II. function prototypes
 * ------------------------------------------------------------------------ */

extern void OZ_main        _PROTOTYPE((int argc,char **argv));


extern OZ_Term OZ_deref    _PROTOTYPE((OZ_Term term));

/* tests */
extern int OZ_isAtom       _PROTOTYPE((OZ_Term));
extern int OZ_isBigInt     _PROTOTYPE((OZ_Term));
extern int OZ_isCell       _PROTOTYPE((OZ_Term));
extern int OZ_isChunk      _PROTOTYPE((OZ_Term));
extern int OZ_isCons       _PROTOTYPE((OZ_Term));
extern int OZ_isFalse      _PROTOTYPE((OZ_Term));
extern int OZ_isFeature    _PROTOTYPE((OZ_Term));
extern int OZ_isFloat      _PROTOTYPE((OZ_Term));
extern int OZ_isInt        _PROTOTYPE((OZ_Term));
extern int OZ_isLiteral    _PROTOTYPE((OZ_Term));
extern int OZ_isName       _PROTOTYPE((OZ_Term));
extern int OZ_isNil        _PROTOTYPE((OZ_Term));
extern int OZ_isObject     _PROTOTYPE((OZ_Term));
extern int OZ_isPair       _PROTOTYPE((OZ_Term));
extern int OZ_isPair2      _PROTOTYPE((OZ_Term));
extern int OZ_isProcedure  _PROTOTYPE((OZ_Term));
extern int OZ_isRecord     _PROTOTYPE((OZ_Term));
extern int OZ_isSmallInt   _PROTOTYPE((OZ_Term));
extern int OZ_isTrue       _PROTOTYPE((OZ_Term));
extern int OZ_isTuple      _PROTOTYPE((OZ_Term));
extern int OZ_isValue      _PROTOTYPE((OZ_Term));
extern int OZ_isVariable   _PROTOTYPE((OZ_Term));

extern int OZ_isList          _PROTOTYPE((OZ_Term, OZ_Term *));
extern int OZ_isString        _PROTOTYPE((OZ_Term, OZ_Term *));
extern int OZ_isVirtualString _PROTOTYPE((OZ_Term, OZ_Term *));

#define OZ_assertList(t) 			\
  { 						\
    OZ_Term var;				\
    if (!OZ_isList(t,&var)) {			\
      if (var == 0) return FAILED; 		\
      OZ_suspendOn(var); 			\
    } 						\
  }

#define OZ_assertString(t) 			\
  { 						\
    OZ_Term var;				\
    if (!OZ_isString(t,&var)) {			\
      if (var == 0) return FAILED; 		\
      OZ_suspendOn(var); 			\
    } 						\
  }
#define OZ_assertVirtualString(t) 		\
  { 						\
    OZ_Term var;				\
    if (!OZ_isVirtualString(t,&var)) {		\
      if (var == 0) return FAILED; 		\
      OZ_suspendOn(var); 			\
    } 						\
  }

/*
 * mm2: should we support this ?
 */
extern OZ_Term OZ_termType _PROTOTYPE((OZ_Term));

/* convert: C from/to Oz datastructure */

extern char *   OZ_atomToC         _PROTOTYPE((OZ_Term));
extern OZ_Term  OZ_atom            _PROTOTYPE((char *));
extern int	OZ_featureCmp      _PROTOTYPE((OZ_Term,OZ_Term));

extern int      OZ_smallIntMin     _PROTOTYPE((void));
extern int      OZ_smallIntMax     _PROTOTYPE((void));
extern OZ_Term  OZ_false 	   _PROTOTYPE((void));
extern OZ_Term  OZ_true 	   _PROTOTYPE((void));
extern OZ_Term  OZ_int             _PROTOTYPE((int));
extern int      OZ_getMinPrio      _PROTOTYPE((void));
extern int      OZ_getMaxPrio      _PROTOTYPE((void));
extern int      OZ_intToC          _PROTOTYPE((OZ_Term));
extern OZ_Term  OZ_CStringToInt    _PROTOTYPE((char *str));
extern char *   OZ_parseInt        _PROTOTYPE((char *s));

extern OZ_Term  OZ_float           _PROTOTYPE((double));
extern double   OZ_floatToC        _PROTOTYPE((OZ_Term));

extern OZ_Term  OZ_CStringToFloat  _PROTOTYPE((char *s));
extern char *   OZ_parseFloat      _PROTOTYPE((char *s));

extern OZ_Term  OZ_CStringToNumber _PROTOTYPE((char *));

extern char *   OZ_toC       _PROTOTYPE((OZ_Term, int, int));

extern OZ_Term  OZ_string    _PROTOTYPE((char *));
extern char *   OZ_stringToC _PROTOTYPE((OZ_Term t));

extern void     OZ_printVirtualString   _PROTOTYPE((OZ_Term t));
#define OZ_printVS(t) OZ_printVirtualString(t)


/* mm2: impl? */
extern OZ_Term  OZ_termToVS  _PROTOTYPE((OZ_Term t));

/* tuples */
extern OZ_Term OZ_label     _PROTOTYPE((OZ_Term));
extern int     OZ_width     _PROTOTYPE((OZ_Term));
extern OZ_Term OZ_tuple     _PROTOTYPE((OZ_Term, int));
#define OZ_tupleC(s,n) OZ_tuple(OZ_atom(s),n)
extern OZ_Term OZ_mkTuple   _PROTOTYPE((OZ_Term label,int arity,...));
extern OZ_Term OZ_mkTupleC  _PROTOTYPE((char *label,int arity,...));

extern void    OZ_putArg  _PROTOTYPE((OZ_Term, int, OZ_Term));
extern OZ_Term OZ_getArg  _PROTOTYPE((OZ_Term, int));
extern OZ_Term OZ_nil       _PROTOTYPE(());
extern OZ_Term OZ_cons      _PROTOTYPE((OZ_Term ,OZ_Term));
extern OZ_Term OZ_head      _PROTOTYPE((OZ_Term));
extern OZ_Term OZ_tail      _PROTOTYPE((OZ_Term));
extern int     OZ_length    _PROTOTYPE((OZ_Term list));


extern OZ_Term OZ_pair      _PROTOTYPE((int));
extern OZ_Term OZ_pair2     _PROTOTYPE((OZ_Term t1,OZ_Term t2));

#define OZ_pairA(s1,t)      OZ_pair2(OZ_atom(s1),t)
#define OZ_pairAI(s1,i)     OZ_pair2(OZ_atom(s1),OZ_int(i))
#define OZ_pairAA(s1,s2)    OZ_pair2(OZ_atom(s1),OZ_atom(s2))
#define OZ_pairAS(s1,s2)    OZ_pair2(OZ_atom(s1),OZ_string(s2))


/* records */
extern OZ_Arity OZ_makeArity     _PROTOTYPE((OZ_Term list));
extern OZ_Term OZ_record         _PROTOTYPE((OZ_Term, OZ_Term));
extern OZ_Term OZ_recordInit     _PROTOTYPE((OZ_Term, OZ_Term));
extern void OZ_putSubtree        _PROTOTYPE((OZ_Term, OZ_Term, OZ_Term));
extern OZ_Term OZ_subtree        _PROTOTYPE((OZ_Term, OZ_Term));
extern OZ_Term OZ_arityList      _PROTOTYPE((OZ_Term));

#define OZ_getRecordArgA(t,s)    OZ_getRecordArg(t,OZ_atom(s))
#define OZ_putRecordArgA(t,s,v)  OZ_putRecordArg(t,OZ_atom(s),v)

/* unification */
extern OZ_Return OZ_unify    _PROTOTYPE((OZ_Term, OZ_Term));
extern int OZ_eq                 _PROTOTYPE((OZ_Term, OZ_Term));

#define OZ_unifyFloat(t1,f)      OZ_unify(t1, OZ_float(f))
#define OZ_unifyInt(t1,i)        OZ_unify(t1, OZ_int(i))
#define OZ_unifyAtom(t1,s)       OZ_unify(t1, OZ_atom(s))

/* create a new oz variable */
extern OZ_Term OZ_newVariable();

extern OZ_Term OZ_newChunk _PROTOTYPE((OZ_Term));

/* cell */
extern OZ_Term OZ_newCell _PROTOTYPE((OZ_Term));
/* exchangeCell, deepFeed */

/* name */
extern OZ_Term OZ_newName ();

/* print warning */
extern void OZ_warning _PROTOTYPE((char * ...));

/* generate the unix error string from an errno (see perror(3)) */
char *OZ_unixError _PROTOTYPE((int err));

/* check for toplevel */
extern int OZ_onToplevel ();

extern int OZ_addBuiltin _PROTOTYPE((char *, int, OZ_CFun));

/* replace new builtins */
struct OZ_BIspec {
  char *name;
  int arity;
  OZ_CFun fun;
};

/* add specification to builtin table */
void OZ_addBISpec _PROTOTYPE((OZ_BIspec *spec));

/* IO */

extern OZ_Return OZ_readSelect  _PROTOTYPE((int, OZ_Term, OZ_Term));
extern OZ_Return OZ_writeSelect _PROTOTYPE((int, OZ_Term, OZ_Term));
extern void    OZ_deSelect    _PROTOTYPE((int));

/* garbage collection */
extern int OZ_protect         _PROTOTYPE((OZ_Term *));
extern int OZ_unprotect       _PROTOTYPE((OZ_Term *));

/* raise exception */
extern OZ_Return OZ_typeError   _PROTOTYPE((int pos,char *type));
extern OZ_Return OZ_raise	    _PROTOTYPE((OZ_Term));

/* Suspending builtins */

OZ_Thread  OZ_makeThread      _PROTOTYPE((OZ_CFun, OZ_Term *, int));
void       OZ_addThread       _PROTOTYPE((OZ_Term, OZ_Thread));

/* for example
   OZ_Thread s = OZ_makeThread(BIplus,OZ_args,OZ_arity);
   OZ_addThread(t1,s);
   OZ_addThread(t2,s);
   */

/* suspend self */
#define OZ_makeSelfThread()   OZ_makeThread(OZ_self,OZ_args,OZ_arity)

void OZ_suspendOnInternal  _PROTOTYPE((OZ_Term));
void OZ_suspendOnInternal2 _PROTOTYPE((OZ_Term,OZ_Term));
void OZ_suspendOnInternal3 _PROTOTYPE((OZ_Term,OZ_Term,OZ_Term));

#define OZ_suspendOn(t1) \
   { OZ_suspendOnInternal(t1); return SUSPEND; }
#define OZ_suspendOn2(t1,t2) \
   { OZ_suspendOnInternal2(t1,t2); return SUSPEND; }
#define OZ_suspendOn3(t1,t2,t3) \
   { OZ_suspendOnInternal3(t1,t2,t3); return SUSPEND; }

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
    extern "C" OZ_Return Name(int OZ_arity, OZ_Term OZ_args[]);

#define OZ_C_proc_header(Name)						      \
    OZ_Return Name(int OZ_arity, OZ_Term OZ_args[]) {

#else

#define OZ_C_proc_proto(Name) 						      \
  OZ_Return Name(OZ_arity, OZ_args)

#define OZ_C_proc_header(Name)						      \
  int OZ_arity; OZ_Term OZ_args[]; {

#endif

#define OZ_C_proc_begin(Name,Arity) 					      \
    OZ_C_proc_proto(Name) 						      \
    OZ_C_proc_header(Name) 						      \
       OZ_CFun OZ_self = Name; 						      \
       if (OZ_arity != Arity && Arity != VarArity) {			      \
	 OZ_warning("Wrong arity in C procedure '%s'. Expected: %d, got %d",  \
		    OZStringify(Name),Arity, OZ_arity);			      \
         return FAILED;							      \
       }

#define OZ_C_proc_end }


#define OZ_C_ioproc_begin(Name,Arity) 				              \
OZ_C_proc_begin(Name,Arity) 				          	      \
  if (!OZ_onToplevel()) {					              \
    OZ_warning("Procedure '%s' only allowed on toplevel",OZStringify(Name));  \
    return FAILED;						              \
  }

#define OZ_C_ioproc_end }


						    
/* access arguments */
#define OZ_getCArg(N) OZ_args[N]

/* useful macros and functions (mm 9.2.93) */

#define OZ_declareArg(ARG,VAR) \
     OZ_Term VAR = OZ_getCArg(ARG);

#define OZ_nonvarArg(ARG)			\
{						\
  if (OZ_isVariable(OZ_getCArg(ARG))) {		\
    OZ_suspendOn(OZ_getCArg(ARG));		\
  }						\
}

#define OZ_declareIntArg(ARG,VAR)		\
 int VAR;					\
 OZ_nonvarArg(ARG);				\
 if (! OZ_isInt(OZ_getCArg(ARG))) {		\
   return OZ_typeError(ARG,"Int");		\
 } else {					\
   VAR = OZ_intToC(OZ_getCArg(ARG));		\
 }

#define OZ_declareFloatArg(ARG,VAR) 					      \
 double VAR; 								      \
 OZ_nonvarArg(ARG); 							      \
 if (! OZ_isFloat(OZ_getCArg(ARG))) {					      \
   return OZ_typeError(ARG,"Float");					      \
   return FAILED;							      \
 } else {								      \
   VAR = OZ_floatToC(OZ_getCArg(ARG));					      \
 }


#define OZ_declareAtomArg(ARG,VAR) 					      \
 char *VAR; 								      \
 OZ_nonvarArg(ARG); 							      \
 if (! OZ_isAtom(OZ_getCArg(ARG))) {					      \
   return OZ_typeError(ARG,"Atom");					      \
 } else {								      \
   VAR = OZ_atomToC(OZ_getCArg(ARG));					      \
 }

#define OZ_declareStringArg(ARG,VAR)		\
 char *VAR;					\
 {						\
   OZ_Term OZ_avar;				\
   if (!OZ_isString(OZ_getCArg(ARG),&OZ_avar)) {	\
     if (OZ_avar == 0) {			\
       return OZ_typeError(ARG,"Atom");		\
     } else {					\
       OZ_suspendOn(OZ_avar);			\
     }						\
   }						\
   VAR = OZ_stringToC(OZ_getCArg(ARG));		\
 }

/* ------------------------------------------------------------------------ *
 * end
 * ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif

/* ------------------------------------------------------------------------ *
 * oz_meta.h
 * ------------------------------------------------------------------------ */

#ifdef __cplusplus
extern "C" { 
#endif

  
typedef enum {
  meta_unconstr     = 0,
  meta_det          = 1,
  meta_left_constr  = 2,
  meta_right_constr = 4,
  meta_fail         = 8
} mur_t;

typedef enum {
  OZ_Type_Cell,
  OZ_Type_Chunk,
  OZ_Type_Cons,
  OZ_Type_HeapChunk,
  OZ_Type_CVar,
  OZ_Type_Float,
  OZ_Type_Int,
  OZ_Type_Literal,
  OZ_Type_Procedure,
  OZ_Type_Record,
  OZ_Type_Tuple,
  OZ_Type_Var,
  OZ_Type_Unknown
} OZ_TermType;


typedef void * OZ_MetaType;

typedef mur_t (* OZ_UnifyMetaDet) (OZ_Term, OZ_Term, OZ_Term, OZ_TermType, OZ_Term *);
typedef mur_t (* OZ_UnifyMetaMeta) (OZ_Term, OZ_Term, OZ_Term, OZ_Term, OZ_MetaType, OZ_Term *);

typedef char * (* OZ_PrintMeta) (OZ_Term, int);
typedef int (* OZ_IsSingleValue) (OZ_Term);


extern OZ_TermType OZ_typeOf        _PROTOTYPE((OZ_Term t));

extern OZ_MetaType OZ_introMetaTerm  _PROTOTYPE((OZ_UnifyMetaDet unify_md,
						OZ_UnifyMetaMeta unify_mm,
						OZ_PrintMeta print,
						OZ_IsSingleValue sgl_val,
						char * name));

extern OZ_Term OZ_makeMetaTerm       _PROTOTYPE((OZ_MetaType t,
						OZ_Term d));

extern OZ_MetaType OZ_getMetaTermType _PROTOTYPE((OZ_Term v));
extern void OZ_putMetaTermType        _PROTOTYPE((OZ_Term v, OZ_MetaType t));

extern OZ_Term OZ_getMetaTermAttr     _PROTOTYPE((OZ_Term v));

extern OZ_Term OZ_makeHeapChunk      _PROTOTYPE((int s));
extern char * OZ_getHeapChunkData    _PROTOTYPE((OZ_Term t));
extern int OZ_getHeapChunkSize       _PROTOTYPE((OZ_Term t));
extern int OZ_isHeapChunk            _PROTOTYPE((OZ_Term t));
extern int OZ_isMetaTerm              _PROTOTYPE((OZ_Term t));
extern int OZ_isSingleValue          _PROTOTYPE((OZ_Term t));
extern OZ_Return OZ_constrainMetaTerm   _PROTOTYPE((OZ_Term v,
						 OZ_MetaType t,
						 OZ_Term d));

extern int OZ_areIdentVars           _PROTOTYPE((OZ_Term v1,
						 OZ_Term v2));

extern OZ_Return OZ_suspendMetaProp    _PROTOTYPE((OZ_CFun, OZ_Term *, int));

#define OZ_MetaPropSuspend OZ_suspendMetaProp(OZ_self, OZ_args, OZ_arity)

#ifdef __cplusplus
}
#endif

#endif /* __OZ_H__ */




