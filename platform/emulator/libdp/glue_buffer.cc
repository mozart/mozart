/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    Erik Klintskog (erik@sics.se)
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


#include "glue_buffer.hh"  

GlueReadBuffer::GlueReadBuffer(BYTE* b, int len) : ByteBuffer(b, len) {}

// This one had to be redefined.  We cannot reuse availableSpace()
// from ByteBuffer since it does not have the 'const' modifier.
int GlueReadBuffer::availableData() const { return endMB - posMB; }

void GlueReadBuffer::commitRead(size_t size) {}

const BYTE GlueReadBuffer::getByte(){ return get(); }

void GlueReadBuffer::readFromBuffer(BYTE* ptr, size_t size) {
  for(; size>0; size--) { *ptr = get(); ptr++; }
}



GlueWriteBuffer::GlueWriteBuffer(BYTE* b, int len) : ByteBuffer(b, len) {}

// We cannot inherit this method from ByteBuffer, since it does not
// have the 'const' modifier.
int GlueWriteBuffer::availableSpace() const { return endMB - posMB;}

void GlueWriteBuffer::putByte(const BYTE& c) { put(c); }  

void GlueWriteBuffer::writeToBuffer(const BYTE* ptr, size_t size) {
  for(; size>0; size--) { put(*ptr); ptr++; }
} 
