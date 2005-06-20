/*
 *  Authors:
 *    Anna Neiderud (annan@sics.se)
 * 
 *  Contributors:
 * 
 *  Copyright:
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
#pragma implementation "msl_transObj.hh"
#endif

#include "msl_transObj.hh"

namespace _msl_internal{

  TransObj::TransObj(const int& s, MsgnLayerEnv* env):
    a_mslEnv(env), a_comObj(NULL), 
    a_bufferSize(s){;}

  
  void TransObj::setOwner(ComObj* com){  a_comObj = com; }
  int  TransObj::getBufferSize() const { return a_bufferSize; }
} //End namespace
