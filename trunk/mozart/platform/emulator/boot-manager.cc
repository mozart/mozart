/*
 *  Authors:
 *    Denys Duchier (duchier@ps.uni-sb.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
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


#include "conf.h"

#include <string.h>

#include "builtins.hh"
#include "os.hh"
#include "am.hh"
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

// print  name -> builtin	(for unmarshalling)
// module name -> module

#include "dictionary.hh"
TaggedRef dictionary_of_builtins;
TaggedRef dictionary_of_modules;


/*
 * Modules that are always in the emulator: Extensions
 */

#include "modProperty-if.cc"
#include "modOS-if.cc"
#include "modPickle-if.cc"
#include "modURL-if.cc"
#include "modFDB-if.cc"
#include "modFSB-if.cc"
#include "modSystem-if.cc"
#include "modApplication-if.cc"
#include "modCTB-if.cc"
#include "modFinalize-if.cc"
#include "modProfile-if.cc"
#include "modDistribution-if.cc"

/*
 * Modules that are always in the emulator: Base
 */

#include "modArray-if.cc"
#include "modAtom-if.cc"
#include "modBitArray-if.cc"
#include "modBool-if.cc"
#include "modCell-if.cc"
#include "modChar-if.cc"
#include "modChunk-if.cc"
#include "modClass-if.cc"
#include "modDictionary-if.cc"
#include "modException-if.cc"
#include "modFloat-if.cc"
#include "modForeignPointer-if.cc"
#include "modInt-if.cc"
#include "modList-if.cc"
#include "modLiteral-if.cc"
#include "modLock-if.cc"
#include "modName-if.cc"
#include "modNumber-if.cc"
#include "modObject-if.cc"
#include "modPort-if.cc"
#include "modProcedure-if.cc"
#include "modRecord-if.cc"
#include "modSpace-if.cc"
#include "modString-if.cc"
#include "modThread-if.cc"
#include "modTime-if.cc"
#include "modTuple-if.cc"
#include "modUnit-if.cc"
#include "modValue-if.cc"
#include "modVirtualString-if.cc"
#include "modBitString-if.cc"
#include "modByteString-if.cc"
#include "modInterFault-if.cc"

/*
 * Modules that are possibly dynamically loaded
 */

#ifdef MODULES_LINK_STATIC

#include "modTk-if.cc"
#include "modFDP-if.cc"
#include "modSchedule-if.cc"
#include "modParser-if.cc"
#include "modFSP-if.cc"
#include "modCompilerSupport-if.cc"
#include "modBrowser-if.cc"
#include "modDebug-if.cc"
#include "modPID-if.cc"
#include "modFault-if.cc"
#include "modDPMisc-if.cc"
#include "modVirtualSite-if.cc"

// Here comes the faked DPB interface
extern "C"
{
  OZ_C_proc_interface * mod_int_DPB(void)
  {
    static OZ_C_proc_interface i_table[] = {
      {0,0,0,0}
    };

    return i_table;
  } /* mod_int_DPB(void) */
} /* extern "C" */

#endif

/*
 * Builtins that depend on particular configuration
 */

#ifdef MISC_BUILTINS
#include "modMisc-if.cc"
#endif

#ifdef CS_PROFILE
OZ_BI_proto(BIgetCloneDiff);

extern "C" 
{
  OZ_C_proc_interface * mod_int_CloneDiff(void) {
    static OZ_C_proc_interface i_table[] = {
      {"get",        1,      1,BIgetCloneDiff},
      {0,0,0,0}
    };
    
    return i_table;
  }
}
#endif




typedef OZ_C_proc_interface * (* init_fun_t) (void);

struct ModuleEntry {
  const char * name;
  init_fun_t   init_function;
};

/*
 * Module table for extensions
 */
static ModuleEntry ext_module_table[] = {
  // Most important stuff
  {"Property",        mod_int_Property},
  {"OS",              mod_int_OS},
  {"URL",             mod_int_URL},
  {"Pickle",          mod_int_Pickle},
  {"System",          mod_int_System},
  {"Application",     mod_int_Application},
  {"Finalize",        mod_int_Finalize},
  {"Profile",         mod_int_Profile},
  {"Distribution",    mod_int_Distribution},
  {"CTB",             mod_int_CTB},
  {"FDB",             mod_int_FDB},
  {"FSB",             mod_int_FSB},

#ifdef MODULES_LINK_STATIC
  {"FSP",             mod_int_FSP},
  {"FDP",             mod_int_FDP},
  {"CompilerSupport", mod_int_CompilerSupport},
  {"Parser",          mod_int_Parser},
  {"Browser",         mod_int_Browser},
  {"Tk",              mod_int_Tk},
  {"Schedule",        mod_int_Schedule},
  {"Debug",           mod_int_Debug},
  {"DPB",             mod_int_DPB},
  {"PID",             mod_int_PID},
  {"Fault",           mod_int_Fault},
  {"DPMisc",          mod_int_DPMisc},
  {"VirtualSite",     mod_int_VirtualSite},
#endif

#ifdef MISC_BUILTINS
  {"Misc",         mod_int_Misc},
#endif

#ifdef CS_PROFILE
  {"CloneDiff", mod_int_CloneDiff},
#endif

  {0, 0},
};


/*
 * Module table for base
 */
static ModuleEntry base_module_table[] = {
  {"Array",		mod_int_Array},
  {"Atom",		mod_int_Atom},
  {"BitArray",		mod_int_BitArray},
  {"Bool",		mod_int_Bool},
  {"Cell",		mod_int_Cell},
  {"Char",		mod_int_Char},
  {"Chunk",		mod_int_Chunk},
  {"Class",		mod_int_Class},
  {"Dictionary",	mod_int_Dictionary},
  {"Exception",		mod_int_Exception},
  {"Float",		mod_int_Float},
  {"ForeignPointer",	mod_int_ForeignPointer},
  {"Int",		mod_int_Int},
  {"List",		mod_int_List},
  {"Literal",		mod_int_Literal},
  {"Lock",		mod_int_Lock},
  {"Name",		mod_int_Name},
  {"Number",		mod_int_Number},
  {"Object",		mod_int_Object},
  {"Port",		mod_int_Port},
  {"Procedure",		mod_int_Procedure},
  {"Record",		mod_int_Record},
  {"Space",		mod_int_Space},
  {"String",		mod_int_String},
  {"Thread",		mod_int_Thread},
  {"Time",		mod_int_Time},
  {"Tuple",		mod_int_Tuple},
  {"Unit",		mod_int_Unit},
  {"Value",		mod_int_Value},
  {"VirtualString",	mod_int_VirtualString},
  {"BitString",		mod_int_BitString},
  {"ByteString",	mod_int_ByteString},
  {"InterFault",	mod_int_InterFault},
  {0,0}
};


OZ_BI_proto(BIcontrolVarHandler);
OZ_BI_proto(BIatRedo);
OZ_BI_proto(BIfail);
OZ_BI_proto(BIurl_load);
OZ_BI_proto(BIload);



static TaggedRef ozInterfaceToRecord(OZ_C_proc_interface * I, 
				     const char * mod_name,
				     Bool isSited) {
  OZ_Term l = oz_nil();

  Builtin *bi;
  char buffer[256];
  int mod_len;
  int nam_len;

  mod_len = (mod_name)?strlen(mod_name):0;
  if (mod_len>=255) OZ_error("module name too long: %s\n",mod_name);
  if (mod_len > 0) {
    memcpy((void*)buffer,(const void*)mod_name,mod_len);
    buffer[mod_len] = '.';
    mod_len += 1;
  }

  while (I && I->name) {
    nam_len = strlen(I->name);
    if ((mod_len+nam_len)>=255)
      OZ_error("builtin name too long: %s.%s\n",mod_name,I->name);
    memcpy((void*)(buffer+mod_len),(const void*)I->name,nam_len);
    buffer[mod_len+nam_len] = '\0';
    bi = new Builtin(buffer,I->inArity,I->outArity,I->func,isSited);
 
    l = oz_cons(oz_pairA(I->name,makeTaggedConst(bi)),l);
    I++;
  }

  return OZ_recordInit(AtomExport,l);
}


OZ_Term getBuiltin_oz(const char*Name)
{
  TaggedRef val;
  return
    (tagged2Dictionary(dictionary_of_builtins)
     ->getArg(oz_atom(Name),val) == PROCEED)
    ? val :
    (OZ_warning("[builtin not found: %s]\n",Name),0);
}


Builtin* getBuiltin_c(const char*Name)
{
  OZ_Term val = getBuiltin_oz(Name);
  Assert(val!=0);
  return tagged2Builtin(val);
}


Builtin* atom2Builtin(TaggedRef a) {
  TaggedRef val;
  return (tagged2Dictionary(dictionary_of_builtins)
	  ->getArg(a,val) == PROCEED)
    ? tagged2Builtin(val) :
    (OZ_warning("[builtin not found: %s]\n",OZ_atomToC(a)),
     ((Builtin*) 0));
}

#include <ctype.h>

void link_base_modules() {
  char buffer[256];
  for (ModuleEntry * E = base_module_table;(E && E->name);E++) {
    int mod_len = strlen(E->name);
    memcpy((void*)buffer,(const void*)E->name,mod_len);
    buffer[mod_len] = '.';
    mod_len += 1;
    OZ_Term list = oz_nil();
    for (OZ_C_proc_interface* I = (E->init_function)();
	 (I && I->name);I++) {
      int nam_len = strlen(I->name);
      // put quotes around non-alpha names
      int need_quote = (isalpha(I->name[0]))?0:1;;
      if (need_quote) buffer[mod_len] = '\'';
      memcpy((void*)(buffer+mod_len+need_quote),(const void*)I->name,nam_len);
      if (need_quote) {
	int n = mod_len+1+nam_len;
	buffer[n] = '\'';
	buffer[n+1] = '\0';
      }
      else buffer[mod_len+nam_len] = '\0';
      //cerr << "[Builtin: " << buffer << "]" << endl;
      Builtin* bi = new Builtin(buffer,I->inArity,I->outArity,I->func,NO);
      OZ_Term oz_bi  = makeTaggedConst(bi);
      list = oz_cons(oz_pairA(I->name,oz_bi),list);
      tagged2Dictionary(dictionary_of_builtins)
	->setArg(oz_atom(buffer),oz_bi);
    }
    tagged2Dictionary(dictionary_of_modules)
      ->setArg(oz_atom(E->name),OZ_recordInit(AtomExport,list));
  }
}

void link_ext_modules() {
  ModuleEntry * me = ext_module_table;

  while (me && me->name) {
    TaggedRef module = ozInterfaceToRecord((* me->init_function)(),
					   me->name,OK);
    tagged2Dictionary(dictionary_of_modules)
      ->setArg(oz_atom(me->name),module);

    me++;
  }

}

#ifdef DLOPEN_UNDERSCORE
#define USC "_"
#else
#define USC ""
#endif

OZ_BI_define(BIObtainNative, 2, 1) {
  oz_declareIN(0, is_boot_tagged);
  oz_declareVirtualStringIN(1, name);

  Bool is_boot = oz_isTrue(oz_deref(is_boot_tagged));
  
  init_fun_t init_function = 0;
  char * if_identifier;
  char * filename;
  char * mod_name = (char *) 0;

  if (is_boot) {
    // Might be something linked in statically, so try to find it in table
    
    TaggedRef module;

    if (tagged2Dictionary(dictionary_of_modules)
	->getArg(oz_atom(name),module) == PROCEED)
      OZ_RETURN(module);

    // Okay, later we will need a filename!
    int n = strlen(ozconf.emuhome);
    int m = strlen(name);
    
    mod_name = name;

    filename = new char[n + m + 64];
    
    strcpy(filename, ozconf.emuhome);
    strcat(filename, "/");
    strcat(filename, name);
    strcat(filename, ".so");
    
    // We have to set the interface identifier
    if_identifier = new char[m + 16];
    
    strcpy(if_identifier, USC "mod_int_");
    strcat(if_identifier, name);
  } else {
    // Here the identifier is always the same
    if_identifier = USC "oz_init_module";
    filename      = name;
  }
  
  void *handle;
  TaggedRef res = osDlopen(filename,&handle);

  if (res) {
    struct stat buf;
    TaggedRef file_atom = oz_atom(filename);
    if (is_boot) 
      free(if_identifier);
  retry:
    if (stat(filename,&buf)<0)
      if (errno==EINTR) {
	goto retry;
      } else {
	// file does not exist (or would need searching LD_LIBRARY_PATH
	// which we don't attempt here - too bad)
	if (is_boot) free(filename);
	return oz_raise(E_SYSTEM,AtomForeign,"dlOpen",1,file_atom);
      }
    // file presumed to exist
    if (is_boot) 
      free(filename);
    return oz_raise(E_ERROR,AtomForeign,"dlOpen",2,
		    file_atom,res);
  }
  
  init_function = (init_fun_t) osDlsym(handle,if_identifier);
  
  // oops, there is no `init_function()'
  if (init_function == 0) {
    return oz_raise(E_ERROR,AtomForeign, "cannotFindOzInitModule", 1,
		    OZ_in(1));
  } 

  if (!mod_name) {
    char * name_sym = USC "oz_module_name";
    mod_name = (char *)  osDlsym(handle,name_sym);
  }
    
  OZ_RETURN(ozInterfaceToRecord((*init_function)(), mod_name, OK));

} OZ_BI_end



void initBuiltins() {
  //
  // create dictionaries for builtins and builtin modules
  //
  dictionary_of_builtins =
    makeTaggedConst(new OzDictionary(oz_rootBoard()));
  OZ_protect(&dictionary_of_builtins);
  dictionary_of_modules =
    makeTaggedConst(new OzDictionary(oz_rootBoard()));
  OZ_protect(&dictionary_of_modules);
  //
  // populate it
  //
  link_base_modules();
  link_ext_modules();

#if 0
  //
  // When renaming builtins: also enter the old names of builtins
  // so that unmarshalling still works. turns this off once a new
  // ozc.ozm has been created.
  //
  static struct { char* oldName; char* newName; }
  *help_ptr, help_table[] = {

    {"IsArray",		"Array.is"},

    {0,0}
  };

  for (help_ptr = help_table; help_ptr->oldName; help_ptr++) {
    OZ_Term oz_old = oz_atom(help_ptr->oldName);
    OZ_Term oz_new = oz_atom(help_ptr->newName);
    if (oz_old != oz_new) {
      TaggedRef val;
      if (tagged2Dictionary(dictionary_of_builtins)
	  ->getArg(oz_new,val) != PROCEED)
	OZ_error("new builtin not found: [old: %s] [new: %s]\n",
		 help_ptr->oldName,help_ptr->newName);
      TaggedRef ignore;
      if (tagged2Dictionary(dictionary_of_builtins)
	  ->getArg(oz_old,ignore) == PROCEED)
	OZ_error("old builtin exists already: [old: %s] [new: %s]\n",
		 help_ptr->oldName,help_ptr->newName);
      tagged2Dictionary(dictionary_of_builtins)
	->setArg(oz_old,val);
    }
  }

#endif

  // General stuff
  BI_send         = getBuiltin_oz("Port.send"		);
  BI_exchangeCell = getBuiltin_oz("Cell.exchangeFun"	);
  BI_assign       = getBuiltin_oz("Object.'<-'"		);
  BI_Delay        = getBuiltin_oz("Time.delay"		);
  BI_Unify        = getBuiltin_oz("Value.'='"		);

  // Exclusively used (not in builtin table)
  BI_controlVarHandler = 
    makeTaggedConst(new Builtin("controlVarHandler", 
				1, 0, BIcontrolVarHandler, OK));

  // Exclusively used (not in builtin table)
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
  BI_dot      = getBuiltin_oz("Value.'.'");
  // not in builtin table...
  BI_load     = 
    makeTaggedConst(new Builtin("load",     2, 0, BIload,     OK));
  BI_url_load = 
    makeTaggedConst(new Builtin("URL.load", 1, 1, BIurl_load, OK));
  // this actually _is_ in the builtin table
  BI_obtain_native =
    makeTaggedConst(new Builtin("OBTAIN_NATIVE", 2, 1, BIObtainNative, OK));

  bi_raise      = getBuiltin_c("Exception.raise");
  bi_raiseError = getBuiltin_c("Exception.raiseError");
}
