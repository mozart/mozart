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


#ifndef __TIME_LEASE_HH
#define __TIME_LEASE_HH



class RRinstance_TL:public RRinstance{
public:
  int seconds;
  
  RRinstance_TL(RRinstance *n);
  RRinstance_TL(int t, RRinstance *n);
  
  void marshal_RR(MarshalerBuffer *buf);
  void unmarshal_RR(MarshalerBuffer *buf);
  virtual ~RRinstance_TL(){}
};

class TL:public GCalgorithm
{
public:
  Bool owner;
  LongTime expireDate;
  TimerElement *timer;
    
  TL(HomeReference *p, GCalgorithm *g);
  TL(RemoteReference *p,RRinstance *r,GCalgorithm *g);
  void updateTimerExpired();
  void leaseTimerExpired();
  RRinstance *getBigReference(RRinstance *r);
  RRinstance *getSmallReference(RRinstance *r);
  void dropReference(DSite *s ,int i);
  Bool mergeReference(RRinstance *r);
  Bool isGarbage();
  Bool isRoot();
  OZ_Term extract_info(OZ_Term in);
  OZ_Term extract_OzId();
  void remove();
  virtual ~TL(){}
};

#endif // __TIME_LEASE_HH















