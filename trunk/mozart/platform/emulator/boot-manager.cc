/*
 *  Authors:
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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */


#include <string.h>

#include "runtime.hh"
#include "builtins.hh"
#include "os.hh"

#include "conf.h"

#ifdef MODULES_LINK_STATIC
#define DYNAMIC_MODULE(m) m
#else
#define DYNAMIC_MODULE(m) 0
#endif

#ifdef MODULES_LINK_STATIC
// Declarations for all modules than can be loaded dynamically

#include "modWif.dcl"
#include "modFDP.dcl"
#include "modSchedule.dcl"
#include "modParser.dcl"
#include "modFSP.dcl"

static OZ_C_proc_interface mod_int_Wif[] = {
#include "modWif.tbl"
 {0,0,0,0}
};

static OZ_C_proc_interface mod_int_FDP[] = {
#include "modFDP.tbl"
 {0,0,0,0}
};

static OZ_C_proc_interface mod_int_Schedule[] = {
#include "modSchedule.tbl"
 {0,0,0,0}
};

static OZ_C_proc_interface mod_int_FSP[] = {
#include "modFSP.tbl"
 {0,0,0,0}
};

static OZ_C_proc_interface mod_int_Parser[] = {
#include "modParser.tbl"
 {0,0,0,0}
};

#endif


struct ModuleEntry {
  const char *          name;
  OZ_C_proc_interface * interface;
};


#include "modProperty.dcl"
#include "modOS.dcl"
#include "modPickle.dcl"
#include "modURL.dcl"
#include "modPID.dcl"
#include "modFDB.dcl"
#include "modFSB.dcl"

static OZ_C_proc_interface mod_int_Property[] = {
#include "modProperty.tbl"
 {0,0,0,0}
};
static OZ_C_proc_interface mod_int_OS[] = {
#include "modOS.tbl"
 {0,0,0,0}
};
static OZ_C_proc_interface mod_int_Pickle[] = {
#include "modPickle.tbl"
 {0,0,0,0}
};
static OZ_C_proc_interface mod_int_URL[] = {
#include "modURL.tbl"
 {0,0,0,0}
};
static OZ_C_proc_interface mod_int_PID[] = {
#include "modPID.tbl"
 {0,0,0,0}
};
static OZ_C_proc_interface mod_int_FDB[] = {
#include "modFDB.tbl"
 {0,0,0,0}
};
static OZ_C_proc_interface mod_int_FSB[] = {
#include "modFSB.tbl"
 {0,0,0,0}
};


static ModuleEntry module_table[] = {
  {"Property", mod_int_Property},
  {"OS",       mod_int_OS},
  {"URL",      mod_int_URL},
  {"Pickle",   mod_int_Pickle},

  {"FDB",      mod_int_FDB},
  {"FSB",      mod_int_FSB},
  {"PID",      mod_int_PID},

  {"Wif",      DYNAMIC_MODULE(mod_int_Wif) },
  {"Parser",   DYNAMIC_MODULE(mod_int_Parser) },
  {"FDP",      DYNAMIC_MODULE(mod_int_FDP) },
  {"Schedule", DYNAMIC_MODULE(mod_int_Schedule) },
  {"FSP",      DYNAMIC_MODULE(mod_int_FSP) },

  {0, 0},
};


static TaggedRef ozInterfaceToRecord(OZ_C_proc_interface * I) {
  OZ_Term l = oz_nil();

  Builtin *bi;

  while (I && I->name) {
    bi = new Builtin(I->name,I->inArity,I->outArity,I->func,OK);
 
    l = oz_cons(oz_pairA(I->name,makeTaggedConst(bi)),l);
    I++;
  }

  return OZ_recordInit(AtomExport,l);
}


OZ_BI_define(BIBootManager, 1, 1) {
  oz_declareVirtualStringIN(0, mod_name);

  // First: find module entry in table

  ModuleEntry * me = module_table;

  while (me && me->name && strcmp(me->name, mod_name)) {
    me++;
  }

  if (!me) {
    fprintf(stderr, "Unknown boot module: %s.\n",mod_name);
    osExit(1);
  }
  
  OZ_C_proc_interface * I = 0;
  
  if (me->interface) {
    // Thats easy, is linked statically
    I = me->interface;
  } else {
    // Not there, try to load dynamically!

    TaggedRef hdl;

    int n = strlen(ozconf.emuhome);
    int m = strlen(mod_name);
  
    char * libfile = new char[n + m + 64];
    
    strcpy(libfile,             ozconf.emuhome);
    strcpy(libfile + n,         "/lib");
    strcpy(libfile + n + 4,     mod_name);
    strcpy(libfile + n + m + 4, ".so");
    
    OZ_Return res = osDlopen(libfile,hdl);
    
    if (res!=PROCEED) {
      fprintf(stderr, "Could not open boot library: %s.\n",libfile);
      osExit(1);
    }

    
    void * handle = OZ_getForeignPointer(hdl);

    char * if_name = new char[m + 16];
    
    strcpy(if_name,     "mod_int_");
    strcpy(if_name + 8, mod_name);

    I = (OZ_C_proc_interface *) osDlsym(handle, if_name);

    if (!I) {
      fprintf(stderr, "Interface %s:\n missing in boot library: %s.\n",
	      if_name, libfile);
      osExit(1);
    }

    delete[] libfile;
    delete[] if_name;
    
  }
  
  
  OZ_RETURN(ozInterfaceToRecord(I));

} OZ_BI_end
