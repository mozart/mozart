/* -*- C++ -*-
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Organization or Person (Year(s))
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __MARSHALER_HH
#define __MARSHALER_HH

#include "base.hh"
#include "hashtbl.hh"
#include "msgbuffer.hh"

//  provided by marshaler to protocol-layer.

OZ_Return export(OZ_Term t);

void marshalTerm(OZ_Term,MsgBuffer*);
void marshalTermRT(OZ_Term t, MsgBuffer *bs);
void marshalSRecord(SRecord *sr, MsgBuffer *bs);
void marshalClass(ObjectClass *cl, MsgBuffer *bs);
void marshalNumber(unsigned int,MsgBuffer*);
void marshalFloat(double d, MsgBuffer *bs);
void marshalShort(unsigned short,MsgBuffer*);
void marshalString(const char *s, MsgBuffer *bs);
void marshalDIF(MsgBuffer *bs, MarshalTag tag) ;
void marshalGName(GName*, MsgBuffer*);

OZ_Term unmarshalTerm(MsgBuffer*);
OZ_Term unmarshalTermRT(MsgBuffer *bs);
int unmarshalNumber(MsgBuffer*);
double unmarshalFloat(MsgBuffer *bs);
char *unmarshalString(MsgBuffer *);
char *unmarshalVersionString(MsgBuffer *);
unsigned short unmarshalShort(MsgBuffer*);
GName* unmarshalGName(TaggedRef*,MsgBuffer*);
SRecord* unmarshalSRecord(MsgBuffer*);

GName *globalizeConst(ConstTerm *t, MsgBuffer *bs);

extern RefTable *refTable;
extern RefTrail *refTrail;

void initMarshaler();

// the names of the difs for statistics

enum {
  MISC_STRING,
  MISC_GNAME,
  MISC_SITE,

  MISC_LAST
};

class SendRecvCounter {
private:
  long c[2];
public:
  SendRecvCounter() { c[0]=0; c[1]=0; }
  void send() { c[0]++; }
  long getSend() { return c[0]; }
  void recv() { c[1]++; }
  long getRecv() { return c[1]; }
};

extern SendRecvCounter dif_counter[];
extern SendRecvCounter misc_counter[];
extern char *misc_names[];

/* *********************************************************************/
/*   classes RefTable RefTrail                              */
/* *********************************************************************/

class RefTable {
  OZ_Term *array;
  int size;
  int nextFree; // only for backwards compatibility
public:
  void reset() { nextFree=0; }
  RefTable()
  {
    reset();
    size     = 100;
    array    = new OZ_Term[size];
  }
  OZ_Term get(int i)
  {
    return (i>=size) ? makeTaggedNULL() : array[i];
  }
  void set(OZ_Term val, int pos)
  {
    if (pos == -1) {
      pos = nextFree++;
    }
    if (pos>=size)
      resize(pos);
    array[pos] = val;
  }
  void resize(int newsize)
  {
    int oldsize = size;
    OZ_Term  *oldarray = array;
    while(size <= newsize) {
      size = (size*3)/2;
    }
    array = new OZ_Term[size];
    for (int i=0; i<oldsize; i++) {
      array[i] = oldarray[i];
    }
    delete oldarray;
  }
};

inline int unmarshalRefTag(MsgBuffer *bs)
{
  return unmarshalNumber(bs);
}

inline void gotRef(TaggedRef val, int index)
{
  refTable->set(val,index);
}



class RefTrail: public HashTable {
  int rtcounter;
public:

  RefTrail(): HashTable(HT_INTKEY,2000), rtcounter(0) {}

  int trail(void *l)
  {
    Assert(find(l)==-1);
    htAdd((intlong)l,ToPointer(rtcounter++));
    return rtcounter-1;
  }

  int find(void *l)
  {
    void *ret = htFind((intlong)l);
    return (ret==htEmpty) ? -1 : (int)ToInt32(ret);
  }

  void unwind()
  {
    rtcounter -= getSize();
    mkEmpty();
    Assert(isEmpty());
  }
  Bool isEmpty() { return rtcounter==0; }
};


void trailCycleOutLine(void *l, MsgBuffer *bs);
Bool checkCycleOutLine(void *l, MsgBuffer *bs);


#endif // __MARSHALER_HH
