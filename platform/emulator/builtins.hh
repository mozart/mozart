/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
  */

// BUILTINs
// This file specifies the access functions which can be used in
// user defined builtin functions

#ifndef __BUILTINSH
#define __BUILTINSH

#ifdef __GNUC__
#pragma interface
#endif

#include "types.hh"

#include "term.hh"
#include "records.hh"
#include "hashTable.hh"
#include "gc.hh"

// special builtins known in emulate
enum BIType {
  BIDefault,
  BIDefaultSusp,
  BIwakeup,
  BIApply,
  BILoadFile,
  BIsolve,
  BIsolveCont,
  BIsolved
};


#define NONVAR(X,term,tag)  					              \
   TaggedRef term = X;							      \
   TypeOfTerm tag;							      \
   { DEREF(term,_myTermPtr,myTag);                                             \
     tag = myTag;		                                              \
     if (isAnyVar(tag)) return SUSPEND;				      \
   }
    

#define STRING(FUN,ARG,VAR) 						      \
 char *VAR; 								      \
 {									      \
   TaggedRef term = args[ARG];						      \
   DEREF(term,_,tag);							      \
   if (isAnyVar(term)) return SUSPEND;				      \
   if ((VAR=getPrintName(term)) == (char *) NULL)				      \
     { warning(FUN ": arg %d must be string",ARG+1); 			      \
       return FAILED; }							      \
 }									      \





#define DECLAREBI_USEINLINEREL1(Name,InlineName)			      \
OZ_C_proc_begin(Name,1)							      \
{									      \
  OZ_Term arg = OZ_getCArg(0);						      \
  State state = InlineName(arg);					      	      \
  if (state == SUSPEND) {						      \
    DEREF(arg,APtr,_1);							      \
    OZ_Suspension *susp = OZ_makeSuspension(Name, OZ_args, OZ_arity);	      \
    OZ_addSuspension(APtr,susp);					      \
    return PROCEED;							      \
  } else {								      \
    return state;							      \
  }									      \
}									      \
OZ_C_proc_end


#define DECLAREBI_USEINLINEREL2(Name,InlineName)			      \
OZ_C_proc_begin(Name,2)							      \
{									      \
  OZ_Term arg0 = OZ_getCArg(0);						      \
  OZ_Term arg1 = OZ_getCArg(1);						      \
  State state = InlineName(arg0,arg1);					      \
  if (state == SUSPEND) {						      \
    DEREF(arg0,APtr,_1);						      \
    DEREF(arg1,BPtr,_2);						      \
    OZ_Suspension *susp = OZ_makeSuspension(Name, OZ_args, OZ_arity);	      \
    if (isAnyVar(arg0)) OZ_addSuspension(APtr,susp);			      \
    if (isAnyVar(arg1)) OZ_addSuspension(BPtr,susp);			      \
    return PROCEED;							      \
  } else {								      \
    return state;							      \
  }									      \
}									      \
OZ_C_proc_end


#define DECLAREBI_USEINLINEFUN1(Name,InlineName)			      \
OZ_C_proc_begin(Name,2)							      \
{									      \
  OZ_Term help;								      \
  									      \
  OZ_Term arg = OZ_getCArg(0);						      \
  State state = InlineName(arg,help);					      \
  switch (state) {							      \
  case SUSPEND:								      \
    DEREF(arg,APtr,_1);							      \
    OZ_Suspension *susp = OZ_makeSuspension(Name, OZ_args, OZ_arity);	      \
    OZ_addSuspension(APtr,susp);					      \
    return PROCEED;							      \
  case FAILED:								      \
    return FAILED;							      \
  case PROCEED:								      \
  default:								      \
    return(OZ_unify(help,OZ_getCArg(1)));				      \
  }									      \
									      \
}									      \
OZ_C_proc_end



#define DECLAREBI_USEINLINEFUN2(Name,InlineName)			      \
OZ_C_proc_begin(Name,3)							      \
{									      \
  OZ_Term help;								      \
  									      \
  OZ_Term arg0 = OZ_getCArg(0);						      \
  OZ_Term arg1 = OZ_getCArg(1);						      \
  State state=InlineName(arg0,arg1,help);		  		      \
  switch (state) {							      \
  case SUSPEND:								      \
    DEREF(arg0,APtr,_1);						      \
    DEREF(arg1,BPtr,_2);						      \
    OZ_Suspension *susp = OZ_makeSuspension(Name, OZ_args, OZ_arity);	      \
    if (isAnyVar(arg0)) OZ_addSuspension(APtr,susp);			      \
    if (isAnyVar(arg1)) OZ_addSuspension(BPtr,susp);			      \
    return PROCEED;							      \
  case FAILED:								      \
    return state;							      \
  case PROCEED:								      \
  default:								      \
    return(OZ_unify(help,OZ_getCArg(2)));				      \
  }									      \
									      \
}									      \
OZ_C_proc_end


#define DECLAREBI_USEINLINEFUN3(Name,InlineName)			      \
OZ_C_proc_begin(Name,4)							      \
{									      \
  OZ_Term help;								      \
  									      \
  OZ_Term arg0 = OZ_getCArg(0);						      \
  OZ_Term arg1 = OZ_getCArg(1);						      \
  OZ_Term arg2 = OZ_getCArg(2);						      \
  State state=InlineName(arg0,arg1,arg2,help);		  		      \
  switch (state) {							      \
  case SUSPEND:								      \
    DEREF(arg0,APtr,_1);						      \
    DEREF(arg1,BPtr,_2);						      \
    DEREF(arg2,CPtr,_3);						      \
    OZ_Suspension *susp = OZ_makeSuspension(Name, OZ_args, OZ_arity);	      \
    if (isAnyVar(arg0)) OZ_addSuspension(APtr,susp);			      \
    if (isAnyVar(arg1)) OZ_addSuspension(BPtr,susp);			      \
    if (isAnyVar(arg2)) OZ_addSuspension(CPtr,susp);			      \
    return PROCEED;							      \
  case FAILED:								      \
    return state;							      \
  case PROCEED:								      \
  default:								      \
    return(OZ_unify(help,OZ_getCArg(3)));				      \
  }									      \
									      \
}									      \
OZ_C_proc_end


// inits
BuiltinTabEntry *BIinit();
BuiltinTabEntry *BIadd(char *name,int arity,BIFun fun,
		       Bool replace = NO, InlineFunOrRel infun=NULL);
BuiltinTabEntry *BIaddSpecial(char *name,int arity,BIType t,
			      Bool replace = NO);
BuiltinTabEntry *BIreplace(char *name,int arity,BIFun fun);

void BIinitSpecial();
void BIinitWIO();
void BIinitDebug();
void BIinitDatatypes();
void BIinitFeatures();
void BIinitFD(void);

// -----------------------------------------------------------------------
// tables

class BuiltinTab : public HashTable {
public:
  BuiltinTab(int size) : HashTable(size) {};
  ~BuiltinTab() {};
};



// access to builtin table:
BuiltinTab &getBuiltinTab();


class BuiltinTabEntry {
  friend class Debugger;
public:
  BuiltinTabEntry (Atom *name,int arty,BIFun fn,
		   InlineFunOrRel infun=NULL)
  : printname(makeTaggedAtom(name)), arity(arty),fun(fn),
    inlineFun(infun), type(BIDefault)
  {
    DebugCheck(!isXAtom(printname),error("BuiltinTabEntry:: no atom"));
  }
  BuiltinTabEntry (char *s,int arty,BIFun fn,
		   InlineFunOrRel infun=NULL)
  : arity(arty),fun(fn), inlineFun(infun), type(BIDefault)
  {
    printname = OZ_stringToTerm(s);
    DebugCheck(!isXAtom(printname),error("BuiltinTabEntry:: no atom"));
  }
  BuiltinTabEntry (char *s,int arty,BIFun fn,BIType t,
		   InlineFunOrRel infun=NULL)
    : arity(arty),fun(fn), inlineFun(infun), type(t) {
      printname = OZ_stringToTerm(s);
    }
  BuiltinTabEntry (char *s,int arty,BIType t, InlineFunOrRel infun=NULL)
    : arity(arty),fun(NULL), inlineFun(infun), type(t)
  {
    printname = OZ_stringToTerm(s);
    DebugCheck(!isXAtom(printname),error("BuiltinTabEntry:: no atom"));
  }

  ~BuiltinTabEntry () {}
  Bool operator== (BuiltinTabEntry &data)
  {
    error("mm2");
    return (((printname == ((BuiltinTabEntry &) data).printname) &&
	     (arity == ((BuiltinTabEntry &) data).arity)) ? OK : NO);
  }
  Bool operator< (BuiltinTabEntry &data)
  {
    return (((*tagged2Atom(printname)
	      < *tagged2Atom(((BuiltinTabEntry &) data).printname)) ||
	     ((printname == ((BuiltinTabEntry &) data).printname) &&
	      (arity < ((BuiltinTabEntry &) data).arity))) ? OK : NO);
  }
  Bool operator> (BuiltinTabEntry &data)
  {
    return (((*tagged2Atom(printname)
	      > *tagged2Atom(((BuiltinTabEntry &) data).printname)) ||
	     ((printname == ((BuiltinTabEntry &) data).printname) &&
	      (arity > ((BuiltinTabEntry &) data).arity))) ? OK : NO);
  }

  Bool operator== (char *s) {
    return printname == OZ_stringToTerm(s) ? OK : NO;
  }
  Bool operator<  (char *s) {
    error("not impl");
    // return (*tagged2Atom(printname) < s);
    return NO;
  }
  Bool operator>  (char *s) {
    error("not impl");
    // return (*tagged2Atom(printname) > s);
    return NO;
  }

  OZPRINT;
  BIFun getFun() { return fun; }
  int getArity() { return arity; }
  char *getPrintName() { return tagged2Atom(printname)->getPrintName(); }
  TaggedRef getName() { return printname; }
  InlineFunOrRel getInlineFun() { return inlineFun; } 
  BIType getType() { return type; } 

private:

  TaggedRef printname; //must be atom
  int arity;
  BIFun fun;
  InlineFunOrRel inlineFun;
  BIType type;
};



class Builtin: public SRecord {
friend void SRecord::gcRecurse();
friend SRecord *SRecord::gc ();
private:
  BuiltinTabEntry *fun;
  TaggedRef suspHandler; // this one is called, when it must suspend
protected:
  RefsArray gRegs;       // context;
public:
  Builtin(Builtin *b)
  : suspHandler(b->suspHandler), fun(b->fun), SRecord(R_BUILTIN),
    gRegs (b->gRegs) {}
  Builtin(BuiltinTabEntry *fn, TaggedRef handler, RefsArray gregs = NULL)
  : suspHandler(handler), fun(fn), SRecord(R_BUILTIN),
    gRegs (gregs) {}
  Builtin(BuiltinTabEntry *fn, TaggedRef handler, RefsArray gregs,
	  Arity *arity, RefsArray features)
  : suspHandler(handler), fun(fn), SRecord(R_BUILTIN, arity, features),
    gRegs (gregs) {}
  OZPRINT;
  OZPRINTLONG;

  int getArity() { return fun->getArity(); }
  BIFun getFun() { return fun->getFun(); }
  char *getPrintName() { return fun->getPrintName(); }
  TaggedRef getName() { return fun->getName(); }
  BIType getType() { return fun->getType(); } 

  SRecord *getSuspHandler() { 
    return suspHandler == makeTaggedNULL()
      ? NULL
      : tagged2SRecord(suspHandler);
  }
  TaggedRef getDBGHandler() { 
    return suspHandler == makeTaggedNULL()
      ? nil()
      : suspHandler;
  }
  Bool compare(Builtin *r) {
    return (fun == r->fun &&
	    suspHandler == r->suspHandler &&
	    gRegs == r->gRegs) ? OK : NO;
  }
  // mm: this is a hack: the id is not always unique
  int getId() {
    return ((int) fun)^((int) suspHandler)^((int) gRegs);
  }
};

class OneCallBuiltin: public Builtin {
private:
  Bool seen;
public:
  USEHEAPMEMORY;

  OneCallBuiltin (BuiltinTabEntry *fn, RefsArray gregs,
		  Arity *arity, RefsArray features)
  : Builtin ((BuiltinTabEntry *)fn, (TaggedRef) 0, gregs, arity, features),
    seen (NO) {}
  inline Bool isSeen () { return (seen); }
  inline void hasSeen () { seen = OK; }
  inline RefsArray &getGRegs() { return(gRegs); }
};

class SolvedBuiltin: public Builtin {
public:
  USEHEAPMEMORY;

  SolvedBuiltin (BuiltinTabEntry *fn, RefsArray gregs,
		 Arity *arity, RefsArray features)
  : Builtin ((BuiltinTabEntry *)fn, (TaggedRef) 0, gregs, arity, features)
    {}
  inline RefsArray &getGRegs() { return(gRegs); }
};


State BIexchangeCellInline(TaggedRef c, TaggedRef out, TaggedRef &in);

#endif

