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
  MemChunks *getNext() { return next; }

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

#ifdef HEAPCURINTOREGISTER
register char * _oz_heap_cur asm("g6");
#else
extern   char * _oz_heap_cur;
#endif

extern   char * _oz_heap_end;


void _oz_getNewHeapChunk(const size_t);

#ifdef DEBUG_CHECK
#define CHECK_ALIGNMENT(p) Assert(!(ToInt32(p) & (OZ_HEAPALIGNMENT - 1)));
#else
#define CHECK_ALIGNMENT(p)
#endif

/*
 * Sum up to next multiple of OZ_HEAPALIGNMENT
 *
 */

#define oz_alignSize(sz) (((sz) + OZ_HEAPALIGNMENT - 1) & (-OZ_HEAPALIGNMENT))

inline
void * oz_heapMalloc(const size_t sz) {
  /*
   * The following invariants are enforced:
   *  - _oz_heap_cur is always aligned to OZ_HEAPALIGMENT
   *  - if alignment == 4, then sz is multiple of 4
   *
   */

  CHECK_ALIGNMENT(_oz_heap_cur);

 retry:

  {
    size_t a_sz = oz_alignSize(sz);

    _oz_heap_cur -= a_sz;

    CHECK_ALIGNMENT(_oz_heap_cur);

    /* _oz_heap_cur might be negative!! */
    if (((long) _oz_heap_end) > ((long) _oz_heap_cur)) {
      Assert(((long) _oz_heap_end) > 0);
      _oz_getNewHeapChunk(a_sz);
      goto retry;
    }

    return _oz_heap_cur;

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
  static void refill(const size_t s);

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
        refill(s);
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

/*
 * Use this for dynamically allocated memory blocks that haven't been
 * allocated with the same size. For example, parts of memory areas
 * that are not longer needed. Unsafe will enforce the right alignment!
 *
 */
#define oz_freeListDisposeUnsafe(p,s)

/*
 * AS YOU CAN SEE: THIS ROUTINE IS MISSING FOR NOW
 *
 */


/*
 * Redefine the operator "new" in every class, that uses memory
 * from the free list.  The same holds for memory used from heap.
 *
 *
 */

#ifdef __GNUC__

#define USEFREELISTMEMORY                               \
  static void *operator new(size_t chunk_size)          \
    { return oz_freeListMalloc(chunk_size); }           \
  static void operator delete(void *,size_t )           \
    { Assert(NO); }                                     \
  static void *operator new[](size_t chunk_size)        \
    { return oz_freeListMalloc(chunk_size); }           \
  static void operator delete[](void *,size_t )         \
    { Assert(NO); }

#define USEHEAPMEMORY                                   \
  static void *operator new(size_t chunk_size)          \
    { return oz_heapMalloc(chunk_size); }               \
  static void operator delete(void *,size_t)            \
    { Assert(NO); }                                     \
  static void *operator new[](size_t chunk_size)        \
    { return oz_heapMalloc(chunk_size); }               \
  static void operator delete[](void *,size_t)          \
    { Assert(NO); }

#else

#define USEFREELISTMEMORY                       \
  static void *operator new(size_t chunk_size)  \
    { return oz_freeListMalloc(chunk_size); }   \
  static void operator delete(void *,size_t )   \
    { Assert(NO); }

#define USEHEAPMEMORY                           \
  static void *operator new(size_t chunk_size)  \
    { return oz_heapMalloc(chunk_size); }       \
  static void operator delete(void *,size_t)    \
    { Assert(NO); }

#endif


#endif
