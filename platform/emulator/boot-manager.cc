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

// print  name -> builtin       (for unmarshalling)
// module name -> module

#include "dictionary.hh"
TaggedRef dictionary_of_builtins;
TaggedRef dictionary_of_modules;

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
  TaggedRef module_atom = oz_atom(mod_name);
  TaggedRef module;

  // Check for builtin module (or previously linked)
  if (tagged2Dictionary(dictionary_of_modules)
      ->getArg(module_atom,module) == PROCEED)
      OZ_RETURN(module);

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

  // enter it into the dictionary of module
  // there can be no clash otherwise we would have found it
  // when we looked it up
  module = ozInterfaceToRecord(I,mod_name,OK);
  tagged2Dictionary(dictionary_of_modules)
    ->setArg(module_atom,module);
  OZ_RETURN(module);

 bomb:
  return oz_raise(E_ERROR,E_SYSTEM,"unknownBootModule",1,
                  module_atom);

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

OZ_BI_proto(BIcontrolVarHandler);
OZ_BI_proto(BIatRedo);
OZ_BI_proto(BIfail);
OZ_BI_proto(BIurl_load);
OZ_BI_proto(BIload);
OZ_BI_proto(BIprobe);
OZ_BI_proto(BIstartTmp);
OZ_BI_proto(BIportWait);

// include all builtin modules
//
#include "modBoot-if.cc"
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

static ModuleEntry bi_module_table[] = {
  {"Boot",              mod_int_Boot},
  {"Array",             mod_int_Array},
  {"Atom",              mod_int_Atom},
  {"BitArray",          mod_int_BitArray},
  {"Bool",              mod_int_Bool},
  {"Cell",              mod_int_Cell},
  {"Char",              mod_int_Char},
  {"Chunk",             mod_int_Chunk},
  {"Class",             mod_int_Class},
  {"Dictionary",        mod_int_Dictionary},
  {"Exception",         mod_int_Exception},
  {"Float",             mod_int_Float},
  {"ForeignPointer",    mod_int_ForeignPointer},
  {"Int",               mod_int_Int},
  {"Literal",           mod_int_Literal},
  {"Lock",              mod_int_Lock},
  {"Name",              mod_int_Name},
  {"Number",            mod_int_Number},
  {"Object",            mod_int_Object},
  {"Port",              mod_int_Port},
  {"Procedure",         mod_int_Procedure},
  {"Record",            mod_int_Record},
  {"Space",             mod_int_Space},
  {"String",            mod_int_String},
  {"Thread",            mod_int_Thread},
  {"Time",              mod_int_Time},
  {"Tuple",             mod_int_Tuple},
  {"Unit",              mod_int_Unit},
  {"Value",             mod_int_Value},
  {"VirtualString",     mod_int_VirtualString},
  {"BitString",         mod_int_BitString},
  {"ByteString",        mod_int_ByteString},
  {0,0}
};

OZ_Term getBuiltin_oz(const char*Name)
{
  TaggedRef val;
  return
    (tagged2Dictionary(dictionary_of_builtins)
     ->getArg(oz_atom(Name),val) == PROCEED)
    ? val :
    (warning("[builtin not found: %s]\n",Name),0);
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
    (warning("[builtin not found: %s]\n",OZ_atomToC(a)),
     ((Builtin*) 0));
}

#include <ctype.h>

void link_bi_modules()
{
  char buffer[256];
  for (ModuleEntry * E = bi_module_table;(E && E->name);E++) {
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
  link_bi_modules();
#if 0
  //
  // When renaming builtins: also enter the old names of builtins
  // so that unmarshalling still works. turns this off once a new
  // ozc.ozm has been created.
  //
  static struct { char* oldName; char* newName; }
  *help_ptr, help_table[] = {
    //
    // Boot
    //
    {"builtin",         "Boot.builtin"},
    {"BootManager",     "Boot.manager"},
    //
    // Array
    //
    {"IsArray",         "Array.is"},
    {"NewArray",        "Array.new"},
    {"Array.high",      "Array.high"},
    {"Array.low",       "Array.low"},
    {"Get",             "Array.get"},
    {"Put",             "Array.put"},
    //
    // Atom
    //
    {"IsAtom",          "Atom.is"},
    {"AtomToString",    "Atom.toString"},
    //
    // BitArray
    //
    {"BitArray.new",    "BitArray.new"},
    {"BitArray.is",     "BitArray.is"},
    {"BitArray.set",    "BitArray.set"},
    {"BitArray.clear",  "BitArray.clear"},
    {"BitArray.test",   "BitArray.test"},
    {"BitArray.low",    "BitArray.low"},
    {"BitArray.high",   "BitArray.high"},
    {"BitArray.clone",  "BitArray.clone"},
    {"BitArray.or",     "BitArray.or"},
    {"BitArray.and",    "BitArray.and"},
    {"BitArray.card",   "BitArray.card"},
    {"BitArray.disjoint","BitArray.disjoint"},
    {"BitArray.nimpl",  "BitArray.nimpl"},
    {"BitArray.toList", "BitArray.toList"},
    {"BitArray.complementToList","BitArray.complementToList"},
    //
    // Bool
    //
    {"IsBool",          "Bool.is"},
    {"Not",             "Bool.not"},
    {"And",             "Bool.and"},
    {"Or",              "Bool.or"},
    //
    // Cell
    //
    {"IsCell",          "Cell.is"},
    {"NewCell",         "Cell.new"},
    {"Exchange",        "Cell.exchange"},
    {"Access",          "Cell.access"},
    {"Assign",          "Cell.assign"},
    //
    // Char
    //
    {"IsChar",          "Char.is"},
    {"Char.isAlNum",    "Char.isAlNum"},
    {"Char.isAlpha",    "Char.isAlpha"},
    {"Char.isCntrl",    "Char.isCntrl"},
    {"Char.isDigit",    "Char.isDigit"},
    {"Char.isGraph",    "Char.isGraph"},
    {"Char.isLower",    "Char.isLower"},
    {"Char.isPrint",    "Char.isPrint"},
    {"Char.isPunct",    "Char.isPunct"},
    {"Char.isSpace",    "Char.isSpace"},
    {"Char.isUpper",    "Char.isUpper"},
    {"Char.isXDigit",   "Char.isXDigit"},
    {"Char.toLower",    "Char.toLower"},
    {"Char.toUpper",    "Char.toUpper"},
    {"Char.toAtom",     "Char.toAtom"},
    {"Char.type",       "Char.type"},
    //
    // Chunk
    //
    {"IsChunk",         "Chunk.is"},
    {"NewChunk",        "Chunk.new"},
    //
    // Class
    //
    {"getClass",        "Class.get"},
    //
    // Dictionary
    //
    {"IsDictionary",            "Dictionary.is"},
    {"NewDictionary",           "Dictionary.new"},
    {"Dictionary.get",          "Dictionary.get"},
    {"Dictionary.condGet",      "Dictionary.condGet"},
    {"Dictionary.put",          "Dictionary.put"},
    {"Dictionary.remove",       "Dictionary.remove"},
    {"Dictionary.removeAll",    "Dictionary.removeAll"},
    {"Dictionary.member",       "Dictionary.member"},
    {"Dictionary.keys",         "Dictionary.keys"},
    {"Dictionary.entries",      "Dictionary.entries"},
    {"Dictionary.items",        "Dictionary.items"},
    {"Dictionary.clone",        "Dictionary.clone"},
    {"Dictionary.markSafe",     "Dictionary.markSafe"},
    //
    // Exception
    //
    {"Exception.raise",         "Exception.raise"},
    {"Exception.raiseError",    "Exception.raiseError"},
    {"Exception.raiseDebug",    "Exception.raiseDebug"},
    {"Exception.raiseDebugCheck","Exception.raiseDebugCheck"},
    {"Exception.taskStackError","Exception.taskStackError"},
    {"Exception.location",      "Exception.location"},
    //
    // Float
    //
    {"IsFloat", "Float.is"},
    {"Exp",     "Float.exp"},
    {"Log",     "Float.log"},
    {"Sqrt",    "Float.sqrt"},
    {"Sin",     "Float.sin"},
    {"Asin",    "Float.asin"},
    {"Cos",     "Float.cos"},
    {"Acos",    "Float.acos"},
    {"Tan",     "Float.tan"},
    {"Atan",    "Float.atan"},
    {"Ceil",    "Float.ceil"},
    {"Floor",   "Float.floor"},
    {"Round",   "Float.round"},
    {"Atan2",   "Float.atan2"},
    {"fPow",    "Float.fPow"},
    {"FloatToString",   "Float.toString"},
    {"FloatToInt",      "Float.toInt"},
    //
    // Foreign Pointer
    //
    {"IsForeignPointer",        "ForeignPointer.is"},
    {"ForeignPointer.toInt",    "ForeignPointer.toInt"},
    //
    // Int
    //
    {"IsInt",           "Int.is"},
    {"IntToFloat",      "Int.toFloat"},
    {"IntToString",     "Int.toString"},
    {"div",             "Int.div"},
    {"mod",             "Int.mod"},
    {"+1",              "Int.'+1'"},
    {"-1",              "Int.'-1'"},
    //
    // Literal
    //
    {"IsLiteral",       "Literal.is"},
    //
    // Lock
    //
    {"IsLock",  "Lock.is"},
    {"NewLock", "Lock.new"},
    {"Lock",    "Lock.lock"},
    {"Unlock",  "Lock.unlock"},
    //
    // Name
    //
    {"IsName",  "Name.is"},
    {"NewName", "Name.new"},
    {"NewUniqueName",   "Name.newUnique"},
    //
    // Number
    //
    {"IsNumber","Number.is"},
    {"Abs",     "Number.abs"},
    {"/",       "Number.'/'"},
    {"*",       "Number.'*'"},
    {"-",       "Number.'-'"},
    {"+",       "Number.'+'"},
    {"~",       "Number.'~'"},
    //
    // Object
    //
    {"IsObject",        "Object.is"},
    {"@",               "Object.'@'"},
    {"<-",              "Object.'<-'"},
    {"ooExch",          "Object.ooExch"},
    {"copyRecord",      "Object.copyRecord"},
    {"makeClass",       "Object.makeClass"},
    {",",               "Object.','"},
    {"send",            "Object.send"},
    {"ooGetLock",       "Object.ooGetLock"},
    {"newObject",       "Object.newObject"},
    {"New",             "Object.new"},
    //
    // Port
    //
    {"IsPort",  "Port.is"},
    {"NewPort", "Port.new"},
    {"Send",    "Port.send"},
    //
    // Procedure
    //
    {"IsProcedure",     "Procedure.is"},
    {"ProcedureArity",  "Procedure.arity"},
    //
    // Record
    //
    {"IsRecord",        "Record.is"},
    {"IsRecordC",       "Record.isC"},
    {"Adjoin",          "Record.adjoin"},
    {"AdjoinList",      "Record.adjoinList"},
    {"record",          "Record.record"},
    {"Arity",           "Record.arity"},
    {"AdjoinAt",        "Record.adjoinAt"},
    {"Label",           "Record.label"},
    {"hasLabel",        "Record.hasLabel"},
    {"TellRecord",      "Record.tellRecord"},
    {"WidthC",          "Record.widthC"},
    {"monitorArity",    "Record.monitorArity"},
    {"tellRecordSize",  "Record.tellRecordSize"},
    {".",               "Record.'.'"},
    {"^",               "Record.'^'"},
    {"Width",           "Record.width"},
    //
    // Space
    //
    {"Space.new",       "Space.new"},
    {"IsSpace",         "Space.is"},
    {"Space.ask",       "Space.ask"},
    {"Space.askVerbose","Space.askVerbose"},
    {"Space.merge",     "Space.merge"},
    {"Space.clone",     "Space.clone"},
    {"Space.commit",    "Space.commit"},
    {"Space.inject",    "Space.inject"},
    //
    // String
    //
    {"IsString",        "String.is"},
    {"StringToAtom",    "String.toAtom"},
    {"StringToInt",     "String.toInt"},
    {"StringToFloat",   "String.toFloat"},
    //
    // Thread
    //
    {"Thread.is",       "Thread.is"},
    {"Thread.this",     "Thread.this"},
    {"Thread.suspend",  "Thread.suspend"},
    {"Thread.resume",   "Thread.resume"},
    {"Thread.injectException",  "Thread.injectException"},
    {"Thread.preempt",  "Thread.preempt"},
    {"Thread.setPriority",      "Thread.setPriority"},
    {"Thread.getPriority",      "Thread.getPriority"},
    {"Thread.isSuspended",      "Thread.isSuspended"},
    {"Thread.state",    "Thread.state"},
    {"Thread.create",   "Thread.create"},
    //
    // Time
    //
    {"Alarm",           "Time.alarm"},
    {"Delay",           "Time.delay"},
    {"Time.time",       "Time.time"},
    //
    // Tuple
    //
    {"IsTuple",         "Tuple.is"},
    {"MakeTuple",       "Tuple.make"},
    //
    // Unit
    //
    {"IsUnit",          "Unit.is"},
    //
    // Value
    //
    {"Wait",            "Value.wait"},
    {"WaitOr",          "Value.waitOr"},
    {"IsFree",          "Value.isFree"},
    {"IsKinded",        "Value.isKinded"},
    {"IsDet",           "Value.isDet"},
    {"Max",             "Value.max"},
    {"Min",             "Value.min"},
    {"HasFeature",      "Value.hasFeature"},
    {"CondSelect",      "Value.condSelect"},
    {"ByNeed",          "Value.byNeed"},
    {"Future",          "Value.future"},
    {"==",              "Value.'=='"},
    {"\\=",             "Value.'\\='"},
    {"<",               "Value.'<'"},
    {"=<",              "Value.'=<'"},
    {">",               "Value.'>'"},
    {">=",              "Value.'>='"},
    {"=",               "Value.'='"},
    {"Value.status",    "Value.status"},
    {"Value.type",      "Value.type"},
    //
    // VirtualString
    //
    {"IsVirtualString",         "VirtualString.is"},
    {"virtualStringLength",     "VirtualString.length"},

    {0,0}
  };

  for (help_ptr = help_table; help_ptr->oldName; help_ptr++) {
    OZ_Term oz_old = oz_atom(help_ptr->oldName);
    OZ_Term oz_new = oz_atom(help_ptr->newName);
    if (oz_old != oz_new) {
      TaggedRef val;
      if (tagged2Dictionary(dictionary_of_builtins)
          ->getArg(oz_new,val) != PROCEED)
        error("new builtin not found: [old: %s] [new: %s]\n",
              help_ptr->oldName,help_ptr->newName);
      TaggedRef ignore;
      if (tagged2Dictionary(dictionary_of_builtins)
          ->getArg(oz_old,ignore) == PROCEED)
        error("old builtin exists already: [old: %s] [new: %s]\n",
              help_ptr->oldName,help_ptr->newName);
      tagged2Dictionary(dictionary_of_builtins)
        ->setArg(oz_old,val);
    }
  }

#endif

  // General stuff
  BI_send         = getBuiltin_oz("Port.send"           );
  BI_exchangeCell = getBuiltin_oz("Cell.exchange"       );
  BI_assign       = getBuiltin_oz("Object.'<-'"         );
  BI_lockLock     = getBuiltin_oz("Lock.lock"           );
  BI_Delay        = getBuiltin_oz("Time.delay"          );
  BI_Unify        = getBuiltin_oz("Value.'='"           );

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
  BI_dot      = getBuiltin_oz("Record.'.'");
  // not in builtin table...
  BI_load     =
    makeTaggedConst(new Builtin("load",     2, 0, BIload,     OK));
  BI_url_load =
    makeTaggedConst(new Builtin("URL.load", 1, 1, BIurl_load, OK));
  // this actually _is_ in the builtin table
  BI_boot_manager = getBuiltin_oz("Boot.manager");


  bi_raise      = getBuiltin_c("Exception.raise");
  bi_raiseError = getBuiltin_c("Exception.raiseError");
  bi_raiseDebug = getBuiltin_c("Exception.raiseDebug");
}
