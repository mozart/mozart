/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Christian Schulte, 1999
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

#ifndef __BITS_HH__

#define __BITS_HH__

#if defined(INTERFACE)
#pragma interface "bits.hh"
#endif

extern const char bits_in_byte[];

static inline
int get_num_of_bits(const int m, const int * ia) {
  int s = 0;

  for (int i = m ; i--; ) {
    unsigned int iai = ia[i];
    s += (bits_in_byte[(iai             ) >> 24] +
	  bits_in_byte[(iai & 0x00ff0000) >> 16] +
	  bits_in_byte[(iai & 0x0000ff00) >>  8] +
	  bits_in_byte[(iai & 0x000000ff)]);
  }
  
  return s;
}

int get_num_of_bits_outline(const int, const int *);

#endif
