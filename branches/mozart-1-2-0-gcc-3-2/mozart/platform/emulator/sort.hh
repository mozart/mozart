/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Christian Schulte, 2001
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

#ifndef __SORT_HH__
#define __SORT_HH__

#include "base.hh"


/*
 * The following algorithms are largely based on the following book:
 * Robert Sedgewick, Algorithms in C++, 3rd edition, 1998, Addison Wesley.
 *
 */
 
template <class T>
inline
void sort_swap(T &a, T &b) {
  T t=a; a=b; b=t;
}

template <class Type, class LessThan>
inline
void sort_exchange(Type &a, Type &b, LessThan &lt) {
  if (lt(b,a)) 
    sort_swap(a,b);
}

static const int QuickSortCutoff = 10;

static const int QuickSortStack_maxsize = 32;

class QuickSortStack {
private:
  int stack[2*QuickSortStack_maxsize];
  int tos;
public:
  QuickSortStack(void);
  bool empty(void) const;
  void push(int, int);
  void pop(int&, int&);
};

inline
QuickSortStack::QuickSortStack(void) : tos(0) {
}
inline
bool QuickSortStack::empty(void) const {
  return tos == 0;
}
inline
void QuickSortStack::push(int l, int r) {
  stack[tos++] = l;
  stack[tos++] = r;
}
inline
void QuickSortStack::pop(int& l, int& r) {
  r = stack[--tos];
  l = stack[--tos];
}


template <class Type, class LessThan>
inline
void insertion(Type * x, int l, int r, LessThan &lt) {
  int i;
  for (i = r; i > l; i--)
    sort_exchange(x[i-1],x[i],lt);
  for (i = l+2; i <= r; i++) {
    int j = i;
    Type v = x[i];
    while (lt(v,x[j-1])) {
      x[j] = x[j-1]; j--;
    }
    x[j] = v;
  }
}

template <class Type, class LessThan>
inline
int partition(Type * x, int l, int r, LessThan &lt) {
  int i = l-1;
  int j = r;
  Type v = x[r];
  while (1) {
    while (lt(x[++i],v));
    while (lt(v,x[--j])) if (j == l) break;
    if (i >= j) break;
    sort_swap(x[i], x[j]);
  }
  sort_swap(x[i],x[r]);
  return i;
}

template <class Type, class LessThan>
inline
void quicksort(Type * x, int l, int r, LessThan &lt) {
  QuickSortStack s;
  s.push(l,r);
  while (!s.empty()) {
    s.pop(l,r);
  nopush:
    if (r-l <= QuickSortCutoff)
      continue;
    sort_swap(x[(l+r)/2],x[r-1]);
    sort_exchange(x[l],x[r-1],lt);
    sort_exchange(x[l],x[r],lt);
    sort_exchange(x[r-1],x[r],lt);
    int i = partition(x, l+1, r-1,lt);
    if (i-l > r-i) {
      s.push(l,i-1); l=i+1; goto nopush;
    } else {
      s.push(i+1,r); r=i-1; goto nopush;
    }
  }
}

template <class Type, class LessThan>
inline
void fastsort(Type* x, int n, LessThan &lt) {
  if (n < 2)
    return;
  if (n > QuickSortCutoff) 
    quicksort(x,0,n-1,lt);
  insertion(x,0,n-1,lt);
}


#endif
