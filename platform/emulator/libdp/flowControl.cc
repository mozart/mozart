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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
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
FlowControler *flowControler;

void FlowControler::addElement(TaggedRef e){
  FlowControlElement *ptr = first; 
  while(ptr!=NULL){
    if (ptr->ele == e) return;
    ptr = ptr->next;}
  FlowControlElement* newE =  (FlowControlElement*) genFreeListManager->getOne_2();
  newE->next = NULL;
  newE->ele  = e;
  if(first==NULL){
    am.setMinimalTaskInterval((void*)this,ozconf.perdioFlowBufferTime);
    first = last = newE;
    return;}
  last->next = newE;
  last = last->next;
  
}

// ERIK-LOOK are the elements removed
void FlowControler::wakeUpExecute(unsigned int t){
     FlowControlElement *ptr = first, *ptrOld = NULL;
     time = t + ozconf.perdioFlowBufferTime; // Check the user put value....
     while(ptr!=NULL){
      if(((PortProxy*)tagged2Const(ptr->ele))->canSend())
	break;
      ptrOld = ptr;
      ptr = ptr->next;}
    if (ptr==NULL) return;
    if (ptrOld==NULL)
      first = ptr->next;
    else
      ptrOld->next = ptr->next;
    if(ptr == last)
      last = ptrOld;
    if(first==NULL) 
      am.setMinimalTaskInterval((void*)this,0);
    ((PortProxy*)tagged2Tert(ptr->ele))->wakeUp();
}


void FlowControler::gcEntries(){
 FlowControlElement *ptr = first;
    while(ptr!=NULL){
      OZ_collectHeapTerm(ptr->ele, ptr->ele);
      ptr = ptr->next;}}  

Bool FlowControlCheck(unsigned long time, void *v){
  return flowControler->wakeUpCheck(time);
}

Bool FlowControlExecute(unsigned long time, void *v){
  flowControler->wakeUpExecute(time);
  return TRUE;
}


