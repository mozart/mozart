/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
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


#if defined(INTERFACE)
#pragma implementation "msl_serialize.hh"
#endif

#include <string.h>
#include "msl_serialize.hh"


  // SSBit just to avoid name clashes
  const unsigned int SSBit = 1<<7;


  int gf_UnmarshalNumber(DssReadBuffer *bs){
    unsigned int ret = 0, shft = 0;
    unsigned int c = bs->getByte();
    while (c >= SSBit) {
      ret += ((c-SSBit) << shft);
      c = bs->getByte();
      shft += 7;
    }
    ret |= (c<<shft);
    return ret;
  }

  void gf_MarshalNumber(DssWriteBuffer *bs, unsigned int i) {
    while(i >= SSBit) {
      bs->putByte((i%SSBit)|SSBit);
      i /= SSBit;
    }
    bs->putByte(i);
  }
