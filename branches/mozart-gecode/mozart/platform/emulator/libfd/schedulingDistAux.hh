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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __SCHEDULING_DIST_AUX_HH__
#define __SCHEDULING_DIST_AUX_HH__

#include "std.hh"

//////////
// FIrsts and Lasts
//////////
class FirstsLasts : public OZ_Propagator {
  friend INIT_FUNC(sched_init);
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

  static OZ_PropagatorProfile profile;
public:
  FirstsLasts(OZ_Term, OZ_Term, OZ_Term, OZ_Term, int);
  ~FirstsLasts();
  virtual size_t sizeOf(void) { return sizeof(FirstsLasts); }
  virtual void gCollect(void);
  virtual void sClone(void);
  virtual OZ_Return propagate(void); 
  virtual OZ_Term getParameters(void) const { RETURN_LIST1(stream); }
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
};



#endif // __SCHEDULING_DIST_AUX_HH__


