#include "dictionary.hh"
#include "builtins.hh"

OZ_Term site_dict = 0;

void SitePropertyInit()
{
  if (site_dict==0) {
    site_dict = makeTaggedConst(new OzDictionary(oz_rootBoard()));
    OZ_protect(&site_dict);
  }
}

#define INIT if (site_dict==0) SitePropertyInit()

OZ_BI_define(BIsitePropertyGet,1,1)
{
  OZ_expectType(0,"Feature",OZ_isFeature);
  INIT;
  OZ_Term t = OZ_deref(OZ_in(0));
  TaggedRef out=tagged2Dictionary(site_dict)->getArg(t);
  if (!out)
    return oz_raise(E_SYSTEM,E_KERNEL,"SitePropertyGet",1,OZ_in(0));
  OZ_RETURN(out);
}
OZ_BI_end

OZ_BI_define(BIsitePropertyPut,2,0)
{
  OZ_expectType(0,"Feature",OZ_isFeature);
  INIT;
  OZ_Term t = OZ_deref(OZ_in(0));
  tagged2Dictionary(site_dict)->setArg(t,OZ_in(1));
  return PROCEED;
}
OZ_BI_end

