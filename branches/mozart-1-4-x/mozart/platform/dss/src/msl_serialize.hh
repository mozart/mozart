/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 2002
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

#ifndef __DSS_SERIALIZE_HH
#define __DSS_SERIALIZE_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "base.hh"
#include "dss_comService.hh"

  // *********** Safe put/get-Byte using functions ************
  void gf_MarshalNumber(DssWriteBuffer *bs, unsigned int i) ;
  int  gf_UnmarshalNumber(DssReadBuffer *bs);

  
  inline void gf_Marshal8bitInt(DssWriteBuffer *bs, unsigned int i){
    Assert((i & 0xFF) == i);
    bs->putByte(static_cast<BYTE>(i));
  }
  inline int gf_Unmarshal8bitInt(DssReadBuffer *bs){
    return static_cast<unsigned int>(bs->getByte());
  }
  
  
  // used by all dcts to annotate the byte streams
  enum MARSHAL_TOKENS{
    MT_START,
    MT_ENTRY,
    MT_END,
    MT_SUSP
  };



#endif // __DSS_SERIALIZE
