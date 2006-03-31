/*
 *  Authors:
 *    Zacharias El Banna
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Zacharias El Banna, 2002
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
#ifndef __DSS_TEMPLATES
#define __DSS_TEMPLATES

// Place nice templates in here for code reusability
//

#include <stdlib.h> // For the macro 
#include "dss_enums.hh" // For the macro 

template<class T>
void t_swap(T& t1, T& t2){
  T tmp = t1; t1 = t2; t2 = tmp;
}

//
// The Oncecontainer should be extended with non-defined functions such as
// - m_makeGCpreps()
// - Destructor
// - m_stringrep() 

  // ****************** SMALL FUNS ********************
  //
  template <class T1>
  T1 t_max(const T1& t1, const T1& t2){ return ((t1 > t2) ? t1 : t2); }

  template <class T1>
  T1 t_min(const T1& t1, const T1& t2){ return ((t1 < t2) ? t1 : t2); }
 
  // **************** One element Containers **********
  //
  template <class T1>
  class OneContainer{
  public:
    T1* const a_contain1;
    OneContainer* a_next;

  public:
    OneContainer(T1* const t, OneContainer* const n):
      a_contain1(t),
      a_next(n){
    };
    inline void m_makeGCpreps(){ a_contain1->m_makeGCpreps(); }

    MACRO_NO_DEFAULT_CONSTRUCTORS(OneContainer);
  };

  template <typename T1>
  class OneTypeContainer{
  public:
    T1 a_contain1;
    OneTypeContainer* a_next;

  public:
    OneTypeContainer(const T1& t, OneTypeContainer* const n):
      a_contain1(t),
      a_next(n){
    };
    inline void m_makeGCpreps(){};

    MACRO_NO_DEFAULT_CONSTRUCTORS(OneTypeContainer);
  };


  // **************** Two element Containers **********
  //
  template <class T1, typename T2>
  class TwoContainer{
  public:
    T1*           a_contain1;
    T2            a_contain2;
    TwoContainer* a_next;

  public:


    TwoContainer(T1* const t1, const T2& t2, TwoContainer* const n):
      a_contain1(t1),
      a_contain2(t2),
      a_next(n){
    };
    
    inline bool operator==(const TwoContainer*& tc){ 
      return (tc->a_contain1 == a_contain1 && tc->a_contain2 == a_contain2); }

    inline void m_makeGCpreps(){ a_contain1->m_makeGCpreps(); }
    ~TwoContainer(){};

    MACRO_NO_DEFAULT_CONSTRUCTORS(TwoContainer);
  };

template <class T1, class T2>
class TwoClassContainer{
  public:
    T1* a_contain1;
    T2* a_contain2;
    TwoClassContainer* a_next;
    TwoClassContainer(T1* const t1, T2* const t2, TwoClassContainer* const n):
      a_contain1(t1),
      a_contain2(t2),
      a_next(n){
    };
    inline void m_makeGCpreps(){ a_contain1->m_makeGCpreps(); a_contain2->m_makeGCpreps(); }
    ~TwoClassContainer(){};
  };

  template <typename T1, typename T2>
  class TwoTypeContainer{
   public:
    T1 a_contain1;
    T2 a_contain2;
    TwoTypeContainer* a_next;
    TwoTypeContainer(const T1& t1, const T2& t2, TwoTypeContainer* const n):
      a_contain1(t1),
      a_contain2(t2),
      a_next(n){
    };
    inline void m_makeGCpreps(){}
    ~TwoTypeContainer(){}

    MACRO_NO_DEFAULT_CONSTRUCTORS(TwoTypeContainer);
  };

  // ************* List Operators *****************
  //

  template<class T1>
  void t_forAllMeth(T1* head, void (T1::*fptr)()){
    while(head){
      (head->*fptr)();
      head  = head->a_next;
    }
  }

  template<class T1, typename T2>
    T2 t_foldMeth(T1* head, void (T1::*fptr)(T2), T2 arg){
    while(head){
      arg = (head->*fptr)(arg);
      head  = head->a_next;
    }
    return arg; 
  }
    

  // ************* Garbage collect a list ************
  //

  template<class T1>
  void t_gcList(T1* const head){
    T1* tmp = head;
    while(tmp){
      tmp->m_makeGCpreps();
      tmp = tmp->a_next;
    }
  }


  // ************* Delete a list ********************
  //
  // Takes a pointer to a list structure and deletes all elements and
  // also sets the pointer to NULL
  //

  template<class T1>
  void t_deleteList(T1*& head){
    while(head){
      T1* tmp = head;
      head  = head->a_next;
      delete tmp;
    }
  }


  template<class T1>
  void t_deleteListCont(T1*& head){
    while(head){
      T1* tmp = head;
      head  = head->a_next;
      delete tmp->a_contain1;
      delete tmp;
    }
  }

  
  template<class T1>
  void t_deleteListAction(T1*& head, void (T1::*fptr)()){
    while(head){
      T1* tmp = head;
      head  = head->a_next;
      (tmp->*fptr)();
      delete tmp;
    }
  }
  
 
  // ************* Delete element from list ********************
  //
  // - return whether the object was found in the list or not
  // - safe to call with any pointer since it's const guarded
  //

  template<class T1>
  bool t_deleteElement(T1** const headptr, T1* elem){
    T1** tmp = headptr;
    while((*tmp) != NULL){
      if ((*tmp) != elem){
	tmp = &((*tmp)->a_next);
      }else {
	T1* tmpdel = (*tmp);
	(*tmp) = (*tmp)->a_next;
	delete tmpdel;
	return true;
      }
    }
    return false;
  }
  
  // *********** Delete Element with container equal to ****** 
  //
  // - return whether the object was found in the list or not
  // - safe to call with any pointer since it's const guarded
  // 

  template<class T1, class T2>
  bool t_deleteCompare(T1** const headptr, T2* const compare){
    T1** tmp = headptr;
    while((*tmp) != NULL){
      if ((*tmp)->a_contain1 != compare){
	tmp = &((*tmp)->a_next);
      } else {
	T1* tmpdel = (*tmp);
	(*tmp) = (*tmp)->a_next;
	delete tmpdel;
	return true;
      }
    }
    return false;
  }

  //
  // Same as above except that it removes all occurances with same
  // element
  //

template<class T1, class T2>
void t_deleteAllCompare(T1** const headptr, T2* const compare){
  T1** tmp = headptr;
  while((*tmp) != NULL){
    if (compare != (*tmp)->a_contain){
	tmp = &((*tmp)->a_next);
    } else {
      T1* tmpdel = (*tmp);
      (*tmp) = (*tmp)->a_next;
      delete tmpdel;
      }
  }
}







  // ******************* SIMPLE FIFO QUEUE *****************
  //
  // - Uses elements which has a "a_next" pointer and might do
  // m_makeGCpreps() (if method not invoked this isn't tested either)
  //
  // !! Remember, the last elements "next"-ptr is never touched !!
  // !! We say last element is when back and front is equal     !!
  //
  template <class T1>
  class FifoQueue{
  private:
    T1* a_front;
    T1* a_back;

    FifoQueue& operator=(const FifoQueue& f);
  public:
    FifoQueue( FifoQueue& f):
      a_front(f.a_front),a_back(f.a_back){
      f.a_back = NULL; 
      f.a_front = NULL;
    };
    FifoQueue():a_front(NULL),a_back(NULL){};
    

    void append(T1* const t){
      if (a_front != NULL){ // empty
	a_back->a_next = t;
	a_back = t;
      }else {
	a_front = a_back = t;
      }
      t->a_next = NULL;
    }
    
    T1* drop(){
      T1* tmp = a_front;
      if(a_front == a_back) a_front = a_back = NULL;   // tmp is last element
      else                  a_front = a_front->a_next; // at least one element left (besides tmp)
      return tmp; // Yes this might return NULL if the queue is empty
    }
    
    T1* peek(){ return a_front;}
    
    bool isEmpty(){ return (a_front == NULL); }

    unsigned int m_size() {
      int i = 0;
      for (T1* tmp = a_front; tmp != NULL; tmp = tmp->a_next) i++;
      return i;
    }

    void m_makeGCpreps(){ t_gcList(a_front); }

    ~FifoQueue(){ t_deleteList(a_front); };
  };



/******************** Templates added by raph ********************/

// Pairs can be used to make a list of tuples in general.  The
// conversion operator is useful when e1 is used as a key.
template <typename T1, typename T2>
struct Pair {
  T1 first; T2 second;
  Pair() {}
  Pair(T1 const &a, T2 const &b) : first(a), second(b) {}
};

template <typename T1, typename T2>
Pair<T1,T2> makePair(T1 const &a, T2 const &b) {
  return Pair<T1,T2>(a, b);
}



// The following templates implement simple linked lists.  I have
// defined the class Position in order to abstract a position in a
// list.  It effectively hides all the pointer juggling used when
// modifying the list.

template <typename T> class Position;
template <typename T> class SimpleList;
template <typename T> class SimpleQueue;

// the underlying nodes of a list
template <typename T> class SimpleNode {
  friend class Position<T>;
  friend class SimpleList<T>;
  friend class SimpleQueue<T>;
private:
  T elem;
  SimpleNode<T>* next;
  SimpleNode(T const &e, SimpleNode<T>* n) : elem(e), next(n) {}
};



// A position is a window on a given element in a list, or after the
// last element in that list.  The after-last position is very
// convenient for appending elements at the end of the list.
template <typename T>
class Position {
private:
  SimpleNode<T>** curPtr;     // a pointer to a pointer to the current node

public:
  void init(SimpleList<T> &s) { curPtr = &(s.first); }

  Position() : curPtr(NULL) {}
  Position(SimpleList<T> &s) { init(s); }
  Position(SimpleList<T> *s) { init(*s); }

  // check whether the position is empty, and return the element
  bool hasElement() const { return *curPtr; }
  bool isEmpty() const { return !hasElement(); }
  T& element() const { return (*curPtr)->elem; }

  // jump to next position
  void next() { curPtr = &((*curPtr)->next); }
  // push an element at the current position (shift current element)
  void push(T const &e) { *curPtr = new SimpleNode<T>(e, *curPtr); }
  // insert an element at current position, and shift position
  void insert(T const &e) { push(e); next(); }
  // remove the element at the current position
  void remove() {
    SimpleNode<T>* node = *curPtr;
    *curPtr = node->next;
    delete node;
  }
  // remove and return the element at the current position
  T pop() { T e = element(); remove(); return e; }
  // find the position of an element (after-last position if not found)
  bool find(T const &e) {
    while (hasElement() && !(element() == e)) next();
    return hasElement();
  }

  // similar to the ones above, but more convenient for pairs
  template <typename T1, typename T2>
  void push(T1 const &e1, T2 const &e2) {
    push(makePair(e1, e2));
  }
  template <typename T1, typename T2>
  void insert(T1 const &e1, T2 const &e2) {
    insert(makePair(e1, e2));
  }
  template <typename T1>
  bool find(T1 const &e1) { // find the pair whose first element is e1
    while (hasElement() && !(element().first == e1)) next();
    return hasElement();
  }

  // basic operator overloading: pos(l) sets pos at the first position
  // in l; pos() returns true if pos is nonempty; *pos returns the
  // element; ++pos and pos++ shift to next position.
  void operator() (SimpleList<T> &s) { init(s); }
  bool operator() () const { return hasElement(); }
  T&   operator*  () const { return element(); }
  void operator++ (int) { next(); }
  void operator++ () { next(); }
};



// The linked list itself.
template <typename T>
class SimpleList {
  friend class Position<T>;

private:
  SimpleNode<T>* first;     // the first node

  SimpleList& operator=(const SimpleList&);
  SimpleList(const SimpleList&);

public:
  SimpleList() : first(NULL) {}
  ~SimpleList() {
    while (first) { SimpleNode<T>* n = first; first = n->next; delete n; }
  }
  bool isEmpty() const { return first == NULL; }
  Position<T> front() { return Position<T>(this); }

  // Only operations push(), pop(), contains() and remove() are
  // provided.  For other operations, use a Position.
  void push(T const &e) { front().push(e); }
  T pop() { return front().pop(); }
  bool contains(T const &e) const {
    Position<T> p(const_cast<SimpleList<T>*>(this));
    return p.find(e);
  }
  bool remove(T const &e) {
    Position<T> p(this);
    return p.find(e) ? p.remove(), true : false;
  }
};



// Use these when your list contains pointers to objects with a method
// m_makeGCpreps().
template <class T>
void t_gcList(SimpleList<T*> &list) {
  for (Position<T*> p(list); p(); p++) (*p)->m_makeGCpreps();
}

template <class C1, class C2>
void t_gcList(SimpleList<Pair<C1*,C2*> > &list) {
  for (Position<Pair<C1*,C2*> > p(list); p(); p++) {
    (*p).first->m_makeGCpreps();
    (*p).second->m_makeGCpreps();
  }
}

template <class C1, typename T2>
void t_gcList(SimpleList<Pair<C1*,T2> > &list) {
  for (Position<Pair<C1*,T2> > p(list); p(); p++)
    (*p).first->m_makeGCpreps();
}



// An implementation of a FIFO queue
template <typename T>
class SimpleQueue : public SimpleList<T> {
private:
  Position<T> afterlast;     // the after-last position, for appending

  SimpleQueue& operator= (const SimpleQueue&);
  SimpleQueue(const SimpleQueue&);

public:
  SimpleQueue() : SimpleList<T>(), afterlast(*this) {}
  ~SimpleQueue() {}
  using SimpleList<T>::isEmpty;
  using SimpleList<T>::front;
  Position<T> rear() { return afterlast; }

  // queue operations: append(), peek(), and pop().
  void append(T const &e) { afterlast.insert(e); }
  T& peek() { return front().element(); }
  T pop() {
    T e = front().pop();
    if (isEmpty()) afterlast(*this);
    return e;
  }

  // call this when you modify the end of the list with a position!
  void check() {
    afterlast(*this);
    while (afterlast()) afterlast++;
  }
};

#endif
