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
#include "fracWRC.hh"



void marshalCredit(MarshalerBuffer *buf, RRinstance *r){
  int len = 0;
  for(RRinstance *tmp = r;tmp!=NULL; tmp = tmp->next,len ++);
  buf->put(DIF_PRIMARY);
  marshalNumber(buf, len);
  for(;r!=NULL; r = r->next)r->marshal_RR(buf);
}

RRinstance *unmarshalCreditRobust(MarshalerBuffer *buf, int *error)
{
  RRinstance *ans=NULL;
  int len;
  MarshalTag mt = (MarshalTag) buf->get();
  Assert(mt == DIF_PRIMARY);
  len = unmarshalNumberRobust(buf, error);
  for(;len>0; len --){
    int type = unmarshalNumberRobust(buf, error);
    switch(type){
    case GC_ALG_WRC:
      ans = new RRinstance_WRC(ans);
      ans->unmarshal_RR(buf, error);
      break;
    case GC_ALG_TL:
      ans = new RRinstance_TL(ans);
      ans->unmarshal_RR(buf, error);
      break;
    default:
      Assert(0);
    }
  }
  return ans;
}

void marshalCreditToOwner(MarshalerBuffer *buf,RRinstance *r,int oti){
  int len= 0 ;
  for(RRinstance *tmp = r;tmp!=NULL; tmp = tmp->next,len ++);
  buf->put(DIF_OWNER);
  marshalNumber(buf, oti);
  marshalNumber(buf, len);
  for(;r!=NULL; r = r->next)r->marshal_RR(buf);
}


RRinstance  *unmarshalCreditToOwnerRobust(MarshalerBuffer *buf,
                                          MarshalTag mt, int &oti,
                                          int *error)
{
  RRinstance *ans=NULL;
  int len;
  oti = unmarshalNumberRobust(buf, error);
  len = unmarshalNumberRobust(buf, error);
  for(;len>0; len --){
    int type = unmarshalNumberRobust(buf, error);
    switch(type){
    case GC_ALG_WRC:
      ans = new RRinstance_WRC(ans);
      ans->unmarshal_RR(buf, error);
      break;
    case GC_ALG_TL:
      ans = new RRinstance_TL(ans);
      ans->unmarshal_RR(buf, error);
      break;
    default:
      Assert(0);
    }
  }
  return ans;
}

RRinstance *CreateRRinstance(int type, int val1, int val2)
{
  RRinstance *ans = NULL;
  switch(type){
  case GC_ALG_WRC:
    ans = (RRinstance *)new RRinstance_WRC(val1,val2,NULL);
    break;
  case GC_ALG_TL:
    ans = (RRinstance *)new RRinstance_TL(val1,NULL);
    break;
  default:
    Assert(0);
  }
  return ans;
}





// Home Reference

Bool HomeReference::isPersistent(){return algs == NULL;}

void HomeReference::makePersistent()
{
  while(algs!=NULL){
    GCalgorithm *tmp = algs->next;
    algs->remove();
    delete algs;
    algs = tmp;
  }
  Assert(algs == NULL);
}
// old hasFullCredit
Bool HomeReference::canBeReclaimed(){
  for(GCalgorithm *tmp = algs;tmp != NULL ; tmp = tmp->next)
    if (tmp->isGarbage()) return TRUE;
  return FALSE;
}

void HomeReference::setUp(int indx, int algss){
  oti = indx;
  algs = NULL;
  if(algss & GC_ALG_WRC)
    algs = new WRC(this,algs);
  if(algss & GC_ALG_TL)
    algs = new TL(this,algs);
}

Bool HomeReference::mergeReference(RRinstance *r){
  // Sweeps all existing gc-algorithms to find
  // the instance that matches. If the matching
  // algorithm thinks the entity is garbage
  // the entity should be reclaimed. This is
  // independantly of what the other algorithms
  // reports.

  for(GCalgorithm *tmp = algs;tmp != NULL ; tmp = tmp->next)
    {
      RRinstance *tmpR = r;
      for(; tmpR != NULL; tmpR=tmpR->next)
        if (tmp->type == tmpR->type && tmp->mergeReference(tmpR) )
          return TRUE;
    }
  return FALSE;
}
RRinstance* HomeReference::getBigReference(){
  RRinstance *ans = NULL;
  for(GCalgorithm *tmp = algs;tmp != NULL ; tmp = tmp->next)
    {
      ans = tmp->getBigReference(ans);
    }
  return ans;
}
RRinstance *HomeReference::getSmallReference(){
  RRinstance *ans = NULL;
  for(GCalgorithm *tmp = algs;tmp != NULL ; tmp = tmp->next)
    {
      ans = tmp->getSmallReference(ans);
    }
  return ans;
}

OZ_Term HomeReference::extract_info(){
  OZ_Term ans = oz_nil();
  for(GCalgorithm *tmp = algs;tmp != NULL ; tmp = tmp->next)
    {
      ans = tmp->extract_info(ans);
    }
  return ans;
};



Bool RemoteReference::canBeReclaimed(){
  Bool ans=FALSE;
  for(GCalgorithm *tmp = algs;tmp != NULL ; tmp = tmp->next)
    ans = ans || tmp->isRoot();
  return !ans;

}

void RemoteReference::setUp(RRinstance *r,DSite* s,int i){
  netaddr.set(s,i);
  algs = NULL;
  for(RRinstance *tmpR = r; tmpR != NULL; tmpR = tmpR->next)
    {
      switch(tmpR->type){
      case GC_ALG_WRC:
        algs = new WRC(this,tmpR,algs);
        break;
      case GC_ALG_TL:
        algs = new TL(this,tmpR,algs);
        break;
      default:
        Assert(0);
      }
    }
}

void HomeReference::removeReference(){
  while(algs!=NULL){
    GCalgorithm *tmp2=algs;
    algs->remove();
    algs=algs->next;
    delete tmp2;
  }
}

Bool HomeReference::removeAlgorithm(OZ_Term id){
  GCalgorithm **tmp = &algs;
  while(*tmp!=NULL){
    if(OZ_eq(id, (*tmp)->extract_OzId())){
      GCalgorithm *del_tmp = *tmp;
      (*tmp)=(*tmp)->next;
      del_tmp->remove();
      delete del_tmp;
      return TRUE;
    }
    tmp = &((*tmp)->next);
  }
  return FALSE;
}



void RemoteReference::dropReference(){
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

void RemoteReference::copyReference(RemoteReference *from){
  netaddr.set(from->netaddr.site,from->netaddr.index);
  algs = from->algs;
}

void RemoteReference::mergeReference(RRinstance *r){

  GCalgorithm **tmp = &algs;
  while((*tmp)!=NULL)
    {
      RRinstance *tmpR = r;
      for(;tmpR != NULL; tmpR = tmpR->next){
        if(tmpR->type == (*tmp)->type)
          break;
      }
      if (tmpR == NULL)
        {
          // Remove the algorithm that where not found
          GCalgorithm *del_tmp = *tmp;
          (*tmp)=(*tmp)->next;
          del_tmp->remove();
          delete del_tmp;
        }
      else
        {
          (*tmp)->mergeReference(tmpR);
          tmp = &((*tmp)->next);
        }
    }
}

RRinstance *RemoteReference::getBigReference(){
  RRinstance *ans = NULL;
  for(GCalgorithm *tmp = algs;tmp != NULL ; tmp = tmp->next)
    {
      ans = tmp->getBigReference(ans);
    }
  return ans;
}
RRinstance *RemoteReference::getSmallReference(){
  RRinstance *ans = NULL;
  for(GCalgorithm *tmp = algs;tmp != NULL ; tmp = tmp->next)
    {
      ans = tmp->getSmallReference(ans);
    }
  return ans;
}

NetAddress* RemoteReference::getNetAddress() {return &netaddr;}

OZ_Term RemoteReference::extract_info(){
  OZ_Term ans = oz_nil();
  for(GCalgorithm *tmp = algs;tmp != NULL ; tmp = tmp->next)
    {
      ans = tmp->extract_info(ans);
    }
  return ans;
}

Bool RemoteReference::isPersistent(){
  return algs == NULL;}

void sendReferenceBack(DSite *entitysite,int entityOTI,int type, int val1, int val2) {
  MsgContainer *msgC;
  msgC = msgContainerManager->newMsgContainer(entitysite);
  msgC->put_M_OWNER_REF(entityOTI,type,val1,val2);

  send(msgC,-1);
}


void sendRRinstanceBack(DSite *s, int oti, RRinstance *r){
  // ERIK
  // Not implemented yet, just because I'm Lazy :)
  ;
}
