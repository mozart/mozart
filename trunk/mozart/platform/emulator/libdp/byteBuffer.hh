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

#include "genhashtbl.hh"

#include "dpBase.hh"
#include "mbuffer.hh"

#define BYTE_MODE_MARSHALING 0
#define BYTE_MODE_UNMARSHALING 1
#define BYTE_MODE_NONE 2
#define BYTE_MODE_WRITING 3
#define BYTE_MODE_READING 4

// Default values
#define BYTE_ByteBuffer_CUTOFF 200

class ByteBufferManager;

class ByteBuffer :public MarshalerBuffer {
  friend ByteBufferManager;
protected:
  BYTE *buf;
  virtual Bool putDebug();
  virtual Bool getDebug();
private:
  BYTE *putptr;
  BYTE *getptr;

  int size;
  int used;
  int mode;
  int framesize; // For unmarshaling

  inline void int2net(int i);
  inline int net2int();
  inline void putSize();
public:
  ByteBuffer();

  void init(int size,BYTE *buf);
  void reinit();
  
  Bool isEmpty();

// For sending
  int getUsed();
  int getWriteParameters(BYTE *&buf);
  void clearWrite(int sizeWritten);

// For marshaler
  void putBegin();
  inline int availableSpace() {
    Assert(mode == BYTE_MODE_MARSHALING || mode == BYTE_MODE_NONE);
    // used does not contain ongoing usage
    // leave one byte for trailer
    if (mode==BYTE_MODE_NONE)
      return size-used-1;
    else if (putptr<=posMB) 
      return size-used-(posMB-putptr)-1;
    else
      return size-used-(posMB-buf+endMB+1-putptr)-1;
  }
  void putInt(int i);
  void putNext(BYTE);
  void putEnd();

// For unmarshaler
  void getBegin();
  inline Bool canGet(int size) {
    Assert(mode == BYTE_MODE_UNMARSHALING);
    if (posMB>=getptr)
      return (used-(posMB-getptr))>=size;
    else
      return (used-(endMB-getptr)+(posMB-buf) +1)>=size;
  }
  void setFrameSize(int size);
  inline Bool frameCanGet(int size) {
    Assert(mode == BYTE_MODE_UNMARSHALING);
    if (posMB>=getptr)
      return (framesize-(posMB-getptr))>=size;
    else
      return (framesize-(endMB-getptr)+(posMB-buf) +1)>=size;
  }
  int getInt();
  BYTE getNext();
  void getCommit();
  void getEnd();

// For receiving
  int getReadParameters(BYTE *&buf);
  void hasRead(int sizeRead);

// Glue and fixes during development.
// To be REMOVED!
  void marshalBegin();
  void marshalEnd();
  void unmarshalBegin();
  void unmarshalEnd();
  DSite* getSite();
  DSite *fixsite;
// End of development fixes
//
};


class ByteBufferManager: public FreeListManager {
public:
  int wc;

  ByteBufferManager();
  ~ByteBufferManager();
  ByteBuffer * getByteBuffer(int size,BYTE *buf);
  BYTE *deleteByteBuffer(ByteBuffer* bb);

  int getCTR(){ return wc;}
}; 

extern ByteBufferManager *byteBufferManager;

#endif


