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

#ifdef MODULES_LINK_STATIC
#define DYNAMIC_MODULE(m) m
#else
#define DYNAMIC_MODULE(m) 0
#endif


#ifndef MODULES_LINK_STATIC

// Declarations for all modules than can be loaded dynamically

#include "moWif.dcl"
#include "moFDP.dcl"
#include "moSchedule.dcl"
#include "moParser.dcl"
#include "moFSP.dcl"

static OZ_C_proc_interface mo_int_Wif[] = {
#include "moWif.tbl"
 {0,0,0,0}
};

static OZ_C_proc_interface mo_int_FDP[] = {
#include "moFDP.tbl"
 {0,0,0,0}
};

static OZ_C_proc_interface mo_int_Schedule[] = {
#include "moSchedule.tbl"
 {0,0,0,0}
};

static OZ_C_proc_interface mo_int_FSP[] = {
#include "moFSP.tbl"
 {0,0,0,0}
};

static OZ_C_proc_interface mo_int_Parser[] = {
#include "moParser.tbl"
 {0,0,0,0}
};

#endif



extern
static module_table[] = {
  {"Wif",      DYNAMIC_MODULE(mo_int_Wif)},
  {"Parser",   DYNAMIC_MODULE(mo_int_Parser)},
  //  {"FDB",      mo_int_FDB},
  {"FDP",      DYNAMIC_MODULE(mo_int_FDP)},
  {"Schedule", DYNAMIC_MODULE(mo_int_Schedule)},
  //  {"FSB",      mo_int_FSB},
  {"FSP",      DYNAMIC_MODULE(mo_int_FSP)},
  0
}
  
OZ_BI_define(BIModuleBoot, 1, 1) {
  oz_declareVirtualStringIN(0, mod_name);
  
} OZ_BI_end

OZ_BI_define(BIdlStaticLoad,1,1)
{


  OZ_C_proc_interface * I = 0;

#ifdef STATIC_LIBWIF
  if (!strcmp(basename, "libwif.so")) {
     I = libwif_interface;
     goto success;
  }
#endif

#ifdef STATIC_LIBFD
  if (!strcmp(basename, "libfd.so")) {
     I = libfd_interface;
     goto success;
  }

  if (!strcmp(basename, "libschedule.so")) {
     I = libschedule_interface;
     goto success;
  }
#endif

#ifdef STATIC_LIBFSET
  if (!strcmp(basename, "libfset.so")) {
     I = libset_interface;
     goto success;
  }
#endif

#ifdef STATIC_LIBPARSER
  if (!strcmp(basename, "libparser.so")) {
     I = libparser_interface;
     goto success;
  }
#endif

  Assert(!I);

  { 
    TaggedRef hdl;

    int n = strlen(ozconf.emuhome);
  
    char * libfile = new char[n + strlen(basename) + 2];
    
    strcpy(libfile, ozconf.emuhome);
    
    libfile[n] = '/';
    
    strcpy(libfile + n + 1, basename);
    
    OZ_Return res = osDlopen(libfile,hdl);
    
    delete[] libfile;
    
    if (res!=PROCEED) 
      return res;
    
    void* handle = OZ_getForeignPointer(hdl);

    I = (OZ_C_proc_interface *) osDlsym(handle,"oz_interface");

  }

  if (!I)
    return oz_raise(E_ERROR,AtomForeign, "cannotFindInterface", 1,
		    OZ_in(0));

 success:
  
  OZ_RETURN(ozInterfaceToRecord(I));
  
} OZ_BI_end

