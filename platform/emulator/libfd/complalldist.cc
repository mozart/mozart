/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: loeckelt
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

// #define OZ_DEBUG

#define DISTINCT // whether to include the code from FD.distinct

// set this according to whether the emulator demands the 'spawner'
// function in propagators to be of type OZ_CFun (old version)
// or OZ_CFunHeader (new version):

#define OLDEMU

#ifdef OZ_DEBUG
  int edgecount;
  int nodecount;
  int dlinkcount;
  int dlinktotal;
  int listcount;
  int listtotal;
#endif

#include "complalldist.hh"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdp_distinctD, 1) {
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorIntVarAny);

  return pe.impose(new CompleteAllDistProp(OZ_args[0]));
}
OZ_C_proc_end

OZ_CFunHeader CompleteAllDistProp::spawner = fdp_distinctD;
int CompleteAllDistProp::init_memory_management = 1;

//=============================================================================

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

  isFailed   = false;
  int &size  = reg_l_sz;

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

  // here starts code from DISTINCT

#ifdef DISTINCT
  OZ_FiniteDomain u(fd_empty);

  for  (int i = size; i--; )
    if (*reg[i] == fd_singl) {
      int s = reg[i]->getSingleElem();
      if (u.isIn(s)) {
        //goto failure;
        return PC.fail();
      } else {
        u += s;
      }
    }

 loop:
  for (int i = size; i--; ) {
    if (*reg[i] != fd_singl) {
      //FailOnEmpty(*reg[i] -= u);
      if ((*reg[i] -= u) == 0) return PC.fail();
      if (*reg[i] == fd_singl) {
        u += reg[i]->getSingleElem();
        goto loop;
      }
    }
  }

  int from, to;
  for (from = 0, to = 0; from < size; from += 1) {
    if (*reg[from] != fd_singl) {
      reg_l[to++] = reg_l[from];
    }
  }
  size = to;

escape:

  // here ends code from DISTINCT

#endif

  // now do the "complete thing" iff there still are variables left.

  if (size) {
    list<node> A;
    list<node> B;
    list<edge> matching;
    graph g;

    buildGraph(g, reg, A, B);

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
  retval = (isFailed)? PC.fail() : PC.leave();

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
