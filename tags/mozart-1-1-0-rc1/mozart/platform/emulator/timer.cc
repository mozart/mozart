// The idea is to tell the emulator when it should deliver a
// timer event

#include "base.hh"

extern void OZ_setTimer(int);
extern unsigned int osTotalTime();

OZ_BI_define(BItimer_setTimer,1,0)
{
  OZ_declareInt(0,t);
  OZ_setTimer(t);
  return PROCEED;
}
OZ_BI_end

OZ_BI_define(BItimer_mTime,0,1)
{
  OZ_RETURN_INT(osTotalTime());
}
OZ_BI_end
