#include "base.hh"
#include "var_future.hh"
#include "am.hh"

OZ_Term event_stream;

void initEvents()
{
  event_stream = oz_newFuture(oz_rootBoard());
  oz_protect(&event_stream);
}

void OZ_eventPush(OZ_Term e)
{
  OZ_Term tail = oz_newFuture(oz_rootBoard());
  DEREF(event_stream,ptr,_);
  oz_bindFuture(ptr,oz_cons(e,tail));
  event_stream = tail;
}

OZ_BI_define(BIgetEventStream,0,1)
{
  OZ_RETURN(event_stream);
} OZ_BI_end
