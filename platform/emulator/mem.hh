/*
 *  Authors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *
 *  Contributors:
 *    Michael Mehl (mehl@dfki.de)
 *    Konstantin Popov (kost@sics.se)
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

#ifdef DEBUG_MEM
#define DebugMem(Code) Code
#else
#define DebugMem(Code)
#endif

/* like malloc(3): return pointer aligned to void* */
#define heapMalloc(chunk_size) (alignedMalloc(chunk_size,sizeof(void *)))

extern void *heapMallocOutline(size_t chunk_size);

#ifdef CS_PROFILE
extern Bool across_chunks;
#endif

class MemChunks {
public:
  static MemChunks *list;
  static Bool isInHeap(TaggedRef term);
  static Bool areRegsInHeap(TaggedRef *regs, int size);

private:

  int xsize;
  char *block;
  MemChunks *next;

public:
  MemChunks(char *bl, MemChunks *n, int sz) : xsize(sz), block(bl), next(n) {};
  void deleteChunkChain();
  Bool isInBlock(void *value) {
    return ((block <= (char *) value) && ((char *) value < block + xsize));
  }

  Bool inChunkChain(void *value);
  MemChunks *getNext() { return next; }
  void print();
};


/*     Redefine the operator "new" in every class, that shall use memory
    from the free list.  The same holds for memory used from heap. */


#ifdef __GNUC__
// `new' and `delete' operators also for arrays

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

// Heap Memory
//  concept: allocate dynamically
//           free with garbage collection


#if __GNUC__ >= 2 && defined(sparc) && defined(REGOPT)
#define HEAPTOPINTOREGISTER 1
#endif

#ifdef HEAPTOPINTOREGISTER
register char *heapTop asm("g6");      // pointer to next free memory block
#else
extern char *heapTop;         // pointer to next free memory block
#endif

extern char *heapEnd;
extern unsigned int heapTotalSize;        // # kilo bytes allocated
extern unsigned int heapTotalSizeBytes;   // # bytes allocated

char *getMemFromOS(size_t size);


/* Allocation functions.
 * Assertion heapTop is alway aligned to int32 boundaries
 */



/* return "chunk_size" aligned to "align" */
inline void *alignedMalloc(size_t chunk_size, int align)
{
  ProfileCode(ozstat.heapAlloced(chunk_size);)
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

#if !defined(DEBUG_CHECK) && !defined(HEAP_PROFILE)
#define int32Malloc(s)                          \
 (((long)heapEnd > (long)(heapTop - (s)))       \
 ? (int32 *) getMemFromOS(s)                    \
 : (int32 *) (heapTop -= (s)))
#else
inline int32 *int32Malloc(size_t chunk_size)
{
  return (int32*) alignedMalloc(chunk_size,sizeof(int32));
}
#endif

/* ptr1 is newer than ptr2 on the heap */
inline Bool heapNewer(void *ptr1, void *ptr2)
{
  return (ptr1 < ptr2);
}

inline
Bool reallyHeapNever(void *ptr1, void *ptr2)
{
  MemChunks *aux = MemChunks::list;
  while(aux) {
    if (aux->isInBlock(ptr1)) {
      return !aux->isInBlock(ptr2) || heapNewer(ptr1, ptr2);
    }
    if (aux->isInBlock(ptr2))
      return NO;
    aux = aux->getNext();
  }
  Assert(0);
  return NO;
}


// free list management
#define freeListMaxSize 64+4

class FreeListMem {
public:
    int32 next;
};

extern FreeListMem* FreeList[freeListMaxSize];

extern size_t nextChopSize;

unsigned int getMemoryInFreeList();

inline
void * oz_hrealloc(const void * p, size_t sz) {
  // Use for blocks where size is not known at compile time
  DebugCheck(sz%sizeof(int) != 0,
             OZ_error("OZ_hrealloc: can only handle word sized blocks"););

  return memcpy(heapMalloc(sz), p, sz);
}

void initMemoryManagement(void);
void deleteChunkChain(char *);
int inChunkChain(void *, void *);
void printChunkChain(void *);
#ifdef DEBUG_MEM
void scanFreeList(void);
#endif

#ifndef OUTLINE
#include "mem.icc"
#else
void * freeListMalloc(size_t chunk_size);
void freeListDisposeNoClean(void *addr, size_t chunk_size);
void freeListDispose(void *addr, size_t chunk_size);

// return free used kilo bytes on the heap
unsigned int getUsedMemory(void);
unsigned int getUsedMemoryBytes(void);
#endif

#endif
