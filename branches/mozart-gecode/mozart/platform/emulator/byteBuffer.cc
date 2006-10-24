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

#if defined(INTERFACE)
#pragma implementation "memaux.hh"
#endif

#include "byteBuffer.hh"

// Utilities
inline void ByteBuffer::int2net(int i)
{
  for (int k=0; k<4; k++) { 
    put(i & 0xFF); 
    i = i>>8;
  }
}

inline int ByteBuffer::net2int()
{
  unsigned int i[4];
  for (int k=0; k<4; k++) {
    i[k]=get();
  }
  return (int) (i[0] + (i[1]<<8) + (i[2]<<16) + (i[3]<<24));
}

inline void ByteBuffer::putSize()
{
  BYTE *tmp = posMB;
  int size;

  Assert(posMB <= endMB);
  Assert(putptr <= endMB);
  if (posMB > putptr) 
    size = posMB-putptr;
  else
    size = ((endMB+1)-putptr) + (posMB-buf);
  // Find position for size 5 bytes from the beginning of this frame
  posMB = putptr+5;
  if (posMB>endMB)
    posMB = buf + (posMB-(endMB+1));
  int2net(size);
  posMB = tmp;
}

void ByteBuffer::init(int size,BYTE *buf)
{
  this->buf=buf;
  this->size=size;

  reinit();

  site = NULL;
}

void ByteBuffer::reinit()
{
  putptr = getptr = buf;
  endMB = buf+size-1;
  used = 0;
  mode = BYTE_MODE_NONE;

#if defined(DEBUG_CHECK)
  // printf("ByteBuffer init %x\n",this);
  for (int i=0; i<size; i++)
    buf[i] = 0xde;
#endif
}

// For sending
int ByteBuffer::getWriteParameters(BYTE *&buf)
{
  Assert(mode==BYTE_MODE_NONE);
  buf = getptr;
  if (getptr < putptr)
    return (putptr-getptr);
  else if (getptr > putptr || (getptr == putptr && used == size))
    return ((endMB+1)-getptr);
  else
    return 0;
}

void ByteBuffer::clearWrite(int sizeWritten)
{
  Assert(getptr+sizeWritten <= endMB+1); // subsumes (used < size);
  Assert(getptr+sizeWritten <= putptr ||
	 putptr < getptr ||
	 // Not allowed to circle on write.
	 (putptr == getptr && used == size));
#if defined(DEBUG_CHECK)
  for (BYTE *p=getptr;p<getptr+sizeWritten;p++)    
    *p=0xde;
#endif

  //
  mode = BYTE_MODE_NONE;
  used -= sizeWritten;
  Assert(used >= 0);
  if (used == 0) {
    reinit();
  } else {
    getptr += sizeWritten;
    if (getptr == endMB+1)
      getptr = buf;
  }
}

// For marshaler
void ByteBuffer::marshalBegin()
{
  Assert(mode == BYTE_MODE_NONE);
  mode = BYTE_MODE_MARSHALING;
  posMB = putptr;
}     

void ByteBuffer::putNext(BYTE b)
{
  Assert(mode == BYTE_MODE_MARSHALING);
  Assert(posMB >= putptr);
  Assert(used+posMB-putptr <= size);
  Assert(posMB == endMB+1);
  posMB = buf;
  *posMB++ = b;
}

void ByteBuffer::putInt(int i)
{
  Assert(mode == BYTE_MODE_MARSHALING);
  int2net(i);
}

void ByteBuffer::marshalEnd()
{
  Assert(mode == BYTE_MODE_MARSHALING);
  // Are we just about to wrap around?
  Assert(posMB <= endMB+1);
  if (posMB > endMB)		// faster than posMB == endMB+1
    posMB = buf;
  putSize();
  if (posMB > putptr) 
    used += posMB-putptr;
  else
    used += ((endMB+1)-putptr) + (posMB-buf);
  Assert(used <= size);
  putptr = posMB;
  mode = BYTE_MODE_NONE;
}

// For unmarshaler
void ByteBuffer::unmarshalBegin()
{
  Assert(mode == BYTE_MODE_NONE);
  mode = BYTE_MODE_UNMARSHALING;
  posMB = getptr;
}

Bool ByteBuffer::getDebug()
{
  return canGet(1);
}

BYTE ByteBuffer::getNext()
{
  Assert(posMB == endMB+1);
  posMB = buf;			// wrap over (not a new chunk);
  return (*posMB++);
}

int ByteBuffer::getInt()
{
  Assert(mode == BYTE_MODE_UNMARSHALING);
  int tmp=net2int();
  return tmp;
}

// getCommit may never be called when no data has been read, that
// case looks equivalent to the case of all data being read.
void ByteBuffer::getCommit()
{
  Assert(mode == BYTE_MODE_UNMARSHALING);
#if defined(DEBUG_CHECK)
  if (posMB > getptr)
    for (BYTE *p=getptr;p<posMB;p++)    
      *p=0xde;
  else { // Fills all of the buffer if getptr==posMB
    for (BYTE *p=getptr;p<endMB+1;p++)    
      *p=0xde;
    for (BYTE *p=buf;p<posMB;p++)    
      *p=0xde;
  }
#endif

  if (posMB == getptr) // Assume all read
    used = 0;
  else if (posMB >= getptr)
    used -= (posMB-getptr);
  else
    used -= (((endMB+1)-getptr) + (posMB-buf));
  Assert(used >= 0);
  if (used==0) {
    reinit();
    mode = BYTE_MODE_UNMARSHALING;
  } else {
    getptr = posMB;
    if (getptr == endMB+1)
      getptr = buf;
    Assert(getptr < endMB+1);
  }
}

void ByteBuffer::unmarshalEnd()
{
  //  Assert(mode == BYTE_MODE_UNMARSHALING);
  mode = BYTE_MODE_NONE;
}

// For receiving
int ByteBuffer::getReadParameters(BYTE *&buf)
{
  Assert(mode == BYTE_MODE_NONE);
  buf = putptr;
  int ret;
  if (putptr > getptr || (putptr == getptr && used == 0))
    ret = (endMB+1)-putptr;
  else if (putptr<getptr)
    ret = getptr-putptr;
  else 
    ret = 0;
  Assert(used+ret <= size);
  return ret;
}

void ByteBuffer::hasRead(int sizeRead)
{
  Assert(putptr+sizeRead <= endMB+1);
  Assert(used+sizeRead <= size);
  Assert(putptr+sizeRead <= getptr ||
	 getptr < putptr ||
	 (getptr == putptr && used == 0));
  // Better test needed to check for overflow AN

  used += sizeRead;
  Assert(used <= size);
  putptr += sizeRead;
  if (putptr == endMB+1)
    putptr=buf;
  Assert(putptr < endMB+1);
  //  mode = BYTE_MODE_NONE;
}

Bool ByteBuffer::putDebug()
{
  return availableSpace()+1>0; // +1 since put trailer uses this too...
};

ByteBufferManager::ByteBufferManager()
  : FreeListManager(BYTE_ByteBuffer_CUTOFF)
{
  wc = 0;
}

ByteBufferManager::~ByteBufferManager(){
  ByteBuffer *byteBuffer;
  FreeListEntry *f;
  int l=length();
  for(int i=0;i<l;i++) {
    f=getOne();
    Assert(f!=NULL);
    byteBuffer = new (f) ByteBuffer;
    delete byteBuffer;
  }
  Assert(length()==0);
}

ByteBuffer * ByteBufferManager::getByteBuffer(int size, BYTE *buf){
  FreeListEntry *f=getOne();
  ByteBuffer *bb;
  if(f==NULL) {
    bb=new ByteBuffer();
  }
  else {
    bb = new (f) ByteBuffer;
  }
  bb->init(size,buf);
  ++wc;
  return bb;
}
  
BYTE *ByteBufferManager::deleteByteBuffer(ByteBuffer* bb){
  BYTE *ret=bb->buf;
  FreeListEntry *f;
  --wc;
  f=(FreeListEntry*)(void*)bb;
  if(putOne(f)) return ret;
  delete bb;
  return ret;
}


