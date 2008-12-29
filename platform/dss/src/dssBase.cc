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
#pragma implementation "dssBase.hh"
#endif

#include "dssBase.hh"

// For the environment


#include <stdio.h>
#include <stdarg.h>

#ifndef WIN32
#include <unistd.h>
#else
#include <process.h>
#endif

namespace _dss_internal{



  const int DssConfigData::DEFAULT_MANAGER_TABLE_SIZE= 100;
  const int DssConfigData::DEFAULT_PROXY_TABLE_SIZE  = 100;
  const int DssConfigData::DEFAULT_NAME_TABLE_SIZE   = 100;

  // distributed reference consistencey

  const int DssConfigData::DP_TL_LEASE               = 1800000; //ms = 30 min
  const int DssConfigData::DP_TL_UPDATE              = 600000;  //ms = 10 min
  const int DssConfigData::DP_WRC_ALPHA              = 10000;



  DssConfigData::DssConfigData():
    gc_wrc_alpha(     10000),   // Milliseconds
    gc_tl_updateTime( 65000),
    gc_tl_leaseTime(  200000){;
  }



  DSS_Environment::DSS_Environment(const DSS_Environment& de):
    a_map(                    NULL),
    a_dksInstHT(              NULL),
    a_proxyTable(             NULL),
    a_coordinatorTable(           NULL),
    a_threadTable(            NULL),
    a_myDSite(                NULL),

    a_dssconf(     DssConfigData()),
    a_dssMslClbk(             NULL),
    a_msgnLayer(              NULL),
    a_dksBackbone(            NULL),
    a_CreateXistRefCounter(      0),
    a_CreateNonXRefCounter(      0),
    a_DuplicateXistRefCounter(   0),
    a_DuplicateNonXRefCounter(   0),
    a_DuplicateToOwnerRefCounter(0)
  {;}





} //End namespace
