/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
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

#ifndef __MSGBUFFER_HH
#define __MSGBUFFER_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "base.hh"
#include "am.hh"
#include "tagged.hh"
#include "dpInterface.hh"

#define MSGFLAG_TEXTMODE  0x1
#define MSGFLAG_ATEND     0x2

class MsgBuffer {
private:
  OZ_Term nogoods;
  int flags;

  //
protected:
  BYTE* posMB;
  BYTE* endMB;

  virtual BYTE getNext()=0;
  virtual void putNext(BYTE)=0;

  //
public: 
  virtual void marshalBegin() = 0;	
  virtual void marshalEnd()=0;
  virtual void unmarshalBegin()=0;
  virtual void unmarshalEnd()=0;

  virtual int getMinor() { return PERDIOMINOR; }
  virtual void getVersion(int *major, int *minor) { 
    *major = PERDIOMAJOR; *minor = PERDIOMINOR; } 

  virtual void init() { 
    nogoods   = oz_nil();
    flags     = 0;
  }

  void setTextmode() { flags |= MSGFLAG_TEXTMODE; }
  Bool textmode()    { return (flags&MSGFLAG_TEXTMODE); }

  void markEnd()  { flags |= MSGFLAG_ATEND; }
  Bool atEnd()    { return (flags&MSGFLAG_ATEND); }

  // NON-virtual!
  BYTE get(){
    if(posMB==endMB){
      return getNext();}
    DebugCode((*maybeDebugBufferGet)(*posMB););
    return *posMB++;}

  void put(BYTE b){
    if (posMB==0) return;
    if(posMB>endMB){
      putNext(b);
      return;}
    *posMB++=b;
    DebugCode((*maybeDebugBufferPut)(b););
  }
      
  //
  virtual char* siteStringrep()=0;
  virtual DSite* getSite()=0;                    // overrided for network/vsite comm
  virtual Bool isPersistentBuffer()=0;
  virtual Bool globalize() { return OK; }
  virtual void unmarshalReset()                 {} // only for network receovery

  virtual Bool visit(OZ_Term t) { return OK; }
  void addNogood(OZ_Term t) { nogoods = oz_cons(t,nogoods); }
  OZ_Term getNoGoods()      { return nogoods; }
};

// 
// kost@  The whole thing should be in 'components.hh' if there would
// be one;
MsgBuffer* getComponentMsgBuffer();
void freeComponentMsgBuffer(MsgBuffer *buf);

/* RS: have to GC the byte stream again !!!!!!!!!*/
#define CheckNogoods(val,bs,id,msg,Cleanup)				\
  { OZ_Term nogoods = bs->getNoGoods();					\
    if (!oz_isNil(nogoods)) {						\
       Cleanup;								\
       return raiseGeneric(id,						\
			   msg,						\
			   oz_mklist(OZ_pairA("Resources",nogoods),	\
				     OZ_pairA("Contained in",val)));	\
    }									\
  }

#endif // __MSGBUFFER_HH
