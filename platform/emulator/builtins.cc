/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
  */


#if defined(__GNUC__) && !defined(NOPRAGMA)
#pragma implementation "builtins.hh"
#endif

#include "am.hh"
#include "assemble.hh"
#include "builtins.hh"

// BuiltinTab

BuiltinTab builtinTab(750);


BuiltinTabEntry *BIadd(char *name,int arity, OZ_CFun funn, Bool replace,
                       IFOR infun)
{
  BuiltinTabEntry *builtin = new BuiltinTabEntry(name,arity,funn,infun);

  if (builtinTab.aadd(builtin,name,replace) == NO) {
    warning("BIadd: failed to add %s/%d\n",name,arity);
    delete builtin;
    return((BuiltinTabEntry *) NULL);
  }
  return(builtin);
}

BuiltinTabEntry *BIaddSpecial(char *name,int arity,BIType t, Bool replace)
{
  BuiltinTabEntry *builtin = new BuiltinTabEntry(name,arity,t);

  if (builtinTab.aadd(builtin,name,replace) == NO) {
    warning("BIadd: failed to add %s/%d\n",name,arity);
    delete builtin;
    return((BuiltinTabEntry *) NULL);
  }
  return(builtin);
}

OZ_C_proc_begin(BIbuiltin,3)
{
  OZ_declareAtomArg("builtin",0,str);
  OZ_Term hdl = OZ_getCArg(1);
  OZ_Term ret = OZ_getCArg(2);

  DEREF(hdl,_3,tag);
  if (!isProcedure(hdl)) {
    if (!isAtom(hdl) || !OZ_unifyString(hdl,"noHandler")) {
      TypeError2("builtin",1,"Procedure or Atom \"noHandler\"",
                 OZ_getCArg(0),hdl);
    }
    hdl = makeTaggedNULL();
  }

  BuiltinTabEntry *found = (BuiltinTabEntry *) builtinTab.ffind(str);

  if (found == htEmpty) {
    warning("builtin: '%s' not in table", str);
    return(FAILED);
  }

  if (hdl != makeTaggedNULL() && found->getInlineFun()) {
    hdl = makeTaggedNULL();
    warning("builtin '%s' is compiled inline, suspension handler ignored",str);
  }

  Builtin *bi = new Builtin(found,hdl);

  return (OZ_unify(ret,makeTaggedConst(bi)));
}
OZ_C_proc_end


extern void BIinitCore(void);
extern void BIinitSpecial(void);
extern void BIinitSystem(void);
extern void BIinitFD(void);
extern void BIinitMeta(void);
extern void BIinitAVar(void);
extern void BIinitObjects();
extern void BIinitUnix();


BuiltinTabEntry *BIinit()
{
  BuiltinTabEntry *bi = BIadd("builtin",3,BIbuiltin);

  if (!bi)
    return bi;

  BIinitCore();
  BIinitSpecial();
  BIinitSystem();

#ifdef ASSEMBLER
  BIinitAssembler();
#endif

  BIinitObjects();

  BIinitFD();
  BIinitMeta();

  BIinitAVar();
  BIinitUnix();

  return bi;
}
