#include "weakdict.hh"
#include "var_future.hh"
#include "atoms.hh"
#include "tagged.hh"
#include "am.hh"

static WeakDictionary* gcLinkedList = 0;

OZ_Term WeakDictionary::printV(int depth)
{
  return oz_pair2(oz_atom("<WeakDictionary n="),
                  oz_pair2(oz_int(table->numelem),
                           oz_atom(">")));
}

OZ_Extension* WeakDictionary::gCollectV()
{
  // copy to the new heap and enter into linked list
  // WeakDictionary's gcRecurseV does nothing.
  // the real worked is done in gCollectWeakDictionaries.
  WeakDictionary* d = new WeakDictionary(table,stream);
  d->next = gcLinkedList;
  gcLinkedList = d;
  return d;
}

OZ_Extension* WeakDictionary::sCloneV() {
  return NULL;
}

inline OZ_Boolean WeakDictionary::get(OZ_Term key,OZ_Term& val)
{
  return ((val = table->lookup(oz_deref(key)))!=0);     // 0 if not found
}

extern int oz_raise(OZ_Term cat, OZ_Term key, const char *label, int arity, ...);

OZ_Return WeakDictionary::getFeatureV(OZ_Term f,OZ_Term& v)
{
  if (!OZ_isFeature(f)) { OZ_typeError(1,"feature"); }
  if (get(f,v)) {
    return PROCEED;
  } else {
    return oz_raise(E_ERROR,E_KERNEL,"WeakDictionary.get",2,
                    oz_makeTaggedExtension(this),f);
  }
}

void WeakDictionary::put(OZ_Term key,OZ_Term val)
{
  if (table->fullTest()) resizeDynamicTable(table);
  if (!table->add(key,val)) {
    resizeDynamicTable(table);
    table->add(key,val);
  }
}

OZ_Return WeakDictionary::putFeatureV(OZ_Term f,OZ_Term  v)
{
  if (!OZ_isFeature(f)) { OZ_typeError(1,"feature"); }
  put(f,v);
  return PROCEED;
}

void gCollectWeakDictionaries()
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
    oz_bindFuture(ptr,oz_nil());
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

inline OZ_Term WeakDictionary::getKeys()
{
  return (table)?table->getKeys():AtomNil;
}

OZ_BI_define(weakdict_keys,1,1)
{
  OZ_declareWeakDict(0,d);
  OZ_RETURN(d->getKeys());
}
OZ_BI_end

inline OZ_Term WeakDictionary::getPairs()
{
  return (table)?table->getPairs():AtomNil;
}

OZ_BI_define(weakdict_entries,1,1)
{
  OZ_declareWeakDict(0,d);
  OZ_RETURN(d->getPairs());
}
OZ_BI_end

inline OZ_Term WeakDictionary::getItems()
{
  return (table)?table->getItems():AtomNil;
}

OZ_BI_define(weakdict_items,1,1)
{
  OZ_declareWeakDict(0,d);
  OZ_RETURN(d->getItems());
}
OZ_BI_end

inline bool WeakDictionary::isEmpty()
{
  return (table==0 || table->numelem==0);
}

OZ_BI_define(weakdict_isempty,1,1)
{
  OZ_declareWeakDict(0,d);
  OZ_RETURN_BOOL(d->isEmpty());
}
OZ_BI_end

inline OZ_Term WeakDictionary::toRecord(OZ_Term label)
{
  return (table)?table->toRecord(label):label;
}

#define OZ_declareLiteral(ARG,VAR)

OZ_BI_define(weakdict_torecord,2,1)
{
  OZ_expectType(0,"Literal",OZ_isLiteral);
  OZ_Term label = OZ_in(0);
  OZ_declareWeakDict(1,d);
  OZ_RETURN(d->toRecord(label));
}
OZ_BI_end

inline void WeakDictionary::remove(OZ_Term key)
{
  if (table) {
    DynamicTable *aux = table->remove(key);
    if (aux!=table) {
      table->dispose();
      table=aux;
    }
  }
}

OZ_BI_define(weakdict_remove,2,0)
{
  OZ_declareWeakDict(0,d);
  OZ_declareFeature(1,key);
  d->remove(key);
  return PROCEED;
}
OZ_BI_end

inline void WeakDictionary::remove_all()
{
  if (table) {
    table->dispose();
    table = DynamicTable::newDynamicTable(DictDefaultSize);
  }
}

OZ_BI_define(weakdict_remove_all,1,0)
{
  OZ_declareWeakDict(0,d);
  d->remove_all();
  return PROCEED;
}
OZ_BI_end

inline bool WeakDictionary::member(OZ_Term key)
{
  return (table->lookup(key) != makeTaggedNULL());
}

OZ_BI_define(weakdict_member,2,1)
{
  OZ_declareWeakDict(0,d);
  OZ_declareFeature(1,key);
  OZ_RETURN_BOOL(d->member(key));
}
OZ_BI_end
