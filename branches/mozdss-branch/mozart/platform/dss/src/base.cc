/*
 *  Authors:
 *    Erik Klintskog
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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


#if defined(INTERFACE)
#pragma implementation "base.hh"
#pragma implementation "dss_classes.hh"
#endif

#include "base.hh"
#include "dss_classes.hh"

// For the environment


#include <stdio.h>
#include <stdarg.h>

#ifndef WIN32
#include <unistd.h>
#else
#include <process.h>
#endif

// ************************ GLOBALS ******************************
 DSS_LOG_LEVEL      g_dssLogLevel = DLL_NOTHING;


  // *********************************************************************
  //
  //  Utilities
  //
  // *********************************************************************

  char *gf_bool2string(const bool& b){
    static char buf[6];
    if(b) sprintf(buf,"TRUE");
    else  sprintf(buf,"FALSE");
    return buf;
  }


  void gf_printBuf(const char* const format,BYTE* const buf, int len){
    BYTE c[4];
    gf_integer2char(c,0);
    printf("%s:%d:",format,len);
    for(int i = 0; i< len; ++i){
      c[0] = buf[i];
      printf("%02x",gf_char2integer(c));
      if(((i+1)%10) == 0 && (i > 0)) printf(" :%d: ",i+1);
    }
    printf("\n");
  }


  void dssError(const char*  format, ...){
    fprintf(stderr,"DSS_FATAL_ERROR:");
    va_list ap;
    va_start(ap,format);
    vfprintf(stderr,format, ap);
    va_end(ap);
    fprintf(stderr,"\n");
#ifdef DEBUG_CHECK
    fprintf(stderr,"ENTERED DEBUG LOOP:");
    bool loop = true;
    while(loop);
#else
    _exit(1);
#endif
  }

  void dssAssert(const char* const file, const int& line, const char* const condition){
    fprintf(stderr,"%s:%d assertion '%s' failed (%d)\n",file,line,condition, getpid());
    bool loop = true;
    while(loop);
  }
 
 
  // We enable the dssLog utility here if flag is set
#ifdef DSS_LOG
  static char * const s_DLL_names[] ={
    "DSS_NOTHING   ",
    "DSS_PRINT_INFO",
    "DSS_IMPORTANT ",
    "DSS_BEHAVIOR  ",
    "DSS_DEBUG_INFO",
    "DSS_EVERYTHING"
  };
  
  void dssLog(DSS_LOG_LEVEL level,const char* const format, ...){
    if(level <= g_dssLogLevel){
      va_list ap;
      va_start(ap,format);
#ifdef DEBUG_CHECK
      fprintf(stderr,"%s::",s_DLL_names[level]); fflush(stderr);
#endif
      vfprintf(stderr,format, ap);
      fprintf(stderr,"\n");
      va_end(ap);
    }
  }

  // ****************** DSS LONGTIME ********************
  //



#endif

//Constructors added to force the inclussion of the symbols in the library.
ThreadMediator::ThreadMediator() {}
MediatorInterface::MediatorInterface() {}
Mediation_Object::Mediation_Object() {}
MutableMediatorInterface::MutableMediatorInterface() {}
MonotonicMediatorInterface::MonotonicMediatorInterface() {}
ImmutableMediatorInterface::ImmutableMediatorInterface() {}
RelaxedMutableMediatorInterface::RelaxedMutableMediatorInterface() {}
PstOutContainerInterface::PstOutContainerInterface() {}
PstInContainerInterface::PstInContainerInterface() {}
