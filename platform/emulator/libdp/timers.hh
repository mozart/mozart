#ifndef __TIMER_HH
#define __TIMER_HH

#include "base.hh"

// A wake-up-procedure should return FALSE if it does not wish to be reinvoked.
// It must then be sure to call setTimer with a NULL ptr the next time
typedef Bool (*TimerWakeUpProc)(void *);
class TimerElement;
class TimerElementManager;

class Timers {
private:
  TimerElement *elems;
  int minint;
  unsigned long prevtime;

public:
  Timers();
  ~Timers();

  // If te!=NULL it must be a te set by this timers-object.
  void setTimer(TimerElement *&te,int timeToWait,
	       TimerWakeUpProc proc,void *arg);
  void clearTimer(TimerElement *&te);

  Bool checkTimers(unsigned long time);
  Bool wakeUpTimers(unsigned long time);
};

class TimerElement {
  friend Timers;
  friend TimerElementManager;
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
