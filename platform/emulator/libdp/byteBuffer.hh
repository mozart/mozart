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

#include "mbuffer.hh"
#include "dsite.hh"

// Default values
#define BYTE_ByteBuffer_CUTOFF 200

class ByteBufferManager;

class ByteBuffer :public MarshalerBuffer {
  friend ByteBufferManager;
protected:
  BYTE *buf;
  virtual Bool putDebug();
private:
  BYTE *putptr;
  BYTE *getptr;

  int size;
  int used;
  int mode;

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
  int availableSpace();
  void putInt(int i);
  void putNext(BYTE);
  void putEnd();

// For unmarshaler
  void getBegin();
  Bool canGet(int size);
  int getInt();
  BYTE getNext();
  void getCommit();
  void getEnd();

// For receiving
  int getReadParameters(BYTE *&buf);
  void hasRead(int sizeRead);

// Glue and fixes during development.
// To be REMOVED!
  OZ_Term getNoGoods();
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
  ByteBuffer * getByteBuffer(int size,BYTE *buf);
  BYTE *deleteByteBuffer(ByteBuffer* bb);

  int getCTR(){ return wc;}
};

extern ByteBufferManager *byteBufferManager;

#endif
