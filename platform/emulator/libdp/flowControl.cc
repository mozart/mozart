/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
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
#pragma implementation "flowControl.hh"
#endif

#include "base.hh"
#include "builtins.hh"
#include "value.hh"
#include "dpBase.hh"
#include "perdio.hh"
#include "port.hh"
#include "table.hh"
#include "controlvar.hh"
#include "dpInterface.hh"
#include "flowControl.hh"
#include "var.hh"

FlowControler *flowControler;

void FlowControler::addElement(TaggedRef e){
  FlowControlElement *ptr = first; 
  while(ptr!=NULL){
    if (ptr->ele == e) return;
    ptr = ptr->next;}
  FlowControlElement* newE = new FlowControlElement(e);
  if(first==NULL){
#ifdef DENYS_EVENTS
    static OZ_Term dp_event_flowControl = oz_atom("dp.flowControl");
    OZ_eventPush(dp_event_flowControl);
#else
    am.setMinimalTaskInterval((void*)this,ozconf.dpFlowBufferTime);
#endif
    first = last = newE;
    return;}
  last->next = newE;
  last = newE;
}

void FlowControler::addElement(TaggedRef e,DSite* s,int i){
  FlowControlElement *ptr = first; 
  FlowControlElement* newE = new FlowControlElement(e,s,i);
  if(first==NULL){
#ifdef DENYS_EVENTS
    static OZ_Term dp_event_flowControl = oz_atom("dp.flowControl");
    OZ_eventPush(dp_event_flowControl);
#else
    am.setMinimalTaskInterval((void*)this,ozconf.dpFlowBufferTime);
#endif
    first = last = newE;
    return;}
  last->next = newE;
  last = newE;
}

Bool FlowControlElement::canSend(){
  if(kind==FLOW_PORT){
    return (((PortProxy*)tagged2Const(ele))->canSend());}
  Assert(kind==FLOW_VAR);
  return varCanSend(site);}

void FlowControlElement::wakeUp(){
  if(kind==FLOW_PORT){
    (((PortProxy*)tagged2Const(ele))->wakeUp());}
  else{
    Assert(kind==FLOW_VAR);
//      printf("flow Control release\n");
    sendRedirect(site,index,ele);}
  free();}

#ifdef DENYS_EVENTS
Bool FlowControler::doTask(){
  FlowControlElement *ptr,*back;

  while(first!=NULL && first->canSend()){
    ptr=first;
    first=ptr->next;
    ptr->wakeUp();}
  if(first==NULL){
    last=NULL;
    return FALSE;}
  back=first;
  ptr=back->next;
  while(ptr!=NULL){
    if(ptr->canSend()){
      back->next=ptr->next;
      ptr->wakeUp();
      ptr=back->next;}
    back=ptr;
    ptr=back->next;}
  last=back;
  return TRUE;
}
#else

void FlowControler::wakeUpExecute(LongTime *t){
  FlowControlElement *ptr,*back;
  time=*t;
  time.increaseTime(ozconf.dpFlowBufferTime); // Check the user put value....

  while(first!=NULL && first->canSend()){
    ptr=first;
    first=ptr->next;
    ptr->wakeUp();}
  if(first==NULL){
    last=NULL;
    am.setMinimalTaskInterval((void*)this,0);
    return;}
  back=first;
  ptr=back->next;
  while(ptr!=NULL){
    if(ptr->canSend()){
      back->next=ptr->next;
      ptr->wakeUp();
      ptr=back->next;}
    back=ptr;
    ptr=back->next;}
  last=back;
}
#endif

void FlowControler::gcEntries(){
 FlowControlElement *ptr = first;
    while(ptr!=NULL){
      oz_gCollectTerm(ptr->ele, ptr->ele);
      if(ptr->kind == FLOW_VAR){
	ptr->site->makeGCMarkSite();}
      ptr = ptr->next;}}  

#ifndef DENYS_EVENTS
Bool FlowControlCheck(LongTime *time, void *v){
  return flowControler->wakeUpCheck(time);
}

Bool FlowControlExecute(LongTime *time, void *v){
  flowControler->wakeUpExecute(time);
  return TRUE;
}
#else
OZ_BI_define(BIdp_task_flowControl,0,1)
{
  OZ_RETURN_BOOL(flowControler->doTask());
}
OZ_BI_end
#endif
