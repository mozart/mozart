/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __dvar__hh__
#define __dvar__hh__


#if defined(INTERFACE)
#pragma interface
#endif

#include "genvar.hh"
#include "oz.h"

enum PV_TYPES {
  PV_MANAGER,
  PV_PROXY
};

class PerdioVar: public GenCVariable {
  TaggedPtr tagged;
public:
  PerdioVar() : GenCVariable(PerdioVariable) {
    tagged.setType(PV_MANAGER);
  }

  PerdioVar(int i) : GenCVariable(PerdioVariable) {
    tagged.setType(PV_PROXY);
    tagged.setIndex(i);
  }

  Bool isManager() { return tagged.getType()==PV_MANAGER; }
  Bool isProxy() { return !isManager(); }
  int getIndex() { return tagged.getIndex(); }
  void setIndex(int i) { tagged.setIndex(i); }

  Bool valid(TaggedRef *varPtr, TaggedRef v);
  
  size_t getSize(void) { return sizeof(PerdioVar); }


  Bool unifyPerdioVar(TaggedRef * vptr, TaggedRef * tptr, Bool prop);

  void gcPerdioVar(void);
};

inline
Bool isPerdioVar(TaggedRef term)
{
  GCDEBUG(term);
  return isCVar(term) && (tagged2CVar(term)->getType() == PerdioVariable);
}

inline
PerdioVar *tagged2PerdioVar(TaggedRef t) {
  Assert(isPerdioVar(t));
  return (PerdioVar *) tagged2CVar(t);
}

#endif
