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
  int reg_nb_tasks_size;

  // Maximal number of tasks on a resource
  int reg_max_nb_tasks;

  // To store the already ordered pairs
  int * reg_ordered;
  int * reg_ordered_resources;

  // Flag whether firstsLasts (0), firsts (1), lasts (2)
  int reg_flag;

  int reg_resource; 

  OZ_Term stream;

  static OZ_CFun spawner;
public:
  FirstsLasts(OZ_Term, OZ_Term, OZ_Term, OZ_Term, int);
  ~FirstsLasts();
  virtual size_t sizeOf(void) { return sizeof(FirstsLasts); }
  virtual void updateHeapRefs(OZ_Boolean);
  virtual OZ_Return propagate(void); 
  virtual OZ_Term getParameters(void) const { RETURN_LIST1(stream); }
  virtual OZ_CFun getHeaderFunc(void) const { return spawner; }
};



#endif // __SCHED_HH__


