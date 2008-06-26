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

// For marshaler
void ByteBuffer::marshalBegin()
{
  printf("So?\n");
}     

void ByteBuffer::putNext(BYTE b)
{
  OZ_error("Should not happened, putNext");
}

void ByteBuffer::marshalEnd()
{
  printf("Ah, marshalEnd\n");
}

// For unmarshaler
void ByteBuffer::unmarshalBegin()
{
  printf("Ah, unmarshalBegin\n");
}

Bool ByteBuffer::getDebug()
{
  return TRUE;
}

BYTE ByteBuffer::getNext()
{
  OZ_error("Should not happened");
  return (BYTE)0;
}

void ByteBuffer::unmarshalEnd()
{
  printf("I see, unmarshalEnd\n");
}

Bool ByteBuffer::putDebug()
{
  return availableSpace()+1>0; // +1 since put trailer uses this too...
};

int ByteBuffer::bufferUsed()
{
  return posMB - buf; 
}


