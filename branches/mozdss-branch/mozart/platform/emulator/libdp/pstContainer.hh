/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 2002
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

#ifndef __PST_CONTAINER_HH
#define __PST_CONTAINER_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"
#include "value.hh"
#include "dpMarshaler.hh"
#include "dss_object.hh"

class PstContainer{
private:
  PstContainer *a_next;
  PstContainer *a_prev;
public:
  OZ_Term       a_term;
  
  PstContainer(OZ_Term trm);
  virtual ~PstContainer();
  
  virtual void gc() = 0;
  virtual void gcStart() = 0;
  virtual void gcFinish() = 0;

  inline PstContainer *getNext() const { return a_next; }
};

class PstInContainer;

class PstOutContainer: public PstContainer,
		       public PstOutContainerInterface
{
private:
  DPMarshaler *a_marshal_cont; 
  bool a_immediate;

public:
#ifdef INTERFACE
  static int a_allocated;
#endif
  
  PstOutContainer(OZ_Term);
  PstOutContainer(PstInContainer *pst);
  virtual ~PstOutContainer(){
#ifdef INTERFACE
    a_allocated--;
#endif
  }

  // ask for immediate marshaling
  void setImmediate() { a_immediate = true; }

  void gc();
  void gcStart();
  void gcFinish();
  
  // Inherited from the PstOutContainer
  virtual bool marshal(DssWriteBuffer*);
  virtual void resetMarshaling();
  virtual void dispose(){ delete this; };
  virtual PstInContainerInterface* loopBack2In();
  virtual PstOutContainerInterface* duplicate();
};


class PstInContainer:public PstContainer,
		     public PstInContainerInterface
{
private:
  Builder* a_builder_cont;
public:
#ifdef INTERFACE
  static int a_allocated;
#endif

  PstInContainer();
  PstInContainer(PstOutContainer *pst);
  virtual ~PstInContainer(){
#ifdef INTERFACE
    a_allocated--;
#endif
  }

  void gc();
  void gcStart();
  void gcFinish();

  // Comes from the PstInContainerIF
  bool unmarshal(DssReadBuffer*);
  void dispose(){ delete this; };
  PstOutContainerInterface* loopBack2Out();
};


// For the Gc of Arguments(OZ_terms); 
void gcPstContainersStart();
void gcPstContainersRoot();
void gcPstContainersFinish();

#endif


