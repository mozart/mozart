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
 *    Raphael Collet, 2006
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

// ****************** SMALL FUNS ********************
//
template <class T1>
T1 t_max(const T1& t1, const T1& t2){ return ((t1 > t2) ? t1 : t2); }

template <class T1>
T1 t_min(const T1& t1, const T1& t2){ return ((t1 < t2) ? t1 : t2); }

  // ************* Garbage collect a list ************
  //
  //Yves: Probably unused, replaced by t_gcList(SimpleList) below.
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
template <typename T> struct SimpleNode {
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
  Position(SimpleList<T> &s) : curPtr(&(s.first)) {}
  Position(SimpleList<T> *s) : curPtr(&(s->first)) {}

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
  Position<T> operator++ (int) { next(); return *this; }
  Position<T>& operator++ () { next(); return *this; }
  bool operator== (Position<T> const &p) { return curPtr == p.curPtr; }
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
  int size() const {
    int len = 0;
    for (Position<T> p(const_cast<SimpleList<T>*>(this)); p(); p++) len++;
    return len;
  }
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
  using SimpleList<T>::size;
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



// An implementation of a ring with SimpleNodes.
template <typename T>
class SimpleRing {
private:
  SimpleNode<T>* pred;     // predecessor of the current node
  int            sz;       // how many elements

public:
  bool isEmpty() const { return pred == NULL; }
  int size() const { return sz; }

  // access the current element, its neighbors, and step forward
  T& predecessor() const { return pred->elem; }
  T& current() const { return pred->next->elem; }
  T& successor() const { return pred->next->next->elem; }
  void step() { pred = pred->next; }

  // basic operations: find(), push(), and pop()
  bool find(T const e) {
    for (int n = size(); n > 0; n--) {
      if (current() == e) return true; else step();
    }
    return false;
  }
  void push(T const e) { // before the current element
    if (pred) {
      pred->next = new SimpleNode<T>(e, pred->next);
    } else {
      pred = new SimpleNode<T>(e, NULL);
      pred->next = pred;
    }
    sz++;
  }
  T pop() { // the current element
    SimpleNode<T>* cur = pred->next;
    T e = cur->elem;
    if (cur == pred) pred = NULL; else pred->next = cur->next;
    delete cur;
    sz--;
    return e;
  }

  SimpleRing() : pred(NULL), sz(0) {}
  ~SimpleRing() { while (!isEmpty()) pop(); }
};

#endif
