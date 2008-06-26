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
  virtual bool canRead(size_t len) const {return this->availableData()>=len;}
  virtual void readFromBuffer(BYTE* ptr, size_t wanted);
  virtual void commitRead(size_t read);

  virtual const BYTE   getByte();
};


class GlueWriteBuffer:public ByteBuffer,
                     public DssWriteBuffer{

public:
  GlueWriteBuffer(BYTE*, int);

  virtual int availableSpace() const;
  virtual bool canWrite(size_t len) const {return this->availableSpace()>=len;}
  virtual void writeToBuffer(const BYTE* ptr, size_t write);

  virtual void         putByte(const BYTE&);

};


// An object GlueMarshalerBuffer just provides the interfaces
// DssReadBuffer and DssWriteBuffer to an existing MarshalerBuffer.
// Note that the available size checks are not correct...
class GlueMarshalerBuffer : public DssReadBuffer, public DssWriteBuffer {
public:
  MarshalerBuffer* buffer;

  GlueMarshalerBuffer() : buffer(NULL) {}     // should not be used
  GlueMarshalerBuffer(MarshalerBuffer* m) : buffer(m) {}
  ~GlueMarshalerBuffer() {}

  // DssReadBuffer interface
  virtual int availableData() const { return 1000000; }     // ahem...
  virtual bool canRead(size_t) const { return true; }
  virtual void readFromBuffer(BYTE*, size_t);
  virtual void commitRead(size_t) {}
  virtual const BYTE getByte() { return buffer->get(); }

  // DssWriteBuffer interface
  virtual int availableSpace() const { return 1000000; }     // ahem...
  virtual bool canWrite(size_t) const { return true; }
  virtual void writeToBuffer(const BYTE* ptr, size_t write);
  virtual void putByte(const BYTE& b) { buffer->put(b); }
};

#endif
