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
  virtual void unmarshal_RR(MarshalerBuffer *buf, int *error)=0;
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

public:
  int oti;
  GCalgorithm *algs;
  //
  Bool isPersistent();
  void makePersistent();
  // old hasFullCredit
  Bool canBeReclaimed();

  void setUp(int indx,int algs);

  Bool mergeReference(RRinstance *r);
  RRinstance *getBigReference();
  RRinstance *getSmallReference();

  void removeReference();

  Bool removeAlgorithm(OZ_Term);
  OZ_Term extract_info();
};


class RemoteReference{
  friend class BorrowEntry;
  friend class BorrowTable;
public:
  RemoteReference(){}

  NetAddress netaddr;
  GCalgorithm *algs;

  Bool isPersistent();

  Bool canBeReclaimed();

  void setUp(RRinstance *r,DSite* s,int i);

  void dropReference();
  void copyReference(RemoteReference *from);
  void mergeReference(RRinstance *r);

  RRinstance *getBigReference();
  RRinstance *getSmallReference();
  NetAddress* getNetAddress();
  OZ_Term extract_info();
};



void marshalCredit(MarshalerBuffer*,RRinstance*);
void marshalCreditToOwner(MarshalerBuffer *buf,RRinstance *,int oti);

RRinstance  *unmarshalCreditRobust(MarshalerBuffer *buf,int *error);
RRinstance  *unmarshalCreditToOwnerRobust(MarshalerBuffer *buf,
                                          MarshalTag mt, int &oti,
                                          int *error);



RRinstance *CreateRRinstance(int type, int val1, int val2);
void sendReferenceBack(DSite *entitysite,int entityOTI,int type, int val1, int val2);
void askForCredit(DSite *entitysite, int entityOTI);
void sendRRinstanceBack(DSite *entitysite,int entityOTI, RRinstance *r);

#endif // __REFERENCE_CONCISTENCY_HH
