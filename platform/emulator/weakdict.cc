#include "weakdict.hh"
#include "var_future.hh"
#include "atoms.hh"
#include "tagged.hh"
#include "am.hh"

static WeakDictionary* gcLinkedList;

OZ_Term WeakDictionary::printV(int depth)
{
  return oz_pair2(oz_atom("<WeakDictionary n="),
                  oz_pair2(oz_int(table->numelem),
                           oz_atom(">")));
}

OZ_Extension* WeakDictionary::gcV()
{
  // copy to the new heap and enter into linked list
  // WeakDictionary's gcRecurseV does nothing.
  // the real worked is done in gcWeakDictionaries.
  WeakDictionary* d = new WeakDictionary(table,stream);
  d->next = gcLinkedList;
  gcLinkedList = d;
  return d;
}

void gcWeakDictionaries()
{
  // This is called after the 1st gc phase has completed.
  // all data reachable outside of weak dictionaries has
  // been marked and copied.  Entries in a weak dictionary
  // now point either to marked values (that have already
  // been copied to the new heap) or to unmarked values
  // (which are no longer reachable from outside).  The
  // weak dictionaries to be processed have been linked
  // together in gcLinkedList.
  for (;gcLinkedList;gcLinkedList=gcLinkedList->next)
    gcLinkedList->weakGC();
  // now gcLinkedList==0 again
}

int oz_raise(OZ_Term cat, OZ_Term key, char *label, int arity, ...);

OZ_BI_define(weakdict_new,0,2)
{
  if (!OZ_onToplevel())
    return oz_raise(E_ERROR,E_KERNEL,"globalState",1,oz_atom("weakDictionary"));
  OZ_Term srm = oz_newFuture(oz_rootBoard());
  WeakDictionary* wd = new WeakDictionary(srm);
  OZ_out(0) = srm;
  OZ_out(1) = OZ_extension(wd);
  return PROCEED;
}
OZ_BI_end

inline int oz_isWeakDictionary(OZ_Term t)
{
  t = OZ_deref(t);
  return OZ_isExtension(t) &&
    OZ_getExtension(t)->getIdV()==OZ_E_WEAKDICTIONARY;
}

inline WeakDictionary* tagged2WeakDictionary(OZ_Term t)
{
  Assert(oz_isWeakDictionary(t));
  return (WeakDictionary*) OZ_getExtension(OZ_deref(t));
}

extern int OZ_isFinalizable(OZ_Term&);

#define OZ_declareWeakDict(ARG,VAR) \
OZ_declareType(ARG,VAR,WeakDictionary*,"weakDictionary",\
        oz_isWeakDictionary,tagged2WeakDictionary)

#define NO_COERCE(X) X

#define OZ_declareFeature(ARG,VAR) \
OZ_declareType(ARG,VAR,OZ_Term,"feature",OZ_isFeature,NO_COERCE)

OZ_BI_define(weakdict_put,3,0)
{
 if (!OZ_onToplevel())
    return oz_raise(E_ERROR,E_KERNEL,"globalState",1,oz_atom("weakDictionary"));
 OZ_declareWeakDict(0,d);
 OZ_declareDetTerm(1,k);
 OZ_declareTerm(2,v);
 TaggedRef w = v;
 DEREF(w,w_ptr,w_tag);
 if (isUVar(w_tag)) {
   // we must bind the UVAR to a CVAR (simple)
   OZ_Return r = oz_unify(makeTaggedRef(newTaggedCVar(oz_newSimpleVar(tagged2VarHome(*w_ptr)))),
                          v);
   if (r!=PROCEED) return r;
 }
 d->put(k,v);
 return PROCEED;
}
OZ_BI_end

void WeakDictionary::put(OZ_Term key,OZ_Term val)
{
  if (table->fullTest()) resizeDynamicTable(table);
  if (!table->add(key,val)) {
    resizeDynamicTable(table);
    table->add(key,val);
  }
}

OZ_BI_define(weakdict_get,2,1)
{
  OZ_declareWeakDict(0,d);
  OZ_declareFeature(1,k);
  if (!OZ_onToplevel() && !d->isLocal())
    return oz_raise(E_ERROR,E_KERNEL,"globalState",1,oz_atom("weakDictionary"));
  OZ_Term v;
  if (!d->get(k,v))
    return oz_raise(E_SYSTEM,E_KERNEL,"weakDictionary",2,OZ_in(0),OZ_in(1));
  OZ_RETURN(v);
}
OZ_BI_end

OZ_BI_define(weakdict_condGet,3,1)
{
  OZ_declareWeakDict(0,d);
  OZ_declareFeature(1,k);
  if (!OZ_onToplevel() && !d->isLocal())
    return oz_raise(E_ERROR,E_KERNEL,"globalState",1,oz_atom("weakDictionary"));
  OZ_Term v;
  if (!d->get(k,v)) OZ_RETURN(OZ_in(2));
  OZ_RETURN(v);
}
OZ_BI_end

OZ_Boolean WeakDictionary::get(OZ_Term key,OZ_Term& val)
{
  return ((val = table->lookup(oz_deref(key)))!=0);     // 0 if not found
}

OZ_BI_define(weakdict_is,1,1)
{
  OZ_declareDetTerm(0,t);
  OZ_RETURN_BOOL(oz_isWeakDictionary(t));
}
OZ_BI_end

void WeakDictionary::close()
{
  if (stream) {
    DEREF(stream,ptr,_);
    oz_bindFuture(ptr,OZ_nil());
    stream=0;
  }
}

OZ_BI_define(weakdict_close,0,0)
{
  OZ_declareWeakDict(0,d);
  if (!OZ_onToplevel() && !d->isLocal())
    return oz_raise(E_ERROR,E_KERNEL,"globalState",1,oz_atom("weakDictionary"));
  d->close();
  return PROCEED;
}
OZ_BI_end

OZ_C_proc_interface * oz_init_module(void)
{
  static OZ_C_proc_interface table[] = {
    {"is"     ,1,1,weakdict_is     },
    {"put"    ,2,0,weakdict_put    },
    {"get"    ,2,1,weakdict_get    },
    {"condGet",3,1,weakdict_condGet},
    {"close"  ,0,0,weakdict_close  },
    {0,0,0,0}
  };
  gcLinkedList = 0;
  return table;
}

char oz_module_name[] = "WeakDictionary";
