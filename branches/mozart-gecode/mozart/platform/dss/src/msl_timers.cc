/*
 *  Authors:
 *    Zacharias El Banna (zeb@sics.se)
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
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

#if defined(INTERFACE)
#pragma implementation "msl_timers.hh"
#endif

#include "msl_timers.hh"

namespace _msl_internal{


#ifdef DEBUG_CHECK
  int TimerElement::a_allocated = 0;
#endif

  // ******************* TIME WHEEL *********************
  //
  // Insert functions used during XXX_ticks

  inline void Timers::m_hrs_insert(TimerElement* const tel){
    tel->a_time -= (MSG - (a_mseconds + SG * a_minutes));
    a_hourList.push(tel);
  }
  
  
  inline void Timers::m_min_insert(TimerElement* const tel){
    unsigned int pos = ((tel->a_time / SG) + a_minutes) % MINUTES;
    tel->a_time %= SG; // remove minutes
    a_minuteWheel[pos].push(tel);
  }
  
  
  inline void Timers::m_sec_insert(TimerElement* const tel){
    //           pos = (             pos from now              +     safety if near now               ) mod. no of positions
    unsigned int pos = ((tel->a_time + a_mseconds)/GRANULARITY + ((tel->a_time > GRANULARITY) ? 0 : 1)) % SECONDS;
    //dssLog(DLL_NOTHING,"TIMER   (%p): SECOND INSERT %p [time:%3d pos:%3d] %s",this,tel,tel->a_time,pos,m_stringrep());
    a_secondWheel[pos].push(tel);
    DebugCode(tel->a_time = 0;);
  }
  
  
  
  // ********************** TICKS ***********************
  //
  // The two first XXX_ticks are called recursively from
  // m_(second)_ticks when the clock hand has turned a whole
  // revolution.
    
  inline void Timers::m_hour_tick(){
    Position<TimerElement*> p(a_hourList);
    while (p.hasElement()) {
      if ((*p)->a_time < MSG) {
	//dssLog(DLL_MOST,"TIMER   (%p): Dropping hour timer %p",this,tmp);
	m_min_insert(p.pop()); // Know we are talking about a_minutes (or seconds)
      } else {
	//dssLog(DLL_MOST,"TIMER   (%p): Postponing hour timer %p",this,(*iter));
	(*p)->a_time -= MSG;
	p.next();
      }
    }
  }
  

  inline void Timers::m_min_tick(){
    a_minutes = (++a_minutes) % MINUTES;
    if(a_minutes == 0)
      m_hour_tick();
    while(!a_minuteWheel[a_minutes].isEmpty()){
      TimerElement* tel = a_minuteWheel[a_minutes].pop();
      if(tel->m_live()){
	//dssLog(DLL_NOTHING,"TIMER   (%p): Dropping minute timer %p",this,tel);
	m_sec_insert(tel);
      } else {
	//dssLog(DLL_NOTHING,"TIMER   (%p): Deleting minute timer %p",this,tel);
	delete tel;
      }
    }
  }


  //
  //

  inline void Timers::m_internalInsert(TimerElement* const tel){
    Assert(tel->a_time > 0);
    // One should optimize so that:
    //
    // if (tel->a_time < GRANULARITY) immediateEvents.insert
    //
    if (tel->a_time < SG){ // within a MINUTE
      m_sec_insert(tel);
    } else if(tel->a_time < MSG){ // within an HOUR
      m_min_insert(tel);
    } else { // more
      m_hrs_insert(tel);
    }
  }
  
  
  void Timers::m_executeList(const unsigned int& pos){
    //if(!a_secondWheel[pos].isEmpty()){
    //  dssLog(DLL_NOTHING,"TIMER   (%p): EXECUTING TIMED EVENTS %s",this,m_stringrep()); 
    //}
    while (!a_secondWheel[pos].isEmpty()) {
      TimerElement* tel = a_secondWheel[pos].pop();
      if (tel->m_dead()) delete tel;  // Most timers are cleared
      else{
	//dssLog(DLL_NOTHING,"TIMER   (%p): Executing %p",this,tmp);
	unsigned int new_time = tel->m_exec();
	if(new_time == 0) delete tel; // Most timers are also not reused (???)
	else {
	  // The below assert should not be valid since the m_sec_insert corrects "near now" values?
	  //Assert(new_time > GRANULARITY);
	  tel->m_reset(new_time);
	  //dssLog(DLL_NOTHING,"TIMRS   (%p): Reinserting %p (time:%d)",this,tmp,tmp->a_time);
	  Assert(a_suspended); // must be during this action
	  a_suspensionList.push(tel);
	}
      }
    }
  }

  

  // ******************* EXPOSED INTERFACE ********************
  //
  
  Timers::Timers():
    a_hourList(),
    a_suspensionList(),
    a_mseconds(0),
    a_minutes(0),
    a_clock(),
    a_suspended(false){
  }
    

  Timers::~Timers(){}

  void Timers::m_ticks(int steps){
    Assert(steps > 0);
    //     Assert(static_cast<unsigned int>(steps) < SG); // In mozart case less than 32700 ~ almost a short

    // Erik, this is probably debug code.
    //if(static_cast<unsigned int>(steps) < SG)  steps = 1; // for debugging! 

    // Save old clocks
    unsigned int old  = a_mseconds;
    int diff = GRANULARITY - (old % GRANULARITY);

    // Update clocks
    a_clock.increaseTime(steps);
    a_mseconds = (a_mseconds + steps) % SG;

    // If passing ANY position
    if(steps >= diff){
      a_suspended = true;         Assert(a_suspensionList.isEmpty());
      steps -= diff;
      old    = (old + diff) % SG; Assert(old % GRANULARITY == 0);

      do {
	if(old == 0) m_min_tick(); // a minute has passed
	m_executeList(old / GRANULARITY); // Fire off all timed events
	steps -= GRANULARITY;
	old    = (old + GRANULARITY) % SG;
      } while(steps >= 0);
      a_suspended = false;
      // Add elements inserted during ticks
      while(!a_suspensionList.isEmpty())
	m_internalInsert(a_suspensionList.pop());
    }
  }


  inline void Timers::m_safeInsert(TimerElement* const tel){
    Assert(tel->a_time > GRANULARITY);
    if(!a_suspended)
      m_internalInsert(tel);
    else
      a_suspensionList.push(tel); 
  }
  
  
  void Timers::setTimer(TimerElement*& tel, const unsigned int& time, TimerWakeUpProc t, void* const arg){
    Assert(time > GRANULARITY);
    if(tel != NULL){
      tel->m_kill(); // Clear is unecessary, we don't need to set tel to NULL
    }
    tel = new TimerElement(t,arg,time);
    //dssLog(DLL_DEBUG,"TIMERS        (%p): %s Insert timer %p : TIMEOUT:%d",this,a_clock.stringrep(),tel,time);
    if(!a_suspended)
      m_internalInsert(tel);
    else
      a_suspensionList.push(tel); 
  }


  // ************************** UTILS *****************************
  char* Timers::m_stringrep(){
      static char buf[100];
      sprintf(buf,"WHEEL:%3d:%4d SUSP:%s CLOCK:%s",
	      a_minutes,
	      a_mseconds,
	      ::gf_bool2string(a_suspended),
	      a_clock.stringrep());
      return buf;
  }

}
