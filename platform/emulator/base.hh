/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
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

#ifndef __BASEHH
#define __BASEHH

#ifdef INTERFACE
#pragma interface
#endif

#include "conf.h"
#include "resources.hh"

#include "wsock.hh"

#define STATIC_FUNCTIONS
#include "mozart.h"

#include "machine.hh"
#include "config.h"
#include "cmem.hh"

#include "ozostream.hh"

#include <string.h>

#ifdef __FCC_VERSION
#undef HAVE_STRDUP
#endif

#if !defined(__GNUC__) && !defined(NULL)
# define NULL 0
#endif

//
#define MAX_DP_STRING		4
// MARSHALERMAJOR "#" MARSHALERMINOR:
#define MARSHALERVERSION	"3#3"
#define MARSHALERMAJOR		3
#define MARSHALERMINOR		3

//
#define PERDIOVERSION     "3#3" /* PERDIOMAJOR "#" PERDIOMINOR */
#define PERDIOMAJOR          3
#define PERDIOMINOR          3


const unsigned int KB = 1024;
const unsigned int MB = KB*KB;

const int WordSize = sizeof(void*);

// see print.cc
#ifdef DEBUG_PRINT
#define OZPRINT								     \
  void printStream(ostream &stream=cout, int depth = 10);    \
  void printLongStream(ostream &stream=cout, int depth = 10, int offset = 0) \
    { printStream(stream,depth); stream << endl; }		     \
  void print(void)							     \
    { printStream(cerr); cerr << endl; cerr.flush(); }		     \
  void printLong(void)							     \
    { printLongStream(cerr); cerr.flush(); }

#define OZPRINTLONG							      \
  void printStream(ostream &stream=cout, int depth = 10);     \
  void printLongStream(ostream &stream=cout, int depth = 10, int offset = 0); \
  void print(void)							      \
    { printStream(cerr); cerr << endl; cerr.flush(); }		      \
  void printLong(void)							      \
    { printLongStream(cerr); cerr.flush(); }
#else
#define OZPRINT
#define OZPRINTLONG
#endif

inline int min(int a, int b) {return a < b ? a : b;}
inline int max(int a, int b) {return a > b ? a : b;}

inline int ozabs(int a) {return a > 0 ? a : -a;}
inline float ozabs(float a) {return a > 0 ? a : -a;}

#define Swap(A,B,Type) { Type help=A; A=B; B=help; }

typedef int Bool;
const Bool NO = 0;
const Bool OK = 1;

/* AIX and OSF/1 define these */
#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif
const Bool TRUE  = 1;
const Bool FALSE = 0;


/*
 * special return values for builtins
 */
#define BI_PREEMPT       1024
#define BI_REPLACEBICALL 1025
#define BI_TYPE_ERROR    1026

typedef unsigned char BYTE;

typedef int32 ByteCode;

typedef ByteCode *ProgramCounter;

#define NOCODE ((ProgramCounter) -1l)

typedef int32 PosInt;

typedef PosInt XReg;
typedef PosInt YReg;
typedef PosInt GReg;

typedef unsigned int32 TaggedRef;

typedef unsigned int32 crc_t;

enum PropCaller {
  pc_propagator = 0,
  pc_std_unif = 1,
  pc_cv_unif = 2,
  pc_all = 3
};

enum GCStep {
  OddGCStep	= 0x0,
  EvenGCStep	= 0x10
};

typedef OZ_Return (*InlineRel1)(TaggedRef In1);
typedef OZ_Return (*InlineRel2)(TaggedRef In1, TaggedRef In2);
typedef OZ_Return (*InlineRel3)(TaggedRef In1, TaggedRef In2, TaggedRef In3);
typedef OZ_Return (*InlineFun1)(TaggedRef In1, TaggedRef &Out);
typedef OZ_Return (*InlineFun2)(TaggedRef In1, TaggedRef In2,
				    TaggedRef &Out);
typedef OZ_Return (*InlineFun3)(TaggedRef In1, TaggedRef In2,
				    TaggedRef In3, TaggedRef &Out);

//  ------------------------------------------------------------------------

/* some macros to help debugging
   DebugCheck:  if 'precondition' then print file and line and execute body
   DebugCheckT: check without precondition
   Assert:      issue an error if Cond is not fulfilled
   */

#ifdef DEBUG_CHECK
#define DebugCheck(Cond,Then)			\
if (Cond) {					\
  fprintf(stderr,"%s:%d ",__FILE__,__LINE__);	\
  Then;						\
}
#define DebugCheckT(Then) Then
#define DebugCode(C) C
#define ExhaustiveSwitch() \
   default: { OZ_error("NON EXHAUSTIVE SWITCH %s: %d\n", __FILE__,__LINE__); }
#else
#define DebugCheck(Cond,Then) 
#define DebugCheckT(Then)
#define DebugCode(C)
#define ExhaustiveSwitch() \
   default: break;
#endif

#ifdef DEBUG_MEM
#define MemDebugCode(C) C
#define MemAssert(C)							\
  if (!(C)) {								\
    OZ_error("%s:%d mem assertion '%s' failed",__FILE__,__LINE__,#C);	\
  }
#else
#define MemDebugCode(C)
#define MemAssert(C)
#endif

#ifdef DEBUG_FD
#define DebugFD(Cond,Then) if (Cond) {Then;}
#else
#define DebugFD(Cond,Then) 
#endif

#ifdef DEBUG_GC
#define DebugGC(Cond,Then) if (Cond) {Then;}
#define DebugGCT(Then) Then
#else
#define DebugGC(Cond,Then) 
#define DebugGCT(Then)
#endif

//  ------------------------------------------------------------------------

#ifdef DEBUG_CONSTRUCTORS
/* Avoid that the compiler generates constructors, destructors and
 * assignment operators which are not wanted in Oz */
#define NO_DEFAULT_CONSTRUCTORS2(aclass)	\
  ~aclass();					\
  aclass(const aclass &);			\
  aclass &operator = (const aclass&)

#define NO_DEFAULT_CONSTRUCTORS1(aclass)	\
  NO_DEFAULT_CONSTRUCTORS2(aclass)		\
  aclass();

#define NO_DEFAULT_CONSTRUCTORS(aclass) NO_DEFAULT_CONSTRUCTORS1(aclass)
#else
#define NO_DEFAULT_CONSTRUCTORS2(aclass)
#define NO_DEFAULT_CONSTRUCTORS1(aclass)
#define NO_DEFAULT_CONSTRUCTORS(aclass)
#endif
/*
   Forward declarations of classes and procedures
*/

class OzVariable;
class OzFDVariable;
class OzBoolVariable;
class OzFSVariable;
class OzCtVariable;
class Failed;
class ReadOnly;
class SimpleVar;
class ExtVar;
class DynamicTable;
class SRecord;
class Arity;
class Abstraction;
class LTuple;
class Literal;
class Float;
class SmallInt;
class BigInt;
class ConstTerm;
class OZ_Extension;
class Cell;
class SChunk;
class OzArray;
class OzPort;

class PendingThreadList;
class OzCell;

class RefTable;
class RefTrail;

class GenTraverser;
class Builder;

class Site;
//class DSite now provided by the Dss.

class Builtin;

class FiniteDomain;

class OZ_FSetValue;

class SuspList;
class OrderedSuspList;
class Suspendable;
class Propagator;
class OZ_Propagator;

class Thread;
class ThreadsPool;
class Group;
class Toplevel;
class Board;
class RunnableThreadBody;

class Trail;

class TaskStack;
class LongTime;
class CallList;

class SuspQueue;

class ProxyList;
class MarshalerBuffer;
class PickleMarshalerBuffer;

// source level debugger
class Atom;
extern Atom * DBG_STEP_ATOM, * DBG_NOSTEP_ATOM, * DBG_EXIT_ATOM;
class OzDebug;

class AM;
extern AM am; // the one and only engine

// assem
class CodeArea;
class PrTabEntry;
class AbstractionEntry;

class CallMethodInfo;
// 
class BuiltinTab;

class DLLStack;

class OzSleep;
class Alarm;

class IHashTable;

class CompStream;

class OzClass;
class OzObject;
class ObjectState;

class OzDictionary;

class OzLock;

class InlineCache;
class OZ_Location;

class NetAddress;
class GName;

class IONode;

// Operations on tabular entities, like chunks, arrays, dictionaries
enum OperationTag {
  OP_MEMBER,                // key -> bool
  OP_GET,                   // key -> val
  OP_CONDGET,               // key x defval -> val
  OP_PUT,                   // key x newval -> ()
  OP_EXCHANGE,              // key x newval -> oldval
  OP_CONDEXCHANGE,          // key x defval x newval -> oldval
  OP_ISEMPTY,               // () -> bool
  OP_REMOVE,                // key -> ()
  OP_REMOVEALL,             // () -> ()
  OP_KEYS,                  // () -> list
  OP_ITEMS,                 // () -> list
  OP_ENTRIES,               // () -> list
  OP_CLONE,                 // () -> val
  OP_TORECORD               // label -> val
};

inline OperationTag toOperationTag(int op) {
  Assert(op >= 0 && op <= OP_TORECORD);
  return static_cast<OperationTag>(op);
}

// operation arity (how many inputs and outputs), and has it side effects?
static int OperationIn[]    = { 1, 1, 2, 2, 2, 3, 0, 1, 0, 0, 0, 0, 0, 1 };
static int OperationOut[]   = { 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1 };
static int OperationWrite[] = { 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0 };

void checkGC();

// see version.sed
void version();

// see emulate.cc

extern TaggedRef XREGS[];
extern TaggedRef XREGS_SAVE[];

int engine(Bool);
void scheduler();

// return code of the emulator
enum ThreadReturn {
  T_PREEMPT,		// thread is preempted
  T_SUSPEND,		// thread must suspend
  T_FAILURE,		// an failure exception must be handled
  T_TERMINATE,		// the thread terminated
  T_ERROR,		// a fatal error occured
  T_OKOK                // NOTHING
};

enum LockRet{
  LOCK_GOT = 0,
  LOCK_PREEMPT=1,
  LOCK_WAIT=2
};


// see ozthread.cc
Bool oz_isThread(TaggedRef term);
Thread *oz_ThreadToC(TaggedRef term);
Thread *oz_ThreadToAliveC(TaggedRef term);
OZ_Term oz_thread(Thread *tt);

// see am.cc
void handlerUSR1(int);
void handlerSEGV(int);
void handlerBUS(int);
void handlerPIPE(int);
void handlerCHLD(int);
void handlerALRM(int);
void handlerUSR2(int);

// unify.cc
OZ_Return oz_unify(OZ_Term t1, OZ_Term t2);
// builtins.cc
OZ_Return oz_eqeq(OZ_Term t1, OZ_Term t2);

// printing (see foreign.cc)
void oz_printStream(OZ_Term term, ostream &out,
		    int depth=-1, int width=-1);
void oz_print(OZ_Term term);
// see also OZ_toC();
char *toC(OZ_Term);

#ifdef DEBUG_PRINT
// debug print (see print.cc)
void ozd_printStream(OZ_Term val, ostream &stream, int depth=20);
void ozd_print(OZ_Term term);
void ozd_printLongStream(OZ_Term val, ostream &stream,
			 int depth = 20, int offset = 0);
void ozd_printLong(OZ_Term term);
void ozd_printBoards();
void ozd_printThreads();
void ozd_printAM();
#endif

char *replChar(char *s,char from,char to);
char *delChar(char *s,char c);

// see perdio.cc
int perdioInit();

// see term.cc
void initLiterals();

// see codearea.cc
extern "C" void displayCode(ProgramCounter from, int ssize);
extern "C" void displayDef(ProgramCounter from, int ssize);

// see builtins.cc
Builtin *BIinit();
extern OZ_Return dotInline(TaggedRef term, TaggedRef fea, TaggedRef &out);
OZ_Return BIarityInline(TaggedRef, TaggedRef &);
OZ_Return adjoinPropList(TaggedRef t0, TaggedRef list, TaggedRef &out,
			     Bool recordFlag);
OZ_Return BIminusInline(TaggedRef A, TaggedRef B, TaggedRef &out);
OZ_Return BIplusInline(TaggedRef A, TaggedRef B, TaggedRef &out);
OZ_Return BILessOrLessEq(Bool callLess, TaggedRef A, TaggedRef B);

OZ_Return oz_bi_wrapper(Builtin *bi,OZ_Term *X);

// see ??
SuspList *oz_installPropagators(SuspList *local_list, SuspList *glob_list,
				Board *glob_home);

void oz_checkAnySuspensionList(SuspList ** suspList, Board *home,
			       PropCaller calledBy);

void oz_checkLocalSuspensionList(SuspList ** suspList,
				 PropCaller calledBy);

void oz_forceWakeUp(SuspList ** suspList);

// see ioHandler.cc
void oz_io_select(int fd, int mode, OZ_IOHandler fun, void *val);
void oz_io_acceptSelect(int fd, OZ_IOHandler fun, void *val);
int  oz_io_select(int fd, int mode,TaggedRef l,TaggedRef r);
void oz_io_acceptSelect(int fd,TaggedRef l,TaggedRef r);
void oz_io_deSelect(int fd,int mode);
void oz_io_deSelect(int fd);
void oz_io_suspend(int fd, int mode);
void oz_io_resume(int fd, int mode);
void oz_io_awakeVar(TaggedRef var);
void oz_io_handle();
void oz_io_check();
void oz_io_stopReadingOnShutdown();
int oz_io_numOfSelected();

Bool oz_protect(TaggedRef *);
Bool oz_unprotect(TaggedRef *);
void oz_unprotectAllOnExit();

_FUNDECL(void,OZ_gCollectBlock,(OZ_Term *, OZ_Term *, int));
_FUNDECL(void,OZ_sCloneBlock,(OZ_Term *, OZ_Term *, int));

inline 
void oz_gCollectTerm(TaggedRef & f, TaggedRef & t) {
  OZ_gCollectBlock(&f, &t, 1);
}

// register a triple (entity, port, item) for post-mortem finalization
void registerPostMortem(TaggedRef, TaggedRef, TaggedRef);

// builtins.cc
OZ_Return oz_sendPort(OZ_Term prt, OZ_Term val, OZ_Term var = 0);

// var_simple.cc, readonly.cc
OZ_Term oz_newSimpleVar(Board *bb);
OZ_Term oz_newReadOnly(Board *bb);

#ifndef HAVE_STRDUP
inline char * strdup(const char *s) {
  char *ret = new char[strlen(s)+1];
  strcpy(ret,s);
  return ret;
}
#endif

inline
int oz_char2uint(char c) {
  return (int) (unsigned char) c;
}


template <class T>
class EnlargeableArray {
private:
  int _size;
  T * _array;
public:
  EnlargeableArray(int s) : _size(s), _array((T *) malloc(s * sizeof(T))) {}
  ~EnlargeableArray() { free(_array); }
  
  inline
  T &operator [](int i) { 
    Assert(0 <= i && i < _size);
    return _array[i]; 
  }
  
  inline 
  void request(int s, int m = 100) { // margin of 100
    if (s >= _size) {
      _size = s + m;
      _array = (T *) realloc(_array, _size * sizeof(T));
    }
  }

  inline
  operator T*() { return _array; } // conversion operator
};

//
// "free list" data manager: try to get a piece of memory from a 
// free list, and fall back with the 'fdmMalloc' function;
typedef void* (*MallocFun)(size_t size);
//
template <class T>
class FreeListDataManager {
private:
  MallocFun mallocFun;
  T* freeList;

  //
public:
  FreeListDataManager(MallocFun fun)
    : mallocFun(fun), freeList((T *) NULL)
  {
    Assert(sizeof(T) >= sizeof(T*));
  }
  ~FreeListDataManager() {}

  //
  T *allocate() {
    if (freeList) {
      T *b = freeList;
      freeList = *((T **) freeList);
      return (b);
    } else {
      return ((T *) mallocFun(sizeof(T)));
    }
  }
  void dispose(T *b) {
    *((T **) b) = freeList;
    freeList = b;
  }
};

#ifdef __cplusplus

extern "C" {
  void message( const char *format ...);
  Bool isDeadSTDOUT();
  void statusMessage( const char *format ...);
  void prefixError();
  void prefixWarning();
  void prefixStatus();  
  void ozperror( const char *msg);
  void ozpwarning( const char *msg);
}

#endif

void errorHeader();
void errorTrailer();

#ifdef __GNUC__
#define NEW_TEMP_ARRAY(Type, Var, Size) Type Var[Size]

#define DELETE_TEMP_ARRAY(Var) 
#else
#define NEW_TEMP_ARRAY(Type, Var, Size) Type * Var = new Type[Size]
#define DELETE_TEMP_ARRAY(Var) delete [] Var
#endif

#endif

