/*
 *  Authors:
 *    Zacharias El Banna (zeb@sics.se)
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

#ifndef __MSL_TIMERS_HH
#define __MSL_TIMERS_HH

#ifdef INTERFACE
#pragma interface
#endif

#include <stdio.h>
#include "mslBase.hh"
#include "dss_templates.hh"
namespace _msl_internal{ //Start namespace

  
  
  //
  // Returns if it which to be reinserted
  // 0  == NO
  // >0 == YES (with value)
  //
  

  
  // ****************** BASIC TIMER ELEMENT *******************
  //
  // live tells if the element user still think it is valid
  //

  class TimerElement: public TimerElementInterface {
    friend class SimpleList<TimerElement>;
    friend class Timers;
  protected:
    // ********** for TimeWheel *************
    TimerElement *a_next;
    unsigned int  a_time;
    // ********* for WheelElement ***********
    TimerWakeUpProc a_contain;
    void* const   a_args;

#ifdef DEBUG_CHECK
  public:
    static int a_allocated;
  protected:
#endif

    // Invoked ONLY
    inline void m_reset(const unsigned int& time){ a_time    = time; DebugCode(a_next = reinterpret_cast<TimerElement*>(0xAABBCCDD);); }
    inline unsigned int  m_exec(){ return (a_contain)(a_args); }

  private: // not to be used
   TimerElement(const TimerElement& te):
      a_next(NULL),
      a_time(0),
      a_contain(NULL),
      a_args(NULL){
    }
    
    TimerElement& operator=(const TimerElement& te){ return *this; }

  public:
    // Kill a timer that is ACTIVE
    inline void m_kill(){
      a_contain = NULL;
    }
    
    // Repoint an ACTIVE timer to a new task
    inline void m_reuse(TimerWakeUpProc proc){ a_contain = proc; }

    // Checks
    inline bool m_dead() const{ return (a_contain == NULL); }
    inline bool m_live() const{ return (a_contain != NULL); }

    // Basic timer creation. Starting it is done through "Timer" operations
    TimerElement(TimerWakeUpProc t, void* const arg, const unsigned int& time):
      a_next(NULL),
      a_time(time),
      a_contain(t),
      a_args(arg){
      Assert(time > 0);
      DebugCode(a_allocated++);
    };


    ~TimerElement(){
      DebugCode(a_allocated--);
    }
  };


  class Timers{
  private:
    static const unsigned int MINUTES=128;    // = 1h 10 minutes -> 128  bytes minute list
    static const unsigned int SECONDS=2048;   // = 32.8 seconds  -> 2048 bytes second list
    static const unsigned int GRANULARITY=16; // = 16 milliseconds
    
    static const unsigned int SG  = (SECONDS * GRANULARITY);
    static const unsigned int MSG = (MINUTES * SG);
    
    SimpleList<TimerElement> a_secondWheel[SECONDS]; // Our smallest wheel
    SimpleList<TimerElement> a_minuteWheel[MINUTES]; // Our bigger wheel
    SimpleList<TimerElement> a_hourList;             // List of those NON EXISTING "out of bounds" elements
    SimpleList<TimerElement> a_suspensionList;       // During execution of ticks.
    
    // Immediate after
    //
    // SimpleList<TimerElement> a_immediateAfter;

    unsigned int a_mseconds;
    unsigned int a_minutes; 
    DSS_LongTime a_clock;
    bool         a_suspended;

    // ********************** Internals *************************
    inline void m_hrs_insert(TimerElement* const tel);
    inline void m_min_insert(TimerElement* const tel);
    inline void m_sec_insert(TimerElement* const tel);
    
    inline void m_hour_tick();    
    inline void m_min_tick();

    void m_executeList(const unsigned int& pos);
    
    // REMEMBER: this one is not guarded under suspension so use
    // the safeInsert instead
    inline void m_internalInsert(TimerElement* const tel);

  public:
    // *********************** INTERFACE *************************
    
    Timers();
    ~Timers();

    inline void m_reset(){  a_mseconds = a_minutes = 0; }
    
    inline DSS_LongTime currTime() {return a_clock;}

    void m_ticks(int steps);
    
    inline void clearTimer(TimerElement*& tel){
      if(tel != NULL){
	tel->m_kill();
	tel = NULL;
      }
    }

    inline void m_safeInsert(TimerElement* const tel);

    void setTimer(TimerElement*& tel, const unsigned int& time, TimerWakeUpProc t, void* const arg);

    // void insertImmediate((TimerElement*& tel, TimerWakeUpProc t, void* const arg);

    char* m_stringrep();

  };
} // end _dss_internal
#endif
