/*
 *  Authors:
 *    Markus Loeckelt (loeckelt@ps.uni-sb.de)
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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

//#define OZ_DEBUG

#define DISTINCT // whether to include the code from FD.distinct

// set this according to whether the emulator demands the 'spawner' 
// function in propagators to be of type OZ_CFun (old version) 
// or OZ_PropagatorProfile (new version):

#define OLDEMU   

#include "complalldist.hh"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef OZ_DEBUG
  int edgecount;
  int nodecount;
  int dlinkcount;
  int dlinktotal;
  int listcount;
  int listtotal;
#endif

CpiHeapClass memory;

//-----------------------------------------------------------------------------

OZ_BI_define(fdp_distinctD, 1, 0) {
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorIntVarAny);

  return pe.impose(new CompleteAllDistProp(OZ_in(0)));
}
OZ_BI_end

OZ_PropagatorProfile CompleteAllDistProp::profile;
int CompleteAllDistProp::init_memory_management;

//=============================================================================

edge graph::find_next_augmenting_path(node s, node t,
                                             node_array<edge>& pred,
                                             edge_array<int>& layered) { 
  node w = NULL;
  edge e = NULL;
  edge f = NULL;

  s->edgeiterator = NULL;
  while ((f == NULL) && (next_adj_edge(e, s))) {
    
    if (layered[e] == mark)               // e is edge of layered network
      if ((w = e->dst) == t) f = e;       // t is reached
      else  
	if (pred[w] == 0) {               // w not reached before 
	  pred[w] = e;
	  f = find_next_augmenting_path(w, t, pred, layered);
	}
  }
  return f;
} 


void graph::dfs_visit(node n, int &time, int direction, int component) {
  // direction = 0 means forward, = 1 backward edges.
  // (this is useful for 'strong components' dfs)

  n->colour = 1; // grey (being processed)
  n->dtime  = ++time;
  
  // traverse n's adjacency list (this version only forward)

  dlink<edge> *i;

  if (direction) { // transponed graph?
    forall(i, n->in) {      
      node m = (i->e)->src;
      if (!m->colour) { // m is new?
        // maybe mark daddy
        dfs_visit(m, time, direction, component);
      }  
    }
  }
  else {
    n->component = component;
    forall(i, n->out) {
      node m = (i->e)->dst;
      if (!m->colour) { // m is new?
        // maybe mark daddy
        dfs_visit(m, time, direction, component);
      }  
    }
  }
  
  n->colour = 2;
  time++;
  n->ftime = time;

  // for the strong components search:
  // this is a bit awkward, but allows to tell the first dfs from the second

  if (direction) order_for_second_dfs.push(n);
  return;
}


void CompleteAllDistProp::buildGraph(graph &g, OZ_FDIntVar *l, 
			      list<node> &A, list<node> &B) {
  // A and B contain the "left" and "right" portions of the bipartite 
  // graph, respectively. A "left" node is created for every variable,
  // a "right" one for every value, and valid pairs are connected.

  OZ_FiniteDomain AuxDom;

  DECL_DYN_ARRAY(node, varnodes, reg_l_sz);  

  int varcount =0;
  node n;

  AuxDom.initEmpty();
  // AuxDom contains all elements from all domains after the loop.
  // for each variable, a node is created and stored in varnodes[i].
  for (int i =0; i < reg_l_sz; i++) { 
    DEBUG(("Union: AuxDom with l[%d]...", i));

    /*
      // this one causes a SEGV under Linux for obscure reasons.
      AuxDom = AuxDom | *l[i];
    */

    // this is a bit awkward, but does not crash:
    
    int auxint=-1;
    while( (auxint = (*l[i]).getNextLargerElem(auxint)) != -1)
      AuxDom += auxint;
      
    // testing all the time is taking too much time
    // if (AuxDom.getSize() <= i) isFailed = true; 

    DEBUG(("okay.\r\n"));
    DEBUG(("Creating new varnode, val %d, vec#%d", varcount, i));
    varnodes[i] = g.new_node(varcount++, true); // one node for every variable  
    A.append(varnodes[i]);
    DEBUG(("; varcount now %d\r\n", varcount));    
  }

  int y = -1;

  // enumerate elements of unified domain, generate nodes,
  // then link with nodes of variables that contain them.

  while ( (y = AuxDom.getNextLargerElem(y)) != -1) {
    n = g.new_node(y, true);
    B.append(n);
    for (int j = 0; j < reg_l_sz; j++) 
      if (l[j]->isIn(y)) 
	g.new_edge(varnodes[j], n);
  }
}

//-----------------------------------------------------------------------------

void CompleteAllDistProp::mark_bfs(graph &g, node n, int &markcounter) {
  // for strong components search 
  edge e;
  forall_adj_edges(e, n) {
    if (e->used == false) {
      e->used = true;
      markcounter++;
      mark_bfs(g, g.target(e), markcounter);
    }
  }
}

//-----------------------------------------------------------------------------

list<edge> CompleteAllDistProp::removeEdgesFromG(graph &g, 
					 list<edge> matching,
					 OZ_FDIntVar *reg) {
  // see the algorithm of R'egin.
  
  edge e;
  node n;
  dlink<edge> *item;
  list<edge> RE;  
    
  // step 1. 
  // create g0; assume that all edges are marked "used = 0"  
  
  g.rev_all_edges();

  // reverse edges in matching

  forall(item, matching) 
    g.rev_edge(item->e);

  // mark edges unused (by constructor)
  // mark non-free edges
  
  forall_edges(e, g) {
    if (matching.search(e)) {
      n = g.source(e);
      n->free = false;
      n = g.target(e);
      n->free = false;
    }    
  }

  // step 2.
  // Look for all directed edges that belong to a directed simple path
  // which begins at a free vertex by a bfs starting from free vertices, and
  // mark them as "used"

  int markcounter = 0;

  forall_nodes(n, g) 
    if (n->free == true) {
      mark_bfs(g, n, markcounter); // do a bfs.
    }

  // step 3.
  // compute the strongly connected components of g0. Mark as "used" any 
  // directed edge that joins two vertices in the same SCC.

  // added later: only do this if not all edges have been marked, ie.:

  if (markcounter - g.number_of_edges()) {

    g.mark_strong_components();

#ifdef OZ_DEBUG
    DEBUG(("components: \r\n"));

    forall_nodes(n,g) 
      DEBUG(("%d ", n->component));
#endif
  
    forall_edges(e, g) 
      if (g.source(e)->component == g.target(e)->component)
	e->used = true;
  }

  // step 4.
  // for each directed edge 'de' marked as "unused" do (...)

  g.rev_all_edges();
  
  forall(item, matching) 
    g.rev_edge(item->e);
  
  forall_edges(e, g)   
    if (e->used == false) {
      if (!matching.search(e)) 
	RE.push(e);
      
      else
        if (0 == (*reg[g.source(e)->id] &= g.target(e)->val)) {
	  isFailed = true; return RE;
        }
	
    }      

  DEBUG(("actually removing edges, RE.size=%d\r\n", RE.size()));

  // now actually remove edges

  forall(item, RE) {
    e = item->e;

    DEBUG((
	   "Removing value '%d' from domain of variable no.%d; domsize=%d\r\n",
	    g.target(e)->val, g.source(e)->id, 
	    reg[g.source(e)->id]->getSize()
    ));

    *reg[g.source(e)->id] -= g.target(e)->val;

    // immediate notification in case of failure:
    // thought it would slow down, but seemingly doesn't. Hmmm.
    
    if (reg[g.source(e)->val]->getSize() == 0) {
      isFailed = true;
      return RE;
    }
    
    g.del_edge(e);
  }

  DEBUG(("leaving removeEdges\r\n"));

  return RE;
}

//-----------------------------------------------------------------------------

OZ_Return CompleteAllDistProp::xpropagate(void) {

  // for debugging purposes, the propagator isn't invoked directly.

  int &size  = reg_l_sz;

  if (size < 2)
    return PROCEED;

  isFailed   = false;

  OZ_Return retval;  
  
  DEBUG(("alldiff: invoked, sz=%d\r\n", reg_l_sz));

  DECL_DYN_ARRAY(OZ_FDIntVar, reg, size);

  for(int i = 0; i < size; i++) 
    reg[i].read(reg_l[i]);

  PropagatorController_VV PC(size, reg);

  DEBUG(("ENTERING PROPAGATOR\r\n"));
#ifdef OZ_DEBUG
  {
    for (int i = 0; i < reg_l_sz; i++) {
      int pos = -1;
      DEBUG(("var#%d: [", i));
      while((pos = reg[i]->getNextLargerElem(pos)) != -1) 
        DEBUG(("%d ", pos));
      DEBUG(("]\r\n"));
    }
  }
#endif

  if (hasEqualVars()) return PC.fail();  

  {
    OZ_FiniteDomain u(fd_empty);
    //
    for  (int i1 = size; i1--; )
      if (*reg[i1] == fd_singl) {
	int s = reg[i1]->getSingleElem();
	if (u.isIn(s)) {
	  return PC.fail();
	} else {
	  u += s;
	}
      }
    //
  loop:
    for (int i2 = size; i2--; ) {
      if (*reg[i2] != fd_singl) {
	if ((*reg[i2] -= u) == 0) return PC.fail();
	if (*reg[i2] == fd_singl) {
	  u += reg[i2]->getSingleElem();
	  goto loop;
	}
      }
    }
  }
  //
  //
  // now do the "complete thing" iff there still are variables left.
  //
  if (size) {
    list<node> A;
    list<node> B;
    list<edge> matching;
    graph g;

    buildGraph(g, reg, A, B);
  
    // overlapping test:
    // propagator is entailed iff a node in the B list (variable values)
    // has more than one incoming edge. (linear in remaining domain size)
    
    bool overlap = false;
    DEBUG(("complalldist: doing overlap test .... \n"));
    dlink<node> *n;
    forall(n, B) {
      if (g.indeg(n->e) > 1) {
        overlap = true;
        break;
      }
    }
  
    if (!overlap) {
      DEBUG(("complalldist: no overlapping - returning sleep\n"));
  
      DEBUG(("%s\n", toString()));

      return PC.leave();
    }
    
    matching = g.MAX_CARD_BIPARTITE_MATCHING(A, B);
  
    if (matching.size() < size) 
      return PC.fail();

#ifdef OZ_DEBUG
    DEBUG(("THE MATCHING:\r\n"));  
    dlink<edge> *item;
    edge e;
    forall(item, matching) {
      e = item->e;
      e->write();
      DEBUG(("   dst val is %d\r\n", (e->dst)->val));
    }
#endif
    
    removeEdgesFromG(g, matching, reg);
    
    for (int i = 0; i < size; i++) {
      if (reg[i]->getSize() == 0) 
	isFailed = true;
      
#ifdef OZ_DEBUG
      int pos = -1;
      DEBUG(("var#%d: [",i));
      while((pos = reg[i]->getNextLargerElem(pos)) != -1) 
	DEBUG(("%d ", pos));      
      DEBUG(("]\r\n"));
#endif
    } 
    
  }
  DEBUG(("\r\nLEAVING PROPAGATOR\r\n"));
#ifdef OZ_DEBUG
  {
    for(int i =0; i < size; i++) {
      int pos = -1;
      DEBUG(("var#%d: [",i));
      while((pos = reg[i]->getNextLargerElem(pos)) != -1) 
	DEBUG(("%d ", pos));
      
      DEBUG(("]\r\n"));
    }
  }
#endif
  
  DEBUG(("%s\n", toString()));
  retval = (isFailed)? PC.fail() : PC.leave();  
  
  // finish above compression
  int from, to;
  for (from = 0, to = 0; from < size; from += 1) {
    if (*reg[from] != fd_singl) {
      reg_l[to++] = reg_l[from];
    }
  }
  size = to;
  //
  return retval;
}

//-----------------------------------------------------------------------------

OZ_Return CompleteAllDistProp::propagate(void) {
#ifdef OZ_DEBUG
  edgecount  =0;
  nodecount  =0;
  dlinkcount =0;
  dlinktotal =0;
  listcount  =0;
  listtotal  =0;
#endif
  OZ_Return retval = xpropagate();
  DEBUG(("COUNTERS node:%d edge:%d dlink:%d/%d list:%d/%d\r\n",
	 nodecount, edgecount, dlinkcount, dlinktotal, listcount, listtotal));

  memory.reset();
  return retval;
}

// eof
//-----------------------------------------------------------------------------
