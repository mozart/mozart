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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __MOZART_H__
#define __MOZART_H__


/* ------------------------------------------------------------------------ *
 * 0. intro
 * ------------------------------------------------------------------------ */

#ifndef NOFINALIZATION
#define FINALIZATION
#endif

/* calling convention "cdecl" under win32 */
#if defined(__WATCOMC__) || defined(__MINGW32__) || defined(__CYGWIN32__)
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

/* we use function pointers only when creating DLLs
 * STATIC_FUNCTIONS is defined when compiling the emulator */
#if defined(OZWIN) && !defined(WINDOWS_EMULATOR)
#define OzFun(fun) (ozcdecl *fun)
#else
#define OzFun(fun) (ozcdecl fun)
#endif

#if defined(__STDC__) || defined(__cplusplus) || __BORLANDC__ || _MSC_VER
#define _FUNDECL(fun,arglist) OzFun(fun) arglist
#define _FUNTYPEDECL(fun,arglist) (ozcdecl *fun) arglist
#if defined(__cplusplus)
extern "C" {
#endif
#else
#define _FUNDECL(fun,ignore) OzFun(fun) ()
#define _FUNTYPEDECL(fun,ignore) (ozcdecl *fun) ()
#endif

#define FUNDECL(fun,args) _FUNDECL(fun,args)

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

typedef int OZ_Boolean;
#define OZ_FALSE 0
#define OZ_TRUE 1

/* ------------------------------------------------------------------------ *
 * II. function prototypes
 * ------------------------------------------------------------------------ */

extern void     _FUNDECL(OZ_main, (int argc,char **argv));


extern OZ_Term  _FUNDECL(OZ_deref,(OZ_Term term));

/* tests */
extern int _FUNDECL(OZ_isBool,(OZ_Term));
extern int _FUNDECL(OZ_isAtom,(OZ_Term));
extern int _FUNDECL(OZ_isBigInt,(OZ_Term));
extern int _FUNDECL(OZ_isCell,(OZ_Term));
extern int _FUNDECL(OZ_isThread,(OZ_Term));
extern int _FUNDECL(OZ_isPort,(OZ_Term));
extern int _FUNDECL(OZ_isChunk,(OZ_Term));
extern int _FUNDECL(OZ_isDictionary,(OZ_Term));
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
extern int _FUNDECL(OZ_isBitString,(OZ_Term));
extern int _FUNDECL(OZ_isByteString,(OZ_Term));

extern int _FUNDECL(OZ_isList,(OZ_Term, OZ_Term *));
extern int _FUNDECL(OZ_isString,(OZ_Term, OZ_Term *));
extern int _FUNDECL(OZ_isProperString,(OZ_Term, OZ_Term *));
extern int _FUNDECL(OZ_isVirtualString,(OZ_Term, OZ_Term *));

/* heap chunks */

extern OZ_Term   _FUNDECL(OZ_makeHeapChunk,(int s));
extern void *    _FUNDECL(OZ_getHeapChunkData,(OZ_Term t));
extern int       _FUNDECL(OZ_getHeapChunkSize,(OZ_Term t));
extern int       _FUNDECL(OZ_isHeapChunk,(OZ_Term t));

extern unsigned int _FUNDECL(OZ_getUniqueId,(void));


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
extern int     _FUNDECL(OZ_boolToC,(OZ_Term));
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
extern char *  _FUNDECL(OZ_stringToC,(OZ_Term t,int*n));

extern char*   _FUNDECL(OZ_vsToC,(OZ_Term t,int*n));
extern char *  _FUNDECL(OZ_virtualStringToC,(OZ_Term t,int*n));
extern OZ_Term _FUNDECL(OZ_mkByteString,(char*,int));


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
#define OZ_recordInitC(s,t) OZ_recordInit(OZ_atom(s),t)
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

/* foreign pointer */
extern OZ_Term  _FUNDECL(OZ_makeForeignPointer,(void*));
extern void*    _FUNDECL(OZ_getForeignPointer,(OZ_Term));
extern int      _FUNDECL(OZ_isForeignPointer,(OZ_Term));

/* interface */

typedef struct {
  const char * name;
  short        inArity;
  short        outArity;
  OZ_CFun      func;
} OZ_C_proc_interface;

/* pickles */

typedef struct {
  char *data;  /* NULL on error */
  int size;    /* contain error code */
} OZ_Datum;


#define OZ_DATUM_UNKNOWNERROR -1
#define OZ_DATUM_OUTOFMEMORY  -2

extern OZ_Return _FUNDECL(OZ_valueToDatum,(OZ_Term  t, OZ_Datum* d));
extern OZ_Return _FUNDECL(OZ_datumToValue,(OZ_Datum d, OZ_Term   t));

/* print warnings/errors */
extern void _FUNDECL(OZ_warning,(CONST char *, ...));
extern void _FUNDECL(OZ_error,(CONST char *, ...));

/* generate the unix error string from an errno (see perror(3)) */
extern char * _FUNDECL(OZ_unixError,(int err));

/* check for toplevel */
extern int _FUNDECL(OZ_onToplevel,());

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
extern void _FUNDECL(OZ_collect,(OZ_Term *));

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
#define OZ_BI_proto(Name)  extern "C" OZ_Return (ozcdecl Name)(OZ_Term [],int [])
#else
#define OZ_BI_proto(Name)  extern OZ_Return (ozcdecl Name)()
#endif

#define OZ_ID_MAP 0
#define OZ_in(N) _OZ_ARGS[_OZ_LOC==OZ_ID_MAP?N:_OZ_LOC[N]]
#define OZ_out(N) _OZ_ARGS[_OZ_LOC==OZ_ID_MAP?_OZ_arity+N:_OZ_LOC[_OZ_arity+N]]
#define OZ_result(V) OZ_out(0)=V

#define OZ_BI_define(Name,Arity_IN,Arity_OUT)                   \
OZ_BI_proto(Name);                                              \
OZ_Return (ozcdecl Name)(OZ_Term _OZ_ARGS[],int _OZ_LOC[]) {    \
    const int _OZ_arity = Arity_IN;

#define OZ_BI_end }

#define OZ_RETURN(V) return ((OZ_result(V)),PROCEED)
#define OZ_RETURN_INT(I) OZ_RETURN(OZ_int(I))
#define OZ_RETURN_ATOM(S) OZ_RETURN(OZ_atom(S))
#define OZ_RETURN_STRING(S) OZ_RETURN(OZ_string(S))

/* ------------------------------------------------------------------------ *
 * More nice macros contributed by Denys Duchier
 * ------------------------------------------------------------------------ */

#define OZ_RETURN_BOOL(X) \
OZ_RETURN((X)?OZ_true():OZ_false())

#define OZ_expectDet(ARG)                       \
{                                               \
  if (OZ_isVariable(OZ_in(ARG)))                \
    { OZ_suspendOn(OZ_in(ARG)); }               \
}


#define OZ__doTerm(TYPE,ARG,VAR)                \
TYPE VAR = OZ_in(ARG);

#define OZ_declareTerm(ARG,VAR)                 \
OZ__doTerm(OZ_Term,ARG,VAR);

#define OZ_setTerm(ARG,VAR)                     \
OZ__doTerm(,ARG,VAR);

#define OZ__doDetTerm(TYPE,ARG,VAR)             \
OZ_expectDet(ARG);                              \
OZ__doTerm(TYPE,ARG,VAR);

#define OZ_declareDetTerm(ARG,VAR)              \
OZ__doDetTerm(OZ_Term,ARG,VAR);

#define OZ_setDetTerm(ARG,VAR)                  \
OZ__doDetTerm(,ARG,VAR);

/*
 * OZ_expectType(ARG,MSG,CHECK)
 *
 * causes the builtin to suspend until argument number ARG
 * is determined. it then uses the single argument function
 * CHECK to test that the term is of the expected type.  If not,
 * a type exception is raised, using MSG as the type name.
 */

#define OZ_expectType(ARG,MSG,CHECK)            \
OZ_expectDet(ARG);                              \
if (!CHECK(OZ_in(ARG))) {                       \
  return OZ_typeError(ARG,MSG);                 \
}

#define OZ_expectBool(ARG)                      \
OZ_expectType(ARG,"Bool",OZ_isBool)

#define OZ_expectInt(ARG)                       \
OZ_expectType(ARG,"Int",OZ_isInt)

#define OZ_expectFloat(ARG)                     \
OZ_expectType(ARG,"Float",OZ_isFloat)

#define OZ_expectAtom(ARG)                      \
OZ_expectType(ARG,"Atom",OZ_isAtom)

#define OZ_expectBitString(ARG)                 \
OZ_expectType(ARG,"BitString",OZ_isBitString)

#define OZ_expectByteString(ARG)                \
OZ_expectType(ARG,"ByteString",OZ_isByteString)

#define OZ_expectForeignPointer(ARG)            \
OZ_expectType(ARG,"ForeignPointer",OZ_isForeignPointer)

/*
 * OZ_declareType(ARG,VAR,TYPE,MSG,CHECK,COERCE)
 *
 * calls OZ_expectType (see above), and, if the call succeeds,
 * declares a variable VAR of TYPE and initializes it with
 * the value that can be obtained from the ARG argument by
 * applying the conversion function COERCE.
 */

#define OZ__doType(ARG,VAR,TYPE,MSG,CHECK,COERCE) \
OZ_expectType(ARG,MSG,CHECK);                   \
TYPE VAR = COERCE(OZ_in(ARG));

#define OZ_declareType(ARG,VAR,TYPE,MSG,CHECK,COERCE) \
OZ__doType(ARG,VAR,TYPE,MSG,CHECK,COERCE)

#define OZ_setType(ARG,VAR,TYPE,MSG,CHECK,COERCE) \
OZ__doType(ARG,VAR,,MSG,CHECK,COERCE)

#define OZ_declareBool(ARG,VAR)                 \
OZ_declareType(ARG,VAR,int,"Bool",OZ_isBool,OZ_boolToC)

#define OZ_setBool(ARG,VAR)                     \
OZ_setType(ARG,VAR,int,"Bool",OZ_isBool,OZ_boolToC)

#define OZ_declareInt(ARG,VAR)                  \
OZ_declareType(ARG,VAR,int,"Int",OZ_isInt,OZ_intToC)

#define OZ_setInt(ARG,VAR)                      \
OZ_setType(ARG,VAR,int,"Int",OZ_isInt,OZ_intToC)

#define OZ_declareFloat(ARG,VAR)                \
OZ_declareType(ARG,VAR,double,"Float",OZ_isFloat,OZ_floatToC)

#define OZ_setFloat(ARG,VAR)                    \
OZ_setType(ARG,VAR,double,"Float",OZ_isFloat,OZ_floatToC)

#define OZ_declareAtom(ARG,VAR)                 \
OZ_declareType(ARG,VAR,CONST char*,"Atom",OZ_isAtom,OZ_atomToC)

#define OZ_setAtom(ARG,VAR)                     \
OZ_setType(ARG,VAR,CONST char*,"Atom",OZ_isAtom,OZ_atomToC)

#define OZ_declareBitString(ARG,VAR)            \
OZ_declareType(ARG,VAR,BitString*,"BitString",  \
               OZ_isBitString,tagged2BitString)

#define OZ_setBitString(ARG,VAR)                \
OZ_setType(ARG,VAR,BitString*,"BitString",      \
               OZ_isBitString,tagged2BitString)

#define OZ_declareByteString(ARG,VAR)           \
OZ_declareType(ARG,VAR,ByteString*,"ByteString",\
               OZ_isByteString,tagged2ByteString)

#define OZ_setByteString(ARG,VAR)               \
OZ_setType(ARG,VAR,ByteString*,"ByteString",    \
               OZ_isByteString,tagged2ByteString)

#define OZ_declareForeignPointer(ARG,VAR)       \
OZ_declareType(ARG,VAR,void*,"ForeignPointer",  \
        OZ_isForeignPointer,OZ_getForeignPointer)

#define OZ_setForeignPointer(ARG,VAR)           \
OZ_setType(ARG,VAR,void*,"ForeignPointer",      \
        OZ_isForeignPointer,OZ_getForeignPointer)

/*
 * OZ_declareForeignType(ARG,VAR,TYPE)
 *
 * this is a specialization of foreign pointer stuff.
 * TYPE should be a pointer type. The foreign pointer is
 * coerced to that type.
 */

#define OZ_declareForeignType(ARG,VAR,TYPE)     \
OZ_expectForeignPointer(ARG);                   \
TYPE VAR = (TYPE) OZ_getForeignPointer(OZ_in(ARG));

#define OZ_setForeignType(ARG,VAR,TYPE) \
OZ_expectForeignPointer(ARG);                   \
VAR = (TYPE) OZ_getForeignPointer(OZ_in(ARG));

/*
 * OZ_expectRecType(ARG,MSG,CHECK)
 *
 * When a value has components, in order to check that it is
 * well formed, we may have to wait until some or all of these
 * subcomponents are determined. CHECK (the predicate that
 * verifies that the value is of the appropriate type, is here
 * assumed to take 2 arguments: the 1st argument is the term
 * to be checked, and the second argument is a pointer to an
 * OZ_Term in which the next undetermined components will be
 * stored in case we need to wait.
 */

#define OZ_expectRecType(ARG,MSG,CHECK)         \
{                                               \
  OZ_Term OZ__aux__;                            \
  if (!CHECK(OZ_in(ARG),&OZ__aux__)) {          \
    if (OZ__aux__ == 0) {                       \
      return OZ_typeError(ARG,MSG);             \
    } else {                                    \
      OZ_suspendOn(OZ__aux__);                  \
    }                                           \
  }                                             \
}

#define OZ_expectString(ARG)                    \
OZ_expectRecType(ARG,"String",OZ_isProperString);

#define OZ_expectVirtualString(ARG)             \
OZ_expectRecType(ARG,"VirtualString",OZ_isVirtualString);

/*
 * OZ_declareRecType(ARG,VAR,TYPE,MSG,CHECK,COERCE)
 *
 * just what you would expect.  Note, however, that strings
 * and virtual strings are converted to char arrays that last
 * only till the next call to the conversion function COERCE.
 */

#define OZ_declareRecType(ARG,VAR,TYPE,MSG,CHECK,COERCE) \
OZ_expectRecType(ARG,MSG,CHECK);                \
TYPE VAR = COERCE(OZ_in(ARG));

#define OZ_setRecType(ARG,VAR,TYPE,MSG,CHECK,COERCE) \
OZ_expectRecType(ARG,MSG,CHECK);                \
VAR = COERCE(OZ_in(ARG));

#define oz_str2c(t) OZ_stringToC(t,0)
#define oz_vs2c(t) OZ_vsToC(t,0)

#define OZ_declareString(ARG,VAR)               \
OZ_declareRecType(ARG,VAR,char*,"String",       \
        OZ_isProperString,oz_str2c);

#define OZ_setString(ARG,VAR)           \
OZ_setRecType(ARG,VAR,char*,"String",   \
        OZ_isProperString,oz_str2c);

#define OZ_declareVirtualString(ARG,VAR)        \
OZ_declareRecType(ARG,VAR,char*,"VirtualString",\
        OZ_isVirtualString,oz_vs2c);

#define OZ_setVirtualString(ARG,VAR)    \
OZ_setRecType(ARG,VAR,char*,"VirtualString",\
        OZ_isVirtualString,oz_vs2c);

/*
 * OZ_declareVS(ARG,VAR,LEN)
 *
 * like OZ_declareVirtualString, but additionally sets LEN to
 * the size of the result.
 */

#define OZ_declareVS(ARG,VAR,LEN)               \
OZ_expectVirtualString(ARG);                    \
int LEN;                                        \
char* VAR = OZ_vsToC(OZ_in(ARG),&LEN);

#define OZ_setVS(ARG,VAR,LEN)                   \
OZ_expectVirtualString(ARG);                    \
VAR = OZ_vsToC(OZ_in(ARG),&LEN);

/* ------------------------------------------------------------------------ *
 * Debugging Support
 * ------------------------------------------------------------------------ */

#ifdef DEBUG_CHECK
#define Assert(Cond)                                                    \
  if (! (Cond)) {                                                       \
    OZ_error("%s:%d assertion '%s' failed",__FILE__,__LINE__,#Cond);    \
  }
#else
#define Assert(Cond)
#endif

/* ------------------------------------------------------------------------ *
 * end
 * ------------------------------------------------------------------------ */

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)
#include "extension.hh"
#endif

#endif
