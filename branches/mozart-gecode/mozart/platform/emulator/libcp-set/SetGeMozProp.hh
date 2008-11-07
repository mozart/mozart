/*
 *  Main authors:
 *     Andres Felipe Barco <anfelbar@univalle.edu.co>
 *
 *  Contributing authors:
 *
 *  Copyright:
 *    Andres Barco, 2008
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

#ifndef __SET_GEMOZ_PROPAGATORS_HH__
#define __SET_GEMOZ_PROPAGATORS_HH__

#include "gecode/kernel.hh"
//#include "gecode/int.hh"
#include "gecode/set.hh"
//#include "gecode/int/linear.hh"
//#include "gecode/int/rel.hh"


using namespace Gecode;
using namespace Gecode::Set;

/**
   Propagator class for FS.monitors. This is not implemented by gecode.
   This is the base class for monitors.
 */

class GeMonitor : public UnaryPropagator<SetView,PC_SET_ANY> {

protected:
  //SetValueM present;
  OZ_Term _stream;
  SetView x0;
  bool isInMonitor;
  
public:
  /// Constructor for cloning \a p
  GeMonitor(Gecode::Space* home, bool share,GeMonitor& cp) :
    Gecode::UnaryPropagator<Gecode::Set::SetView, PC_SET_ANY>(home, share, cp), 
    x0(cp.x0), _stream(cp._stream) {}

  /// Constructor for posting
  GeMonitor(Gecode::Space* home, SetView v0, OZ_Term stream) :
    Gecode::UnaryPropagator<Gecode::Set::SetView, PC_SET_ANY>(home, v0), _stream(stream), x0(v0) {}
  
  /// Copy propagator during cloning
  virtual Actor* copy(Gecode::Space* home, bool share){
    return new(home) GeMonitor(home, share, *this);
  }
  /// Perform propagation
  virtual ExecStatus propagate(Gecode::Space* home, ModEventDelta med);

  void isInOrOut(bool inOut){
    isInMonitor = inOut;
  }

};

Gecode::ExecStatus GeMonitor::propagate(Gecode::Space *s, Gecode::ModEventDelta)
{  
   OZ_Stream stream(_stream);
   bool vanish = false;
  // find elements imposed from outside to stream and constrain
  // _fsetvar_ appropriately
  while (!stream.isEostr()) {
    OZ_Term elem = stream.get();
    if (OZ_isSmallInt(elem)) {
      int e = OZ_intToC(elem);
      if(isInMonitor){
	GECODE_ME_CHECK(x0.include(s,e));
      } else {
	GECODE_ME_CHECK(x0.exclude(s,e));
      }
    } else {
      stream.fail();
      return ES_FAILED;
    }
  }

  // now the other way around: 
  // check if stream is closed, i.e., _fsetvar_ becomes determined
  if (stream.isClosed()) {
    static int max_card = lim_sup  + 1;
    // fsetvar has to become a value
    int known_in = (isInMonitor
		    ? x0.glbSize()
		    : max_card - x0.unknownSize());
    GECODE_ME_CHECK(x0.cardMin(s, known_in));
    GECODE_ME_CHECK(x0.cardMax(s, known_in));
    vanish = true;
  } else { // if the stream is _not_ closed ...
    OZ_Term tail = stream.getTail();
    //for now, only take the elemets from 0 to lim_sup... avoid block.
    SetVar notPresents(s, Gecode::IntSet::empty, 0, lim_sup);
    SetView svv(notPresents);

    // append new elements to tail of the stream
    //put in stream all new known values in the set, we use an iterator
    if(isInMonitor){
      for(Gecode::SetVarGlbValues itr(x0); itr(); ++itr){
	tail = stream.put(tail, OZ_int(itr.val()));
      }
    } else {
      Gecode::SetVarLubRanges itrr(x0);
      int mini = itrr.min();  int maxi = itrr.max();
      //exclude range of values that are in the UpperBound of the
      //set variables from the temporal SetView svv
      GECODE_ME_CHECK(svv.exclude(s, itrr.min(), itrr.max()));
           
      //iterate the LubValues of temporal SetView and put the values
      // in the stream (monitorOut behaviour)
      for(Gecode::SetVarLubValues itr11(svv); itr11(); ++itr11){
	tail = stream.put(tail, OZ_int(itr11.val()));
      }
    }
      
    if(x0.assigned()){
      if (OZ_unify(tail, OZ_nil()) == FAILED){
	stream.fail();
	return ES_FAILED;
      }
      vanish = true;
    }
    
    
  }
  
  stream.leave();
  _stream = stream.getTail();
  return vanish ? ES_FIX : ES_NOFIX;
}

  
/**
   Propagator class for FS.monitorIn. This is not implemented by gecode.
   anfelbar@: This monitor set the GeMonitor::isInMonitor in TRUE.
   Should we change this and eliminate this class?? Maybe another argument
   to the constructor will do the trick.
 */
class GeMonitorIn : public GeMonitor {  
public:
  /// Constructor for cloning \a p
  GeMonitorIn(Gecode::Space* home, bool share,GeMonitorIn& cp) :
    GeMonitor(home, share, cp)
  {
    GeMonitor::isInOrOut(true);
  }

  /// Constructor for posting
  GeMonitorIn(Gecode::Space* home, SetView v0, OZ_Term stream) :
    GeMonitor(home, v0, stream)
  {
    GeMonitor::isInOrOut(true);
  }
  
  /// Copy propagator during cloning
  virtual Actor* copy(Gecode::Space* home, bool share){
    return new(home) GeMonitorIn(home, share, *this);
  }
  /// Perform propagation
  virtual ExecStatus propagate(Gecode::Space* home, ModEventDelta med){
    return GeMonitor::propagate(home, med);
  }      
  
};


/**
   Propagator class for FS.monitorOut. This is not implemented by gecode.
   anfelbar@: This monitor set the GeMonitor::isInMonitor in FALSE.
   Should we change this and eliminate this class?? Maybe another argument
   to the constructor will do the trick.
 */
class GeMonitorOut : public GeMonitor {  
public:
  /// Constructor for cloning \a p
  GeMonitorOut(Gecode::Space* home, bool share,GeMonitorOut& cp) :
    GeMonitor(home, share, cp)
  {
    GeMonitor::isInOrOut(false);
  }

  /// Constructor for posting
  GeMonitorOut(Gecode::Space* home, SetView v0, OZ_Term stream) :
    GeMonitor(home, v0, stream)
  {
    GeMonitor::isInOrOut(false);
  }
  
  /// Copy propagator during cloning
  virtual Actor* copy(Gecode::Space* home, bool share){
    return new(home) GeMonitorOut(home, share, *this);
  }
  /// Perform propagation
  virtual ExecStatus propagate(Gecode::Space* home, ModEventDelta med){
    return GeMonitor::propagate(home, med);
  }      
  
};

void MonitorIn(Gecode::Space *gs, Gecode::Set::SetView sv, OZ_Term stream);
void MonitorOut(Gecode::Space *gs, Gecode::Set::SetView sv, OZ_Term stream);

#endif /* __SET_GEMOZ_PROPAGATORS_HH__  */

