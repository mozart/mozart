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

#include "byteBuffer.hh"

#define BYTE_MODE_MARSHALING 0
#define BYTE_MODE_UNMARSHALING 1
#define BYTE_MODE_NONE 2
#define BYTE_MODE_WRITING 3
#define BYTE_MODE_READING 4

static const int netIntSize=4;

//Utilities
inline void ByteBuffer::int2net(int i) { 
  for (int k=0; k<4; k++) { 
    put(i & 0xFF); 
    i = i>>8;
  }
}

inline int ByteBuffer::net2int(){
  unsigned int i[4];
  for (int k=0; k<4; k++) {
    i[k]=get();
  }
  return (int) (i[0] + (i[1]<<8) + (i[2]<<16) + (i[3]<<24));
}

inline void ByteBuffer::putSize() {
  BYTE *tmp = posMB;
  int size;
  
  if(posMB>putptr) 
    size=posMB-putptr;
  else
    size=endMB-putptr+posMB-buf + 1;

  // Find position for size 5 bytes from the beginning of this frame
  posMB = putptr+5;

  int2net(size);
  posMB=tmp;
}

// Class methods
ByteBuffer::ByteBuffer() {
}

void ByteBuffer::init(int size,BYTE *buf) {
  this->buf=buf;
  this->size=size;

  reinit();

  // Develc
  fixsite=NULL;
}

void ByteBuffer::reinit() {
  putptr = getptr = buf;
  endMB = buf+size -1;
  used = 0;
  mode = BYTE_MODE_NONE;

  //DEBUG
#ifdef DEBUG_CHECK
  // printf("ByteBuffer init %x\n",this);
  for(int i=0;i<size;i++)
    buf[i]=0xde;
#endif
}

Bool ByteBuffer::isEmpty() {
  return (used==0);
}

// For sending
int ByteBuffer::getUsed() {
  return used;
}

int ByteBuffer::getWriteParameters(BYTE *&buf) {
  Assert(mode==BYTE_MODE_NONE);
  buf = getptr;
  if (getptr<putptr)
    return putptr-getptr;
  else if (getptr>putptr || (getptr==putptr && used==size))
    return endMB-getptr +1;
  else
    return 0;
}

void ByteBuffer::clearWrite(int sizeWritten) {
  Assert(sizeWritten+getptr<=endMB+1);
  Assert(sizeWritten+getptr<=putptr ||
	 putptr<getptr ||
	 putptr==getptr&&used==size);  // Not allowed to circle on write.
#ifdef DEBUG_CHECK
  for (BYTE *p=getptr;p<getptr+sizeWritten;p++)    
    *p=0xde;
#endif

  getptr+=sizeWritten;
  if (getptr==endMB+1) getptr=buf;
  used-=sizeWritten;
  mode=BYTE_MODE_NONE;
  if (used==0) 
    reinit();
}

// For marshaler
void ByteBuffer::putBegin () {
  Assert(mode == BYTE_MODE_NONE);
  mode = BYTE_MODE_MARSHALING;
  posMB=putptr;
}     

void ByteBuffer::putNext(BYTE b) {
  Assert(mode == BYTE_MODE_MARSHALING);
  Assert(posMB >= putptr);
  Assert(used+posMB-putptr < size);
  Assert(posMB==endMB+1);
  posMB=buf;
  *posMB++=b;
}

void ByteBuffer::putInt(int i) {
  Assert(mode == BYTE_MODE_MARSHALING);
  int2net(i);
}

void ByteBuffer::putEnd() {
  Assert(mode == BYTE_MODE_MARSHALING);
  putSize();
  if(posMB>putptr) 
    used+=posMB-putptr;
  else
    used+=endMB-putptr+posMB-buf +1;
  putptr=posMB;
  mode = BYTE_MODE_NONE;
}

// For unmarshaler
void ByteBuffer::getBegin() {
  Assert(mode == BYTE_MODE_NONE);
  mode = BYTE_MODE_UNMARSHALING;
  posMB = getptr;
}

BYTE ByteBuffer::getNext() {
  Assert(posMB==endMB);
  BYTE b=*posMB;  // Must get last byte due to unsymmetry in msgbuffer.hh
  posMB=buf;
  return b;
}

int ByteBuffer::getInt() {
  Assert(mode == BYTE_MODE_UNMARSHALING);
  int tmp=net2int();
  return tmp;
}

void ByteBuffer::getCommit() {
  Assert(mode == BYTE_MODE_UNMARSHALING);
#ifdef DEBUG_CHECK
  if (posMB>=getptr)
    for (BYTE *p=getptr;p<posMB;p++)    
      *p=0xde;
  else {
    for (BYTE *p=getptr;p<endMB+1;p++)    
      *p=0xde;
    for (BYTE *p=buf;p<posMB;p++)    
      *p=0xde;
  }
#endif

  if (posMB>=getptr)
    used-= (posMB-getptr);
  else
    used -= (endMB-getptr)+(posMB-buf) +1;
  getptr=posMB;
  if (used==0) {
    reinit();
    mode=BYTE_MODE_UNMARSHALING;
  }
}

void ByteBuffer::getEnd() {
  //  Assert(mode == BYTE_MODE_UNMARSHALING);
  mode = BYTE_MODE_NONE;
}

// For receiving
int ByteBuffer::getReadParameters (BYTE *&buf) {
  Assert(mode == BYTE_MODE_NONE);
  buf = putptr;
  int ret;
  if (putptr>getptr || (putptr==getptr && used==0))
    ret= endMB+1-putptr;
  else if (putptr<getptr)
    ret= getptr-putptr;
  else 
    ret= 0;
  Assert(ret<=size-used);
  return ret;
}

void ByteBuffer::hasRead(int sizeRead) {
  Assert(sizeRead+putptr<=endMB+1);
  Assert(sizeRead<=size-used);
  Assert(sizeRead+putptr<=getptr ||
	 getptr<putptr ||
	 getptr==putptr&&used==0);
  // Better test needed to check for overflow AN

  used+=sizeRead;
  putptr+=sizeRead;
  if (putptr==endMB+1) {
    putptr=buf;
  }
  //  mode = BYTE_MODE_NONE;
}

Bool ByteBuffer::putDebug() {
  return availableSpace()+1>0; // +1 since put trailer uses this too...
};

// Glue and fixes during development.
// To be REMOVED!
void ByteBuffer::marshalBegin() {
  PD((TCP_INTERFACE,"byteBuffer: OLD marshalBegin USED"));
}

void ByteBuffer::marshalEnd() {
  PD((TCP_INTERFACE,"byteBuffer: OLD marshalEnd USED"));
}

void ByteBuffer::unmarshalBegin() {
  PD((TCP_INTERFACE,"byteBuffer: OLD unmarshalBegin USED"));
  getBegin ();
}

void ByteBuffer::unmarshalEnd() {
  PD((TCP_INTERFACE,"byteBuffer: OLD unmarshalEnd USED"));
  getCommit ();
  getEnd ();
}

// kost@:  still needed for marshaling tertiaries and variables?
DSite* ByteBuffer::getSite() {
  PD((TCP_INTERFACE,"byteBuffer: OLD getSite USED"));
  return fixsite;
}

// End of development fixes
//

ByteBufferManager::ByteBufferManager():
  FreeListManager(BYTE_ByteBuffer_CUTOFF){
  wc = 0;
}

ByteBuffer * ByteBufferManager::getByteBuffer(int size, BYTE *buf){
  FreeListEntry *f=getOne();
  ByteBuffer *bb;
  if(f==NULL) {
    bb=new ByteBuffer();
  }
  else {
    GenCast(f,FreeListEntry*,bb,ByteBuffer*);
  }
  bb->init(size,buf);
  ++wc;
  return bb;
}
  
BYTE *ByteBufferManager::deleteByteBuffer(ByteBuffer* bb){
  BYTE *ret=bb->buf;
  FreeListEntry *f;
  --wc;
  GenCast(bb,ByteBuffer*,f,FreeListEntry*);
  if(putOne(f)) return ret;
  delete bb;
  return ret;
}


