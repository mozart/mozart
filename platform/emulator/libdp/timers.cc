/*
 *  Authors:
 *    Anna Neiderud (annan@sics.se)
 * 
 *  Contributors:
 * 
 *  Copyright:
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

#include "timers.hh"
#include "am.hh"
#include "dpDebug.hh"
#include "genhashtbl.hh"

#define TIMER_RES 1000

static Bool timers_checkTimers(LongTime *time,void *timers);
static Bool timers_wakeUpTimers(LongTime *time,void *timers);

#define TimerElement_CUTOFF 200
class TimerElementManager: public FreeListManager {
public:
  TimerElementManager():FreeListManager(TimerElement_CUTOFF){wc = 0;}
  ~TimerElementManager() {
    TimerElement *te;
    FreeListEntry *f;
    int l=length();
    for(int i=0;i<l;i++) {
      f=getOne();
      Assert(f!=NULL);
      te = (TimerElement*) (void*) f;
      delete te;
    }
    Assert(length()==0);
  }

  int wc;

  TimerElement * getTimerElement(){
    FreeListEntry *f=getOne();
    TimerElement *bb;
    if(f==NULL) {
      bb=new TimerElement();
    }
    else {
      bb = (TimerElement*) (void*) f;
    }
    bb->init();
    ++wc;
    return bb;
  }
  
  void deleteTimerElement(TimerElement* bb){
    FreeListEntry *f;
    --wc;
    f = (FreeListEntry*)(void*) bb;
    if(putOne(f)) 
      return;
    else
      delete bb;
    return;
  }

  int getCTR(){ return wc;}
}; 
TimerElementManager *timerElementManager;

Timers::Timers() {
  elems=NULL;
  minint=-1;
  timerElementManager=new TimerElementManager();
}

Timers::~Timers() {
  am.removeTask((void*) this, timers_checkTimers);
  delete timerElementManager;
  TimerElement *tmp;
  while(elems!=NULL) {
    tmp=elems;
    elems=elems->next;
    delete tmp;
  }
}

// If te!=NULL it must be a te set by this timers-object.
void Timers::setTimer(TimerElement *&te,int timeToWait,
		     TimerWakeUpProc proc,void *arg) {
  //  printf("timer (re)set for te %x proc %x arg %x this %x\n",te,proc,arg,this);
  //  printf("st %x;%x %d\n",arg,proc,timeToWait);
  // Only if te is NULL a new one needs to be inserted...
  Assert(!(te!=NULL && elems==NULL));
  if(te==NULL) {
    te=timerElementManager->getTimerElement();
    
    if(elems==NULL) {
      elems=te;
      if(!am.registerTask((void*) this, 
			  timers_checkTimers, timers_wakeUpTimers)) {
	OZ_error("Unable to register Timers task");
	return;
      }
      am.setMinimalTaskInterval((void *) this,TIMER_RES);
      prevtime=*(am.getEmulatorClock());
    }
    else {
      te->next=elems;
      elems=te;
    }
  }
    
  te->timeToWait=timeToWait;
  // Must add the time allready passed since prevtime was set
  te->timeLeft=timeToWait+(*am.getEmulatorClock()-prevtime);
  te->proc=proc;
  te->arg=arg;

//    if(minint==-1||timeToWait<minint) {
//      minint=timeToWait;
//      am.setMinimalTaskInterval((void *) this,timeToWait);
//    }
}

void Timers::clearTimer(TimerElement *&te) {
  // Only invalidate, traverse and remove at wakeup.
  if(te!=NULL) {
    te->proc=NULL;
    te=NULL;
  }
}

Bool Timers::checkTimers(LongTime *time) {
  int timepassed=*time-prevtime;
  TimerElement *cur;

  PD((TCP_INTERFACE,"Time to check timers at %s (since last %d)",
      am.getEmulatorClock()->toString(),timepassed));

  // Remove any invalidated timers at the beginning
  while(elems!=NULL && elems->proc==NULL) {
    cur=elems; 
    elems=elems->next;
    timerElementManager->deleteTimerElement(cur);
  }
  if(elems==NULL) {
    PD((TCP_INTERFACE,"No more timers, removing timer task"));
    am.setMinimalTaskInterval((void *) this,0); // Wakeup no longer needed
    am.removeTask((void*) this, timers_checkTimers);
    minint=-1;
  }
  else {
    // Check if at least one timer needs to be woken
    cur=elems;
    while(cur!=NULL) {
      if(cur->proc!=NULL&&cur->timeLeft<=timepassed)
	return TRUE;
      cur=cur->next;
    }
  }
  return FALSE;
}
   
Bool Timers::wakeUpTimers(LongTime *time) {
  int timepassed=*time-prevtime;
  prevtime=*time;
  PD((TCP_INTERFACE,"Time to wake up timers at %s (since last %d)",
      am.getEmulatorClock()->toString(),timepassed));

  TimerElement *cur=elems;
  TimerElement *prev;
  TimerElement *tmp;

  while(cur!=NULL) {
    // printf("in tl %x;%x;%d\n",cur->arg,cur->proc,cur->timeLeft-timepassed);
    if(cur->proc!=NULL) {
      cur->timeLeft-=timepassed;
      //      printf("wakeUp: cur %x ,timeLeft %d\n",cur,cur->timeLeft);
      if(cur->timeLeft<=0) {
	if(!(cur->proc)(cur->arg))
	  cur->proc=NULL; // invalidate, remove below...
	else
	  cur->timeLeft=cur->timeToWait;
      }
    }
    cur=cur->next;
  }

  // Remove invalidated timers
  cur=elems; // Start over, new timers may have been inserted
  prev=NULL;
  while(cur!=NULL) {
    if(cur->proc==NULL) { // invalidated, remove
      tmp=cur;
      if(prev!=NULL) {
	prev->next=cur->next;
	cur=cur->next;
      }
      else {
	elems=cur->next;
	cur=elems;
      }

      timerElementManager->deleteTimerElement(tmp);
      continue;
    }
    else {
      prev=cur;
      cur=cur->next;
    }
  }

  if(elems==NULL) {
    PD((TCP_INTERFACE,"No more timers, removing timer task"));
    am.setMinimalTaskInterval((void *) this,0); // Wakeup no longer needed
    am.removeTask((void*) this, timers_checkTimers);
    minint=-1;
  }
  return TRUE; // Allways done for now to taskhandler
}

static Bool timers_checkTimers(LongTime *time,void *timers) {
  return ((Timers *) timers)->checkTimers(time);
}

static Bool timers_wakeUpTimers(LongTime *time,void *timers) {
  return ((Timers *) timers)->wakeUpTimers(time);
}


