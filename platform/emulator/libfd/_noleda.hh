// noleda.hh

// implementation of the LEDA functions used by the all-different propagator
// (and ONLY these)
// loeckelt@ps.uni-sb.de

// edge_array and node_array implementations:
// the size of the generated array is _not_ the size of elements, but the
// MAXIMAL size of the elements in the graph up to the point of generation.
// this should not affect anything in the current case, since these array are
// not created multiple times with a shrunken graph.

#include "_generic.hh"
#include "_list.hh"
#include "_bqueue.hh"

class edge_struct;
class node_struct;

class graph;

typedef edge_struct* edge;
typedef node_struct* node;

template<class t> class edge_array;
template<class t> class node_array;

#define forall_adj_edges(e, n) \
 n->edgeiterator=NULL; while (n->next_adj_edge(e,n))

// CAUTION. these macros differ from the leda implementation in that they
// do solely depend on the graph iterator settings in taking successors.
#define forall_nodes(v,G) for (v = (G).first_node(); v; v = (G).next_node())
#define forall_edges(e,G) for (e = (G).first_edge(); e; e = (G).next_edge())

struct node_struct :public generic {
  node_struct(int _val, bool _free) {
    free = _free; 
    val = _val; 
    component = dtime = ftime = -1; // means undefined (used in scc)
    edgeiterator = NULL;
  }
  virtual ~node_struct() { }

  bool free;
  int val;
  int id;
  int component;
  list_item edgeiterator;

  // these are needed for the scc dfs, maybe to be abandoned later.
  int colour; // 0 = white, 1 = grey, 2 = black
  int dtime;  // discovery time
  int ftime;  // finishing time

  list<edge> out; // leaving edges
  list<edge> in;  // incoming edges
  
  bool next_adj_edge(edge &e, node v) const;

  void write() const {
    DEBUG((
    "node#%d [in %d out %d] (free %d val %d) <dtime %d ftime %d> scc: %d\r\n",
      id, in.size(), out.size(), free, val, dtime, ftime, component));
  }
};


struct edge_struct :public generic {
  edge_struct() {
    used      = false; 
    vital     = false; 
    bfs_visit = false;
  }
  virtual ~edge_struct() {}
  bool used;
  bool vital;
  bool bfs_visit;
  int id;
  node src; // source node
  node dst; // destination node
  void write() const {
    DEBUG(("edge#%d (%d -> %d)", id, src->id, dst->id));
  }
};


// ---------------------------------------------------------------------------

bool node_struct::next_adj_edge(edge &e, node v) const {
  if (v->edgeiterator) { // already set
    if (!v->edgeiterator->next) {
      e=NULL;
      return false;
    }
    else 
      v->edgeiterator = v->edgeiterator->next;
  }
  else { // not set yet
    v->edgeiterator = v->out.first();   
  }
  if (v->edgeiterator) {
    e = (edge)v->edgeiterator->e;
    return true;
  }
  else {
    e = NULL;
    return false;
  }
}

// ---------------------------------------------------------------------------

template<class t>
class edge_array :public generic {
friend class graph;
private:
  t *array;
  int size;
public:
  edge_array(graph &g, t initval) {
    size = g.edgemax();
    array = new t[size];
    // initialise
    for (int i = size; i-- ;) array[i] = initval;
  }
  virtual ~edge_array() { delete [] array; }
  t &operator[](edge e) {
    int pos = e->id;
    return array[pos];
  }
  void write() const {
    DEBUG(("edge_array, size=%d\r\n", size));
  }
};

template<class t>
class node_array :public generic {
friend class graph;
  t *array;
  int size;
public:
  node_array(graph &g, t initval) {
    size = g.nodemax();
    array = new t[size];
    // initialise
    for (int i = size; i--;) array[i] = initval;
  }
  virtual ~node_array() { delete [] array; }
  t &operator[](node n) {
    int pos = n->id;
    return array[pos];
  }
  void write() const {
    DEBUG(("node_array, size=%d\r\n", size));
  }
};


// ---------------------------------------------------------------------------

class graph :public generic {
friend class edge_array;
friend class node_array;

private:
  int mark; // for mcb matching

  list<node> N;
  list<edge> E;
  int nodecount;
  int edgecount;
  list<node> order_for_second_dfs;

  list_item nodeiterator;
  list_item edgeiterator;

  void dfs_visit(node n, int &time, int direction, int component);

  // from mcb:
  edge find_next_augmenting_path(graph& G, node s, node t,
                                 node_array<edge>& pred,
                                 edge_array<int>& layered);

  bool bfs(graph& G, node s,node t,edge_array<int>& layered);
  list<edge> MAX_CARD_BIPARTITE_MATCHING(const list<node>& A, 
                                         const list<node>& B );
  
public:
  graph() { 
    nodecount = edgecount = 0; 
    edgeiterator = nodeiterator = NULL;
  }
  ~graph() {
    DEBUG(("graph destructor\r\n"));
    // delete nodes and edges manually
  }
  void waste() {
    edge e = first_edge();
    while(e) {
      del_edge(e);
      e = first_edge();
    }
    node n = first_node();
    while(n) {
      del_node(n);
      n = first_node();
    }
    order_for_second_dfs.clear();
  }
  inline node new_node(int _val, bool _free = true);
  inline edge new_edge(node n1, node n2);
  inline void del_edge(edge d);
  inline void del_node(node n);
  inline void rev_all_edges();
  inline void rev_edge(edge e);
  inline node source(edge e) { return e->src; }
  inline node target(edge e) { return e->dst; }
  inline int indeg(node n) { return n->in.size(); }
  inline int outdeg(node n) { return n->out.size(); }
  void mark_strong_components();

  list<edge> MAX_CARD_BIPARTITE_MATCHING();

  void build(OZ_FDIntVar *l, int reg_l_sz); // specific for alldiff

  inline node first_node() { 
    nodeiterator = N.first();
    return (nodeiterator)? (node)nodeiterator->e : (node)NULL;
  }
  inline node next_node() { 
    nodeiterator = N.next(nodeiterator);
    return (nodeiterator)? (node)nodeiterator->e : (node)NULL;
  }

  inline edge first_edge() { 
    edgeiterator = E.first();
    return (edgeiterator)? (edge)edgeiterator->e : (edge)NULL;
  }
  inline edge next_edge() {
    edgeiterator = E.next(edgeiterator);
    return (edgeiterator)? (edge)edgeiterator->e : (edge)NULL;
  }

  inline bool next_adj_edge(edge &e, node v) const {
    return v->next_adj_edge(e, v);
  }

  inline int nodemax() { return nodecount; }
  inline int edgemax() { return edgecount; }
  inline int number_of_nodes() { return N.size(); }

  inline void reset() { // resets iterators.
    edgeiterator = nodeiterator = NULL;
  }
  void write() const;
  
};

void graph::write() const {
    DEBUG(("graph with %d nodes and %d edges\r\n", N.size(), E.size()));

    DEBUG(("nodes:\r\n"));
    list_item n;
    forall(n,N) 
      ((node)n->e)->write();

    DEBUG(("edges:\r\n"));
    list_item e;
    forall(e,E) 
      ((edge)e->e)->write();
}

inline node graph::new_node(int _val, bool _free =true) {
  node n = new node_struct(_val, _free);
  n->id = nodecount++;
  N.append(n);
  return n;
}

inline edge graph::new_edge(node s, node t) {
  edge e = new edge_struct();
  e->id  = edgecount++;
  e->src = s;
  e->dst = t;
  s->out.append(e);
  t->in.append(e);
  E.append(e);
  return e;
}

inline void graph::del_edge(edge d) {
  edge e = (edge)E.remove(d);
  (e->src)->out.remove(e);
  (e->dst)->in.remove(e);
  delete e;
  
}

inline void graph::del_node(node n) {
  // has to delete all incident edges, too.
  list_item i;
  
  forall(i, n->out) 
    del_edge((edge)i->e);
  
  forall(i, n->in) 
    del_edge((edge)i->e);

  N.remove(n);
  delete n;
}

inline void graph::rev_edge(edge e) {

  // change source/target
  node temp = e->dst;
  e->dst = e->src;
  e->src = temp;

  // change at nodes i/o lists
  e->dst->in.push(e);
  e->dst->out.remove(e);
  e->src->out.push(e);
  e->src->in.remove(e);
}

inline void graph::rev_all_edges() {
  // seems to be an error in here (list copying? -> null pointer xcpt)
  list_item i;
  edge e;
  node n;
  // revert edges
  forall(i, E) {
    e      = (edge)i->e;
    n      = e->src;
    e->src = e->dst;
    e->dst = n;
  }
  // change node in/out lists
  list<edge> temp;
  forall(i, N) {
    n = (node)i->e;
    n->out.swap(n->in);
  }
}

void graph::dfs_visit(node n, int &time, int direction, int component) {
  // direction =0 means forward, =1 backward edges.

  n->colour = 1; // grey (in Bearbeitung)
  n->dtime  = ++time;
  
  // gehe durch die Adjazenzliste von n (this version only forward)
  list_item i;

  if (direction) { // transponed graph?
    forall(i, n->in) {      
      node m = ((edge)i->e)->src;
      if (m->colour == 0) { // m is new
        // maybe mark daddy
        dfs_visit(m, time, direction, component);
      }  
    }
  }
  else {
    n->component = component;
    forall(i, n->out) {
      node m = ((edge)i->e)->dst;
      if (m->colour == 0) { // m is new
        // maybe mark daddy
        dfs_visit(m, time, direction, component);
      }  
    }
  }
  
  n->colour = 2;
  time++;
  n->ftime = time;
  // this is a bit awkward to tell the first dfs from the second
  if (direction) order_for_second_dfs.push(n);
  return;
}


void graph::mark_strong_components() {
  // Vorgehensweise:
  // a. mache eine DFS im transponierten Graphen.
  // b. mache eine DFS im normalen Graphen, Knotenreihenfolge nach
  //    absteigenden finish-Werten der ersten DFS.
  // c. markiere die Knoten im jeweils gleichen DFS-Baum als zusammenenhängend.

  node n;               // iterator
  int time = 0;         // dfs time
  int component = 0;    // component counter

  list_item item;

  DEBUG(("Mark_strong_components entered\r\n"));

  order_for_second_dfs.clear();

  DEBUG(("order cleared\r\n"));

  // paint all nodes white (might not be necessary)

  forall(item, N) ((node)item->e)->colour = 0;
  
  forall (item, N) {
    n = (node)item->e;
    // transponierte dfs:
    if (n->colour == 0) dfs_visit(n, time, 1, 0);     
  }

  // paint nodes white again and reset timer
  forall(item, N) ((node)item->e)->colour = 0;
  time = 0;

  forall (item, order_for_second_dfs) {
    n = (node)item->e;
    // normale dfs:
    if (n->colour == 0) dfs_visit(n, time, 0, ++component);
  }
  DEBUG(("leaving strong_components\r\n"));
}


edge graph::find_next_augmenting_path(graph& G, node s, node t,
                                             node_array<edge>& pred,
                                             edge_array<int>& layered) { 
  node w;
  edge e;
  edge f = 0;

  s->edgeiterator = NULL;
  while (f == 0 && G.next_adj_edge(e,s)) {

    if (layered[e] == mark)               // e is edge of layered network
      if ((w = e->dst) == t) f=e;         // t is reached
      else  
	if (pred[w] == 0) {               // w not reached before 
	  pred[w] = e;
	  f = find_next_augmenting_path(G, w, t, pred, layered);
	}
  }
  return f;
} 



bool graph::bfs(graph& G, node s,node t,edge_array<int>& layered) { 

  node_array<int> dist(G,-1);
  b_queue<node> Q(G.number_of_nodes());
  node v,w;
  edge e;

  Q.append(s);
  dist[s] = 0;

  while (!Q.empty()) { 
    v = Q.pop();
    if (v == t) {
      return true;
    }
    int dv = dist[v];

    forall_adj_edges(e,v) {
      w = e->dst;
      if (dist[w] < 0) {
	Q.append(w); 
        dist[w] = dv + 1;
      }
      if (dist[w] == dv+1) layered[e] = mark;
    }
  }
  return false;
}


list<edge> graph::MAX_CARD_BIPARTITE_MATCHING() {
  list<node> A,B;
  node v;

  forall_nodes(v,*this)
  if (outdeg(v))  A.append(v);
  else 
    if (indeg(v)) B.append(v);

  return MAX_CARD_BIPARTITE_MATCHING(A, B); 
}

/* ------------------------------------------------------------------------ */

list<edge> graph::MAX_CARD_BIPARTITE_MATCHING(const list<node>& A, 
					      const list<node>& B ) {
  // G is a bipartite graph with sides A and B, all edges must be directed 
  // from A to B. We compute a matching of maximum cardinality using the 
  // algorithm of Hopcroft and Karp. The running time is O(sqrt(n)*m).

  node v;
  edge e;
  list_item i;

  // heuristic for initial matching
  forall_edges(e, *this)
    if (indeg(e->src) == 0 && outdeg(e->dst) == 0) rev_edge(e);

  list<edge> EL;

  // TG: added values below
  node s = new_node(-1);
  node t = new_node(-1);

  node_array<edge> pred(*this, 0); 

  forall(i, A) {
    v = (node)i->e;
    if (!indeg(v)) new_edge(s, v);
  }

  forall(i, B) {
    v = (node)i->e;
    if (!outdeg(v)) new_edge(v, t);
  }

  edge_array<int> layered(*this, 0);
  mark = 1;

  reset();

  for(;;) {
    forall_nodes(v, *this) { 
      pred[v] = 0; 
    }
    mark++;

    if (bfs(*this, s, t, layered)) {
      // there is at least one augmenting path 
      while ((e = find_next_augmenting_path(*this, s, t, pred, layered))) 
        EL.append(e); 
    
      reset();
      while (!EL.empty()) {
        edge e = (edge)EL.pop();
        edge x = pred[e->src];
        while (x->src != s) {
          rev_edge(x);
          x = pred[x->dst];
        }
        del_edge(e);  // edge into t
        del_edge(x);  // edge out of s
      } 
    }
    else break;
  } 

  list<edge> result;

  forall(i, B) {
    v = (node)i->e;
    { forall_adj_edges(e, v) 
        if (e->dst != t) 
           result.append(e);
        else
           EL.append(e);
    }
  }

  // restore graph:

  forall(i, EL) {
    e = (edge)i->e;
    del_edge(e);
  }

  forall(i, result) {
    e = (edge)i->e;
    rev_edge(e);
  }

  del_node(s);
  del_node(t);
  
  return result;
}

void graph::build(OZ_FDIntVar *l, int reg_l_sz) {
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
    varnodes[i] = new_node(varcount++, true); // a var node.      
  }
  int y = -1;

  // enumerate elements of unified domain, generate nodes,
  // then link with appropriate variables
  while ( (y = AuxDom.getNextLargerElem(y)) != -1) {
    n = new_node(y, true);
    for (int i = 0; i < reg_l_sz; i++)
      if (l[i]->isIn(y)) new_edge(varnodes[i], n);
  }
}








