/*
 *  Authors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Michael Mehl (mehl@dfki.de)
 *    Konstantin Popov (kost@sics.se)
 *    Christian Schulte <schulte@ps.uni-sb.de>
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


#ifndef __MEMORYH
#define __MEMORYH

#ifdef INTERFACE
#pragma interface
#endif


#include "base.hh"
#include "statisti.hh"

#include <stdlib.h>
#include <stdio.h>


#ifdef CS_PROFILE
extern Bool across_chunks;
#endif

class MemChunks {
public:
  static MemChunks *list;

#ifdef DEBUG_MEM
  static Bool isInHeap(TaggedRef term);
  static Bool areRegsInHeap(TaggedRef *regs, int size);
#endif

private:
  int xsize;
  char *block;
  MemChunks *next;

public:
  MemChunks(char *bl, MemChunks *n, int sz) : xsize(sz), block(bl), next(n) {};
  void deleteChunkChain(void);
  Bool isInBlock(void *value) {
    return ((block <= (char *) value) && ((char *) value < block + xsize));
  }

#ifdef DEBUG_MEM
  Bool inChunkChain(void *value);
  void print(void);
#endif

};



/*
 * Basic aligned allocation routine:
 *  - Supports alignment of 8
 *  - OZ_HEAPALIGNMENT MUST BE POWER OF 2
 *
 */

#define OZ_HEAPALIGNMENT 8

#ifndef HEAPCURVOLATILE
#define HEAPCURVOLATILE
#endif

#ifndef HEAPENDVOLATILE
#define HEAPENDVOLATILE
#endif

extern HEAPCURVOLATILE char * _oz_heap_cur;
extern HEAPENDVOLATILE char * _oz_heap_end;

#ifdef TRACE_ALOVER
extern size_t _oz_alover;
#endif


void _oz_getNewHeapChunk(const size_t);

// Static(local) checks. The tagged.hh's 'isSTAligned()' are not
// available here.
#define oz_isHeapAligned(p)       (!(ToInt32(p) & (OZ_HEAPALIGNMENT - 1)))
#define oz_isDoubleHeapAligned(p) (!(ToInt32(p) & (2*OZ_HEAPALIGNMENT - 1)))


/*
 * Sum up to next multiple of OZ_HEAPALIGNMENT
 *
 */

#define oz_alignSize(sz) (((sz) + OZ_HEAPALIGNMENT - 1) & (-OZ_HEAPALIGNMENT))

inline
void * oz_heapMalloc(const size_t sz)
{
  /*
   * The following invariants are enforced:
   *  - _oz_heap_cur is always aligned to OZ_HEAPALIGMENT
   *
   */

 retry:
  {
    Assert(oz_isHeapAligned(_oz_heap_cur));
    size_t a_sz = oz_alignSize(sz);

#ifdef TRACE_ALOVER
    if (a_sz != sz) {
      _oz_alover += 4;
    }
#endif

    _oz_heap_cur -= a_sz;
    Assert(oz_isHeapAligned(_oz_heap_cur));

    /* _oz_heap_cur might be negative!! */
    if (((unsigned long) _oz_heap_end) > ((unsigned long) _oz_heap_cur)) {
      _oz_getNewHeapChunk(a_sz);
      goto retry;
    }

    return (void*) _oz_heap_cur;
  }
}


void initMemoryManagement(void);
void deleteChunkChain(char *);

#ifdef DEBUG_MEM

int inChunkChain(void *, void *);
void printChunkChain(void *);

#endif



/*
 * Misc
 *
 */

#define heapNewer(ptr1,ptr2) (((void *) (ptr1)) < ((void *) (ptr2)))


inline
void * oz_hrealloc(const void * p, size_t sz) {
  return memcpy(oz_heapMalloc(sz), p, sz);
}
 
/*
 * Memory management inquiry
 *
 */

extern unsigned int heapTotalSize;        // # kilo bytes allocated
extern unsigned int heapTotalSizeBytes;   // # bytes allocated

inline 
unsigned int getUsedMemory(void) {
  return heapTotalSize - (_oz_heap_cur - _oz_heap_end)/KB;
}

inline 
unsigned int getUsedMemoryBytes(void) {
  return heapTotalSizeBytes - (_oz_heap_cur - _oz_heap_end);
}



/*
 * Freelist management
 *
 */

// Maximal size of block in free list
#define FL_MinSize  8
#define FL_MaxSize  64

// Transformations between FreeListIndex and Size
#define FL_SizeToIndex(sz) ((sz) >> 3)
#define FL_IndexToSize(i)  ((i)  << 3)

// Alignment restrictions
#define FL_IsValidSize(sz) (!((sz) & (OZ_HEAPALIGNMENT - 1)))


/*
 * Small free memory blocks:
 *   Used for allocation
 */
class FL_Small {
private:
  FL_Small * next;
public:
  FL_Small * getNext(void) {
    return next;
  }
  void setNext(FL_Small * n) {
    next = n;
  }
};


/*
 * Large free memory blocks:
 *   Not used for allocation but to create new small blocks
 */
class FL_Large {
private:
  FL_Large * next;
  size_t     size;
public:
  FL_Large * getNext(void) {
    return next;
  }
  size_t getSize(void) {
    return size;
  }
  void setBoth(FL_Large * n, size_t s) {
    next = n;
    size = s;
  }
};


/*
 * Free List Manager
 *
 */

class FL_Manager {

private:
  static FL_Small * smmal[FL_SizeToIndex(FL_MaxSize) + 1];
  static FL_Large * large;

private:
  static void refill(void);

public:  
  static void init(void);

  static void * alloc(const size_t s) {
    Assert(FL_IsValidSize(s));
    if (s > FL_MaxSize) {
      return oz_heapMalloc(s);
    } else {
      FL_Small * f = smmal[FL_SizeToIndex(s)];
      Assert(f);
      FL_Small * n = f->getNext();
      smmal[FL_SizeToIndex(s)] = n;
      if (!n) 
	refill();
      return f;
    }
  }
  
  static void free(void * p, const size_t s) {
    Assert(FL_IsValidSize(s));
    if (s > FL_MaxSize) {
      FL_Large * f = (FL_Large *) p;
      f->setBoth(large,s);
      large = f;
    } else {
      FL_Small * f  = (FL_Small *) p;
      f->setNext(smmal[FL_SizeToIndex(s)]);
      smmal[FL_SizeToIndex(s)] = f;
    }
  }

  static unsigned int getSize(void);

};



/*
 * The real allocation routines:
 *
 */

#define oz_freeListMalloc(s)     (FL_Manager::alloc(oz_alignSize(s)))

#ifdef CS_PROFILE
#define freeListDispose(p,s)
#else
#define oz_freeListDispose(p,s) FL_Manager::free((p),oz_alignSize(s))
#endif

inline 
void * oz_heapDoubleMalloc(const size_t sz)
{
  /*
   * Returns block that is aligned to twice OZ_HEAPALIGNMENT
   *
   */
  size_t a_sz = oz_alignSize(sz);
  char * c = (char *) oz_heapMalloc(a_sz + OZ_HEAPALIGNMENT);
  if (ToInt32(c) & OZ_HEAPALIGNMENT) {
    oz_freeListDispose(c,OZ_HEAPALIGNMENT);
    return c+OZ_HEAPALIGNMENT;
  } else {
    oz_freeListDispose(c+a_sz,OZ_HEAPALIGNMENT);
    return c;
  }
}    


/*
 * Use this for dynamically allocated memory blocks that haven't been
 * allocated with the same size. For example, parts of memory areas
 * that are not longer needed. Unsafe will enforce the right alignment!
 *
 */
inline void oz_freeListDisposeUnsafe(void * p, size_t s) {
  char * p_c = (char *) p;
  size_t p_o = ((OZ_HEAPALIGNMENT - 
		 (ToInt32(p_c) & (OZ_HEAPALIGNMENT - 1))) 
		& (OZ_HEAPALIGNMENT - 1));
  size_t s_o = s - p_o;
  size_t s_a = s_o & (-OZ_HEAPALIGNMENT);
  if (s_a > 0) {
    oz_freeListDispose(p_c + p_o, s_a);
  }
} 



/*     
 * Redefine the operator "new" in every class, that uses memory
 * from the free list.  The same holds for memory used from heap. 
 *
 *
 */

#ifdef __GNUC__

#define USEFREELISTMEMORY				\
  static void *operator new(size_t chunk_size)		\
    { return oz_freeListMalloc(chunk_size); }		\
  static void operator delete(void *,size_t )		\
    { Assert(NO); }					\
  static void *operator new[](size_t chunk_size)	\
    { return oz_freeListMalloc(chunk_size); }		\
  static void operator delete[](void *,size_t )		\
    { Assert(NO); }

#define USEHEAPMEMORY					\
  static void *operator new(size_t chunk_size)		\
    { return oz_heapMalloc(chunk_size); }		\
  static void operator delete(void *,size_t)		\
    { Assert(NO); }					\
  static void *operator new[](size_t chunk_size)	\
    { return oz_heapMalloc(chunk_size); }		\
  static void operator delete[](void *,size_t)		\
    { Assert(NO); }

#else

#define USEFREELISTMEMORY			\
  static void *operator new(size_t chunk_size)	\
    { return oz_freeListMalloc(chunk_size); }	\
  static void operator delete(void *,size_t )	\
    { Assert(NO); }

#define USEHEAPMEMORY				\
  static void *operator new(size_t chunk_size)	\
    { return oz_heapMalloc(chunk_size); }	\
  static void operator delete(void *,size_t)	\
    { Assert(NO); }

#endif

#undef oz_isHeapAligned
#undef oz_isDoubleHeapAligned

#endif
