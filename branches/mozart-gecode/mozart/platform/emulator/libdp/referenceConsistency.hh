/*
 *  Authors:
 *    Erik Klintskog (erikd@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 1998
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


#ifndef __REFERENCE_CONCISTENCY_HH
#define __REFERENCE_CONCISTENCY_HH

#include "dsite.hh"
#include "table.hh"


enum GC_ALGORITMS{      
  NO_GC_ALG  = 0,
  GC_ALG_WRC = 1,
  GC_ALG_TL  = 2
};


class RemoteReference;
class HomeReference;


class RRinstance{
public:
  RRinstance *next;
  int type;
  virtual void marshal_RR(MarshalerBuffer *buf)=0;
  virtual void unmarshal_RR(MarshalerBuffer *buf)=0;
  virtual ~RRinstance(){}
};


class GCalgorithm
{
public:
  GCalgorithm *next;
  int type;
  union{
    RemoteReference *rr;
    HomeReference *hr;
  } parent;
  
  virtual RRinstance *getBigReference(RRinstance *)=0;
  virtual RRinstance *getSmallReference(RRinstance *)=0;
  virtual void dropReference(DSite *,int)=0;

  virtual Bool mergeReference(RRinstance*)=0;
  // Called by homeRefs 
  virtual Bool isGarbage()=0;
  // Called by RemoteRefs 
  virtual Bool isRoot()=0;

  virtual OZ_Term extract_info(OZ_Term)=0;
  virtual OZ_Term extract_OzId()=0;
  virtual void remove()=0;
  virtual ~GCalgorithm(){}
};





class HomeReference{
  friend class OB_Entry;
private:
  Ext_OB_TIndex extOTI;
  
public:
  GCalgorithm *algs;

  HomeReference() {
    DebugCode(extOTI = (Ext_OB_TIndex) -1;);
    DebugCode(algs = (GCalgorithm *) -1;);
  }
  ~HomeReference() {
    while (algs != NULL){
      GCalgorithm *tmp2 = algs;
      algs->remove();
      algs = algs->next;
      delete tmp2;
    }
    DebugCode(extOTI = (Ext_OB_TIndex) -1;);
    DebugCode(algs = (GCalgorithm *) -1;);
  }

  // 
  Bool isPersistent() { return (algs == NULL); }
  void makePersistent();
  // old hasFullCredit
  Bool canBeReclaimed();

  // 'setUp()' respects GC algorithm settings (ozconf.dpUseTimeLease,
  // ozconf.dpUseFracWRC);
  void setUp(Ext_OB_TIndex indx);
  
  Bool mergeReference(RRinstance *r);
  RRinstance *getBigReference();
  RRinstance *getSmallReference();
  
  Bool removeAlgorithm(OZ_Term);
  OZ_Term extract_info();

  Ext_OB_TIndex getExtOTI() { return (extOTI); }
};


class RemoteReference{
  friend class BorrowEntry;
  friend class BorrowTable;

private:
  NetAddress netaddr;

public:
  GCalgorithm *algs;

  RemoteReference() { DebugCode(algs = (GCalgorithm *) -1;); }
  void setUp(RRinstance *r, DSite* s, int i);

  Bool isPersistent() { return (algs == NULL); }
  NetAddress* getNetAddress() { return (&netaddr); }

  Bool canBeReclaimed();
  
  void dropReference() {
    NetAddress *na = getNetAddress();
    DSite* site = na->site;
    int index = na->index;
    GCalgorithm *tmp=algs;
    while(tmp!=NULL){
      GCalgorithm *tmp2=tmp;
      tmp->dropReference(site,index);
      tmp=tmp->next;
      delete tmp2;
    }
  }
  void copyReference(RemoteReference *from);
  void mergeReference(RRinstance *r);

  RRinstance *getBigReference();
  RRinstance *getSmallReference();

  OZ_Term extract_info();
};



void marshalCredit(MarshalerBuffer*, RRinstance*);
void marshalCreditToOwner(MarshalerBuffer*, RRinstance*, Ext_OB_TIndex);

RRinstance *unmarshalCredit(MarshalerBuffer*);
RRinstance *unmarshalCreditToOwner(MarshalerBuffer*, MarshalTag,
				   Ext_OB_TIndex&);

RRinstance *CreateRRinstance(int type, int val1, int val2);
void sendReferenceBack(DSite *entitysite, Ext_OB_TIndex, int type,
		       int val1, int val2);
void sendRRinstanceBack(DSite *entitysite, Ext_OB_TIndex, RRinstance*);

#endif // __REFERENCE_CONCISTENCY_HH
