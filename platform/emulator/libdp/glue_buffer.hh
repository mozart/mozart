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
#ifndef __GLUE_BUFFER_HH
#define __GLUE_BUFFER_HH

#include "dss_object.hh"
#include "byteBuffer.hh"

class GlueReadBuffer:public ByteBuffer, 
		     public DssReadBuffer{
  
public: 
  GlueReadBuffer(BYTE*, int); 

  virtual int availableData() const; 

  virtual void readFromBuffer(BYTE* ptr, size_t wanted);
  virtual void commitRead(size_t read);

  virtual const BYTE   getByte(); 
};


class GlueWriteBuffer:public ByteBuffer, 
		     public DssWriteBuffer{
  
public: 
  GlueWriteBuffer(BYTE*, int); 

  virtual int availableSpace() const; 

  virtual void writeToBuffer(const BYTE* ptr, size_t write); 

  virtual void         putByte(const BYTE&); 

};

#endif

