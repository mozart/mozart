/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller,popow
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __LPS_H__
#define __LPS_H__

#ifdef INTERFACE
#pragma interface
#endif

#include "oz_cpi.hh"

#ifdef OUTLINE
#define inline
#endif


/*
 *  Local propagation store;
 *
 */ 

#ifndef DONT_LP 
#define TM_LP
#endif

class LocalPropagationQueue {
private:
  int head, tail, size, maxsize;
  int backup_head, backup_tail, backup_size, backup_maxsize;
#ifdef DEBUG_CHECK  
  enum {initmaxsize = 0x10}; // expected to be power of 2
#else
  enum {initmaxsize = 0x1000}; // expected to be power of 2
#endif
  struct queue_t {
    Thread *thr;
  } *queue, *backup_queue;

public:
  void resize(void);
  
  void enqueue (Thread *thr) {
    if (size == maxsize) resize ();
    tail = (tail + 1) & (maxsize - 1); // reason for maxsize to be power of 2
    queue[tail].thr = thr;
    size += 1;
  }

  Thread *dequeue () {
    if (size == 0) 
      error ( "Cannot dequeue from empty queue.");
    Thread *thr = queue[head].thr;
    head = (head + 1) & (maxsize - 1);
    size -= 1;
    return (thr);
  }

  LocalPropagationQueue ()
  : maxsize (initmaxsize), head (0), tail (initmaxsize - 1), size (0)
  {
    queue = new queue_t[maxsize];
  }
  
  void backupQueue (int s);
  void restoreQueue ();
  
  Bool isEmpty () { return (size == 0); }
  void reset () {
    head = 0; 
    tail = maxsize - 1; 
    size = 0;
  }
  int getSize () { return (size); }
  void printDebug ();
};

/*
 *  Local propagation queue of *propagators*;
 *
 */
class LocalPropagationStore : protected LocalPropagationQueue {
private:
  Bool in_local_propagation, backup_in_local_propagation;
  Bool propagate_locally ();
#ifndef TM_LP
  Bool useit;
#endif

  DebugCode (Bool checkIsPropagator (Thread *thr);)
public:
  LocalPropagationStore ()
  : in_local_propagation(FALSE)
#ifndef TM_LP
       , useit(FALSE)
#endif
  {};

  Bool reset () {
    LocalPropagationQueue::reset ();
    return (in_local_propagation = FALSE);
  }

  Bool isInLocalPropagation () { return (in_local_propagation); }
  
  Bool isEmpty () {
    return (LocalPropagationQueue::isEmpty ());
  }

  Bool do_propagation () {
    if (!isEmpty ()) return (propagate_locally ());
    return (TRUE);
  }

  void push (Thread *thr) {
    Assert (checkIsPropagator (thr));
    enqueue (thr);
  }

  Thread *pop () {
    return (dequeue ());
  }

  void backup (int s) {
    backup_in_local_propagation = in_local_propagation;
    backupQueue (s);
  }

  void restore () {
    in_local_propagation = backup_in_local_propagation;
    restoreQueue ();
  }

#ifndef TM_LP
  void setUseIt () { useit = TRUE; }
  void unsetUseIt () { useit = FALSE; }
  Bool isUseIt () { return (useit); }
#endif    
};

extern LocalPropagationStore localPropStore;

#ifdef TM_LP
#  define LOCAL_PROPAGATION(CODE) CODE
#else
#  define LOCAL_PROPAGATION(CODE)
#endif

#ifdef OUTLINE
#undef inline
#else
#include "lps.icc"
#endif


#endif //__LPS_H__
