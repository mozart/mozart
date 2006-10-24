/*
 *  Authors:
 *    Some ANONYMOUS folks - introduction;
 *      kost@ : lazy enough to trace all the contributors.
 *              I suspect the anonymity was deliberate.. the coding style
 *              was IMHO beyond obscurity, on the edge of obfuscation ;-[
 *              There is no way I could not put my signature under it.
 *    Konstantin Popov <kost@sics.se> - re-implementation
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Konstantin Popov 2001
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

#ifndef __COMPONENTBUFFER_HH
#define __COMPONENTBUFFER_HH

#if defined(INTERFACE)
#pragma interface
#endif

#include "base.hh"
#include "pickleBase.hh"

//
// 'PickleBuffer's are what pickling proceeds into. It implements two
// interfaces:
// . 'PickleMarshalerBuffer'
// . 'PickleMemoryBuffer'
// The first supports marshaling, the second supports reading/writing
// to/from the buffer. Exported for components.cc and friends.
//
// 'CByteBuffer's are the memory chunks 'PickleBuffer's keep their
// data in. Internal (for 'PickleBuffer').

//
#if defined(DEBUG_CHECK)
enum PickleMemoryBufferState {
  PMB_None,			// 
  PMB_ReadingChunk,		// a chunk is filled up;
  PMB_WritingChunk,		// a chunk is written out;
  PMB_AccessingChunk,		// a chunk is processed;
  PMB_Invalid			// (destroyed);
};
#endif

//
class PickleMemoryBuffer {
protected:
  // loading;
  void loadBegin();
  void loadEnd();
  // get first/next chunk of (contiguous) memory for reading in;
  BYTE *allocateFirst(int &size);
  BYTE *allocateNext(int &size);
  // once reading is done, call this:
  void chunkRead(int sizeReadIn);

  // writing out;
  void saveBegin();
  void saveEnd();
  // 'unlinkFirst()'/'unlinkNext()' provides for sequential emptying
  // of the buffer:
  BYTE *unlinkFirst(int &size);
  BYTE *unlinkNext(int &size);
  // once chunk is done, call this:
  void chunkWritten();
  // 'accessFirst()'/'accessNext()' are similar, but do not delete
  // chunks (e.g. for computing the checksums);
  BYTE *accessFirst(int &size);
  BYTE *accessNext(int &size);
  // once accessing is done, call this:
  void chunkDone();
  // once loading is finished (or we've decided to drop an output
  // buffer), use this:
  void dropBuffers();
};

//
#if defined(DEBUG_CHECK)
enum PickleBufferState {
  PB_None,			// 
  PB_Marshal,			// marshaling is in progress;
  PB_Save,			// writing out is in progress;
  PB_Load,			// loading into memory is in progress;
  PB_Unmarshal,			// unmarshaling is in progress;
  PB_Invalid			// (destroyed);
};
#endif

//
class CByteBuffer : public CppObjMemory {
protected:
  BYTE buf[PICKLEBUFFER_SIZE];
  CByteBuffer *next;

  //
public: 
  CByteBuffer() : next((CByteBuffer *) 0) {}
  ~CByteBuffer() { DebugCode(next = (CByteBuffer *) -1;); }
  
  BYTE* head() { return (buf); }
  BYTE* tail() { return (buf + PICKLEBUFFER_SIZE - 1); }
  int size() { return (PICKLEBUFFER_SIZE); }
  //
  void setNext(CByteBuffer *nextIn) { next = nextIn; }
  CByteBuffer *getNext() { return (next); }
};

//
class PickleBuffer : public PickleMarshalerBuffer,
		     public PickleMemoryBuffer,
		     public CppObjMemory {
private:
  DebugCode(PickleBufferState pbState;);
  DebugCode(PickleMemoryBufferState pmbState;);
  CByteBuffer *first; 
  CByteBuffer *last;
  // 'lastChunkSize' is maintained during both loading and saving:
  int lastChunkSize;
  // 'current' supports 'unlinkFirst()'/'unlinkNext()', as well as
  // unmarshaler's 'getNext()':
  CByteBuffer *current;

  //
public:
  PickleBuffer();
  virtual ~PickleBuffer();

  // from PickleMemoryBuffer;
  void loadBegin();
  void loadEnd();
  BYTE *allocateFirst(int &size);
  BYTE *allocateNext(int &size);
  void chunkRead(int sizeReadIn);
  void saveBegin();
  void saveEnd();
  BYTE *unlinkFirst(int &size);
  BYTE *unlinkNext(int &size);
  void chunkWritten();
  BYTE *accessFirst(int &size);
  BYTE *accessNext(int &size);
  void chunkDone();
  void dropBuffers();

  // from PickleMarshalerBuffer;
  virtual void marshalBegin();
  virtual void marshalEnd();
  virtual void unmarshalBegin();
  virtual void unmarshalEnd();
  virtual BYTE getNext();
  virtual void putNext(BYTE);
};

#endif // __COMPONENTBUFFER_HH
