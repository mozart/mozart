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

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "componentBuffer.hh"
#endif

#include "componentBuffer.hh"

PickleBuffer::PickleBuffer()
{
  DebugCode(pbState = PB_None;);
  DebugCode(pmbState = PMB_None;);
  DebugCode(lastChunkSize = -1;);
  DebugCode(posMB = endMB = (BYTE *) -1;);
  DebugCode(current = (CByteBuffer *) -1;);
  first = last = (CByteBuffer *) 0;
}

PickleBuffer::~PickleBuffer()
{
  Assert(first == (CByteBuffer *) 0);
  DebugCode(pbState = PB_Invalid;);
  DebugCode(pmbState = PMB_Invalid;);
  DebugCode(lastChunkSize = -1;);
  DebugCode(first = last = current = (CByteBuffer *) -1;);
  DebugCode(posMB = endMB = (BYTE *) -1;);
}

void PickleBuffer::loadBegin()
{
  Assert(pbState == PB_None);
  DebugCode(pbState = PB_Load;);
  Assert(first == (CByteBuffer *) 0);
  Assert(last == (CByteBuffer *) 0);
  Assert(lastChunkSize == -1);
}

void PickleBuffer::loadEnd()
{
  Assert(pbState == PB_Load);
  DebugCode(pbState = PB_None;);
}

BYTE* PickleBuffer::allocateFirst(int &size)
{
  Assert(pbState == PB_Load);
  Assert(pmbState == PMB_None);
  DebugCode(pmbState = PMB_ReadingChunk;);
  Assert(first == (CByteBuffer *) 0);
  Assert(last == (CByteBuffer *) 0);
  //
  first = last = new CByteBuffer;
  size = first->size();
  return (first->head());
}

BYTE* PickleBuffer::allocateNext(int &size)
{
  Assert(pbState == PB_Load);
  Assert(pmbState == PMB_None);
  DebugCode(pmbState = PMB_ReadingChunk;);
  Assert(first != (CByteBuffer *) 0);
  Assert(last != (CByteBuffer *) 0);
  Assert(last->getNext() == (CByteBuffer *) 0);
  //
  CByteBuffer *nb = new CByteBuffer;
  last->setNext(nb);
  last = nb;
  size = nb->size();
  return (nb->head());
}

void PickleBuffer::chunkRead(int sizeReadIn)
{
  Assert(pbState == PB_Load);
  Assert(pmbState == PMB_ReadingChunk);
  DebugCode(pmbState = PMB_None;);
  // always for the last recorded chunk:
  lastChunkSize = sizeReadIn;
}

void PickleBuffer::saveBegin()
{
  Assert(pbState == PB_None);
  DebugCode(pbState = PB_Save;);
}

void PickleBuffer::saveEnd()
{
  Assert(pbState == PB_Save);
  DebugCode(pbState = PB_None;);
  Assert(first == (CByteBuffer *) 0);
  DebugCode(lastChunkSize = -1;);
  first = last = (CByteBuffer *) 0;
}

BYTE* PickleBuffer::unlinkFirst(int &size)
{
  Assert(pbState == PB_Save);
  Assert(pmbState == PMB_None);
  DebugCode(pmbState = PMB_WritingChunk;);
  Assert(first != (CByteBuffer *) -1);
  current = first;
  if (current == last) 
    size = lastChunkSize;
  else
    size = current->size();
  return (current->head());
}

BYTE* PickleBuffer::unlinkNext(int &size)
{
  Assert(pbState == PB_Save);
  Assert(pmbState == PMB_None);
  Assert(current != (CByteBuffer *) -1);
  if (current) {
    DebugCode(pmbState = PMB_WritingChunk;);
    if (current == last) 
      size = lastChunkSize;
    else
      size = current->size();
    return (current->head());
  } else {
    DebugCode(current = (CByteBuffer *) -1;);
    DebugCode(size = -1;);
    return ((BYTE *) 0);
  }
  Assert(0);
}

void PickleBuffer::chunkWritten()
{
  Assert(pbState == PB_Save);
  Assert(pmbState == PMB_WritingChunk);
  DebugCode(pmbState = PMB_None;);
  Assert(current != (CByteBuffer *) -1);
  Assert(current != (CByteBuffer *) 0);
  CByteBuffer *nb = current->getNext();
  delete current;
  current = first = nb;
}

BYTE* PickleBuffer::accessFirst(int &size)
{
  Assert(pbState == PB_Save);
  Assert(pmbState == PMB_None);
  DebugCode(pmbState = PMB_AccessingChunk;);
  Assert(first != (CByteBuffer *) -1);
  current = first;
  if (current == last) 
    size = lastChunkSize;
  else
    size = current->size();
  return (current->head());
}

BYTE* PickleBuffer::accessNext(int &size)
{
  Assert(pbState == PB_Save);
  Assert(pmbState == PMB_None);
  Assert(current != (CByteBuffer *) -1);
  if (current) {
    DebugCode(pmbState = PMB_AccessingChunk;);
    if (current == last) 
      size = lastChunkSize;
    else
      size = current->size();
    return (current->head());
  } else {
    DebugCode(current = (CByteBuffer *) -1;);
    DebugCode(size = -1);
    return ((BYTE *) 0);
  }
  Assert(0);
}

void PickleBuffer::chunkDone()
{
  Assert(pbState == PB_Save);
  Assert(pmbState == PMB_AccessingChunk);
  DebugCode(pmbState = PMB_None;);
  Assert(current != (CByteBuffer *) -1);
  Assert(current != (CByteBuffer *) 0);
  current = current->getNext();
}

void PickleBuffer::dropBuffers()
{
  Assert(first != (CByteBuffer *) -1);
  while (first) {
    CByteBuffer *nb = first->getNext();
    delete first;
    first = nb;
  }
  Assert(first == (CByteBuffer *) 0);
}

void PickleBuffer::marshalBegin()
{
  Assert(pbState == PB_None);
  DebugCode(pbState = PB_Marshal;);
  Assert(posMB == (BYTE *) -1);
  Assert(endMB == (BYTE *) -1);
  Assert(first == (CByteBuffer *) 0);
  Assert(last == (CByteBuffer *) 0);
  //
  first = last = new CByteBuffer;
  posMB = first->head();
  endMB = first->tail();
}

void PickleBuffer::marshalEnd()
{
  Assert(pbState == PB_Marshal);
  DebugCode(pbState = PB_None;);
  Assert(lastChunkSize == -1);
  //
  lastChunkSize = posMB - last->head();
  DebugCode(posMB = (BYTE *) -1;);
  DebugCode(endMB = (BYTE *) -1;);
}

void PickleBuffer::unmarshalBegin()
{
  Assert(pbState == PB_None);
  DebugCode(pbState = PB_Unmarshal;);
  Assert(posMB == (BYTE *) -1);
  Assert(endMB == (BYTE *) -1);
  Assert(first != (CByteBuffer *) 0);
  Assert(last != (CByteBuffer *) 0);
  Assert(first != (CByteBuffer *) -1);
  Assert(last != (CByteBuffer *) -1);
  //
  current = first;
  posMB = current->head();
  // In debug mode, insist that the last chunk contains exactly as
  // many bytes as have been read:
#if defined(DEBUG_CHECK)
  if (current == last) 
    endMB = posMB + lastChunkSize;
  else
    endMB = current->tail();
#else
  endMB = current->tail();
#endif
}

void PickleBuffer::unmarshalEnd()
{
  Assert(pbState == PB_Unmarshal);
  DebugCode(pbState = PB_None;);
  Assert(posMB == last->head() + lastChunkSize);
  DebugCode(posMB = (BYTE *) -1;);
  DebugCode(endMB = (BYTE *) -1;);
}

BYTE PickleBuffer::getNext()
{
  Assert(pbState == PB_Unmarshal);
  Assert(current != (CByteBuffer *) -1);
  Assert(current != (CByteBuffer *) 0);
  //
  current = current->getNext();
#if defined(DEBUG_CHECK)
  Assert(current);
#else
  // In real life, allocate another chunk, init it, and pretend it's
  // fine.  (a) shouldn't happen because there a crc check, and (b)
  // unmarshaler should recognize that the data is bogus and bail out.
  if (current == (CByteBuffer *) 0) {
    current = new CByteBuffer;
    last->setNext(current);
    last = current;
    BYTE *ptr = current->head();
    BYTE *end = current->tail();
    while (ptr <= end) 
      *ptr++ = (BYTE) 0;
  }
#endif

  posMB = current->head();
  // In debug mode, insist that the last chunk contains exactly as
  // many bytes as have been read:
#if defined(DEBUG_CHECK)
  if (current == last) 
    endMB = posMB + lastChunkSize;
  else
    endMB = current->tail();
#else
  endMB = current->tail();
#endif
  return (*posMB++);
}

void PickleBuffer::putNext(BYTE b)
{
  Assert(pbState == PB_Marshal);
  Assert(lastChunkSize == -1);
  //
  CByteBuffer *nb = new CByteBuffer;
  last->setNext(nb);
  last = nb;
  posMB = nb->head();
  endMB = nb->tail();
  Assert(nb->size() > 0);
  *posMB++ = b;
}
