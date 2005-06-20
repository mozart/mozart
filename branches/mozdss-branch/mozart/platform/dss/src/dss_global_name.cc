/*
 *  Authors:
 *    Per Sahlin (sahlin@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Per Sahlin, 2004
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
#pragma implementation "dss_global_name.hh"
#endif

#include "msl_serialize.hh"
#include "dss_global_name.hh"
namespace _dss_internal{ //Start namespace
  
  GlobalName::~GlobalName() {
   }
  
  void GlobalName::marshal(DssWriteBuffer* bb){
    gf_MarshalNumber(bb, a_pk);
    gf_MarshalNumber(bb, a_sk);
  }


  GlobalNameTable::~GlobalNameTable() {
    
  }

  

} //end namespace
