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
typedef double OZ_Float;

typedef enum {
  FAILED,
  PROCEED,
  SUSPEND,
  SLEEP,
  RAISE
} OZ_Bool;

typedef int OZ_Boolean;
#define OZ_FALSE 0
#define OZ_TRUE  1


typedef void *OZ_Thread;

typedef OZ_Bool (*OZ_CFun) _PROTOTYPE((int, OZ_Term *));

/* ------------------------------------------------------------------------ *
 * II. function prototypes
 * ------------------------------------------------------------------------ */

/* tests */
extern int OZ_isAtom       _PROTOTYPE((OZ_Term));
extern int OZ_isCell       _PROTOTYPE((OZ_Term));
extern int OZ_isChunk      _PROTOTYPE((OZ_Term));
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

extern OZ_Bool OZ_typeError   _PROTOTYPE((int pos,char *type));

/* convert: C from/to Oz datastructure */

extern char *   OZ_atomToC    _PROTOTYPE((OZ_Term));
extern char *   OZ_literalToC _PROTOTYPE((OZ_Term));
extern OZ_Term  OZ_CToAtom    _PROTOTYPE((char *));

extern int      OZ_getMinInt     _PROTOTYPE((void));
extern int      OZ_getMaxInt     _PROTOTYPE((void));
extern OZ_Term  OZ_getNameFalse  _PROTOTYPE((void));
extern OZ_Term  OZ_getNameTrue   _PROTOTYPE((void));
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
#define OZ_CfloatToCString(F) OZ_normFloat(OZ_floatToCString(OZ_CToFloat(F)))


extern OZ_Term  OZ_CStringToNumber _PROTOTYPE((char *));

extern char *   OZ_toC       _PROTOTYPE((OZ_Term));
extern char *   OZ_toC1      _PROTOTYPE((OZ_Term, int));

extern OZ_Term  OZ_CToString _PROTOTYPE((char *));
extern char *   OZ_stringToC _PROTOTYPE((OZ_Term t));

extern void     OZ_printVS   _PROTOTYPE((OZ_Term t));
extern OZ_Term  OZ_termToVS  _PROTOTYPE((OZ_Term t));

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

extern OZ_Term OZ_pair      _PROTOTYPE((OZ_Term t1,OZ_Term t2));

#define OZ_pairA(s1,t)      OZ_pair(OZ_CToAtom(s1),t)
#define OZ_pairAI(s1,i)     OZ_pair(OZ_CToAtom(s1),OZ_CToInt(i))
#define OZ_pairAA(s1,s2)    OZ_pair(OZ_CToAtom(s1),OZ_CToAtom(s2))
#define OZ_pairAS(s1,s2)    OZ_pair(OZ_CToAtom(s1),OZ_CToString(s2))


/* records */
extern OZ_Term OZ_record         _PROTOTYPE((OZ_Term, OZ_Term));
extern OZ_Term OZ_recordInit     _PROTOTYPE((OZ_Term, OZ_Term));
extern void OZ_putRecordArg      _PROTOTYPE((OZ_Term, OZ_Term, OZ_Term));
extern OZ_Term OZ_getRecordArg   _PROTOTYPE((OZ_Term, OZ_Term));
extern OZ_Term OZ_getRecordArity _PROTOTYPE((OZ_Term));

#define OZ_getRecordArgA(t,s)    OZ_getRecordArg(t,OZ_CToAtom(s))
#define OZ_putRecordArgA(t,s,v)  OZ_putRecordArg(t,OZ_CToAtom(s),v)

/* unification */
extern OZ_Bool OZ_unify       _PROTOTYPE((OZ_Term, OZ_Term));

#define OZ_unifyFloat(t1,f)      OZ_unify(t1, OZ_CToFloat(f))
#define OZ_unifyInt(t1,i)        OZ_unify(t1, OZ_CToInt(i))
#define OZ_unifyString(t1,s)     OZ_unify(t1, OZ_CToAtom(s))

/* create a new oz variable */
extern OZ_Term OZ_newVariable();

extern OZ_Term OZ_newChunk _PROTOTYPE((OZ_Term));

/* cell */
extern OZ_Term OZ_newCell _PROTOTYPE((OZ_Term));
/* exchangeCell, deepFeed */

/* name */
extern OZ_Term OZ_newName ();

/* string storage */
extern void OZ_free _PROTOTYPE((char *));

/* print warning */
extern void OZ_warning _PROTOTYPE((char * ...));

/* generate the unix error string from an errno (see perror(3)) */
char *OZ_unixError _PROTOTYPE((int err));

/* check for toplevel */
extern int OZ_onToplevel ();

/* replace new builtins */
extern int OZ_addBuiltin _PROTOTYPE((char *, int, OZ_CFun));

/* IO */

extern OZ_Bool OZ_readSelect  _PROTOTYPE((int, OZ_Term, OZ_Term));
extern OZ_Bool OZ_writeSelect _PROTOTYPE((int, OZ_Term, OZ_Term));
extern void    OZ_deSelect    _PROTOTYPE((int));

/* garbage collection */
extern int OZ_protect         _PROTOTYPE((OZ_Term *));
extern int OZ_unprotect       _PROTOTYPE((OZ_Term *));

/* raise exception */

OZ_Bool OZ_raise              _PROTOTYPE((OZ_Term));

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

OZ_Bool OZ_suspendOnVar  _PROTOTYPE((OZ_Term));
OZ_Bool OZ_suspendOnVar2 _PROTOTYPE((OZ_Term,OZ_Term));
OZ_Bool OZ_suspendOnVar3 _PROTOTYPE((OZ_Term,OZ_Term,OZ_Term));

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

#define OZ_C_proc_proto(Name)                                                 \
    extern "C" OZ_Bool Name(int OZ_arity, OZ_Term OZ_args[]);

#define OZ_C_proc_header(Name)                                                \
    OZ_Bool Name(int OZ_arity, OZ_Term OZ_args[]) {

#else

#define OZ_C_proc_proto(Name)                                                 \
  OZ_Bool Name(OZ_arity, OZ_args)

#define OZ_C_proc_header(Name)                                                \
  int OZ_arity; OZ_Term OZ_args[]; {

#endif

#define OZ_C_proc_begin(Name,Arity)                                           \
    OZ_C_proc_proto(Name)                                                     \
    OZ_C_proc_header(Name)                                                    \
       OZ_CFun OZ_self = Name;                                                \
       if (OZ_arity != Arity && Arity != VarArity) {                          \
         OZ_warning("Wrong arity in C procedure '%s'. Expected: %d, got %d",  \
                    OZStringify(Name),Arity, OZ_arity);                       \
         return FAILED;                                                       \
       }

#define OZ_C_proc_end }


#define OZ_C_ioproc_begin(Name,Arity)                                         \
OZ_C_proc_begin(Name,Arity)                                                   \
  if (!OZ_onToplevel()) {                                                     \
    OZ_warning("Procedure '%s' only allowed on toplevel",OZStringify(Name));  \
    return FAILED;                                                            \
  }

#define OZ_C_ioproc_end }



/* access arguments */
#define OZ_getCArg(N) OZ_args[N]

/* useful macros and functions (mm 9.2.93) */

#define OZ_declareArg(ARG,VAR) \
     OZ_Term VAR = OZ_getCArg(ARG);

#define OZ_nonvarArg(ARG)                                                     \
{                                                                             \
  if (OZ_isVariable(OZ_getCArg(ARG))) {                                       \
    return OZ_suspendOnVar(OZ_getCArg(ARG));                                  \
  }                                                                           \
}

#define OZ_declareIntArg(FUN,ARG,VAR)                                         \
 int VAR;                                                                     \
 OZ_nonvarArg(ARG);                                                           \
 if (! OZ_isInt(OZ_getCArg(ARG))) {                                           \
   return OZ_typeError(ARG,"Int");                                            \
 } else {                                                                     \
   VAR = OZ_intToC(OZ_getCArg(ARG));                                          \
 }

#define OZ_declareFloatArg(FUN,ARG,VAR)                                       \
 OZ_Float VAR;                                                                \
 OZ_nonvarArg(ARG);                                                           \
 if (! OZ_isFloat(OZ_getCArg(ARG))) {                                         \
   return OZ_typeError(ARG,"Float");                                          \
   return FAILED;                                                             \
 } else {                                                                     \
   VAR = OZ_floatToC(OZ_getCArg(ARG));                                        \
 }


#define OZ_declareAtomArg(FUN,ARG,VAR)                                        \
 char *VAR;                                                                   \
 OZ_nonvarArg(ARG);                                                           \
 if (! OZ_isAtom(OZ_getCArg(ARG))) {                                          \
   return OZ_typeError(ARG,"Atom");                                           \
 } else {                                                                     \
   VAR = OZ_atomToC(OZ_getCArg(ARG));                                         \
 }

/* the following one is obsolete */
#define OZ_declareStringArg(FUN,ARG,VAR) OZ_declareAtomArg(FUN,ARG,VAR)

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
extern OZ_Bool OZ_constrainMetaTerm   _PROTOTYPE((OZ_Term v,
                                                 OZ_MetaType t,
                                                 OZ_Term d));

extern int OZ_areIdentVars           _PROTOTYPE((OZ_Term v1,
                                                 OZ_Term v2));

extern OZ_Bool OZ_suspendMetaProp    _PROTOTYPE((OZ_CFun, OZ_Term *, int));

#define OZ_MetaPropSuspend OZ_suspendMetaProp(OZ_self, OZ_args, OZ_arity)

#ifdef __cplusplus
}
#endif

#endif // __OZ_H__
