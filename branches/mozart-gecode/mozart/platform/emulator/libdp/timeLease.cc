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

#include "msgContainer.hh"      
#include "msgType.hh"    
#include "table.hh"
#include "dpMarshaler.hh"
#include "timers.hh"
#include "referenceConsistency.hh"
#include "timeLease.hh"


#define LEASE_TIME ozconf.dp_tl_leaseTime
#define UPDATE_LEASE ozconf.dp_tl_updateTime


/************* TIMER WRAPPERS ***************/

Bool tl_update_timer_expired(void *v)
{
  ((TL*)v)->updateTimerExpired();
  return FALSE; 
}

Bool tl_lease_timer_expired(void *v)
{
  ((TL*)v)->leaseTimerExpired();
  return FALSE; 
}
/**************** RRinstance  ***********************/

RRinstance_TL::RRinstance_TL(RRinstance *n){
  type = GC_ALG_TL;
  next = n;
}


RRinstance_TL::RRinstance_TL(int t, RRinstance *n){
  type = GC_ALG_TL;
  next = n;
  seconds = t;
}

void RRinstance_TL::marshal_RR(MarshalerBuffer *buf){
  marshalNumber(buf,type);
  marshalNumber(buf, seconds);
}

void RRinstance_TL::unmarshal_RR(MarshalerBuffer *buf)
{
  seconds = unmarshalNumber(buf);
}

/*********************** TL *************************/


TL::TL(HomeReference *p, GCalgorithm *g)
{
  type = GC_ALG_TL;
  owner = TRUE;
  timer = NULL;
  expireDate = *(am.getEmulatorClock());
  expireDate.increaseTime(LEASE_TIME);
  next = g;
    parent.hr = p;
    timers->setTimer(timer,LEASE_TIME,
		     tl_lease_timer_expired,this);
}
  
TL::TL(RemoteReference *p,RRinstance *r,GCalgorithm *g){
  RRinstance_TL *tmp = (RRinstance_TL*)r;
  int lease_ms = tmp->seconds * 1000;
  next = g;
  owner= FALSE; 
  parent.rr = p;
  type = GC_ALG_TL;
  expireDate = *(am.getEmulatorClock());
  expireDate.increaseTime(lease_ms);
  timer = NULL;
  if (lease_ms < UPDATE_LEASE)
    {
      updateTimerExpired();
    }
  else
    {
      timers->setTimer(timer,lease_ms - UPDATE_LEASE,
		       tl_update_timer_expired,this);
    }
}
  
void TL::updateTimerExpired()
{
  Assert(owner == FALSE);
  int lt = expireDate - *(am.getEmulatorClock());
  timer = NULL; 
  if(lt > UPDATE_LEASE){
    timers->setTimer(timer,lt - UPDATE_LEASE,
		     tl_update_timer_expired,this);
  }
  else{
    MsgContainer *msgC =
      msgContainerManager->newMsgContainer(((parent.rr)->getNetAddress())->site);
    msgC->put_M_UPDATE_REFERENCE(((parent.rr)->getNetAddress())->index);
    send(msgC);
  }
}

  
void TL::leaseTimerExpired()
{
  timer = NULL;
  int lt = expireDate - *(am.getEmulatorClock());
  if (lt > 0){
    timers->setTimer(timer,lt,tl_lease_timer_expired,this);
  }
  else{
    printf("Forced localizing %d!\n",(parent.hr)->getExtOTI());
    OT->extOTI2ownerEntry((parent.hr)->getExtOTI())->localize();
  }
}

RRinstance *TL::getBigReference(RRinstance *r)
{
  if(owner){
    expireDate = *(am.getEmulatorClock());
    expireDate.increaseTime(LEASE_TIME);}
  return (new RRinstance_TL((expireDate - (*(am.getEmulatorClock()))) / 1000, r));
}
RRinstance *TL::getSmallReference(RRinstance *r)
{
  return getBigReference(r);
}
void TL::dropReference(DSite *s ,int i)
{
  timers->clearTimer(timer);
}

Bool TL::mergeReference(RRinstance *r){
  RRinstance_TL *tmp = (RRinstance_TL*)r;
  if(!owner){
    LongTime lt = *(am.getEmulatorClock());
    int lease_ms = tmp->seconds * 1000;
	lt.increaseTime(tmp->seconds * 1000);
	if (lt > expireDate){
	  expireDate = lt;
	  // If the new time received is a "valid" time 
	  // install a timer if none exists
	  if (timer == NULL &&  lease_ms > UPDATE_LEASE)
	    timers->setTimer(timer,lease_ms - UPDATE_LEASE,
			     tl_update_timer_expired,this);
	}
      }
      return FALSE; 
}


  // Called by homeRefs 
Bool TL::isGarbage()
{
  return (*(am.getEmulatorClock()) > expireDate);
}
// Called by RemoteRefs 
Bool TL::isRoot()
{
  return FALSE;
}
  
OZ_Term TL::extract_info(OZ_Term in)
{
  return oz_cons(oz_pairAI("tl",expireDate - *(am.getEmulatorClock())),in);
}

OZ_Term TL::extract_OzId()
{
  return oz_atom("tl");
}



void TL::remove()
{
  timers->clearTimer(timer);
}



