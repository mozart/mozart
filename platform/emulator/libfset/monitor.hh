/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __MONITOR_HH__
#define __MONITOR_HH__

#include "fsstd.hh"

/*
class MonitorInPropagator : public OZ_Propagator {
  friend INIT_FUNC(fsp_init);
private:
  OZ_FSetValue _in_sofar;
  OZ_Term _fsetvar, _stream;
  static OZ_PropagatorProfile profile;
public:
  MonitorInPropagator(OZ_Term fsetvar, OZ_Term stream)
    : _fsetvar(fsetvar) , _stream(stream), _in_sofar(fs_empty) { }

  virtual size_t sizeOf(void) { return sizeof(MonitorInPropagator); }
  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_updateHeapTerm(_stream);
    OZ_updateHeapTerm(_fsetvar);
  }
  virtual OZ_Return propagate(void);
  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_fsetvar, OZ_cons(_stream, OZ_nil()));
  }
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
};
*/

class MonitorPropagator : public OZ_Propagator {
private:
  OZ_FSetValue _present_sofar;
  OZ_Term _fsetvar, _stream;

public:
  MonitorPropagator(OZ_Term fsetvar, OZ_Term stream)
    : _fsetvar(fsetvar) , _stream(stream), _present_sofar(fs_empty) { }

  virtual size_t sizeOf(void) { return sizeof(MonitorPropagator); }

  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_updateHeapTerm(_stream);
    OZ_updateHeapTerm(_fsetvar);
  }

  virtual OZ_Return propagate(OZ_Boolean is_inprop);

  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_fsetvar, OZ_cons(_stream, OZ_nil()));
  }
};

//-----------------------------------------------------------------------------

class MonitorInPropagator : public MonitorPropagator {
  friend INIT_FUNC(fsp_init);

private:
  static OZ_PropagatorProfile profile;

public:
  MonitorInPropagator(OZ_Term fsetvar, OZ_Term stream)
    : MonitorPropagator(fsetvar, stream) { }

  virtual OZ_Return propagate(void) {
    return MonitorPropagator::propagate(OZ_TRUE);
  }

  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
};

class MonitorOutPropagator : public MonitorPropagator {
  friend INIT_FUNC(fsp_init);

private:
  static OZ_PropagatorProfile profile;

public:
  MonitorOutPropagator(OZ_Term fsetvar, OZ_Term stream)
    : MonitorPropagator(fsetvar, stream) { }

  virtual OZ_Return propagate(void) {
    return MonitorPropagator::propagate(OZ_FALSE);
  }
  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }
};

#endif /* __MONITOR_HH__ */

//-----------------------------------------------------------------------------
// eof
