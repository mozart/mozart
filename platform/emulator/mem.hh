/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
*/

#ifndef __MEMORYH
#define __MEMORYH

#ifdef INTERFACE
#pragma interface
#endif


#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

#include "types.hh"
#include "error.hh"


#ifdef DEBUG_MEM
#define DebugMem(Code) Code
#else
#define DebugMem(Code)
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
  MemChunks(char *bl, MemChunks *n, int sz) : block(bl), next(n), xsize(sz) {};
  void deleteChunkChain();
  Bool inChunkChain(void *value);
  void print();
};


/*     Redefine the operator "new" in every class, that shall use memory
    from the free list.  The same holds for memory used from heap. */


#define USEFREELISTMEMORY \
  static void *operator new(size_t chunk_size) \
    { return freeListMalloc(chunk_size); } \
  static void operator delete(void *,size_t ) \
    { error("deleting free list mem"); }


#define USEHEAPMEMORY \
  static void *operator new(size_t chunk_size) \
    { return heapMalloc(chunk_size); }\
  static void operator delete(void *,size_t) \
    { error("deleting heap mem");}



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
extern unsigned int heapTotalSize;   // # kilo bytes allocated

void getMemFromOS(size_t size);

// return free used kilo bytes on the heap
inline unsigned int getUsedMemory()
{
  return heapTotalSize - (heapTop - heapEnd)/KB;
}

inline unsigned int getAllocatedMemory() {
  return heapTotalSize;
}


/* Allocation functions.
 * Assertion heapTop is alway aligned to int32 boundaries
 */


void *heapMallocOutline(size_t chunk_size);


/* Assert: heapTop grows to LOWER address */
#define HeapTopAlign(align) heapTop = (char *)((long)heapTop & (-align));


inline void *mallocBody(size_t chunk_size, int align)
{
  Assert(ToInt32(heapTop)%sizeof(int32) == 0);

 retry:
  heapTop -= chunk_size;
  if (heapEnd <= heapTop) {
    if (sizeof(int32) != align) {
      HeapTopAlign(align);
    }
#ifdef DEBUG_MEM
    memset((char *)heapTop,0x5A,chunk_size);
#endif
    return heapTop;
  }
  getMemFromOS(chunk_size);
  goto retry;
}


/* return pointer aligned to sizeof(int32) */
inline int32 *int32Malloc(size_t chunk_size)
{
  return (int32 *) mallocBody(chunk_size,sizeof(int32));
}


/* return "chunk_size" aligned to "align" */
inline void *alignedMalloc(size_t chunk_size, int align)
{
  return mallocBody(chunk_size,align);
}


/* ptr1 is newer than ptr2 on the heap */
inline Bool heapNewer(void *ptr1, void *ptr2)
{
  return (ptr1 < ptr2);
}


// free list management
const int freeListMaxSize = 1000;

extern void *FreeList[freeListMaxSize];

unsigned int getMemoryInFreeList();


extern "C" void* memset(void*, int, size_t);

int initMemoryManagement(void);
void deleteChunkChain(char *);
int inChunkChain(void *, void *);
void printChunkChain(void *);

#ifndef OUTLINE
#include "mem.icc"
#else
void * freeListMalloc(size_t chunk_size);
void freeListDispose(void *addr, size_t chunk_size);
void *heapMalloc(size_t chunk_size);
#endif

#endif
