/*
 *  Authors:
 *    Joerg Wuertz (wuertz@dfki.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Organization or Person (Year(s))
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     http://www.mozart-oz.org/
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#include "rel.hh"
#include "pel_fncts.hh"

template class EnlargeableArrayWithBase<int, HeapAlloc>;
template class PushArray<int, HeapAlloc>;

template class EnlargeableArrayWithBase<char,HeapAlloc>;

template class EnlargeableArrayWithBase<int, PropAlloc>;
template class ResizeableArray<int, PropAlloc>;

//-----------------------------------------------------------------------------

void _PEL_EventList::wakeup(_PEL_PropQueue * pq, PEL_Engine * engine) {
  for (int i = _high; i--; ) {
    int idx = operator[](i);
    PEL_Propagator * prop = engine->getPropTable().getPropagator(idx);
    if (! prop->isScheduled()) {
      CDM((" <waking up>"));
      pq->enqueue(idx);
      prop->setScheduled();
    }
  }
}

//-----------------------------------------------------------------------------

pf_return_t PEL_Engine::apply(void)
{
  int idx = dequeue();
  PEL_Propagator * prop = _pe->_prop_table.getPropagator(idx);
  //
  pf_return_t r = prop->propagate(*this);
  //
  if (r == pf_entailed) {
    decAPF();
    prop->setDead();
  } else if (r == pf_failed) {
    setFailed();
  } else {
    CDM(("apply sleep\n"));
    prop->unsetScheduled();
  }
  return r;
}

//-----------------------------------------------------------------------------

// converts a integer starting from `p' and moves `p' to the next
// unprocessed character
inline
int getNumber(const char * &p) {
  char * n;
  int i = 1;
  p += 1;
  if (isdigit(*p)) {
    i = strtol(p, &n, 10);
    p = n;
  }
  return i < 1 ? 1 : i;
}

PEL_Engine::PEL_Engine(PEL_PersistentEngine &pe, const char * f, ...) 
  : _PEL_PropQueue(), _pe(&pe) {
  //
  // D - finite domain, d - integer
  // S - finite set domain , s - finite set
  // C - ct domain, c - ct value
  //
  // determine number of variables according to type string
  //
  int nb_vars = 0;
  const char * p;
  for (p = f; *p; ) {
    switch (*p) {
    case 'D': case 'd': case 'S': case 's': case 'C': case 'c': 
      nb_vars += getNumber(p);
      break;
    case ' ': case '\t':
      p += 1;
      break;
    default:
      CASSERT(0);
      break;
    }
  }
  //
  CDM(("%d vars\n", nb_vars));
  _vs = (PEL_Var **) 
    OZ_CPIVar::operator new(sizeof(PEL_Var *) * nb_vars); 
  //
  // process individual variables
  //
  va_list ap;
  va_start(ap, f);
  //
  for (p = f; *p; ) {
    switch (*p) {
    case 'D': // finite domain
    case 'd': // integer
      {
	CDM(("%c", *p));
	const int n = getNumber(p);
	CDM(("[%d] ", n));
	PEL_PersistentFDIntVar * pfdv = va_arg(ap, PEL_PersistentFDIntVar *);
	PEL_FDIntVar * fdv = va_arg(ap, PEL_FDIntVar *);
	for (int i = 0; i < n; i += 1) {
	  // set variable id for variables not being a constraint's
	  // parameter
	  if (pfdv[i].getId() == -1) {
	    pfdv[i].newId(_pe->_current_id);
	  }
	  CASSERT((0 <= pfdv[i].getId()) && (pfdv[i].getId() < nb_vars));
	  _vs[pfdv[i].getId()] = fdv[i].init(pfdv[i], *this);
	}
      }
      break;
    case 'S': // integer set domain
    case 's': // integer set
      {
	CDM(("%c", *p));
	const int n = getNumber(p);
	CDM(("[%d] ", n));
	PEL_PersistentFSetVar * pfsv = va_arg(ap, PEL_PersistentFSetVar *);
	PEL_FSetVar * fsv = va_arg(ap, PEL_FSetVar *);
	for (int i = 0; i < n; i += 1) {
	  // set variable id for variables not being a constraint's
	  // parameter
	  if (pfsv[i].getId() == -1) {
	    pfsv[i].newId(_pe->_current_id);
	  }
	  CASSERT((0 <= pfsv[i].getId()) && (pfsv[i].getId() < nb_vars));
	  _vs[pfsv[i].getId()] = fsv[i].init(pfsv[i], *this);
	}
      }
      break;
    case 'C': // ct domain
    case 'c': // ct value
      {
	CDM(("%c", *p));
	const int n = getNumber(p);
	CDM(("[%d] ", n));
      }
      break;
    case ' ': case '\t': // white spaces
      CDM(("'ws' "));
      p += 1;
      break;
    default:
      CASSERT(0);
      break;
    }
  }
  CDM(("\n"));
  
} // PEL_Engine::PEL_Engine

//-----------------------------------------------------------------------------
// static members

int PEL_PersistentEngine::_current_params = 0;
_PEL_EventList * PEL_PersistentEngine::_ela[MAX_PARAMS];

int PEL_Propagator::_last_i;
PEL_PersistentEngine * PEL_Propagator::_pe;
_PEL_PropagatorTable * PEL_Propagator::_pt;


