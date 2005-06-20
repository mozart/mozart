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

#ifndef __BYTEBUFFERHH
#define __BYTEBUFFERHH

#include "base.hh"
#include "memaux.hh"
#include "mbuffer.hh"

//bmc: The structure of the ByteBuffer has changed a lot

//bmc: No more class ByteBufferManager

class ByteBuffer :public MarshalerBuffer {
protected:
  BYTE *buf;
  virtual Bool putDebug();
  virtual Bool getDebug();
private:
  //
public:
  /*bmc: This commented code should be deleted in the future
  // need these to specify placement
  void* operator new(size_t,void*p) { return p; }
  void* operator new(size_t n) { return ::new char[n]; }
  */

  ByteBuffer(BYTE* ptr, int len) {
    buf = ptr;
    posMB = ptr;
    endMB = ptr + len;
  }

  void marshalBegin();
  inline int availableSpace() { return endMB - posMB; }
  
  void putNext(BYTE);
  void marshalEnd();

  // For unmarshaler
  void unmarshalBegin();
  BYTE getNext();
  void unmarshalEnd();
  
  int bufferUsed();
  
  BYTE* getCurrPtr(){return posMB;}
  void incCurrPtr(int inc){posMB+=inc; Assert(endMB>posMB);}
  
  //
  DebugCode(BYTE* getGetptr() { return (NULL); })
  DebugCode(BYTE* getPutptr() { return (NULL); })
  DebugCode(BYTE* getPosMB() { return (posMB); })
  DebugCode(BYTE* getEndMB() { return (endMB); })

  //bmc: methods that go no more:
  // init()
  // reinit()
  // isEmpty()
  // getUsed()
  // getWriteParameters(BYTE *&buf)
  // clearWrite(int sizeWritten)
  // putInt(int i)
  // getInt()
  // canGet(int cgSize)
  // setFrameSize(int size)
  // canGetInFrame(int cgSize)
  // getCommit()
  // getReadParameters(BYTE *&buf)
  // hasRead(int sizeRead)

};

#endif


