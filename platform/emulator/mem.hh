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

#if __GNUC__ >= 2 && defined(sparc) && defined(REGOPT)
#define HEAPTOPINTOREGISTER 1
#endif

#ifdef HEAPTOPINTOREGISTER
register char *heapTop asm("g6");
#else
extern char *heapTop;
#endif

extern char *heapEnd;

char *getMemFromOS(size_t size);

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
 * Basic aligned allocation routine
 *
 */

inline
void * alignedMalloc(const size_t chunk_size, const int align) {
  Assert(ToInt32(heapTop)%sizeof(int32) == 0);

  heapTop -= chunk_size;

retry:
  if (sizeof(int32) != align) {
    heapTop = (char *)((long)heapTop & (-align));
  }

  /* heapTop might now be negative!! */
  if ((long)heapEnd > (long)heapTop) {
    Assert((long)heapEnd>0); // otherwise the above test is wrong
    (void) getMemFromOS(chunk_size);
    goto retry;
  }

#ifdef DEBUG_MEM
  if (heapTop)
    memset((char *)heapTop,0x5A,chunk_size);
#endif

  return heapTop;
}


#define int32Malloc(sz)  (alignedMalloc((sz),sizeof(int32)))
#define doubleMalloc(sz) (alignedMalloc((sz),sizeof(double)))
#define heapMalloc(sz)   (alignedMalloc((sz),sizeof(void *)))


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
  return heapTotalSize - (heapTop - heapEnd)/KB;
}

inline
unsigned int getUsedMemoryBytes(void) {
  return heapTotalSizeBytes - (heapTop - heapEnd);
}



/*
 * Freelist management
 *
 */

// Maximal size of block in free list
#define FL_MaxSize  64

// Transformations between FreeListIndex and Size
#define FL_SizeToIndex(sz) ((sz) >> 2)
#define FL_IndexToSize(i)  ((i) << 2)

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
  static FL_Small * small[FL_SizeToIndex(FL_MaxSize) + 1];
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
      FL_Small * f = small[FL_SizeToIndex(s)];
      Assert(f);
      FL_Small * n = f->getNext();
      small[FL_SizeToIndex(s)] = n;
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
      f->setNext(small[FL_SizeToIndex(s)]);
      small[FL_SizeToIndex(s)] = f;
    }
  }

  static unsigned int getSize(void);

};



/*
 * The real allocation routines
 *
 */

#define freeListMalloc(s)    (FL_Manager::alloc((s)))

#ifdef CS_PROFILE
#define freeListDispose(p,s)
#else
#define freeListDispose(p,s) FL_Manager::free((p),(s))
#endif



/*
 * Redefine the operator "new" in every class, that uses memory
 * from the free list.  The same holds for memory used from heap.
 *
 *
 */

#ifdef __GNUC__

#define USEFREELISTMEMORY                               \
  static void *operator new(size_t chunk_size)          \
    { return freeListMalloc(chunk_size); }              \
  static void operator delete(void *,size_t )           \
    { Assert(NO); }                                     \
  static void *operator new[](size_t chunk_size)        \
    { return freeListMalloc(chunk_size); }              \
  static void operator delete[](void *,size_t )         \
    { Assert(NO); }

#define USEHEAPMEMORY                                   \
  static void *operator new(size_t chunk_size)          \
    { return heapMalloc(chunk_size); }                  \
  static void operator delete(void *,size_t)            \
    { Assert(NO); }                                     \
  static void *operator new[](size_t chunk_size)        \
    { return heapMalloc(chunk_size); }                  \
  static void operator delete[](void *,size_t)          \
    { Assert(NO); }

#define USEHEAPMEMORY32                                 \
  static void *operator new(size_t chunk_size)          \
    { return int32Malloc(chunk_size); }                 \
  static void operator delete(void *,size_t)            \
    { Assert(NO); }                                     \
  static void *operator new[](size_t chunk_size)        \
    { return int32Malloc(chunk_size); }                 \
  static void operator delete[](void *,size_t)          \
    { Assert(NO); }

#else

#define USEFREELISTMEMORY                       \
  static void *operator new(size_t chunk_size)  \
    { return freeListMalloc(chunk_size); }      \
  static void operator delete(void *,size_t )   \
    { Assert(NO); }

#define USEHEAPMEMORY                           \
  static void *operator new(size_t chunk_size)  \
    { return heapMalloc(chunk_size); }          \
  static void operator delete(void *,size_t)    \
    { Assert(NO); }

#define USEHEAPMEMORY32                         \
  static void *operator new(size_t chunk_size)  \
    { return int32Malloc(chunk_size); }         \
  static void operator delete(void *,size_t)    \
    { Assert(NO); }

#endif



#endif
