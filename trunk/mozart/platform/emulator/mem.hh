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
 *  - Supports alignment of 4 and 8
 *
 */

#ifdef HEAPCURINTOREGISTER
register char * _oz_heap_cur asm("g6");
#else
extern   char * _oz_heap_cur;
#endif

extern   char * _oz_heap_end;


void _oz_getNewHeapChunk(const size_t);


inline 
void * _oz_amalloc(const size_t sz, const size_t align) {
  /*
   * The following invariants are enforced:
   *  - _oz_heap_cur is always aligned to sizeof(int32)
   *  - alignment can be only 4 or 8
   *  - if alignment == 4, then sz is multiple of 4
   *
   */

  Assert(((sz & 3) == 0) || (align == 8));

  Assert(ToInt32(_oz_heap_cur) % sizeof(int32) == 0);

  Assert(align == 4 || align == 8);

 retry:

  _oz_heap_cur -= sz;

  if (align == 8)
    _oz_heap_cur = (char *) (((long) _oz_heap_cur) & (~((long) 7)));
  
  Assert(((long) _oz_heap_cur) % align == 0);

  /* _oz_heap_cur might be negative!! */
  if (((long) _oz_heap_end) > ((long) _oz_heap_cur)) {
    Assert(((long) _oz_heap_end) > 0);
    _oz_getNewHeapChunk(sz);
    goto retry;
  }

  return _oz_heap_cur;
}

/*
 * Routine that rounds a requested size to a multiple of 
 * sizeof(int32)
 *
 */

#define HMALLOC_SAFE_SZ(sz) \
   (((sz) + (sizeof(int32)-1)) & (~(sizeof(int32)-1)))

/*
 * Unsafe allocation routines:
 *  - various alignment types
 *  - requested sz must be multiple of sizeof(int32)
 *
 */

#define int32Malloc(sz)  (_oz_amalloc((sz),sizeof(int32)))
#define doubleMalloc(sz) (_oz_amalloc((sz),sizeof(double)))
#define heapMalloc(sz)   (_oz_amalloc((sz),sizeof(void *)))

/*
 * Safe allocation routines:
 *  - various alignment types
 *  - requested sz can be arbitrary
 *
 */

#define int32MallocSafe(sz)  int32Malloc(HMALLOC_SAFE_SZ(sz))
#define doubleMallocSafe(sz) doubleMalloc(sz)
#define heapMallocSafe(sz)   heapMalloc(HMALLOC_SAFE_SZ(sz))


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
  return memcpy(heapMalloc(sz), p, sz);
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
#define FL_SizeToIndex(sz) ((sz) >> 2)
#define FL_IndexToSize(i)  ((i)  << 2)

// Alignment restrictions
#define FL_IsValidSize(sz) (!((sz) & 3))


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
      return heapMalloc(s);
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
 *  - aligment is fixed to 4
 *  - for difference between Safe and other see above
 *
 */

#define freeListMalloc(s)     (FL_Manager::alloc((s)))
#define freeListMallocSafe(s) freeListMalloc(HMALLOC_SAFE_SZ(s))

#ifdef CS_PROFILE
#define freeListDispose(p,s)
#else
#define freeListDispose(p,s) FL_Manager::free((p),(s))
#endif

#define freeListDisposeSafe(p,s) freeListDispose(p,HMALLOC_SAFE_SZ(s))



/*     
 * Redefine the operator "new" in every class, that uses memory
 * from the free list.  The same holds for memory used from heap. 
 *
 *
 */

#ifdef __GNUC__

#define USEFREELISTMEMORY				\
  static void *operator new(size_t chunk_size)		\
    { return freeListMalloc(chunk_size); }		\
  static void operator delete(void *,size_t )		\
    { Assert(NO); }					\
  static void *operator new[](size_t chunk_size)	\
    { return freeListMalloc(chunk_size); }		\
  static void operator delete[](void *,size_t )		\
    { Assert(NO); }

#define USEHEAPMEMORY					\
  static void *operator new(size_t chunk_size)		\
    { return heapMalloc(chunk_size); }			\
  static void operator delete(void *,size_t)		\
    { Assert(NO); }					\
  static void *operator new[](size_t chunk_size)	\
    { return heapMalloc(chunk_size); }			\
  static void operator delete[](void *,size_t)		\
    { Assert(NO); }

#define USEHEAPMEMORY32					\
  static void *operator new(size_t chunk_size)		\
    { return int32Malloc(chunk_size); }			\
  static void operator delete(void *,size_t)		\
    { Assert(NO); }					\
  static void *operator new[](size_t chunk_size)	\
    { return int32Malloc(chunk_size); }			\
  static void operator delete[](void *,size_t)		\
    { Assert(NO); }

#else

#define USEFREELISTMEMORY			\
  static void *operator new(size_t chunk_size)	\
    { return freeListMalloc(chunk_size); }	\
  static void operator delete(void *,size_t )	\
    { Assert(NO); }

#define USEHEAPMEMORY				\
  static void *operator new(size_t chunk_size)	\
    { return heapMalloc(chunk_size); }		\
  static void operator delete(void *,size_t)	\
    { Assert(NO); }

#define USEHEAPMEMORY32				\
  static void *operator new(size_t chunk_size)	\
    { return int32Malloc(chunk_size); }		\
  static void operator delete(void *,size_t)	\
    { Assert(NO); }

#endif



#endif
