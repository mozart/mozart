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
#include "genhashtbl.hh"
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

    ///=filled area
    view each line as a byte and the pointers as pointing to the
    byte of their line

    buf:      the beginning of the buffer
    endMB:    the end (last byte) of the buffer
    getptr:   the beginning of the filled area
    posMB:    the next byte to be read (unmarshaling) or written (marshaling)
    putptr:   the beginning of the empty area

    getptr and putptr are static while marshaling or unmarshaling and will
    be adjusted together with the used field when getCommit or putEnd are
    called.

    posMB is moved by the get and put methods implemented in
    mbuffer.hh. For efficiency reasons it is not possible to adjust
    anything else in those methods. If posMB reaches the end of the
    buffer the corresponding putNext or getNext method will be called,
    allowing this buffer to be circular.

    Note that the condition on when posMB reaches the end is
    assymetric in mbuffer.hh (posMB==endMB vs posMB>endMB) and that
    endMB points to the last byte making the size=endMB+1-buf.

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
      return (used-((endMB-getptr)+(posMB-buf))+1)>=size;
  }
  void setFrameSize(int size);
  inline Bool frameCanGet(int size) {
    Assert(mode == BYTE_MODE_UNMARSHALING);
    if (posMB>=getptr)
      return (framesize-(posMB-getptr))>=size;
    else
      return (framesize-((endMB-getptr)+(posMB-buf)) +1)>=size;
  }
  int getInt();
  BYTE getNext();
  // getCommit may never be called when no data has been read, that
  // case looks equivalent to the case of all data being read.
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

  //
  DebugCode(BYTE* getGetptr() { return (getptr); })
  DebugCode(BYTE* getPutptr() { return (putptr); })
  DebugCode(BYTE* getPosMB() { return (posMB); })
  DebugCode(BYTE* getEndMB() { return (endMB); })
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
