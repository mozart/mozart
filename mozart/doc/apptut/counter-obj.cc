#include "mozart.h"

class Counter : public OZ_Extension {
public:
  long * n;
  Counter();
  Counter(long*);
  static int id;
  virtual int getIdV();
  virtual OZ_Term typeV();
  virtual OZ_Extension* gcV();
  virtual OZ_Term printV(int depth = 10);
};

Counter::Counter() { n = new long[1]; n[0]=1; }

OZ_BI_define(counter_new,0,1)
{
  OZ_RETURN(OZ_extension(new Counter));
}
OZ_BI_end

int Counter::id;
int Counter::getIdV() { return id; }

inline OZ_Boolean OZ_isCounter(OZ_Term t)
{
  t = OZ_deref(t);
  return OZ_isExtension(t) &&
    OZ_getExtension(t)->getIdV()==Counter::id;
}

OZ_BI_define(counter_is,1,1)
{
  OZ_declareDetTerm(0,t);
  OZ_RETURN_BOOL(OZ_isCounter(t));
}
OZ_BI_end

OZ_Term Counter::typeV() { return OZ_atom("counter"); } 

inline Counter* OZ_CounterToC(OZ_Term t)
{
  return (Counter*) OZ_getExtension(OZ_deref(t));
}

#define OZ_declareCounter(ARG,VAR) \
OZ_declareType(ARG,VAR,Counter*,"counter",OZ_isCounter,OZ_CounterToC)

OZ_BI_define(counter_get,1,1)
{
  OZ_declareCounter(0,c);
  OZ_RETURN_INT(*c->n);
}
OZ_BI_end

OZ_BI_define(counter_set,2,0)
{
  OZ_declareCounter(0,c);
  OZ_declareInt(1,i);
  *c->n=i;
  return PROCEED;
}
OZ_BI_end

OZ_BI_define(counter_next,1,1)
{
  OZ_declareCounter(0,c);
  long i = *c->n;
  *c->n = i+1;
  OZ_RETURN_INT(i);
}
OZ_BI_end

OZ_Term Counter::printV(int depth = 10)
{
  return OZ_mkTupleC("#",3,
		     OZ_atom("<counter "),
		     OZ_int(*n),
		     OZ_atom(">"));
}

Counter::Counter(long*p):n(p){}
OZ_Extension* Counter::gcV() { return new Counter(n); }

OZ_BI_define(counter_free,1,0)
{
  OZ_declareCounter(0,c);
  free(c->n);
  return PROCEED;
}
OZ_BI_end

OZ_C_proc_interface * oz_init_module(void)
{
  static OZ_C_proc_interface table[] = {
    {"new",0,1,counter_new},
    {"get",1,1,counter_get},
    {"set",2,0,counter_set},
    {"next",1,1,counter_next},
    {"free",1,0,counter_free},
    {0,0,0,0}
  };
  Counter::id = OZ_getUniqueId();
  return table;
}
