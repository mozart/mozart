/*
 *  Authors:
 *    Denys Duchier (duchier@ps.uni-sb.de)
 *    Christian Schulte (schulte@dfki.de)
 * 
 *  Copyright:
 *    Christian Schulte, 1998
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


#ifdef HAVE_CONFIG_H
#include "conf.h"
#endif

#include <string.h>

#include "builtins.hh"
#include "os.hh"
#include "am.hh"


/*
 * Record of unsited builtins (needed for pickling and compiler)
 */

TaggedRef builtinRecord;


/*
 * Builtins that are always in the emulator
 */

#include "modProperty-if.cc"
#include "modOS-if.cc"
#include "modPickle-if.cc"
#include "modURL-if.cc"
#include "modPID-if.cc"
#include "modFDB-if.cc"
#include "modFSB-if.cc"
#include "modSystem-if.cc"
#include "modCTB-if.cc"
#include "modFinalize-if.cc"
#include "modProfile-if.cc"
#include "modForeign-if.cc"
#include "modFault-if.cc"
#include "modDistribution-if.cc"
#include "modBitString-if.cc"
#include "modByteString-if.cc"

/*
 * Builtins that are possibly dynamically loaded
 */

#ifdef MODULES_LINK_STATIC

#include "modWif-if.cc"
#include "modFDP-if.cc"
#include "modSchedule-if.cc"
#include "modParser-if.cc"
#include "modFSP-if.cc"
#include "modCompilerSupport-if.cc"
#include "modBrowser-if.cc"
#include "modDebug-if.cc"

#endif


/*
 * Builtins that depend on particular configuration
 */

#ifdef MISC_BUILTINS
#include "modMisc-if.cc"
#endif

#ifdef VIRTUALSITES
#include "modVirtualSite-if.cc"
#endif

typedef OZ_C_proc_interface * (* init_fun_t) (void);

struct ModuleEntry {
  const char * name;
  init_fun_t   init_function; //OZ_C_proc_interface * interface;
};


static ModuleEntry module_table[] = {
  // Most important stuff
  {"Property",        mod_int_Property},
  {"OS",              mod_int_OS},
  {"URL",             mod_int_URL},
  {"Pickle",          mod_int_Pickle},
  {"System",          mod_int_System},
  {"Finalize",        mod_int_Finalize},
  {"Profile",         mod_int_Profile},
  {"Foreign",         mod_int_Foreign},
  {"Fault",           mod_int_Fault},
  {"Distribution",    mod_int_Distribution},
  {"CTB",             mod_int_CTB},
  {"PID",             mod_int_PID},
  {"FDB",             mod_int_FDB},
  {"FSB",             mod_int_FSB},
  {"BitString",       mod_int_BitString},
  {"ByteString",      mod_int_ByteString},

#ifdef MODULES_LINK_STATIC
  {"FSP",             mod_int_FSP},
  {"FDP",             mod_int_FDP},
  {"CompilerSupport", mod_int_CompilerSupport},
  {"Parser",          mod_int_Parser},
  {"Browser",         mod_int_Browser},
  {"Wif",             mod_int_Wif},
  {"Schedule",        mod_int_Schedule},
  {"Debug",           mod_int_Debug},
#endif

#ifdef MISC_BUILTINS
  {"Misc",         mod_int_Misc},
#endif

#ifdef VIRTUALSITES
  {"VirtualSite",  mod_int_VirtualSite},
#endif

  {0, 0},
};


static TaggedRef ozInterfaceToRecord(OZ_C_proc_interface * I, 
				     const char * mod_name,
				     Bool isSited) {
  OZ_Term l = oz_nil();

  Builtin *bi;
  char buffer[256];
  int mod_len;
  int nam_len;

  mod_len = (mod_name)?strlen(mod_name):0;
  if (mod_len>=255) error("module name too long: %s\n",mod_name);
  if (mod_len > 0) {
    memcpy((void*)buffer,(const void*)mod_name,mod_len);
    buffer[mod_len] = '.';
    mod_len += 1;
  }

  while (I && I->name) {
    nam_len = strlen(I->name);
    if ((mod_len+nam_len)>=255)
      error("builtin name too long: %s.%s\n",mod_name,I->name);
    memcpy((void*)(buffer+mod_len),(const void*)I->name,nam_len);
    buffer[mod_len+nam_len] = '\0';
    bi = new Builtin(buffer,I->inArity,I->outArity,I->func,isSited);
 
    l = oz_cons(oz_pairA(I->name,makeTaggedConst(bi)),l);
    I++;
  }

  return OZ_recordInit(AtomExport,l);
}

OZ_BI_define(BIBootManager, 1, 1) {
  oz_declareVirtualStringIN(0, mod_name);


  // Check for builtins
  if (!strcmp("Builtins", mod_name)) 
    OZ_RETURN(builtinRecord);


  // First: find module entry in table  
  ModuleEntry * me = module_table;

  while (me && me->name && strcmp(me->name, mod_name)) {
    me++;
  }

  OZ_C_proc_interface * I = 0;
  
  if (me && me->init_function) {
    // Thats easy, is linked statically
    I = (* me->init_function)();
  } else {

#ifndef MODULES_LINK_STATIC
    // Not there, try to load dynamically!

    TaggedRef hdl;

    int n = strlen(ozconf.emuhome);
    int m = strlen(mod_name);
  
    char * libfile = new char[n + m + 64];
    
    strcpy(libfile,             ozconf.emuhome);
    strcpy(libfile + n,         "/");
    strcpy(libfile + n + 1,     mod_name);
    strcpy(libfile + n + m + 1, ".so");
    
    TaggedRef res = osDlopen(libfile,hdl);
    
    if (res)
      goto bomb;
    
    void * handle = OZ_getForeignPointer(hdl);

    char * if_name = new char[m + 16];
    
    strcpy(if_name,     "mod_int_");
    strcpy(if_name + 8, mod_name);

    init_fun_t init_function = (init_fun_t) osDlsym(handle, if_name);

    // oops, there is no `init_function()'
    if (init_function == 0)
      goto bomb;

    // retrieve table
    I = (* init_function)();
    
    delete[] libfile;
    delete[] if_name;
   
#endif 
  }

  if (!I)
    goto bomb;
  
  OZ_RETURN(ozInterfaceToRecord(I,mod_name,OK));

 bomb:
  return oz_raise(E_ERROR,E_SYSTEM,"unknownBootModule",1,
		  oz_atom(mod_name));

} OZ_BI_end


OZ_BI_define(BIdlLoad,1,1)
{
  oz_declareVirtualStringIN(0,filename);

  TaggedRef hdl;
  TaggedRef res = osDlopen(filename,hdl);
  if (res) return oz_raise(E_ERROR,AtomForeign,"dlOpen",2,
			   oz_atom(filename),res);

  void* handle = OZ_getForeignPointer(hdl);

  init_fun_t init_function = (init_fun_t) osDlsym(handle,"oz_init_module");

  // oops, there is no `init_function()'
  if (init_function == 0) {
    return oz_raise(E_ERROR,AtomForeign, "cannotFindOzInitModule", 1,
		    OZ_in(0));
  } 
  
  // `init_function()' returns the interface table
  OZ_C_proc_interface * i_table = (*init_function)();

  OZ_RETURN(oz_pair2(hdl, ozInterfaceToRecord(i_table, 0, OK)));
} OZ_BI_end

extern void BIinitPerdio();

OZ_C_proc_proto(BIcontrolVarHandler);
OZ_C_proc_proto(BIatRedo);
OZ_C_proc_proto(BIfail);
OZ_C_proc_proto(BIurl_load);
OZ_C_proc_proto(BIload);
OZ_C_proc_proto(BIprobe);
OZ_C_proc_proto(BIstartTmp);
OZ_C_proc_proto(BIportWait);


#include "modBuiltins-if.cc"

void initBuiltins() {
  builtinRecord = ozInterfaceToRecord((*mod_int_Builtins)(), 0, NO);

  OZ_protect(&builtinRecord);

  // General stuff
  BI_send         = makeTaggedConst(string2Builtin("Send"));
  BI_exchangeCell = makeTaggedConst(string2Builtin("Exchange"));
  BI_assign       = makeTaggedConst(string2Builtin("<-"));
  BI_lockLock     = makeTaggedConst(string2Builtin("Lock"));
  BI_Delay        = makeTaggedConst(string2Builtin("Delay"));
  BI_Unify        = makeTaggedConst(string2Builtin("="));

  // Exclusively used (not in builtin table)
  BI_controlVarHandler = 
    makeTaggedConst(new Builtin("controlVarHandler", 
				1, 0, BIcontrolVarHandler, OK));


  BIinitPerdio();
  
  
  // Exclusively used (not in builtin table)
  BI_probe     = 
    makeTaggedConst(new Builtin("probe", 
				1, 0, BIprobe, OK));
  BI_startTmp  = 
    makeTaggedConst(new Builtin("startTmp",
				2, 0, BIstartTmp, OK));
  BI_portWait  = 
    makeTaggedConst(new Builtin("portWait", 
				2, 0, BIportWait, OK));
  BI_atRedo    =
    makeTaggedConst(new Builtin("atRedo", 
				2, 0, BIatRedo, OK));
  BI_fail      =
    makeTaggedConst(new Builtin("fail", 
				0, 0, BIfail, OK));

  // if mapping from cfun to builtin fails
  BI_unknown =
    makeTaggedConst(new Builtin("UNKNOWN", 0, 0, BIfail,     OK));
    
  // to execute boot functor in am.cc
  BI_dot      =
    makeTaggedConst(string2Builtin("."));
  // not in builtin table...
  BI_load     = 
    makeTaggedConst(new Builtin("load",     2, 0, BIload,     OK));
  BI_url_load = 
    makeTaggedConst(new Builtin("URL.load", 1, 1, BIurl_load, OK));
  BI_boot_manager =
    makeTaggedConst(new Builtin("BootManager", 1, 1, BIBootManager, OK));


  bi_raise      = string2Builtin("Exception.raise");
  bi_raiseError = string2Builtin("Exception.raiseError");
  bi_raiseDebug = string2Builtin("Exception.raiseDebug");
}
