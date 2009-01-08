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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
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

/* calling convention "cdecl" under win32 */
#if defined(__WATCOMC__) || defined(__BORLANDC__)
#  define ozdeclspec
#  define ozcdecl
#  define OZWIN
#elif defined(__CYGWIN32__) || defined(__MINGW32__) || defined(_MSC_VER)
#  ifdef WINDOWS_EMULATOR
#    define ozdeclspec __declspec(dllexport)
#  else
#    define ozdeclspec __declspec(dllimport)
#  endif
#  define ozcdecl __cdecl
#  define OZWIN
#else
#  define ozdeclspec
#  define ozcdecl
#endif

#if defined(__STDC__) || defined(_MSC_VER)
#define OZStringify(Name) #Name
#define OZ_CONST const
#else
#define OZStringify(Name) "Name"
#define OZ_CONST
#endif

#if defined(__STDC__) || defined(__cplusplus) || __BORLANDC__ || _MSC_VER
#define _FUNDECL(rettype,fun,arglist) extern ozdeclspec rettype ozcdecl fun arglist
#define _FUNTYPEDECL(rettype,fun,arglist) ozdeclspec rettype (ozcdecl *fun) arglist
#else
#define _FUNDECL(rettype,fun,ignore) extern ozdeclspec rettype ozcdecl fun ()
#define _FUNTYPEDECL(rettype,fun,ignore) ozdeclspec rettype (ozcdecl *fun) ()
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#define FUNDECL(rettype,fun,args) _FUNDECL(rettype,fun,args)

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

typedef _FUNTYPEDECL(OZ_Return,OZ_CFun,(OZ_Term **));

typedef int OZ_Boolean;
#define OZ_FALSE 0
#define OZ_TRUE 1

/* ------------------------------------------------------------------------ *
 * II. function prototypes
 * ------------------------------------------------------------------------ */

_FUNDECL(void, OZ_main, (int argc,char **argv));


_FUNDECL(OZ_Term,OZ_deref,(OZ_Term term));

/* tests */
_FUNDECL(int,OZ_isBool,(OZ_Term));
_FUNDECL(int,OZ_isAtom,(OZ_Term));
_FUNDECL(int,OZ_isBigInt,(OZ_Term));
_FUNDECL(int,OZ_isCell,(OZ_Term));
_FUNDECL(int,OZ_isThread,(OZ_Term));
_FUNDECL(int,OZ_isPort,(OZ_Term));
_FUNDECL(int,OZ_isChunk,(OZ_Term));
_FUNDECL(int,OZ_isDictionary,(OZ_Term));
_FUNDECL(int,OZ_isCons,(OZ_Term));
_FUNDECL(int,OZ_isFalse,(OZ_Term));
_FUNDECL(int,OZ_isFeature,(OZ_Term));
_FUNDECL(int,OZ_isFloat,(OZ_Term));
_FUNDECL(int,OZ_isInt,(OZ_Term));
_FUNDECL(int,OZ_isNumber,(OZ_Term));
_FUNDECL(int,OZ_isLiteral,(OZ_Term));
_FUNDECL(int,OZ_isName,(OZ_Term));
_FUNDECL(int,OZ_isNil,(OZ_Term));
_FUNDECL(int,OZ_isObject,(OZ_Term));
_FUNDECL(int,OZ_isPair,(OZ_Term));
_FUNDECL(int,OZ_isPair2,(OZ_Term));
_FUNDECL(int,OZ_isProcedure,(OZ_Term));
_FUNDECL(int,OZ_isRecord,(OZ_Term));
_FUNDECL(int,OZ_isSmallInt,(OZ_Term));
_FUNDECL(int,OZ_isTrue,(OZ_Term));
_FUNDECL(int,OZ_isTuple,(OZ_Term));
_FUNDECL(int,OZ_isUnit,(OZ_Term));
_FUNDECL(int,OZ_isValue,(OZ_Term));
_FUNDECL(int,OZ_isVariable,(OZ_Term));
_FUNDECL(int,OZ_isBitString,(OZ_Term));
_FUNDECL(int,OZ_isByteString,(OZ_Term));
_FUNDECL(int,OZ_isFSetValue,(OZ_Term));

_FUNDECL(int,OZ_isList,(OZ_Term, OZ_Term *));
_FUNDECL(int,OZ_isString,(OZ_Term, OZ_Term *));
_FUNDECL(int,OZ_isProperString,(OZ_Term, OZ_Term *));
_FUNDECL(int,OZ_isVirtualStringNoZero,(OZ_Term, OZ_Term *));
_FUNDECL(int,OZ_isVirtualString,(OZ_Term, OZ_Term *));

/* heap chunks */

_FUNDECL(OZ_Term,OZ_makeHeapChunk,(int s));
_FUNDECL(void*,OZ_getHeapChunkData,(OZ_Term t));
_FUNDECL(int,OZ_getHeapChunkSize,(OZ_Term t));
_FUNDECL(int,OZ_isHeapChunk,(OZ_Term t));

_FUNDECL(unsigned int,OZ_getUniqueId,(void));


_FUNDECL(OZ_Term ,OZ_termType,(OZ_Term));

/* convert: C from/to Oz datastructure */

_FUNDECL(OZ_CONST char* ,OZ_atomToC,(OZ_Term));
_FUNDECL(OZ_Term ,OZ_atom,(OZ_CONST char *));
_FUNDECL(int     ,OZ_featureCmp,(OZ_Term,OZ_Term));

_FUNDECL(int     ,OZ_smallIntMin,(void));
_FUNDECL(int     ,OZ_smallIntMax,(void));
_FUNDECL(OZ_Term ,OZ_false,(void));
_FUNDECL(OZ_Term ,OZ_true,(void));
_FUNDECL(OZ_Term ,OZ_unit,(void));
_FUNDECL(OZ_Term ,OZ_int,(int));
_FUNDECL(OZ_Term ,OZ_long,(long));
_FUNDECL(OZ_Term ,OZ_unsignedInt,(unsigned int));
_FUNDECL(OZ_Term ,OZ_unsignedLong,(unsigned long));
_FUNDECL(int     ,OZ_getLowPrio,(void));
_FUNDECL(int     ,OZ_getMediumPrio,(void));
_FUNDECL(int     ,OZ_getHighPrio,(void));
_FUNDECL(int     ,OZ_intToC,(OZ_Term));
_FUNDECL(long    ,OZ_intToCL,(OZ_Term));
_FUNDECL(unsigned long,OZ_intToCulong,(OZ_Term));
_FUNDECL(int     ,OZ_boolToC,(OZ_Term));
_FUNDECL(OZ_Term ,OZ_CStringToInt,(const char *str));
_FUNDECL(char *  ,OZ_parseInt,(char *s));

_FUNDECL(OZ_Term ,OZ_float,(double));
_FUNDECL(double  ,OZ_floatToC,(OZ_Term));

_FUNDECL(OZ_Term ,OZ_CStringToFloat,(char *s));
_FUNDECL(char *  ,OZ_parseFloat,(char *s));

_FUNDECL(OZ_Term ,OZ_CStringToNumber,(char *));

_FUNDECL(char *  ,OZ_toC,(OZ_Term, int, int));
_FUNDECL(char *  ,OZ__toC,(OZ_Term, int, int, int*));
_FUNDECL(int     ,OZ_termGetSize,(OZ_Term, int, int));

_FUNDECL(OZ_Term ,OZ_string,(OZ_CONST char *));
_FUNDECL(char *  ,OZ_stringToC,(OZ_Term t,int*n));

_FUNDECL(char*   ,OZ_vsToC,(OZ_Term t,int*n));
_FUNDECL(char *  ,OZ_virtualStringToC,(OZ_Term t,int*n));
_FUNDECL(OZ_Term ,OZ_mkByteString,(const char*,int));


/* tuples */
_FUNDECL(OZ_Term  ,OZ_label,(OZ_Term));
_FUNDECL(int      ,OZ_width,(OZ_Term));
_FUNDECL(OZ_Term ,OZ_tuple,(OZ_Term, int));
#define OZ_tupleC(s,n) OZ_tuple(OZ_atom(s),n)
_FUNDECL(OZ_Term  ,OZ_mkTuple,(OZ_Term label,int arity,...));
_FUNDECL(OZ_Term  ,OZ_mkTupleC,(const char *label,int arity,...));

_FUNDECL(void     ,OZ_putArg,(OZ_Term, int, OZ_Term));
_FUNDECL(OZ_Term  ,OZ_getArg,(OZ_Term, int));
_FUNDECL(OZ_Term  ,OZ_nil,());
_FUNDECL(OZ_Term  ,OZ_cons,(OZ_Term ,OZ_Term));
_FUNDECL(OZ_Term  ,OZ_head,(OZ_Term));
_FUNDECL(OZ_Term  ,OZ_tail,(OZ_Term));
_FUNDECL(int      ,OZ_length,(OZ_Term list));
_FUNDECL(OZ_Term  ,OZ_toList,(int, OZ_Term *));


_FUNDECL(OZ_Term  ,OZ_pair,(int));
_FUNDECL(OZ_Term  ,OZ_pair2,(OZ_Term t1,OZ_Term t2));

#define OZ_pairA(s1,t)      OZ_pair2(OZ_atom(s1),t)
#define OZ_pairAI(s1,i)     OZ_pair2(OZ_atom(s1),OZ_int(i))
#define OZ_pairAA(s1,s2)    OZ_pair2(OZ_atom(s1),OZ_atom(s2))
#define OZ_pairAS(s1,s2)    OZ_pair2(OZ_atom(s1),OZ_string(s2))


/* records */
_FUNDECL(OZ_Arity ,OZ_makeArity,(OZ_Term list));
_FUNDECL(OZ_Term  ,OZ_record,(OZ_Term, OZ_Term));
_FUNDECL(OZ_Term  ,OZ_recordInit,(OZ_Term, OZ_Term));
#define OZ_recordInitC(s,t) OZ_recordInit(OZ_atom(s),t)
_FUNDECL(void     ,OZ_putSubtree,(OZ_Term, OZ_Term, OZ_Term));
_FUNDECL(OZ_Term  ,OZ_subtree,(OZ_Term, OZ_Term));
_FUNDECL(OZ_Term  ,OZ_arityList,(OZ_Term));
_FUNDECL(OZ_Term  ,OZ_adjoinAt,(OZ_Term, OZ_Term, OZ_Term));

/* unification */
_FUNDECL(OZ_Return ,OZ_unify,(OZ_Term, OZ_Term));
_FUNDECL(void      ,OZ_unifyInThread,(OZ_Term,OZ_Term));
_FUNDECL(int       ,OZ_eq,(OZ_Term, OZ_Term));

#define OZ_unifyFloat(t1,f)      OZ_unify(t1, OZ_float(f))
#define OZ_unifyInt(t1,i)        OZ_unify(t1, OZ_int(i))
#define OZ_unifyAtom(t1,s)       OZ_unify(t1, OZ_atom(s))

#define OZ_eqAtom(t1,s)       OZ_eq(t1, OZ_atom(s))
#define OZ_eqInt(t1,s)        OZ_eq(t1, OZ_int(s))
#define OZ_eqFloat(t1,s)      OZ_eq(t1, OZ_float(s))

/* create a new oz variable */
_FUNDECL(OZ_Term ,OZ_newVariable,());

_FUNDECL(OZ_Term ,OZ_newChunk,(OZ_Term));

/* cell */
_FUNDECL(OZ_Term ,OZ_newCell,(OZ_Term));
/* exchangeCell, deepFeed */

/* port */
_FUNDECL(OZ_Term ,OZ_newPort,(OZ_Term));
_FUNDECL(void ,OZ_send,(OZ_Term,OZ_Term));

/* name */
_FUNDECL(OZ_Term ,OZ_newName,());

/* bit strings */
#define OZBitString void*
_FUNDECL(OZ_Boolean,OZ_BitStringGet,(OZBitString,int));
_FUNDECL(OZBitString,OZ_getBitString,(OZ_Term));


/* foreign pointer */
_FUNDECL(OZ_Term        ,OZ_makeForeignPointer,(void*));
_FUNDECL(void*  ,OZ_getForeignPointer,(OZ_Term));
_FUNDECL(int    ,OZ_isForeignPointer,(OZ_Term));

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


/* declare here, so C linkage is used and symbol is exported */
#ifndef WINDOWS_EMULATOR
#if defined(__WATCOMC__) || defined(__BORLANDC__)
extern char __export oz_module_name[];
OZ_C_proc_interface * __export ozcdecl oz_init_module();
#elif defined(__CYGWIN32__) || defined(__MINGW32__) || defined(_MSC_VER)
__declspec(dllexport) extern char oz_module_name[];
__declspec(dllexport) OZ_C_proc_interface * ozcdecl oz_init_module();
#else
OZ_C_proc_interface * ozcdecl oz_init_module();
#endif
#endif

#define OZ_DATUM_UNKNOWNERROR -1
#define OZ_DATUM_OUTOFMEMORY  -2

_FUNDECL(OZ_Return ,OZ_valueToDatum,(OZ_Term  t, OZ_Datum* d));
_FUNDECL(OZ_Return ,OZ_datumToValue,(OZ_Datum d, OZ_Term   t));

/* print warnings/errors */
_FUNDECL(void ,OZ_warning,(OZ_CONST char *, ...));
_FUNDECL(void ,OZ_error,(OZ_CONST char *, ...));

/* generate the unix error string from an errno (see perror(3)) */
_FUNDECL(char * ,OZ_unixError,(int err));

/* check for toplevel */
_FUNDECL(int ,OZ_onToplevel,());

/* IO */

/*
  OZ_*Select functions will unify the terms at earliest during the
  next i/o handling, even if there is some data available immediately
  (that is, they do not perform yet another 'select()' on their own).
 */

_FUNDECL(OZ_Return ,OZ_readSelect,(int, OZ_Term, OZ_Term));
_FUNDECL(OZ_Return ,OZ_writeSelect,(int, OZ_Term, OZ_Term));
_FUNDECL(OZ_Return ,OZ_acceptSelect,(int, OZ_Term, OZ_Term));

_FUNDECL(void      ,OZ_deSelect,(int));

/*
 * OZ_IOHandler is called with fd + static pointer given when registered
 *   if it returns TRUE, it is unregistered
 *   else (return FALSE) its called again, when something is available
 */

typedef _FUNTYPEDECL(int,OZ_IOHandler,(int,void *));

_FUNDECL(void ,OZ_registerReadHandler,(int,OZ_IOHandler,void *));
_FUNDECL(void ,OZ_unregisterRead,(int));

_FUNDECL(void ,OZ_registerWriteHandler,(int,OZ_IOHandler,void *));
_FUNDECL(void ,OZ_unregisterWrite,(int));

_FUNDECL(void ,OZ_registerAcceptHandler,(int,OZ_IOHandler,void *));

/* garbage collection */
_FUNDECL(int ,OZ_protect,(OZ_Term *));
_FUNDECL(int ,OZ_unprotect,(OZ_Term *));
_FUNDECL(void ,OZ_gCollect,(OZ_Term *));
_FUNDECL(void ,OZ_sClone,(OZ_Term *));

/* raise exception */
_FUNDECL(OZ_Return ,OZ_typeError,(int pos,const char *type));
_FUNDECL(OZ_Return ,OZ_raise,(OZ_Term));
_FUNDECL(OZ_Return ,OZ_raiseDebug,(OZ_Term));
_FUNDECL(OZ_Return ,OZ_raiseC,(const char *label,int arity,...));
_FUNDECL(OZ_Return ,OZ_raiseError,(OZ_Term));
_FUNDECL(OZ_Return ,OZ_raiseErrorC,(const char *label,int arity,...));
_FUNDECL(OZ_Term   ,OZ_makeException,(OZ_Term kind,OZ_Term key,const char* label,int arity,...));

/* Suspending builtins */

_FUNDECL(OZ_Thread ,OZ_newRunnableThread,());
_FUNDECL(void      ,OZ_makeRunnableThread,(OZ_Term, OZ_Term *, int));
_FUNDECL(OZ_Thread ,OZ_newSuspendedThread,());
_FUNDECL(OZ_Thread ,OZ_makeSuspendedThread,(OZ_Term, OZ_Term *, int));
_FUNDECL(void      ,OZ_addThread,(OZ_Term, OZ_Thread));
_FUNDECL(void      ,OZ_pushCall,(OZ_Thread,OZ_Term,OZ_Term *,int));

_FUNDECL(OZ_Return ,OZ_suspendOnInternal,(OZ_Term));
_FUNDECL(OZ_Return ,OZ_suspendOnInternal2,(OZ_Term,OZ_Term));
_FUNDECL(OZ_Return ,OZ_suspendOnInternal3,(OZ_Term,OZ_Term,OZ_Term));

#define OZ_suspendOn(t1) \
   { return OZ_suspendOnInternal(t1); }
#define OZ_suspendOn2(t1,t2) \
   { return OZ_suspendOnInternal2(t1,t2); }
#define OZ_suspendOn3(t1,t2,t3) \
   { return OZ_suspendOnInternal3(t1,t2,t3); }

/* event mechanism */

_FUNDECL(void,OZ_eventPush,(OZ_Term));

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
#define OZ_BI_proto(Name)  extern "C" OZ_Return (ozcdecl Name)(OZ_Term * [])
#else
#define OZ_BI_proto(Name)  extern OZ_Return (ozcdecl Name)()
#endif

_FUNDECL(OZ_Term, _OZ_LOC_TO_LIST,(int,OZ_Term**));

#define OZ_in(N)      (*(_OZ_LOC[(N)]))
#define OZ_out(N)     (*(_OZ_LOC[_OZ_arity+(N)]))
#define OZ_result(V)  OZ_out(0)=V
#define OZ_inAsList() (_OZ_LOC_TO_LIST(_OZ_arity,_OZ_LOC))

#define OZ_BI_define(Name,Arity_IN,Arity_OUT)    \
OZ_BI_proto(Name);                               \
OZ_Return (ozcdecl Name)(OZ_Term * _OZ_LOC[]) {  \
    const int _OZ_arity = Arity_IN;

#define OZ_BI_end }

#define OZ_RETURN(V)        return ((OZ_result(V)),PROCEED)
#define OZ_RETURN_INT(I)    OZ_RETURN(OZ_int(I))
#define OZ_RETURN_LONG(I)   OZ_RETURN(OZ_long(I))
#define OZ_RETURN_ATOM(S)   OZ_RETURN(OZ_atom(S))
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
OZ__doType(ARG,VAR,;,MSG,CHECK,COERCE)

#define OZ_declareBool(ARG,VAR)                 \
OZ_declareType(ARG,VAR,int,"Bool",OZ_isBool,OZ_boolToC)

#define OZ_setBool(ARG,VAR)                     \
OZ_setType(ARG,VAR,int,"Bool",OZ_isBool,OZ_boolToC)

#define OZ_declareInt(ARG,VAR)                  \
OZ_declareType(ARG,VAR,int,"Int",OZ_isInt,OZ_intToC)

#define OZ_declareLong(ARG,VAR)                 \
OZ_declareType(ARG,VAR,long,"Int",OZ_isInt,OZ_intToCL)

#define OZ_setInt(ARG,VAR)                      \
OZ_setType(ARG,VAR,int,"Int",OZ_isInt,OZ_intToC)

#define OZ_declareFloat(ARG,VAR)                \
OZ_declareType(ARG,VAR,double,"Float",OZ_isFloat,OZ_floatToC)

#define OZ_setFloat(ARG,VAR)                    \
OZ_setType(ARG,VAR,double,"Float",OZ_isFloat,OZ_floatToC)

#define OZ_declareAtom(ARG,VAR)                 \
OZ_declareType(ARG,VAR,OZ_CONST char*,"Atom",OZ_isAtom,OZ_atomToC)

#define OZ_setAtom(ARG,VAR)                     \
OZ_setType(ARG,VAR,OZ_CONST char*,"Atom",OZ_isAtom,OZ_atomToC)

#define OZ_declareBitString(ARG,VAR)            \
OZ_declareType(ARG,VAR,OZBitString,"BitString", \
               OZ_isBitString,OZ_getBitString)

#define OZ_setBitString(ARG,VAR)                \
OZ_setType(ARG,VAR,OzBitString,"BitString",     \
               OZ_isBitString,OZ_getBitString)

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

#define OZ_expectVirtualStringNoZero(ARG)               \
OZ_expectRecType(ARG,"VirtualStringNoZero",OZ_isVirtualStringNoZero);
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
OZ_declareRecType(ARG,VAR,char*,"VirtualStringNoZero",\
        OZ_isVirtualStringNoZero,oz_vs2c);

#define OZ_setVirtualString(ARG,VAR)    \
OZ_setRecType(ARG,VAR,char*,"VirtualStringNoZero",\
        OZ_isVirtualStringNoZero,oz_vs2c);

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
#define DebugArg(C) , C
#else
#define Assert(Cond)
#define DebugArg(C)
#endif

/* ------------------------------------------------------------------------ *
 * end
 * ------------------------------------------------------------------------ */

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)

#include <stdlib.h>

/* some internal functions to make the extension class work */
_FUNDECL(void*,_OZ_new_OZ_Extension,(size_t n));
_FUNDECL(OZ_Boolean,_OZ_isLocal_OZ_Extension,(void*));
_FUNDECL(void*,_OZ_currentBoard,());

#define OZ_CONTAINER_TAG 0

class OZ_Container {
public:
  unsigned int tag;
  void init(unsigned int t) {
    tag = t << 1;
  }
  void initAsExtension(void) {
    init(OZ_CONTAINER_TAG);
  }
  unsigned int cacIsMarked(void) {
    return tag&1;
  }
  void cacMark(OZ_Container * c) {
    tag = ((unsigned int) c) | 1;
  }
  unsigned int ** cacGetMarkField(void) {
    return (unsigned int **) &tag;
  }
  OZ_Container * cacGetFwd(void) {
    Assert(cacIsMarked());
    return (OZ_Container *) (tag&~1);
  }
};

#include "extension.hh"

#endif

#endif
