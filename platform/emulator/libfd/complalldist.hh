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

#include <stdio.h>
#include <stdlib.h>

#ifdef INTERFACE
#pragma interface "complalldist.hh"
#endif

#include "std.hh"


#ifdef OZ_DEBUG
  extern int edgecount;
  extern int nodecount;
  extern int dlinkcount;
  extern int dlinktotal;
  extern int listcount;
  extern int listtotal;
#endif

#ifdef SUNPRO
enum bool {false, true};
#endif

//#undef OZ_DEBUG
//=============================================================================
//#include "_generic.hh"


// memory management:

#define CPIHEAPINITSIZE 100000

class CpiHeapClass {
private:
  int _init_heap_size;

  char * _heap, * _heap_top;
  int _heap_size, _heap_left;
  struct _heap_t { 
    char * heap; _heap_t * next;
        _heap_t(char * h,  _heap_t * p) : heap(h), next(p) {}
  } * _aux_heaps;  
public:
  CpiHeapClass(void) {}

  void init (int size = CPIHEAPINITSIZE) 
  {
    _init_heap_size = _heap_size = _heap_left = size;
    _aux_heaps = NULL;
    _heap = _heap_top = new char[_heap_size];
  }

  ~CpiHeapClass(void) 
  {
    delete [] _heap_top;
  }
  void * alloc (size_t s) 
  {
    //printf("entering alloc (par=%d)\r\n", s);
    int tmp_size = (s + (8 - (s & 7)));
    _heap_left -= tmp_size;

    if (_heap_left >= 0) {
      char * tmp = _heap;
      
      _heap += tmp_size;
      
      return tmp;
    } else { 
      if (tmp_size > _heap_size)
	_init_heap_size = tmp_size;
      

      _aux_heaps = new _heap_t(_heap_top, _aux_heaps);

      _heap = (_heap_top = new char[_heap_size]) + tmp_size;

      if (!_heap)
	printf("CPI heap memory exhausted.\r\n");
      _heap_left = _heap_size - tmp_size;
      return _heap_top;
    }

  }
  void reset(void) 
  {
    if (_aux_heaps) {
      int nb_heaps = 1;

      delete [] _heap_top;

      while (_aux_heaps) {
	nb_heaps += 1;
	
	delete [] _aux_heaps->heap;
	
	_heap_t * aux = _aux_heaps;     
	_aux_heaps = _aux_heaps->next;
	delete aux;
      }
      _aux_heaps = NULL;

      _heap_left = _heap_size = nb_heaps * _init_heap_size;
      _heap = _heap_top = new char[_heap_size];
    } else {
      _heap = _heap_top;
      _heap_left = _heap_size;
    }
  }
};

extern CpiHeapClass CpiHeap;

extern CpiHeapClass memory;

typedef void* GenPtr;

#ifdef OZ_DEBUG
#define DEBUG(C) printf C; fflush(stdout);
#else
#define DEBUG(C)
#endif

class generic {
public:
  virtual void write() const {
    DEBUG(("generic class (should be overloaded)\r\n"));
  }
  virtual void read() const {    
  }

  static void* operator new(size_t s) {
    return memory.alloc(s);
  }

  static void operator delete(void*, size_t) {}

#ifndef SUNPRO
  static void* operator new[](size_t s) {
    return memory.alloc(s);
  }

  static void operator delete[](void*, size_t) {}
#endif
};

//=============================================================================
//#include "_list.hh"

// doubly linked list, template content

#define forall(e,L) for (e=(L).first(); e; e=(L).next(e) )

template<class T>
struct dlink :public generic {
  dlink<T> *last;
  dlink<T> *next;
  T e;

  dlink(dlink <T>*_last, dlink<T> *_next, T _e) 
  : last(_last), next(_next), e(_e) {
#ifdef OZ_DEBUG
    dlinkcount++;
    dlinktotal++;
#endif
  }
  // caution.
  virtual ~dlink() { 
#ifdef OZ_DEBUG
    dlinkcount--;
#endif 
    //if (next) delete next; 
  }
};

template<class T>
class list :public generic {
  dlink<T> *root;
  dlink<T> *end;
  int sz;
public:
  list<T> () 
  : root(NULL), end(NULL), sz(0) { 
#ifdef OZ_DEBUG
    listcount++;
    listtotal++;
#endif
  }

  list<T> (list<T> &E) :generic() {
    dlink<T> *p = E.root;
    dlink<T> *q = NULL;
    dlink<T> *r = NULL;
    sz = 0;
    while(p) {
      q = new dlink<T>(r, NULL, p->e);
      if (!sz) root  = q;
      if (r) r->next = q;
      r = q;
      p = p->next;
      sz++;
    }
    end = q;
#ifdef OZ_DEBUG
    listcount++;
    listtotal++;
#endif
  }

  virtual ~list() { 
    //clear();
#ifdef OZ_DEBUG 
    listcount--;
#endif
  }

  inline void clear() {        
    if(sz == 0) { 
      root = end = NULL; 
      return; 
    }
    
    if (root) delete root;
    root = end = NULL;
    sz = 0;
  }
   
  inline list<T> &operator=(const list<T> &E) {
    if (this != &E) {
      dlink<T> *p = E.root;
      dlink<T> *q = NULL;
      dlink<T> *r = NULL;
      clear(); 
      sz = 0;
      while(p) {
        q = new dlink<T>(r, NULL, p->e);
        if (!sz) root  = q;
        if (r) r->next = q;
        r = q;
        p = p->next;
        sz++;
      }
      end = q;
    }
    return *this;
  }

  inline int size() const { return sz; }

  inline void remove(T e) {
    dlink<T> *p = search(e);
    if (p) { 
      dlink<T> *n = p->next;
      dlink<T> *l = p->last;
      if (l) l->next = n; else root = n;
      if (n) n->last = l; else end = l;
      sz--;
      p->next = NULL;
      p->last = NULL;
      delete p;
    }
  }

  inline dlink<T> *append(T e) {
    dlink<T> *i = new dlink<T>(end, NULL, e);
    if (end) 
      end->next = i; 
    else {
      root = i;
      //NEW. SHOULD BE OKAY, TAKEN OUT SINCE NOT TESTED.
      i->last = NULL;
    }
    end = i;
    sz++;
    return i;
  }

  inline dlink<T> *push(T e) {
    dlink<T> *i = new dlink<T>(NULL, root, e);
    if (root) 
      root->last = i;      
    else 
      end = i;
    root = i;
    sz++;
    return i;
  }

  inline T pop() {
    T retval = root->e;
    remove(root->e);
    return retval;
  }  

  inline void swap(list<T> &a) {
    dlink<T> *t_root = a.root;
    dlink<T> *t_end  = a.end;
    int t_sz         = a.sz;

    a.root = root;
    a.end  = end;
    a.sz   = sz;
    root   = t_root;
    end    = t_end;
    sz     = t_sz;
  } 

  inline dlink<T> *search(T x) {
    dlink<T> *p = root;
    while(p) {
      if (p->e == x) return p;
      p = p->next;
    }
    return NULL;
  }

  inline bool empty() { return (sz == 0); }

  inline dlink<T> *first() const { return root; }

  inline dlink<T> *last() const { return end; }

  inline dlink<T> *next(dlink<T> *current) const {
    if (!current)       
      return NULL;
    if (!current->next) 
      return NULL;
    return current->next;
  }

  void write() const {
    DEBUG(("List(size=%d) [", sz));
    dlink<T> *i = root;
    while(i) {
      /*
      DEBUG((" %p", (T*)(i->e)));
      i = i->next;
      */
    }
    DEBUG(("]\r\n"));
  }
}; 

//=============================================================================
//#include "_bqueue.hh"

// bounded queue template implementation

template <class t>
class b_queue :public generic {
private:
  t *q; // the queue
  int start;
  int end;
  int size;
  int maxsize;

public:
  b_queue(int sz) 
  : maxsize(sz), size(0), start(0), end(0) {
#ifdef OZ_DEBUG
    if (!sz) {
      DEBUG(("bqueue: size 0, no way.\r\n"));
      exit(-1);
    }
#endif
    q = new t[maxsize];
  }
  virtual ~b_queue() {
    delete [] q;
  }

  inline void append(t e) {
#ifdef OZ_DEBUG
    if (size == maxsize) {
      DEBUG(("b_queue: Overflow -- append ignored\r\n"));
      return;
    }
#endif
    if (size == 0) 
      q[end] = e;
    else {
      end = (end + 1) % maxsize;
      q[end] = e;
    }
    size++;
  }
  inline bool empty() { return (size == 0); }
  inline t pop() {
#ifdef OZ_DEBUG
    if (empty()) {
      DEBUG(("pop err: queue empty\r\n"));
      exit(-1);
    }
#endif
    t retval = q[start];
    if (start != end) start=(start+1) % maxsize;
    size--;
    return retval;  
  }

  void write() const {
    DEBUG(("b_queue ["));
    int pos = start;
		      
    //if (size) DEBUG(("%d", q[start]));
    if (start != end) do {
      pos = (pos+1) % maxsize;
      //DEBUG((" %d", q[pos]));
    } while (pos != end);    
    DEBUG(("] "));
    DEBUG(("start: %d, end: %d, curr_size: %d/%d\r\n", 
	   start, end, size, maxsize));
  }
};

//=============================================================================
//
//#include "_noleda.hh"

// implementation of the LEDA functions used by the all-different propagator
// (and ONLY these)
// WARNING. Functionality has been altered for efficiency in this particular
//          application, so the semantics may NOT be correct for arbitrary
//          graphs.
// eg. edge_array and node_array implementations:
// the size of the generated array is _not_ the size of elements, but the
// MAXIMAL size of the elements in the graph up to the point of generation.
// this does not affect anything in the current case, since these array are
// not created multiple times with a shrunken graph.

//#include "_generic.hh"

class edge_struct;
class node_struct;

typedef edge_struct* edge;
typedef node_struct* node;

template<class t> class edge_array;
template<class t> class node_array;

#define forall_adj_edges(e, n) \
 n->edgeiterator = NULL; while (n->next_adj_edge(e,n))

// CAUTION: these macros differ from the LEDA implementation in that they
// do solely depend on the graph iterator settings in taking successors.

#define forall_nodes(v, G) for (v = (G).first_node(); v; v = (G).next_node())
#define forall_edges(e, G) for (e = (G).first_edge(); e; e = (G).next_edge())

struct node_struct :public generic {
  node_struct(int _val, bool _free) 
    : free(_free), val(_val), component(-1), dtime(-1), ftime(-1),
      edgeiterator(NULL) {
#ifdef OZ_DEBUG    
    nodecount++;
#endif
  }
  virtual ~node_struct() { 
#ifdef OZ_DEBUG
    nodecount--;
#endif 
}
  bool free;
  int val;
  int id;
  int component;
  dlink<edge> *edgeiterator;

  // these are needed for the scc dfs, maybe to be abandoned later.
  int colour; // 0 = white, 1 = grey, 2 = black
  int dtime;  // discovery time
  int ftime;  // finishing time

  list<edge> out; // leaving edges
  list<edge> in;  // incoming edges
  
  inline bool next_adj_edge(edge &e, node v) const;

  void write() const {
    DEBUG((
    "node#%d [in %d out %d] (free %d val %d) <dtime %d ftime %d> scc: %d\r\n",
      id, in.size(), out.size(), free, val, dtime, ftime, component));
  }
};

struct edge_struct :public generic {
  edge_struct() {
    used      = false; 
    bfs_visit = false;
#ifdef OZ_DEBUG
    edgecount++;
#endif
  }
  virtual ~edge_struct() { 
#ifdef OZ_DEBUG
    edgecount--;
#endif
  }
  bool used;
  //bool vital;   // not needed in this version
  bool bfs_visit;
  int id;
  node src; // source node
  node dst; // destination node
  void write() const {
    DEBUG(("edge#%d (%d -> %d)", id, src->id, dst->id));
  }
};

// ---------------------------------------------------------------------------

inline bool node_struct::next_adj_edge(edge &e, node v) const {
  //  DEBUG(("next_adj_edge entered, iterator=%d\r\n", v->edgeiterator));
  if (v->edgeiterator) { // already set
    if (!v->edgeiterator->next) {
      e = NULL;
      return false;
    }
    else 
      v->edgeiterator = v->edgeiterator->next;
  }
  else { // not set yet
    v->edgeiterator = v->out.first();   
  }

  if (v->edgeiterator) e = v->edgeiterator->e; 
  else e = NULL;

  return (e)? true : false;
}

// ---------------------------------------------------------------------------

class graph :public generic {
friend class edge_array<int>;
friend class node_array<edge>;
private:
  int mark; // for mcb matching

  list<node> N;
  list<edge> E;
  int nodecount;
  int edgecount;
  list<node> order_for_second_dfs;

  dlink<node> *nodeiterator;
  dlink<edge> *edgeiterator;


  void dfs_visit(node n, int &time, int direction, int component);

  // from mcb match:
  edge find_next_augmenting_path(node s, node t,
                                 node_array<edge>& pred,
                                 edge_array<int>& layered);

  bool bfs(node s, node t, edge_array<int>& layered);
  
public:
  inline list<edge> MAX_CARD_BIPARTITE_MATCHING(const list<node>& A, 
						const list<node>& B );

  graph()
    : nodecount(0), edgecount(0), edgeiterator(NULL), nodeiterator(0),
      mark(0) { }

  virtual ~graph() {
    // waste();
  }
  
  inline void waste();
  inline node new_node(int _val, bool _free = true);
  edge new_edge(node n1, node n2);
  inline void del_edge(edge d);
  inline void del_node(node n);
  inline void rev_all_edges();
  inline void rev_edge(edge e);
  inline node source(edge e) { return e->src; }
  inline node target(edge e) { return e->dst; }
  inline int indeg(node n)   { return n->in.size(); }
  inline int outdeg(node n)  { return n->out.size(); }
  void mark_strong_components();
  inline list<edge> MAX_CARD_BIPARTITE_MATCHING();

  inline node first_node() { 
    nodeiterator = N.first();
    node retval;
    if (nodeiterator) retval = nodeiterator->e; 
    else retval = NULL;
    return retval;
  }

  inline node next_node() { 
    node retval;
    nodeiterator = N.next(nodeiterator);
    if (nodeiterator) retval = nodeiterator->e; 
    else retval = NULL;
    return retval;
  }

  inline edge first_edge() { 
    edgeiterator = E.first();
    edge retval;
    if (edgeiterator) retval = edgeiterator->e; 
    else retval = NULL;
    return retval;
  }

  inline edge next_edge() {
    edge retval;
    edgeiterator = E.next(edgeiterator);
    if (edgeiterator) retval = edgeiterator->e; 
    else retval = NULL;
    return retval;
  }

  inline bool next_adj_edge(edge &e, node v) const {
    return v->next_adj_edge(e, v);
  }

  inline int nodemax()         { return nodecount; }
  inline int edgemax()         { return edgecount; }
  inline int number_of_nodes() { return N.size(); }
  inline int number_of_edges() { return E.size(); }

  inline void reset() { // for iterators.
    edgeiterator = NULL; 
    nodeiterator = NULL;
  }

  void write() const;  
};

inline
void graph::write() const {
    dlink<node> *n;
    dlink<edge> *e;

    DEBUG(("graph with %d nodes and %d edges\r\n", N.size(), E.size()));
    DEBUG(("nodes:\r\n"));
    N.write();
    forall(n, N) 
      (n->e)->write();      
    
    DEBUG(("edges:\r\n"));
    E.write();
    forall(e, E) 
      (e->e)->write();    
}

inline node graph::new_node(int _val, bool _free) {
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
  (d->src)->out.remove(d);
  (d->dst)->in.remove(d);
  E.remove(d);
  delete d;
}

inline void graph::del_node(node n) {
  // does delete all incident edges, too.
  /* GRAH!
     dlink<edge> *i;
     forall(i, n->out) del_edge(i->e);
     forall(i, n->in)  del_edge(i->e);
  */
  while((n->out).size()) del_edge((n->out).first()->e);
  while((n->in).size())  del_edge((n->in).first()->e);
  N.remove(n);
  delete n;
}

inline void graph::waste() {
  // delete all nodes (and all edges with it).
  // can not be called from within destructor for some strange reason.
  int j = N.size();
  for(int i = j; i--;)
    del_node(N.first()->e);
}


inline void graph::rev_edge(edge e) {
  // change the edge
  node temp = e->dst;
  e->dst    = e->src;
  e->src    = temp;

  // change at nodes i/o lists
  e->dst->in.push(e);
  e->dst->out.remove(e);
  e->src->out.push(e);
  e->src->in.remove(e);
}

inline void graph::rev_all_edges() {
  // faster than calling for each edge individually
  dlink<edge> *i;
  dlink<node> *j;
  edge e;
  node n;

  // revert edges
  forall(i, E) {
    e      = i->e;
    n      = e->src;
    e->src = e->dst;
    e->dst = n;
  }
  // change node in/out lists
  forall(j, N) 
    (j->e)->out.swap((j->e)->in);
}

inline
void graph::mark_strong_components() {
  // Vorgehensweise:
  // a. do a DFS in the transponed graph.
  // b. do a DFS in the normal graph with node ordered by falling 'finish'
  //    values from first DFS.
  // c. mark nodes in the same DFS tree as strongly connected. 

  int time;            // dfs time
  int component = 0;   // component counter
  dlink<node> *item;

  order_for_second_dfs.clear();

  // paint all nodes white (might not be necessary)

  forall(item, N) (item->e)->colour = 0;
  time = 0;
  forall (item, N) {
    // transponed dfs:
    if (!(item->e)->colour) dfs_visit(item->e, time, 1, 0);     
  }

  // paint nodes white again and reset timer
  forall(item, N) (item->e)->colour = 0;
  time = 0;

  forall (item, order_for_second_dfs) {
    // normal dfs:
    if (!(item->e)->colour) dfs_visit(item->e, time, 0, ++component);
  }
  DEBUG(("leaving strong_components\r\n"));
}

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
    // initialize
    for (int i = 0; i < size; i++) array[i] = initval;
  }
  virtual ~edge_array() { 
    delete [] array; 
  }
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
    // initialize
    for (int i = 0; i < size; i++) array[i] = initval;
  }
  virtual ~node_array() { 
    delete [] array; 
  }
  t &operator[](node n) {
    int pos = n->id;
    return array[pos];
  }
  void write() const {
    DEBUG(("node_array, size=%d\r\n", size));
  }
};

inline
bool graph::bfs(node s, node t, edge_array<int>& layered) { 

  node_array<int> dist(*this, -1);
  b_queue<node> Q(number_of_nodes());
  node v;
  node w;
  edge e;

  Q.append(s);
  dist[s] = 0;

  while (!Q.empty()) { 
    v = Q.pop();
    if (v == t) 
      return true;
    
    int dv = dist[v];

    forall_adj_edges(e, v) {
      w = e->dst;
      if (dist[w] < 0) {
        
	Q.append(w); 
        dist[w] = dv+1;
      }
      if (dist[w] == dv+1) layered[e] = mark;
    }
  }
  return false;
}


/* ------------------------------------------------------------------------ */

inline list<edge> graph::MAX_CARD_BIPARTITE_MATCHING(const list<node>& A, 
					      const list<node>& B ) {

  // G is a bipartite graph with sides A and B, all edges must be directed 
  // from A to B. We compute a matching of maximum cardinality using the 
  // algorithm of Hopcroft and Karp. The running time is O(sqrt(n)*m).

  node v;
  edge e;
  dlink<node> *i;

   // heuristic for initial matching
  
  forall_edges(e, *this)
    if ((!indeg(e->src)) && (!outdeg(e->dst))) {
      rev_edge(e);
    }

  list<edge> EL;

  // source and target nodes get invalid values

  node s = new_node(-1);
  node t = new_node(-1);

  node_array<edge> pred(*this, 0); 
  
  DEBUG(("\r\nIterating A:"));
  forall(i, A) 
    if (!indeg(i->e)) 
      new_edge(s, i->e);
    
  DEBUG(("\r\nIterating B:"));
  forall(i, B) 
    if (!outdeg(i->e)) 
      new_edge(i->e, t);

  edge_array<int> layered(*this, 0);
  mark = 1;

  edgeiterator = NULL; 
  nodeiterator = NULL;

  for(;;) {
    forall_nodes(v, *this)  
      pred[v] = 0; 

    mark++;

    if (bfs(s, t, layered)) {
      // there is at least one augmenting path 
      while ((e = find_next_augmenting_path(s, t, pred, layered))) 
        EL.append(e); 
    
      edgeiterator = NULL;
      nodeiterator = NULL;      

      while (!EL.empty()) {
        edge e = EL.pop();
        edge x = pred[e->src];
        while (x->src != s) {
          rev_edge(x);
          x = pred[x->dst];
        }
        del_edge(e);  // edge into target
        del_edge(x);  // edge out of source
      } 
    }
    else break;
  } 
  
  list<edge> result;

  forall(i, B) 
    { forall_adj_edges(e, i->e) 
        if (e->dst != t) 
           result.append(e);
        else
           EL.append(e);
    }
  

  // restore old graph:
  dlink<edge> *j;

  forall(j, EL) 
    del_edge(j->e);
  
  forall(j, result) 
    rev_edge(j->e);
  

  del_node(s);
  del_node(t);
  
  DEBUG(("Matching size:%d\r\n", result.size()));
  return result;
}









//=============================================================================

class CompleteAllDistProp : public Propagator_VD {
  friend INIT_FUNC(fdp_init);
private:
  bool isFailed;
  OZ_NonMonotonic _nm;

  static OZ_PropagatorProfile profile;

  static int init_memory_management;
 
  void buildGraph(graph &g, OZ_FDIntVar *l, list<node> &A, list<node> &B);

  list<edge> removeEdgesFromG(graph &g, list<edge> matching, OZ_FDIntVar *reg);

  void computeMaximumMatching(list<edge> &matching, graph &g);

  void mark_bfs(graph &g, node n, int &markcounter);

  OZ_Return xpropagate();
  
public: 
  virtual OZ_Boolean isMonotonic(void) const { 
    return OZ_FALSE; 
  }

  virtual OZ_NonMonotonic::order_t getOrder(void) const {
    return _nm.getOrder();
  }
  
  CompleteAllDistProp(OZ_Term t) : Propagator_VD(t) 
  {
    if (init_memory_management) {
      memory.init(1000000);
      init_memory_management = 0;
    }
  };

  virtual OZ_Return propagate(void);

  virtual OZ_PropagatorProfile * getProfile(void) const {
    return &profile;
  }
  
  virtual OZ_Term getParameters(void) const {
    return Propagator_VD::getParameters(); 
  }

  virtual void gCollect(void) {
    Propagator_VD::gCollect();
  }

  virtual void sClone(void) {
    Propagator_VD::sClone();
  }

  virtual size_t sizeOf(void) {
    return sizeof(*this);
  }
};

// eof
//=============================================================================
