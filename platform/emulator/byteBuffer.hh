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

/*

  A guide to the pointers of this circular buffer:

   Static            Dynamic

          ----------
      buf |        |
	  |////////| getptr
	  |////////|
	  |////////| posMB
	  |////////|
	  |        | putptr
	  |        |
    endMB |        |
          ----------

    /// = filled area
    view each line as a byte and the pointers as pointing to the 
    byte of their line

    buf:      the beginning of the buffer
    endMB:    the end (last byte) of the buffer
    getptr:   the beginning of the filled area, 
              except in the case of an empty buffer: putptr==getptr
    posMB:    the next byte to be read (unmarshaling) or written (marshaling)
    putptr:   the beginning of the empty area,
              except in the case of a full buffer:   putptr==getptr

    getptr and putptr are static while marshaling or unmarshaling and
    will be adjusted together with the used field when getCommit or
    marshalEnd are called.

    posMB is moved by the get and put methods implemented in
    mbuffer.hh. For efficiency reasons it is not possible to adjust
    anything else in those methods. If posMB reaches the end of the
    buffer the corresponding putNext or getNext method will be called,
    allowing this buffer to be circular.

    Note:
    . getPtr,putPtr always point to a byte WITHIN buffer;
    . posMB can point (temporarily) to the byte immediately after
      the buffer;
    . endMB points to the last byte, thus size = (endMB+1)-buf

*/

#define BYTE_MODE_MARSHALING 0
#define BYTE_MODE_UNMARSHALING 1
#define BYTE_MODE_NONE 2
#define BYTE_MODE_WRITING 3
#define BYTE_MODE_READING 4

// Default values
#define BYTE_ByteBuffer_CUTOFF 200

class ByteBufferManager;

class ByteBuffer :public MarshalerBuffer {
  friend class ByteBufferManager;
protected:
  BYTE *buf;
  virtual Bool putDebug();
  virtual Bool getDebug();
private:
  BYTE *putptr;
  BYTE *getptr;
  int size;
  int used;
  //
  int mode;
  int framesize;		// For unmarshaling;
  DSite *site;

  inline void int2net(int i);
  inline int net2int();
  inline void putSize();

  //
public:
  ByteBuffer() {}

  void init(int size,BYTE *buf);
  void reinit();

  Bool isEmpty() { return (used == 0); }

// For sending
  int getUsed() { return (used); }
  // prepare for writing out a portion of the buffer.
  // A size/location of a sequential chunk of bytes is returned:
  int getWriteParameters(BYTE *&buf);
  // ... and report writing out that portion:
  void clearWrite(int sizeWritten);

// For marshaler
  void marshalBegin();
  inline int availableSpace() {
    Assert(mode == BYTE_MODE_MARSHALING || mode == BYTE_MODE_NONE);
    // 'used' does not contain ongoing usage;
    // always leave one byte for trailer ("-1");
    if (mode==BYTE_MODE_NONE)
      return ((size-used) - 1);
    else if (putptr<=posMB) 
      return ((size-used) - (posMB-putptr) - 1);
    else
      return ((size-used) - ((posMB-buf) + ((endMB+1)-putptr)) - 1);
  }
  void putInt(int i);
  void putNext(BYTE);
  void marshalEnd();

// For unmarshaler
  void unmarshalBegin();
  inline Bool canGet(int cgSize) {
    Assert(mode == BYTE_MODE_UNMARSHALING);
    if (posMB >= getptr)
      return ((used - (posMB-getptr)) >= cgSize);
    else
      return ((used - (((endMB+1)-getptr) + (posMB-buf))) >= cgSize);
  }

  //
  void setFrameSize(int size) { framesize = size; }
  inline Bool canGetInFrame(int cgSize) {
    Assert(mode == BYTE_MODE_UNMARSHALING);
    if (posMB >= getptr)
      return ((framesize - (posMB-getptr)) >= cgSize);
    else
      return ((framesize - (((endMB+1)-getptr) + (posMB-buf))) >= cgSize);
  }
  int getInt();
  BYTE getNext();
  // 'getCommit()' is called when a frame is successfully unmarshaled;
  // 'getCommit()' may never be called when no data has been read,
  // that case looks equivalent to the case of all data being read.
  void getCommit();
  void unmarshalEnd();

// For receiving
  // locate a char buffer for reading:
  int getReadParameters(BYTE *&buf);
  // ... and make the byteBuffer of the read operation:
  void hasRead(int sizeRead);

  // kost@:  still needed for marshaling tertiaries and variables..
  DSite* getSite() { return (site); }
  void setSite(DSite* siteIn) { site = siteIn; }

  //
  DebugCode(BYTE* getGetptr() { return (getptr); })
  DebugCode(BYTE* getPutptr() { return (putptr); })
  DebugCode(BYTE* getPosMB() { return (posMB); })
  DebugCode(BYTE* getEndMB() { return (endMB); })
};


class ByteBufferManager : public FreeListManager {
public:
  int wc;

  ByteBufferManager();
  ~ByteBufferManager();
  ByteBuffer * getByteBuffer(int size,BYTE *buf);
  BYTE *deleteByteBuffer(ByteBuffer* bb);

  //  int getCTR(){ return wc;}
}; 

extern ByteBufferManager *byteBufferManager;

#endif


