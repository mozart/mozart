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
#include "fracWRC.hh"

#define MAXENUMERATOR INT_MAX >> 1 



/************  Handeling the list of enum denum pairs **************/

class EnumDenumPair
{
public:

  EnumDenumPair(int e, int d, EnumDenumPair *n)
  {
    enumerator = e;
    denominator = d;
    next = n;
  }
  
  int enumerator, denominator;
  EnumDenumPair *next;
};


class 
  EnumDenumPair *FracHandler::findPair(int k){
    EnumDenumPair *tmp = frac;
    while(tmp!=NULL && tmp->denominator != k)
      tmp = tmp->next;
    return tmp;
  }
  Bool FracHandler::insertPair(int e, int k){
    if (k == 0) return TRUE;
    //    printf("InsertingPair %d/%d\n",e,k);
    EnumDenumPair **tmp = &frac;
    while((*tmp)!=NULL && (*tmp)->denominator < k){
      tmp = &((*tmp)->next);
    }
    // printf("FoundPair %d/%d\n",(*tmp)->enumerator,(*tmp)->denominator);
    if ((*tmp) == NULL || (*tmp)->denominator > k){
      //printf("Creatingpair\n");
      *tmp = new EnumDenumPair(e,k,*tmp);
      return FALSE;
    }
    if (e + (*tmp)->enumerator == MAXENUMERATOR) {
      //printf("Add, filling, overflow\n");
      EnumDenumPair *ttmp = (*tmp)->next;
      delete (*tmp);
      *tmp = ttmp;
      return insertPair(1,k-1);
    }
    if (e + (*tmp)->enumerator > MAXENUMERATOR) {
      //printf("Add,  overflow\n");
      (*tmp)->enumerator = e + (*tmp)->enumerator - MAXENUMERATOR;
      return insertPair(1,k-1);
    }

    (*tmp)->enumerator = e + (*tmp)->enumerator;
    //printf("Adding %d/%d\n",(*tmp)->enumerator,(*tmp)->denominator);
    return FALSE;
  }
  EnumDenumPair *FracHandler::findLargest(){
    EnumDenumPair **tmp = &frac;
    while((*tmp)->enumerator <= 1 && (*tmp)->next != NULL){
      tmp = &((*tmp)->next);
    }
    if ((*tmp)->enumerator  > 1) return *tmp;
    Assert((*tmp)->next == NULL);
    EnumDenumPair *ttmp = *tmp;
    *tmp = new EnumDenumPair(MAXENUMERATOR,(*tmp)->denominator + 1,NULL);
    delete ttmp;
    return *tmp;
  }

/*****************************************************/



RRinstance_WRC::RRinstance_WRC(RRinstance *n){
    type = GC_ALG_WRC;
    next = n;
  }


RRinstance_WRC::RRinstance_WRC(int e, int d, RRinstance *n){
    type = GC_ALG_WRC;
    next = n;
    enumerator = e;
    denominator = d;
  }
  
void RRinstance_WRC::marshal_RR(MarshalerBuffer *buf)
{
  marshalNumber(buf,type);
  marshalNumber(buf, enumerator);
  marshalNumber(buf, denominator);
}
  
void RRinstance_WRC::unmarshal_RR(MarshalerBuffer *buf)
{
  enumerator = unmarshalNumber(buf);
  denominator = unmarshalNumber(buf);
}



int GiveSize(int enumerator){
  if (enumerator < ozconf.dp_wrc_alpha) 
    return 1;
  return (enumerator / ozconf.dp_wrc_alpha);
}

RRinstance *WRC::getBigReference(RRinstance *in){
  int enumerator, denominator; 
  EnumDenumPair *edp =findLargest();
  denominator=edp->denominator;
  enumerator=GiveSize(edp->enumerator);
  edp->enumerator = edp->enumerator - enumerator;
  return ((RRinstance *) (new RRinstance_WRC(enumerator,denominator,in))); 
}

RRinstance *WRC::getSmallReference(RRinstance *in)
{
  int enumerator, denominator; 
  EnumDenumPair *edp =findLargest();
  denominator=edp->denominator;
  enumerator=1;
  edp->enumerator = edp->enumerator - enumerator;
  return (RRinstance *)(new RRinstance_WRC(enumerator,denominator,in));}

Bool WRC::mergeReference(RRinstance* tmp){
  RRinstance_WRC *weight = (RRinstance_WRC*) tmp;
  return insertPair(weight->enumerator, weight->denominator);
  }

Bool WRC::isGarbage(){return FALSE;}

OZ_Term WRC::extract_info(OZ_Term in){
  EnumDenumPair *tmp = frac;
  OZ_Term frac = oz_nil();
  while(tmp!=NULL){
    frac = oz_cons(oz_pairII(tmp->enumerator, tmp->denominator),frac);
    tmp = tmp->next;
    }
    
    return oz_cons(oz_pairA("wrc",frac), in);
}
WRC::WRC(HomeReference *p,GCalgorithm *g){
  frac = new EnumDenumPair(MAXENUMERATOR,1,NULL);
  type = GC_ALG_WRC;
  next = g;
  parent.hr = p;
}
WRC::WRC(RemoteReference *p, RRinstance *r,GCalgorithm *g){
  RRinstance_WRC *tmp = (RRinstance_WRC*)r;
  next = g;
  frac =new EnumDenumPair(tmp->enumerator,tmp->denominator,NULL);
  type = GC_ALG_WRC;
  parent.rr = p;
}


void WRC::remove(){
  while(frac!=NULL){
    EnumDenumPair *tmp = frac->next;
    delete frac;
    frac = tmp;
  }
}

void WRC::dropReference(DSite* site, int index)
{ 
  while(frac != NULL){
    sendReferenceBack(site,index,type,frac->enumerator,frac->denominator);
    EnumDenumPair *tmp=frac;
    frac = frac->next;
      delete tmp;
  }
}

Bool WRC::isRoot(){ return FALSE;}

OZ_Term WRC::extract_OzId()
{
  return oz_atom("wrc");
}
