// all-oz.cc

// complete "all-different"

// loeckelt@ps.uni-sb.de

//#define MY_DEBUG
//#define OZ_DEBUG

#include "_alldiff.hh"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef LINUX
// Linux hack (Ark!):
extern "C" {
  int __eprintf(...) {
    DEBUG(("Trapped in Linux!!!\r\n"));
  }
}
#endif


//-----------------------------------------------------------------------------

/*
// this one is outdated and was built into the graph class.
// still here in the source just for safekeeping.

graph alldiffProp::buildGraph(OZ_FDIntVar *l) {
  graph g;
  OZ_FiniteDomain AuxDom;
  DECL_DYN_ARRAY(node, varnodes, reg_l_sz);  
  int varcount  = 0;
  int nodecount = 0;  

  node n;

  AuxDom.initEmpty();
  // AuxDom contains all elements from all domains after the loop.
  // for each variable, a node is created and stored in varnodes[i].
  for (int i =0; i < reg_l_sz; i++) { 
    AuxDom = AuxDom | *l[i];
    varnodes[i] = g.new_node(varcount++,true); // a var node.      
  }
  int y = -1;

  // enumerate elements of unified domain, generate nodes,
  // then link with appropriate variables
  while ( (y = AuxDom.getNextLargerElem(y)) != -1) {
    n = g.new_node(y,true);
    for (int i =0; i < reg_l_sz; i++)
      if (l[i]->isIn(y)) g.new_edge(varnodes[i], n);
  }
  return g;
}
*/

void alldiffProp::mark_bfs(graph &g, node n) {
  // rather dfs??
  edge e;

  forall_adj_edges(e, n) {
    if (e->used == false) {
      e->used = true;
      mark_bfs(g, g.target(e));
    }
  }
}

list<edge> alldiffProp::removeEdgesFromG(graph &g, list<edge> matching,
					 OZ_FDIntVar *reg) {
  
  DEBUG(("removeEdges entered\r\n"));
  edge e; // iterator
    
  // step 1. creating g0; assume that they are marked "used=0"  
  
  DEBUG(("step 1\r\n"));

  g.rev_all_edges();
  DEBUG(("rev_all_edges ok\r\n"));
  
  // reverse edges in matching
  list_item item;
  forall(item, matching) 
    g.rev_edge((edge)item->e);

  DEBUG(("rev_matching_edges ok\r\n"));
  // mark edges false/unused (by constructor) and mark non-free edges
  
  node n;
  forall_edges(e, g) {
    if (matching.search(e) != NULL) {
      n = g.source(e);
      n->free = false;
      n = g.target(e);
      n->free = false;
    }    
  }

  // set RE to empty.

  list<edge> RE;
 
  // step 2.
  // Look for all directed edges that belong to a directed simple path
  // which begins at a free vertex by a bfs starting from free vertices, and
  // mark them as "used"

  DEBUG(("step 2\r\n"));
  
  forall_nodes(n, g) 
    if (n->free == true) mark_bfs(g, n); // do a bfs.
  
  DEBUG(("step 3\r\n"));

  // step 3.
  // compute the strongly connected components of g0. Mark as "used" any 
  // directed edge that joins two vertices in the same SCC.

  g.mark_strong_components();

  DEBUG(("components: \r\n"));

#ifdef MY_DEBUG
  forall_nodes(n,g) 
    DEBUG(("%d ", n->component));

  
  DEBUG(("\r\nscc marked\r\n"));
#endif

  forall_edges(e, g) 
    if (g.source(e)->component == g.target(e)->component)
      e->used = true;
  
  DEBUG(("step 4\r\n"));
  
  // step 4.
  // for each directed edge 'de' marked as "unused" do (...)
  g.rev_all_edges();
  
  forall(item, matching) 
    g.rev_edge((edge)item->e);
  
  forall_edges(e, g) {  
    if (e->used == false) {
      if (matching.search(e)) {
        e->vital = true;
      }
      else {
        RE.push(e);      
      }
    }
  }

  DEBUG(("actually removing edges, RE.size=%d\r\n", RE.size()));
  // now actually remove edges
  forall(item, RE) {
    e = (edge)item->e;

    DEBUG((
	   "Removing value '%d' from domain of variable no.%d; domsize=%d\r\n",
	    g.target(e)->val, g.source(e)->id, 
	    reg[g.source(e)->id]->getSize()
    ));

    *reg[g.source(e)->id] -= g.target(e)->val;

    /// immediate notification in case of failure:
    //  slows down, but if you like it...

    // if (reg[g.inf(g.source(e)).val]->getSize() == 0) {
    //   isFailed = true;
    //   return RE;
    // }
    g.del_edge(e);
  }

  DEBUG(("leaving removeEdges\r\n"));

  return RE;
}


OZ_C_proc_begin(fdtest_alldiff, 1) {
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD);
  PropagatorExpect pe;
  OZ_EXPECT(pe, 0, expectVectorIntVarSingl);
  DEBUG(("imposing...\r\n"));
  return pe.impose(new alldiffProp(OZ_args[0]));
}
OZ_C_proc_end


OZ_Return alldiffProp::propagate(void) {
  bool isFailed  = false;
  int size = reg_l_sz;
  OZ_Return retval;  
  
  DEBUG(("alldiff: invoked, sz=%d\r\n", reg_l_sz));

  list<edge> matching;

  DECL_DYN_ARRAY(OZ_FDIntVar, reg, size);

  for(int i = 0; i < size; i++) 
    reg[i].read(reg_l[i]);

  PropagatorController_VV PC(size, reg);

  if (hasEqualVars()) {
    DEBUG(("hasEqualVars: FAIL\r\n"));
    return PC.fail();  
  }
  //graph g = buildGraph(reg);
  graph g;
  g.build(reg, reg_l_sz);
  //g.write();
  matching = g.MAX_CARD_BIPARTITE_MATCHING();

#ifdef MY_DEBUG
  DEBUG(("got a matching, removeEdgesfromG\r\n"));
  DEBUG(("THE MATCHING:\r\n"));  
  list_item item;
  edge e;
  forall(item, matching) {
    e=(edge)item->e;
    e->write();
    DEBUG(("   dst val is %d\r\n", (e->dst)->val));
  }
#endif

  removeEdgesFromG(g, matching, reg);
  DEBUG(("returned from removeEdges\r\n"));

  for (int i = 0; i < reg_l_sz; i++) {

    if (reg[i]->getSize() == 0) {
      DEBUG(("FAILED\r\n"));
      isFailed = true;
    }
#ifdef MY_DEBUG
    int pos=-1;
    DEBUG(("var#%d: [",i));
    while((pos=reg[i]->getNextLargerElem(pos))!=-1) 
      DEBUG(("%d ", pos));

    DEBUG(("]\r\n"));
#endif
  } 

  g.waste();
  retval = (isFailed)? PC.fail() : PC.leave();  
#ifdef MY_DEBUG
  DEBUG(("Leaving Propagator\r\n"));
  /*
  printf("#dlink=%d(%d) #node=%d(%d) #edge=%d(%d)\r\n", 
	 dlinkcounter, sizeof(dlink),
	 nodecounter, sizeof(node_struct),
         edgecounter, sizeof(edge_struct));
  */
#endif
  return retval;
}

// static members
OZ_CFunHeader alldiffProp::spawner     = fdtest_alldiff;














