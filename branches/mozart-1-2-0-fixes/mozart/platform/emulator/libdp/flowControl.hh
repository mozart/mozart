/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *
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

#ifndef __FLOWCNTRL_HH
#define __FLOWCNTRL_HH

#ifdef INTERFACE
#pragma interface
#endif

enum FlowControlKind{
  FLOW_PORT,
  FLOW_VAR};


class FlowControlElement{
public:
  FlowControlElement *next;
  FlowControlKind kind;
  TaggedRef ele;
  DSite* site;
  int index;

  void *operator new(size_t size){
    Assert(size==20);
    return (FlowControlElement*) genFreeListManager->getOne_5();}

  void free(){
    genFreeListManager->putOne_5((FreeListEntry*) this);}

  FlowControlElement(TaggedRef e){
    ele=e;
    next=NULL;
    kind=FLOW_PORT;}

  FlowControlElement(TaggedRef e,DSite* s,int i){
    ele=e;
    next=NULL;
    site=s;
    index=i;
    kind=FLOW_VAR;}

  Bool canSend();
  void wakeUp();

};
    
  

class FlowControler{
  FlowControlElement *first, *last;
#ifndef DENYS_EVENTS
  LongTime time;
#endif
public:
  
  FlowControler(){
    first = last = NULL;
  } 
  
  void addElement(TaggedRef e);
  void addElement(TaggedRef e,DSite*,int);

#ifndef DENYS_EVENTS
  Bool wakeUpCheck(LongTime *t){
    return first != NULL && *t > time;}
  
  void wakeUpExecute(LongTime *t);
#else
  Bool doTask();
#endif
  
  void gcEntries();

};


extern FlowControler *flowControler;


Bool FlowControlCheck(LongTime *, void *v);

Bool FlowControlExecute(LongTime *, void *v);


#endif











