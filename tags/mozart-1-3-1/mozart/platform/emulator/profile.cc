/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Peter van Roy (pvr@info.ucl.ac.be)
 *    Denys Duchier (duchier@ps.uni-sb.de)
 *    Leif Kornstaedt (kornstae@ps.uni-sb.de)
 * 
 *  Copyright:
 *    Michael Mehl, 1997,1998
 *    Kostja Popow, 1997
 *    Ralf Scheidhauer, 1997
 *    Christian Schulte, 1997
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

#include "base.hh"
#include "builtins.hh"
#include "value.hh"
#include "statisti.hh"

void OZ_PropagatorProfile::profileReset(void) {
  OZ_PropagatorProfile * aux = getFirst();
  while (aux) {
    aux->_calls   = 0;
    aux->_samples = 0;
    aux->_heap = 0;
    aux = aux->getNext();
  }
}


void PrTabEntry::profileReset(void) {
  PrTabEntry * aux = allPrTabEntries;
  while (aux) {
    PrTabEntryProfile * prf = aux->pprof;
    if (prf)
      delete prf;
    aux->pprof = NULL;
    aux = aux->next;
  }
}


void Statistics::initProfile(void) {
  currAbstr = NULL;
  PrTabEntry::profileReset();
  OZ_PropagatorProfile::profileReset();
}

TaggedRef PrTabEntry::getProfileStats(void) {

  
  TaggedRef ret      = oz_nil();
  TaggedRef ps       = oz_atom("profileStats");
  TaggedRef samples  = oz_atom("samples");
  TaggedRef heap     = oz_atom("heap");
  TaggedRef calls    = oz_atom("calls");
  TaggedRef closures = oz_atom("closures");
  TaggedRef name     = oz_atom("name");
  TaggedRef line     = oz_atom("line");
  TaggedRef column   = oz_atom("column");
  TaggedRef file     = oz_atom("file");

  TaggedRef list = oz_cons(file,
		   oz_cons(line,
		   oz_cons(column,
		   oz_cons(name,
		   oz_cons(samples,
		   oz_cons(heap,
		   oz_cons(calls,
		   oz_cons(closures,
		   oz_nil()))))))));
  Arity *arity = aritytable.find(sortlist(list,oz_fastlength(list)));

  {
    PrTabEntry *aux = allPrTabEntries;

    while (aux) {
      if (aux->pprof && 
	  (aux->pprof->numClosures || aux->pprof->numCalled || 
	   aux->pprof->heapUsed    || aux->pprof->samples)) {
	SRecord *rec = SRecord::newSRecord(ps,arity);
	rec->setFeature(samples,oz_unsignedInt(aux->pprof->samples));
	rec->setFeature(calls,oz_unsignedInt(aux->pprof->numCalled));
	rec->setFeature(heap,oz_unsignedInt(aux->pprof->heapUsed));
	rec->setFeature(closures,oz_unsignedInt(aux->pprof->numClosures));
	rec->setFeature(line,oz_int(aux->line));
	rec->setFeature(column,oz_int(aux->colum));
	rec->setFeature(name,aux->printname);
	rec->setFeature(file,aux->file);
	ret = oz_cons(makeTaggedSRecord(rec),ret);
      }
      aux = aux->next;
    }
  }

  {
    OZ_PropagatorProfile *aux = OZ_PropagatorProfile::getFirst();
    TaggedRef noname = oz_atom("");
    while(aux) {
      if (aux->getSamples() || aux->getCalls()) {
	SRecord *rec = SRecord::newSRecord(ps,arity);
	rec->setFeature(samples,oz_unsignedInt(aux->getSamples()));
	rec->setFeature(calls,oz_unsignedInt(aux->getCalls()));
	rec->setFeature(heap,oz_unsignedInt(aux->getHeap()));
	rec->setFeature(closures,oz_int(0));
	rec->setFeature(line,oz_int(1));
	rec->setFeature(name,oz_atom(aux->getPropagatorName()));
	rec->setFeature(file,noname);
	ret = oz_cons(makeTaggedSRecord(rec),ret);
      }
      aux = aux->getNext();
    }
  }

  return ret;
}

OZ_BI_define(BIstatisticsReset, 0,0)
{
  ozstat.initProfile();
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIstatisticsGetProcs, 0,1)
{
  OZ_RETURN(PrTabEntry::getProfileStats());
} OZ_BI_end

OZ_BI_define(BIsetProfileMode, 1,0)
{
  oz_declareIN(0,onoff);
  ozstat.initProfile();
  if (oz_isTrue(oz_deref(onoff))) {
    am.setProfileMode();
  } else {    
    am.unsetProfileMode();
  }
  return PROCEED;
} OZ_BI_end


#ifndef MODULES_LINK_STATIC

#include "modProfile-if.cc"

#endif

