#include <stdlib.h>
#include "_generic.hh"


#define forall(e,L) for (e=(L).first(); e; e=(L).next(e) )

struct dlink {
  dlink *last;
  dlink *next;
  GenPtr e;

  dlink(dlink *_last, dlink *_next, GenPtr _e) {
    last = _last; next = _next; e = _e;
  }
  ~dlink() {     
    if (next) delete next;
  }
};

typedef dlink* list_item;

template<class T> class list :public generic {
  list_item root;
  list_item end;  // end of list
  int sz;
public:
  list<T> () { 
    root = NULL; 
    end = NULL; 
    sz = 0; 
  }
  inline list<T> (list<T> &E) {
    // cout << "list: copy constructor" << endl;
    list_item p = E.root;
    list_item q = NULL;
    list_item r = NULL;
    sz = 0;
    while(p) {
      q = new dlink(r, NULL, p->e);
      if (!sz) root = q;
      if (r) r->next = q;
      r = q;
      p = p->next;
      sz++;
    }
    end = q;
  }
  virtual ~list() { 
    clear(); 
  }

  inline void clear() {
    if(!sz) { root=NULL; end=NULL; return; }
    if (root) delete root;
    root = NULL;
    end = NULL;
    sz = 0;
  }
   
  inline list<T> &operator=(const list<T> &E) {
    if (this != &E) {
      list_item p = E.root;
      list_item q = NULL;
      list_item r = NULL;
      clear(); 
      sz = 0;
      while(p) {
        q = new dlink(r, NULL, p->e);
        if (!sz) root = q;
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

  inline GenPtr remove(GenPtr e) {
    list_item p = search(e);
    if (p) { 
      dlink *n = p->next;
      dlink *l = p->last;
      if (l) p->last->next = n; else root = n;
      if (n) p->next->last = l; else end = l;
      sz--;
      free(p);
    }
    if (p) return e; else return NULL;
  }

  inline list_item append(GenPtr e) {
    list_item i = new dlink(end, NULL, e);
    if (end) end->next = i; 
      else root = i;
    end = i;
    sz++;
    return i;
  }

  inline list_item push(GenPtr e) {
    list_item i = new dlink(NULL, root, e);
    if (root) root->last = i;
      else end = i;
    root = i;
    sz++;
    return i;
  }

  inline GenPtr pop() { // new.
    GenPtr retval = (root->e);
    remove(root->e);
    return retval;
  }  

  inline void swap(list<T> &a) {
    list_item t_root = a.root;
    list_item t_end = a.end;
    int t_sz = a.sz;

    a.root = root;
    a.end = end;
    a.sz = sz;

    root = t_root;
    end = t_end;
    sz = t_sz;
  } 

  inline list_item search(GenPtr x) {
    list_item p = root;
    while(p) {
      if (p->e == x) return p;
      p = p->next;
    }
    return NULL;
  }

  inline bool empty() { return (sz == 0); }

  inline list_item first() const { 
    return root; 
  }

  inline list_item last() const {
    return end;
  }

  inline list_item next(list_item current) const {
    if (!current) return NULL;
    if (!current->next) return NULL;
    return current->next;
  }

  void write() const {
    DEBUG(("List(size=%d) [", sz));
    list_item i = root;
    while(i) {
      DEBUG((" %p", (T*)(i->e)));
      i = i->next;
    }
    DEBUG(("]\r\n"));
  }
}; 


/*
int main() {
  
   
  list<int> L;
  int *w=new int(1);  
  int *x=new int(2);
  int *y=new int(3);
  int *z=new int(4);
  

  list<int> M;
  
  L.append(x);
  L.append(y);
  L.append(z);
  M.append(x);
  cout << L << endl;
  cout << L.search(y) << endl;
  cout << L.search(w) << endl;
  M = L;
  cout << M << endl;
  list_item i;
  forall(i,L) {
    cout << *(int*)i->e << endl;
  }  
  cout << L.remove(L.next(L.next(L.first()))->e);
  cout << L << endl;

  L.clear();
  cout << L << endl;
  L.append(w);
  L.append(x);
  cout << L << endl;
  list_item i;
  i=L.remove(x);
  cout << L << endl;
  i=L.remove(w);
  cout << L << endl;
  

  L.clear();
  cout << L << endl;
  
  
}

*/



