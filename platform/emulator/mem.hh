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

#ifdef __GNUC__
#pragma interface
#endif


#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

#include "types.hh"
#include "error.hh"



class MemChunks {
public:
  static MemChunks *list;
  char *block;
  MemChunks *next;
  int size;
  MemChunks(char *bl, MemChunks *n, int sz) : block(bl), next(n), size(sz) {};
  void deleteChunkChain();
  Bool inChunkChain(void *value);
  void print();
};


/*     Redefine the operator "new" in every class, that shall use memory
    from the free list.  The same holds for memory used from heap. */


#define USEFREELISTMEMORY \
  static inline void *operator new(size_t chunk_size) \
    { return freeListMalloc(chunk_size); } \
  static inline void operator delete(void *,size_t ) \
    { error("deleting free list mem"); }


#define USEHEAPMEMORY \
  static inline void *operator new(size_t chunk_size) \
    { return heapMalloc(chunk_size); }\
  static inline void operator delete(void *,size_t) \
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
extern int   heapTotalSize;   // # bytes allocated

void getMemFromOS(size_t size);

// return free used bytes on the heap
inline unsigned int getUsedMemory() {
  return heapTotalSize - 
    (heapEnd - heapTop);
}

inline unsigned int getAllocatedMemory() {
  return heapTotalSize;
}

void *heapMallocOutline(size_t chunk_size);

inline
void *heapMalloc(size_t chunk_size)
{
  char *oldTop = heapTop;
  heapTop += chunk_size;
  if (heapTop > heapEnd) {
    getMemFromOS(chunk_size);
    oldTop = heapTop;
    heapTop += chunk_size;
  }

#ifdef DEBUG_MEM
  memset((char *)oldTop, 0x5A, chunk_size);
#endif
  return oldTop;
} // heapMalloc


// free list management
const int freeListMaxSize = 500;

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
#endif

#endif
