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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
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
  unsigned int time;
public:

  FlowControler(){
    first = last = NULL;
    time = 0;}

  void addElement(TaggedRef e);
  void addElement(TaggedRef e,DSite*,int);

  Bool wakeUpCheck(unsigned int t){
    return first != NULL && t > time;}

  void wakeUpExecute(unsigned int t);

  void gcEntries();

};


extern FlowControler *flowControler;


Bool FlowControlCheck(unsigned long time, void *v);

Bool FlowControlExecute(unsigned long time, void *v);


#endif
