/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Christian Schulte, 1999
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

#if defined(INTERFACE)
#pragma implementation "distributor.hh"
#endif

#include "distributor.hh"

DistBag * DistBag::addIt(Distributor * d, Bool isUnary) {
  // Gives preferences to unary distributors: they are inserted
  // at the head

  if (isUnary || !this || dist->getAlternatives()>1) {

    return new DistBag(d,this,isUnary);
    
  } else {
    
    DistBag * pb = this;
    DistBag * db = pb->next;
    
    while (db && db->isUnary) {
      pb = db; db = db->next;
    }

    pb->next = new DistBag(d,db,NO);

    return this;

  }

}

inline
void DistBag::dispose(void) {
  freeListDispose(this,sizeof(DistBag));
}

DistBag * DistBag::get(Distributor ** gd) {

  if (this) {
    DistBag * db = this;

    while (db) {
      
      Distributor * d = db->dist;
      DistBag * t;

      if (db->isUnary) {
	t = db;
	db = db->next;
	t->dispose();
	*gd=d;
	return db;
      }
      
      if (d->getAlternatives()>1) {
	*gd=d;
	return db;
      } 
	
      d->dispose(); 

      t = db;
      db = db->next;

      t->dispose();

    }

    return db;
    
  } else {
    return this;
  }

}

DistBag * DistBag::merge(DistBag * db) {
  if (this) {

    DistBag * m = this;
    DistBag * t;

    while (db) {
      m  = m->addIt(db->dist, db->isUnary);
      t = db;
      db = db->next;
      t->dispose();
    }

    return m;

  } else {
    return db;
  }
}
