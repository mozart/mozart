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

#ifndef __TIMER_HH
#define __TIMER_HH

#include "base.hh"
#include "am.hh"

/*
  class Timers contains an abstraction on top of the task manager to make
  it easy to set and reset an arbitrary number of timers.
  The users are responsible of storing a timerelement that is handled by Timers
  and contains necesary information. This may not be alterd by the users, and
  must be cleared by clearTimer before the user seises to exist.
  Elapsed timers cause procedures provided when the timer is set to be invoked.
*/

// A wake-up-procedure should return FALSE if it does not wish to be reinvoked.
// It must then be sure to call setTimer with a NULL ptr the next time
typedef Bool (*TimerWakeUpProc)(void *);
class TimerElement;
class TimerElementManager;

class Timers {
private:
  TimerElement *elems;
  int minint;
  LongTime prevtime;

public:
  Timers();
  ~Timers();
  
  // methods to set and clear timers. te is a TimerElement handled only by 
  // Timers but stored by users. timeToWait is the desired timeout. proc is the
  // procedure to be called at wakeup with argument arg. proc should return
  // TRUE if it wishes to be awoken after timeToWait again and FALSE otherwise.
  // arg may be anything and is not used inside Timers, just put in the call of
  // proc.
  //
  // If te!=NULL it must be a te set by this timers-object.
  void setTimer(TimerElement *&te,int timeToWait,
	       TimerWakeUpProc proc,void *arg);
  void clearTimer(TimerElement *&te);

  // Internal methods, public for technical reasons.
  Bool checkTimers(LongTime *time);
  Bool wakeUpTimers(LongTime *time);
};

class TimerElement {
  friend class Timers;
  friend class TimerElementManager;
protected:
  TimerElement *next;
  TimerWakeUpProc proc;
  int timeToWait;
  int timeLeft;
  void *arg;

  void init() {
    next=NULL;
  }
};

extern Timers *timers;
#endif
