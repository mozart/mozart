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

class PerdioVar: public GenCVariable {

public:
  PerdioVar(TaggedRef v) : GenCVariable(PerdioVariable) {}

  Bool valid(TaggedRef *varPtr, TaggedRef v);
  
  size_t getSize(void) { return sizeof(PerdioVar); }


  Bool unifyPerdioVar(TaggedRef * vptr, TaggedRef * tptr, Bool prop);
  void bindPerdioVar(TaggedRef *lPtr, TaggedRef *rPtr);

  void gcPerdioVar(void);
};

inline
Bool isPerdioVar(TaggedRef term)
{
  GCDEBUG(term);
  return isCVar(term) && (tagged2CVar(term)->getType() == PerdioVariable);
}

void handleAsk(TaggedRef *dvar, TaggedRef other = makeTaggedNULL());

#endif
