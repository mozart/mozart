/*
 *  Authors:
 *    Joerg Wuertz (wuertz@dfki.de)
 * 
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#include "streamProps.hh"
#include "rel.hh"
#include "auxcomp.hh"
#include <stdlib.h>

//-----------------------------------------------------------------------------

//--------------------------------------------------------------

class Min_max {
public:
  int min, max;
};

//////////
// CONSTRUCTOR
//////////
DisjunctivePropagatorStream::DisjunctivePropagatorStream(OZ_Term fds, OZ_Term durs, OZ_Term st) 
{
  stream = st;
  reg_durs = vectorToInts(durs, reg_size);
  reg_fds = vectorToOzTerms(fds, reg_size);
}


//////////
// DESTRUCTOR
//////////
DisjunctivePropagatorStream::~DisjunctivePropagatorStream()
{
  OZ_hfreeCInts(reg_durs, reg_size);
  OZ_hfreeOzTerms(reg_fds, reg_size);

}


//////////
// BUILTIN
//////////
OZ_BI_define(sched_disjunctiveStream, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD "," OZ_EM_VECT OZ_EM_INT "," OZ_EM_STREAM);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorIntVarAny);
  OZ_EXPECT(pe, 1, expectVectorInt);
  OZ_EXPECT(pe, 2, expectStream);

  return pe.impose(new DisjunctivePropagatorStream(OZ_in(0), OZ_in(1), OZ_in(2)));
}
OZ_BI_end


//////////
// COPYING
//////////
void DisjunctivePropagatorStream::gCollect(void) {
  OZ_gCollectTerm(stream);
  reg_durs = OZ_copyCInts(reg_size, reg_durs);
  reg_fds  = OZ_gCollectAllocBlock(reg_size, reg_fds);
}

void DisjunctivePropagatorStream::sClone(void) {
  OZ_sCloneTerm(stream);
  reg_durs = OZ_copyCInts(reg_size, reg_durs);
  reg_fds  = OZ_sCloneAllocBlock(reg_size, reg_fds);
}


//////////
// RUN METHOD
//////////

OZ_Return DisjunctivePropagatorStream::propagate(void)
{
  OZ_Stream st(stream);

  int i, j;
  int &ts  = reg_size;


  int disjFlag = 0;
  int new_items = 0;
  struct MyListA 
  {
    OZ_Term fd;
    int         dur;
    struct MyListA * next;
  };
  struct MyListA * my_list = NULL;
  int only_stream = 1;


  while (!st.isEostr()) {
    OZ_Term e = st.get();
    if (OZ_isTuple(e) && ! OZ_isLiteral(e)) {
      const char * label = OZ_atomToC(OZ_label(e));
      if (! strcmp("#", label)) {
	OZ_Term new_fd  = OZ_getArg(e, 0);
	int new_dur = OZ_intToC(OZ_getArg(e, 1));
	MyListA * newItem = new MyListA;
	newItem->fd = new_fd;
	newItem->dur = new_dur;
	if (new_items == 0)
	  {
	  newItem->next = NULL;
	  my_list = newItem;
	  }
	else {
	  newItem->next = my_list;
	  my_list = newItem;
	}
	new_items++;
      }
      else {
	st.fail(); 
	
	return FAILED;
      }
    }
  }

  
  int new_ts = ts + new_items;
  DECL_DYN_ARRAY(OZ_FDIntVar, x, new_ts);

  if (new_items > 0) {
    int * new_reg_durs    = OZ_hallocCInts(new_ts);
    OZ_Term * new_reg_fds = OZ_hallocOzTerms(new_ts);

    for (i = ts; i--; ) {
      new_reg_durs[i] = reg_durs[i];
      new_reg_fds[i] = reg_fds[i];
    }

    OZ_hfreeOzTerms(reg_fds,ts);
    OZ_hfreeCInts(reg_durs,ts);

    for (i = 0; i<new_items; i++ ) {
      new_reg_durs[ts+i] = my_list->dur;
      new_reg_fds[ts+i] = my_list->fd;
      imposeOn(my_list->fd);
      :: delete my_list;
      my_list = my_list->next;
    }
    reg_durs = new_reg_durs;
    reg_fds  = new_reg_fds;
  }

  ts = new_ts;

  for (i = ts; i--; )
    x[i].read(reg_fds[i]);

  int * dur = reg_durs;

  DECL_DYN_ARRAY(Min_max, MinMax, ts);
  for (i=ts; i--;){
    MinMax[i].min = x[i]->getMinElem();
    MinMax[i].max = x[i]->getMaxElem();
  }


reifiedloop:

   for (i=0; i<ts; i++)
     for (j=i+1; j<ts; j++) {
       int xui = MinMax[i].max, di = dur[i], xlj = MinMax[j].min;
       if (xui + di <= xlj) continue;
       int xuj = MinMax[j].max, dj = dur[j], xli = MinMax[i].min;
       if (xuj + dj <= xli) continue;
       if (xli + di > xuj) {
	 if (xuj > xui - dj) {
	   disjFlag = 1;
	   FailOnEmpty(*x[j] <= xui - dj);
	   MinMax[j].max = x[j]->getMaxElem();
	 }
	 if (xli < xlj + dj) {
	   disjFlag = 1;
	   FailOnEmpty(*x[i] >= xlj + dj);
	   MinMax[i].min = x[i]->getMinElem();
	 }
       }
       if (xlj + dj > xui) {
	 if (xui > xuj - di) {
	   disjFlag = 1;
	   FailOnEmpty(*x[i] <= xuj - di);
	   MinMax[i].max = x[i]->getMaxElem();
	 }
	 if (xlj < xli + di) {
	   disjFlag = 1;
	   FailOnEmpty(*x[j] >= xli + di);
	   MinMax[j].min = x[j]->getMinElem();
	 }
       }
     }

  if (disjFlag == 1) {
    disjFlag = 0;
    goto reifiedloop;
  }

  if (!st.isValid()) 
    goto failure;
  

  stream = st.getTail();

  for (i = ts; i--; ) {
    if (*x[i] != fd_singl) 
      only_stream = 0;
    x[i].leave();
  }
  
  if (only_stream)
    return st.leave() ? SLEEP : PROCEED;
  else {
    st.leave();
    return SLEEP;
  }

failure:
  st.fail(); 

  for (i = ts; i--; )
    x[i].fail();

  return FAILED;

}

//-----------------------------------------------------------------------------

//////////
// CONSTRUCTOR
//////////
DistinctPropagatorStream::DistinctPropagatorStream(OZ_Term fds, OZ_Term st) 
{
  stream = st;
  reg_fds = vectorToOzTerms(fds, reg_size);
}


//////////
// DESTRUCTOR
//////////
DistinctPropagatorStream::~DistinctPropagatorStream()
{
  OZ_hfreeOzTerms(reg_fds, reg_size);
}


//////////
// BUILTIN
//////////
OZ_BI_define(fdp_distinctStream, 2, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD "," OZ_EM_STREAM);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorIntVarAny);
  OZ_EXPECT(pe, 1, expectStream);

  return pe.impose(new DistinctPropagatorStream(OZ_in(0), OZ_in(1)));
}
OZ_BI_end


//////////
// COPYING
//////////
void DistinctPropagatorStream::gCollect(void) {
  OZ_gCollectTerm(stream);
  reg_fds = OZ_gCollectAllocBlock(reg_size, reg_fds);
}

void DistinctPropagatorStream::sClone(void) {
  OZ_sCloneTerm(stream);
  reg_fds = OZ_sCloneAllocBlock(reg_size, reg_fds);
}


//////////
// RUN METHOD
//////////

OZ_Return DistinctPropagatorStream::propagate(void)
{
  OZ_Stream st(stream);

  int i;
  int &ts  = reg_size;
  OZ_FiniteDomain u(fd_empty);

  int new_items = 0;
  struct MyListB
  {
    OZ_Term fd;
    struct MyListB * next;
  };
  struct MyListB * my_list = NULL;
  int only_stream = 1;


  while (!st.isEostr()) {
    OZ_Term e = st.get();
    if (OZ_isVariable(e)) {
      MyListB * newItem = ::new MyListB;
      newItem->fd = e;
      if (new_items == 0)
	{
	  newItem->next = NULL;
	  my_list = newItem;
	}
      else {
	newItem->next = my_list;
	my_list = newItem;
      }
      new_items++;
    }
    else {
      st.fail(); 
      
      return FAILED;
    }
  }

  
  int new_ts = ts + new_items;
  DECL_DYN_ARRAY(OZ_FDIntVar, x, new_ts);

  if (new_items > 0) {
    OZ_Term * new_reg_fds = OZ_hallocOzTerms(new_ts);

    for (i = ts; i--; ) 
      new_reg_fds[i] = reg_fds[i];

    OZ_hfreeOzTerms(reg_fds,ts);

    for (i = 0; i<new_items; i++ ) {
      new_reg_fds[ts+i] = my_list->fd;
      imposeOn(my_list->fd);
      MyListB * aux = my_list;
      my_list = my_list->next;
      :: delete aux;
    }
    reg_fds  = new_reg_fds;
  }

  ts = new_ts;

  for (i = ts; i--; )
    x[i].read(reg_fds[i]);


  // here comes the propagation
  if (mayBeEqualVars()) {
    int * is = OZ_findEqualVars(ts, reg_fds);
    
    for (i = ts; i--; )
      if (is[i] != -1 && is[i] != i) 
	goto failure;
  }    
  
  for  (i = ts; i--; )
    if (*x[i] == fd_singl) {
      int s = x[i]->getSingleElem();
      if (u.isIn(s)) {
	goto failure;
      } else {
	u += s;
      }
    }

 loop:
  for (i = ts; i--; ) {
    if (*x[i] != fd_singl) {
      FailOnEmpty(*x[i] -= u);
      
      if (*x[i] == fd_singl) {
	u += x[i]->getSingleElem();
	goto loop;
      }
    }
  }


  if (!st.isValid()) 
    goto failure;
  

  stream = st.getTail();

  for (i = ts; i--; ) {
    if (*x[i] != fd_singl) 
      only_stream = 0;
    x[i].leave();
  }
  
  if (only_stream)
    return st.leave() ? SLEEP : PROCEED;
  else {
    st.leave();
    return SLEEP;
  }

failure:
  st.fail(); 

  for (i = ts; i--; )
    x[i].fail();

  return FAILED;

}

//-----------------------------------------------------------------------------
// static member

OZ_PropagatorProfile DisjunctivePropagatorStream::profile;
OZ_PropagatorProfile DistinctPropagatorStream::profile;

