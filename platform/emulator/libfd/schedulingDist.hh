/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller, wuertz
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __SCHED_HH__
#define __SCHED_HH__

#include "std.hh"

#include <iostream.h>

//////////
// Task Intervals
//////////
class TaskIntervalsProof : public OZ_Propagator {
private:

  // The finite domains
  OZ_Term * reg_fds;
  // overall number of tasks
  int reg_fds_size;

  // The durations
  int ** reg_durs;  // reg_durs_size equals reg_fds_size

  // Number of tasks in each resource
  int * reg_nb_tasks;
  // Number of resources
  //  int reg_nb_resources;
  int reg_nb_tasks_size;

  // Maximal number of tasks on a resource
  int reg_max_nb_tasks;

  // The end task finishing the schedule
  OZ_Term * reg_pe;

  // To store the already ordered pairs
  int * reg_order_vector;
  int reg_order_vector_size;

  // Flag whether proof or optimum variant (0 = proof)
  int reg_flag;

  OZ_Term stream;

  static OZ_CFun spawner;
public:
  TaskIntervalsProof(OZ_Term, OZ_Term, OZ_Term, OZ_Term, int);
  ~TaskIntervalsProof(void);
  virtual size_t sizeOf(void) { return sizeof(TaskIntervalsProof); }
  virtual void updateHeapRefs(OZ_Boolean);
  virtual OZ_Return propagate(void);
  virtual OZ_Term getParameters(void) const { RETURN_LIST1(stream); }
  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
};

//////////
// FIrsts and Lasts
//////////
class FirstsLasts : public OZ_Propagator {
private:

  // The finite domains
  OZ_Term * reg_fds;
  // overall number of tasks
  int reg_fds_size;

  // The durations
  int ** reg_durs;  // reg_durs_size equals reg_fds_size

  // Number of tasks in each resource
  int * reg_nb_tasks;
  // Number of resources
  //  int reg_nb_resources;
  int reg_nb_tasks_size;

  // Maximal number of tasks on a resource
  int reg_max_nb_tasks;

  // To store the already ordered pairs
  int * reg_ordered;


  OZ_Term stream;

  static OZ_CFun spawner;
public:
  FirstsLasts(OZ_Term, OZ_Term, OZ_Term, OZ_Term, int);
  virtual size_t sizeOf(void) { return sizeof(FirstsLasts); }
  virtual void updateHeapRefs(OZ_Boolean);
  virtual OZ_Return propagate(void);
  virtual OZ_Term getParameters(void) const { RETURN_LIST1(stream); }
  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
};



#endif // __SCHED_HH__
