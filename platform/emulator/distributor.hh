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

  virtual Distributor * gc(void) = 0;
  virtual void dispose(void) = 0;

};


class DistBag {
private:
  Distributor * dist;
  DistBag     * next;
  Bool          isUnary;

  DistBag(Distributor *d, DistBag *b, Bool iu) { 
    dist = d; next = b; isUnary = iu;
  }

  DistBag(Distributor *d, Bool iu) { 
    dist = d; isUnary = iu;
  }

  void dispose(void);
  
public:
  USEFREELISTMEMORY;

  DistBag * addIt(Distributor *d, Bool isUnary);
  DistBag * add(Distributor * d) {
    return addIt(d,NO);
  }

  DistBag * get(Distributor ** d);

  Distributor * getFirst(void) {
    Assert(this);
    return dist;
  }

  DistBag * merge(DistBag * b);

  DistBag * gc(void);

};


#endif



