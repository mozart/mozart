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
#include "gecode/set.hh"
#include "gecode/set/int.hh"
#include "gecode/set/rel.hh"

using namespace Gecode;
//using namespace Gecode::Int;
using namespace Gecode::Set;
using namespace Gecode::Set::Int;
//using namespace Gecode::Set::Rel;


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


/**
   Propagator class for FS.int.minN. This is not implemented by gecode.
*/
//TODO: Inherits form Set::Int::Match is the best way?
//Should we inherits from Gecode::Propagator?
class IntMinNElement : 
  public Gecode::Set::Int::Match< SetView > {

protected:
  using Match<SetView>::x0;
  using Match<SetView>::xs;

public:
  /// Constructor for cloning \a p
  IntMinNElement(Gecode::Space* home, bool share, IntMinNElement& p)
    : Match<SetView> (home, share, p) {}

  /// Constructor for posting
  IntMinNElement(Gecode::Space* home, SetView y0, ViewArray<Gecode::Int::IntView> y1)
    : Match<SetView> (home, y0, y1) {}


  /// Copy propagator during cloning
  virtual Actor* copy(Gecode::Space* home, bool share){
    return new (home) IntMinNElement(home,share,*this);
  }
  
  /// Perform propagation
  virtual ExecStatus propagate(Gecode::Space* home, ModEventDelta med);

};

/**This method is an adaptation of min from gecode, 
 * see http://www.gecode.org/gecode-doc-latest/minmax_8icc-source.html#l00100
 * to obtain more information
 */
Gecode::ExecStatus
IntMinNElement::propagate(Gecode::Space *home, ModEventDelta){
  //x1 is an element of x0.ub
  //x1 =< smallest element of x0.lb
  //x1 =< x0.cardinialityMin-est largest element of x0.ub
  //(these 2 take care of determined x0)
  //No element in x0 is smaller than x1
  //if x1 is determined, it is part of the ub.

  //if the intvararg has more variables than maximal cardinatity
  // the propagators is failed. Also in the builtin define.
  if(x0.cardMax() < xs.size()){
    return ES_FAILED;
  }
  
  
  LubRanges<SetView> ub(x0);
  for(int i=0; i < xs.size(); i++){
    GECODE_ME_CHECK(xs[i].inter_r(home,ub,false));
  }
  
  
  Gecode::SetVarGlbValues svg(x0);
  for(int i=0; svg() && i < xs.size(); ++svg, ++i){
    GECODE_ME_CHECK(xs[i].lq(home,svg.val()));
  }
  
  //if cardMin>lbSize?
  //assert(x0.cardMin()>=1);
  
  {
    /// Compute n-th largest element in x0.lub for n = x0.cardMin()-1
    int size = 0;
    int maxN = BndSet::MAX_OF_EMPTY;
    for (LubRanges<SetView> ubr(x0); ubr(); ++ubr, ++size) {}
    GECODE_AUTOARRAY(int, ub, size*2);

    int i=0;
    for (LubRanges<SetView> ubr(x0); ubr(); ++ubr, ++i) {
      ub[2*i]   = ubr.min();
      ub[2*i+1] = ubr.max();
    }

    int x0cm = x0.cardMin()-1;
    for (int i=size; i--;) {
      int width = ub[2*i+1]-ub[2*i]+1;
      if (width > x0cm) {
	maxN = ub[2*i+1]-x0cm;
	break;
      }
      x0cm -= width;
    }
    
    for(int p=0; p < xs.size(); p++){
      GECODE_ME_CHECK(xs[p].lq(home,maxN));
    }
  }
  
  //min-1 of the firts element must not be in the set
  GECODE_ME_CHECK(x0.exclude(home,
			     Set::Limits::min, xs[0].min()-1) );
  
  //this loop ensure every arraty argument to 
  //be greater or equal than the element before it
  for(int p=0; p<xs.size()-1; p++){
    GECODE_ME_CHECK(xs[p+1].gq(home, xs[p].min()+1));
  }

  //this loop ensure every arraty argument to 
  //be less or equal than the element before it
  for(int p=1; p<xs.size(); p++){
    GECODE_ME_CHECK(xs[p-1].lq(home, xs[p].max()-1));
  }

  //this loop ensure that no value between array arguments
  // are part of the set
  for(int p=0; p<xs.size()-1; p++){
    int beg = xs[p].max()+1;
    int end = xs[p+1].min()-1;
    if(end - beg >= 0)
      GECODE_ME_CHECK(x0.exclude(home, beg, end));
  }

  
  int det = 0;
  for(int p=0; p<xs.size(); p++){
    if (xs[p].assigned()) {
      GECODE_ME_CHECK(x0.include(home,xs[p].val()));
      ++det;
    }
  }
  
  if(det == xs.size())
    return ES_SUBSUMED(this,home);
  
  return ES_FIX;
  
}

void IntMinN(Gecode::Space *gs, Gecode::Set::SetView sv, ViewArray<Gecode::Int::IntView> iva);


/**
   Propagator class for FS.int.maxN. This is not implemented by gecode.
*/
//TODO: Inherits form Set::Int::Match is the best way?
//Should we inherits from Gecode::Propagator?
class IntMaxNElement : 
  public Gecode::Set::Int::Match< SetView > {

protected:
  using Match<SetView>::x0;
  using Match<SetView>::xs;

public:
  /// Constructor for cloning \a p
  IntMaxNElement(Gecode::Space* home, bool share, IntMaxNElement& p)
    : Match<SetView> (home, share, p) {}

  /// Constructor for posting
  IntMaxNElement(Gecode::Space* home, SetView y0, ViewArray<Gecode::Int::IntView> y1)
    : Match<SetView> (home, y0, y1) {}


  /// Copy propagator during cloning
  virtual Actor* copy(Gecode::Space* home, bool share){
    return new (home) IntMaxNElement(home,share,*this);
  }
  
  /// Perform propagation
  virtual ExecStatus propagate(Gecode::Space* home, ModEventDelta med);

};

/**This method is an adaptation of max from gecode, 
 * see http://www.gecode.org/gecode-doc-latest/minmax_8icc-source.html#l00208
 * to obtain more information
 */
Gecode::ExecStatus
IntMaxNElement::propagate(Gecode::Space *home, ModEventDelta){
  //if the intvararg has more variables than maximal cardinatity
  // the propagators is failed. Also in the builtin define.
  if(x0.cardMax() < xs.size()){
    return ES_FAILED;
  }  
  
  LubRanges<SetView> ub(x0);
  for(int i=0; i < xs.size(); i++){
    GECODE_ME_CHECK(xs[i].inter_r(home,ub,false));
  }
  
  //In vdo we put the values glb in descending order
  int vdoSize = xs.size() > x0.glbSize() ? x0.glbSize() : xs.size();
  int vdo[vdoSize];
  
  Gecode::SetVarGlbValues svg(x0);
  for(int i=0; svg() && i < xs.size(); ++svg, i++){
    vdo[vdoSize-1-i] = svg.val();
  }

  //Restrict domain of Intvars to be greater or equal
  //than a minimal allowed value
  for(int i=0; i<vdoSize; i++){
    GECODE_ME_CHECK(xs[xs.size()-1-i].gq(home, vdo[i]));
  }
  
  for(int i=0; i<xs.size() && (x0.cardMin()-i-1) > 0; i++){
    GECODE_ME_CHECK(xs[xs.size()-1-i].gq(home,x0.lubMinN(x0.cardMin()-1-i)));
  }

  GECODE_ME_CHECK(x0.exclude(home,
			     xs[xs.size()-1].max()+1,Set::Limits::max) );


  //this loop ensure that no value between array arguments
  // are part of the set
  for(int p=0; p<xs.size()-1; p++){
    int beg = xs[p].max()+1;
    int end = xs[p+1].min()-1;
    if(end - beg >= 0)
      GECODE_ME_CHECK(x0.exclude(home, beg, end));
  }

  
  //this loop ensure every arraty argument to 
  //be greater or equal than the element before it
  for(int p=0; p<xs.size()-1; p++){
    GECODE_ME_CHECK(xs[p+1].gq(home, xs[p].min()+1));
  }

  //this loop ensure every arraty argument to 
  //be less or equal than the element after it
  for(int p=1; p<xs.size(); p++){
    GECODE_ME_CHECK(xs[p-1].lq(home, xs[p].max()-1));
  }

  
  int det = 0;
  for(int p=0; p<xs.size(); p++){
    if (xs[p].assigned()) {
      GECODE_ME_CHECK(x0.include(home,xs[p].val()));
      ++det;
    }
  }
  
  if(det == xs.size())
    return ES_SUBSUMED(this,home);
  
  return ES_FIX;
  
}

void IntMaxN(Gecode::Space *gs, Gecode::Set::SetView sv, ViewArray<Gecode::Int::IntView> iva);


  /**
   * \brief %Reified isIn propagator
   * This is a adaptation of ReSubSet propagator form gecode.
   * The change is in the propagator actuation.
   */
class ReifiedIsIn :
  public Gecode::Set::Rel::ReSubset<SingletonView,SetView> {
public:
  using Gecode::Set::Rel::ReSubset<SingletonView,SetView>::x0;
  using Gecode::Set::Rel::ReSubset<SingletonView,SetView>::x1;
  using Gecode::Set::Rel::ReSubset<SingletonView,SetView>::b;

  /// Constructor for cloning \a p
  ReifiedIsIn(Gecode::Space* home, bool share, ReifiedIsIn& ri)
    : Gecode::Set::Rel::ReSubset<SingletonView,SetView> (home, share, ri) {}
  
  /// Constructor for posting
  ReifiedIsIn(Gecode::Space* home, SingletonView iv, SetView sv, BoolView bv)
    : Gecode::Set::Rel::ReSubset<SingletonView,SetView> (home, iv, sv, bv) {}
    
  /// Copy propagator during cloning
  virtual Actor* copy(Gecode::Space* home, bool share){
    return new (home) ReifiedIsIn(home,share,*this);
  }
    
  /// Perform propagation
  virtual ExecStatus propagate(Gecode::Space* home, ModEventDelta med);
};


ExecStatus ReifiedIsIn::propagate(Gecode::Space* home, ModEventDelta){  
  // check whether cardinalities still allow subset
  if (x0.cardMin() > x1.cardMax()) {
    GECODE_ME_CHECK(b.zero_none(home));
    return ES_SUBSUMED(this,home);
  }
  
  // check lub(x0) subset glb(x1)
  {
    LubRanges<SingletonView> x0ub(x0);
    GlbRanges<SetView> x1lb(x1);
    Iter::Ranges::Diff<LubRanges<SingletonView>,GlbRanges<SetView> > d(x0ub,x1lb);
    if (!d()) {
      GECODE_ME_CHECK(b.one_none(home));
      return ES_SUBSUMED(this,home);
    }
  }
  
  // check glb(x0) subset lub(x1)
  {
    GlbRanges<SingletonView> x0lb(x0);
    LubRanges<SetView> x1ub(x1);
    Iter::Ranges::Diff<GlbRanges<SingletonView>,LubRanges<SetView> > d(x0lb,x1ub);
    if (d()) {
      GECODE_ME_CHECK(b.zero_none(home));
      return ES_SUBSUMED(this,home);
    } else if (x0.assigned() && x1.assigned()) {
      GECODE_ME_CHECK(b.one_none(home));
      return ES_SUBSUMED(this,home);
    }
  }
  
  if (x0.cardMin() > 0) {
    LubRanges<SingletonView> x0ub(x0);
    LubRanges<SetView> x1ub(x1);
    Iter::Ranges::Inter<LubRanges<SingletonView>,LubRanges<SetView> >
      i(x0ub,x1ub);
    if (!i()) {
      GECODE_ME_CHECK(b.zero_none(home));
      return ES_SUBSUMED(this,home);
    }
  }
  
  if (b.one())
    GECODE_REWRITE(this,(Gecode::Set::Rel::SubSet<SingletonView,SetView>::post(home,x0,x1)));
  
  if (b.zero())
    GECODE_REWRITE(this,(Gecode::Set::Rel::NoSubSet<SingletonView,SetView>::post(home,x0,x1)));
  

  return ES_FIX;
}



void IsInReified(Gecode::Space *gs, Gecode::Set::SetView sv, SingletonView sin, BoolView bv);

#endif /* __SET_GEMOZ_PROPAGATORS_HH__  */

