/*
 *  Authors:
 *    Konstantin Popov <kost@sics.se>
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Konstantin Popov (2000)
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __VAR_CLASS__HH__
#define __VAR_CLASS__HH__

#if defined(INTERFACE)
#pragma interface
#endif

#include "var_lazy.hh"

//
// 'DSiteList' keeps (d)sites the requests for the class were already
// sent to, so sites are asked exactly once for a particular class.
// This is necessary for the lazy objects protocol since (a) the
// object's class has to be known before an object's proxy is
// converted to an object; (b) we cann't afford sending a
// "give-me-the-class" request for each object of the same class; (c)
// objects of the same class can come from different sites, so a
// reliable way to guarantee the point (a) is to ask for the class
// exactly the site where the object came from.
class DSiteList {
private:
  //
  DSite *ds;
  DSiteList *next;

public:
  DSiteList(DSite *dsIn, DSiteList *nIn)
    : ds(dsIn), next(nIn)
  {}

  DSite *getDSite() { return (ds); }
  DSiteList *getNext() { return (next); }
};

//
class ClassVar : public LazyVar {
private:
  DSiteList *dsl;

private:
  Bool lookupDSite(DSite *ds) {
    DSiteList *l = dsl;
    while (l) {
      if (l->getDSite() == ds)
        return (OK);
      else
        l = l->getNext();
    }
    return (NO);
  }
  void addDSite(DSite *ds) {
    dsl = new DSiteList(ds, dsl);
  }

public:
  ClassVar(Board *bb, OB_TIndex indexIn, GName *gnclass)
    : LazyVar(bb, indexIn, gnclass), dsl((DSiteList *) 0)
  {}

  //
  virtual LazyType getLazyType();
  virtual void sendRequest();
  virtual void marshal(ByteBuffer *);

  //
  Bool sendRequest(DSite *ds);

  //
  virtual ExtVar *gCollectV() { return (new ClassVar(*this)); }
  virtual void gCollectRecurseV(void);
  virtual void disposeV(void);

  //
  void transfer(OZ_Term cl, OZ_Term *cvtp);
};

//
TaggedRef newClassProxy(OB_TIndex bi, GName *gnclass);

#endif
