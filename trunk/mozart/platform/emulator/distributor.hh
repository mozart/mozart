/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Christian Schulte, 1997
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

#ifndef __DISTRIBUTOR_H_
#define __DISTRIBUTOR_H_

#ifdef INTERFACE
#pragma interface
#endif

#include "mem.hh"

class Distributor {
public:
  USEFREELISTMEMORY;
  
  virtual int getAlternatives(void) = 0;
  virtual int commit(Board *, int, int) = 0;

  virtual Distributor * gCollect(void) = 0;
  virtual Distributor * sClone(void) = 0;

  virtual void dispose(void) = 0;

};


class DistBag {
private:
  Distributor * dist;
  DistBag     * next;

  DistBag(Distributor *d, DistBag *b) { 
    dist = d; next = b;
  }

  DistBag(Distributor *d) { 
    dist = d;
  }

  void dispose(void);
  
public:
  USEFREELISTMEMORY;

  DistBag * add(Distributor * d);
  
  DistBag * get(Distributor ** d);
  
  Distributor * getFirst(void) {
    Assert(this);
    return dist;
  }

  DistBag * merge(DistBag * b);

  DistBag * gCollect(void);
  DistBag * sClone(void);

};


#endif



