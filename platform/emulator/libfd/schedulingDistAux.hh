/*
 *  Authors:
 *    Joerg Wuertz (wuertz@dfki.de)
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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
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

  static OZ_CFunHeader spawner;
public:
  FirstsLasts(OZ_Term, OZ_Term, OZ_Term, OZ_Term, int);
  ~FirstsLasts();
  virtual size_t sizeOf(void) { return sizeof(FirstsLasts); }
  virtual void updateHeapRefs(OZ_Boolean);
  virtual OZ_Return propagate(void);
  virtual OZ_Term getParameters(void) const { RETURN_LIST1(stream); }
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
};



#endif // __SCHED_HH__
