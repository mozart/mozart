/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
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

#ifndef __CPI_HEAP__H__
#define __CPI_HEAP__H__

#include <stdio.h>

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
  CpiHeapClass(int size = CPIHEAPINITSIZE) 
    : _init_heap_size(size),
      _heap_size(size), _heap_left(size), _aux_heaps(NULL)
  {
    _heap = _heap_top = new char[_heap_size];
  }
  ~CpiHeapClass(void) 
  {
    delete [] _heap_top;
  }
  void * alloc (size_t s) 
  {
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
	OZ_error("CPI heap memory exhausted.");
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

#endif /* __CPI_HEAP__H__ */

// End of File
//-----------------------------------------------------------------------------

