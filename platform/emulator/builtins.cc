/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
  */


#ifdef __GNUC__
#pragma implementation "builtins.hh"
#endif

#include "builtins.hh"

// BuiltinTab

BuiltinTab builtinTab(500);


BuiltinTabEntry *BIadd(char *name,int arity, OZ_CFun funn, Bool replace,
		       InlineFunOrRel infun)
{
  BuiltinTabEntry *builtin = new BuiltinTabEntry(name,arity,funn,infun);

  if (! builtinTab.add(builtin,name,replace)) {
    warning("BIadd: failed to add %s/%d\n",name,arity);
    delete builtin;
    return((BuiltinTabEntry *) NULL);
  }
  return(builtin);
}

BuiltinTabEntry *BIaddSpecial(char *name,int arity,BIType t, Bool replace)
{
  BuiltinTabEntry *builtin = new BuiltinTabEntry(name,arity,t);

  if (! builtinTab.add(builtin,name,replace)) {
    warning("BIadd: failed to add %s/%d\n",name,arity);
    delete builtin;
    return((BuiltinTabEntry *) NULL);
  }
  return(builtin);
}

OZ_C_proc_begin(BIbuiltin,3)
{
  OZ_declareStringArg("builtin",0,str);
  OZ_Term hdl = OZ_getCArg(1);
  OZ_Term ret = OZ_getCArg(2);

  DEREF(hdl,_3,tag);
  if (!isProcedure(hdl)) {
    if (!isXAtom(hdl) || !OZ_unifyString(hdl,"noHandler")) {
      warning("builtin: '%s' second arg '%s' must be a predicate or noHandler",
              str, tagged2String(hdl)
              );
    }
    hdl = makeTaggedNULL();
  }

  BuiltinTabEntry *found = (BuiltinTabEntry *) builtinTab.find(str);

  if (!found) {
    warning("builtin: '%s' not in table", str);
    return(FAILED);
  }

  if (hdl != makeTaggedNULL() && found->getInlineFun()) {
    hdl = makeTaggedNULL();
    warning("builtin '%s' is compiled inline, suspension handler ignored",str);
  }

  Builtin *bi = new Builtin(found,hdl);

  return (OZ_unify(ret,makeTaggedSRecord(bi)));
}
OZ_C_proc_end


extern void BIinitCore();
extern void BIinitSpecial();
extern void BIinitSystem();
extern void BIinitFD(void);

#ifndef DYNLINKING
extern void MyinitUnix();
#endif


BuiltinTabEntry *BIinit()
{
  BuiltinTabEntry *init = BIadd("builtin",3,BIbuiltin);

  if (!init)
    return init;

  BIinitCore();
  BIinitSpecial();
  BIinitSystem();

  BIinitFD();

#ifndef DYNLINKING
  MyinitUnix();
#endif

  return init;
}
