/*
 *  Authors:
 *    Leif Kornstaedt <kornstae@ps.uni-sb.de>
 *
 *  Copyright:
 *    Leif Kornstaedt, 1999
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation of Oz 3:
 *    http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *    http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#include "base.hh"
#include "am.hh"
#include "value.hh"
#include "dictionary.hh"
#include "gname.hh"
#include "site.hh"
#include "mozart.h"

OZ_Term string2Builtin(const char *);
void makeFSetValue(OZ_Term, OZ_Term *);

//--** define a macro for OZ_declareGName(0, gname, &ret)

static GName *makeGName(OZ_Term tup, OZ_Term *ret) {
  int ip = OZ_intToC(OZ_subtree(tup, oz_int(1)));
  int stamp = OZ_intToC(OZ_subtree(tup, oz_int(2)));
  int pid = OZ_intToC(OZ_subtree(tup, oz_int(3)));
  int i1 = OZ_intToC(OZ_subtree(tup, oz_int(4)));
  int i2 = OZ_intToC(OZ_subtree(tup, oz_int(5)));
  int type = OZ_intToC(OZ_subtree(tup, oz_int(6)));

  TimeStamp timeStamp(stamp, pid);
  Site tryS(ip, 0, timeStamp);
  Site *site = siteTable->find(&tryS);
  if (site == NULL) {
    site = new Site(&tryS);
    siteTable->insert(site);
  }

  GName gname;
  gname.site = site;
  gname.id.setNumber(1, i1);
  gname.id.setNumber(0, i2);
  gname.gnameType = (GNameType) type;

  OZ_Term aux = oz_findGName(&gname);
  if (aux != makeTaggedNULL()) {
    *ret = aux;
    return NULL;
  } else
    return new GName(gname);
}

OZ_BI_define(compat_importFloat, 2, 1)
{
  OZ_declareInt(0, i1);
  OZ_declareInt(1, i2);

  union {
    unsigned char c[sizeof(double)];
    int i[sizeof(double) / sizeof(int)];
    double d;
  } x;

  x.i[0] = 1;
  if (x.c[0] == 1) {   // little endian
    x.i[0] = i1;
    x.i[1] = i2;
  } else {
    x.i[0] = i2;
    x.i[1] = i1;
  }
  OZ_RETURN(OZ_float(x.d));
}
OZ_BI_end

OZ_BI_define(compat_importName, 2, 1)
{
  OZ_Term ret;
  GName *gname = makeGName(OZ_in(1), &ret);
  if (gname) {
    OZ_declareVirtualString(0, printName);
    Name *nm;
    if (printName[0] == '\0')
      nm = Name::newName(am.currentBoard());
    else
      nm = NamedName::newNamedName(strdup(printName));
    nm->import(gname);

    ret = makeTaggedLiteral(nm);
    addGName(gname, ret);
  }
  OZ_RETURN(ret);
}
OZ_BI_end

OZ_BI_define(compat_importBuiltin, 1, 1)
{
  OZ_declareVirtualString(0,name);
  OZ_RETURN(string2Builtin(name));   //--** check whether it existed
}
OZ_BI_end

OZ_BI_define(compat_importClass, 3, 1)
{
  OZ_Term ret;
  GName *gname = makeGName(OZ_in(0), &ret);
  if (gname) {
    OZ_declareInt(1, flags);
    TaggedRef t_feat = oz_deref(OZ_in(2));
    SRecord *feat = tagged2SRecord(t_feat);

    ObjectClass *cl =
      new ObjectClass(makeTaggedNULL(), 
		      makeTaggedNULL(), 
		      makeTaggedNULL(), 
		      makeTaggedNULL(), 
		      NO, NO, am.currentBoard());
    cl->setGName(gname);

    ret = makeTaggedConst(cl);
    addGName(gname, ret);

    TaggedRef ff = oz_deref(feat->getFeature(NameOoFeat));
    cl->import(t_feat,
	       oz_deref(feat->getFeature(NameOoFastMeth)),
	       oz_isSRecord(ff) ? ff: makeTaggedNULL(),
	       oz_deref(feat->getFeature(NameOoDefaults)),
	       flags);
  }
  OZ_RETURN(ret);
}
OZ_BI_end

OZ_BI_define(compat_importChunk, 2, 1)
{
  OZ_Term ret;
  GName *gname = makeGName(OZ_in(0), &ret);
  if (gname) {
    SChunk *sc = new SChunk(am.currentBoard(), 0);
    sc->setGName(gname);

    ret = makeTaggedConst(sc);
    addGName(gname, ret);

    sc->import(oz_deref(OZ_in(1)));
  }
  OZ_RETURN(ret);
}
OZ_BI_end

OZ_BI_define(compat_importFSetValue, 1, 1)
{
  OZ_Term ret;
  makeFSetValue(OZ_in(0), &ret);
  OZ_RETURN(ret);
}
OZ_BI_end


/*
 * The builtin table
 */

#ifndef MODULES_LINK_STATIC

#include "modCompat-if.cc"

#endif
