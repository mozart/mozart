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

template <class T,Bool(*lt)(const T&,const T&)>
inline
void sort_exchange(T &a, T &b) {
  if (lt(b,a))
    sort_swap(a,b);
}

class QuickSortStack {
private:
  static const int maxsize = 32;
  int stack[2*maxsize];
  int tos;
public:
  QuickSortStack() : tos(0) {};
  int isEmpty(void) {
    return tos == 0;
  }
  void push(int l, int r) {
    stack[tos++] = l;
    stack[tos++] = r;
  }
  void pop(int &l, int &r) {
    r = stack[--tos];
    l = stack[--tos];
  }
};

template <class T,Bool(*lt)(const T&,const T&)>
inline
void insertion(T * x, int l, int r) {
  int i;
  for (i = r; i > l; i--)
    sort_exchange<T,lt>(x[i-1],x[i]);
  for (i = l+2; i <= r; i++) {
    int j = i;
    T v = x[i];
    while (lt(v,x[j-1])) {
      x[j] = x[j-1]; j--;
    }
    x[j] = v;
  }
}

int const QuickSortCutoff = 10;

template <class T,Bool(*lt)(const T&,const T&)>
inline
int partition(T * x, int l, int r) {
  int i = l-1;
  int j = r;
  T v = x[r];
  while (1) {
    while (lt(x[++i],v));
    while (lt(v,x[--j])) if (j == l) break;
    if (i >= j) break;
    sort_swap_d(x[i], x[j]);
  }
  sort_swap_d(x[i],x[r]);
  return i;
}

template <class T,Bool(*lt)(const T&,const T&)>
inline
void quicksort(T * x, int l, int r) {
  QuickSortStack s;
  s.push(l,r);
  while (!s.isEmpty()) {
    s.pop(l,r);
  nopush:
    if (r-l <= QuickSortCutoff)
      continue;
    swap(x[(l+r)/2],x[r-1]);
    sort_exchange<T,lt>(x[l],x[r-1]);
    sort_exchange<T,lt>(x[l],x[r]);
    sort_exchange<T,lt>(x[r-1],x[r]);
    int i = partition<T,lt>(x, l+1, r-1);
    if (i-l > r-i) {
      s.push(l,i-1); l=i+1; goto nopush;
    } else {
      s.push(i+1,r); r=i-1; goto nopush;
    }
  }
}

template <class T,Bool(*lt)(const T&,const T&)>
inline
void fastsort(T * x, int n) {
  if (n < 2)
    return;
  if (n > QuickSortCutoff) 
    quicksort<T,lt>(x,0,n-1);
  insertion<T,lt>(x,0,n-1);
}

#endif
