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
#include "error.h"


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


// allocate 2000 kilo byte chuncks of memory
const heapMaxSize = 2000 * 1024;
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
extern char *heapStart;       // pointer start of free memory

inline char *heapGetStart(void) {return heapStart;}
inline int heapGetMaxSize(void) {return heapMaxSize;}


void getMemFromOS(size_t size);

// return free used bytes on the heap
inline unsigned int getUsedMemory() {
  return heapTotalSize -
    (heapEnd - heapTop);
}

inline unsigned int getAllocatedMemory() {
  return heapTotalSize;
}

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


inline
void *freeListMalloc(size_t chunk_size)
{
#ifdef DEBUG_CHECK
  if (chunk_size % 4 != 0) {
    error("freeListMalloc: not aligned to word boundaries");
  }
#endif

  void *aux = chunk_size < freeListMaxSize ? FreeList[chunk_size] : (void *)NULL;

  if (aux == (void *) NULL)
    aux = heapMalloc(chunk_size);
  else {
    FreeList[chunk_size] = *(void **)aux;
  }
#ifdef DEBUG_MEM
  memset((char *)aux,0x5A,chunk_size);
#endif
  return aux;
}


extern "C" void* memset(void*, int, size_t);


inline void freeListDispose(void *addr, size_t chunk_size)
{
#ifdef DEBUG_CHECK
  if (chunk_size % 4 != 0) {
    error("freeListDispose: not aligned to word boundaries");
  }
#endif
#ifdef DEBUG_MEM

// clear mem, so every reference leads to strange errors
// and turn off free list memory, do not reuse the memory
  memset((char *)addr,0x5A,chunk_size);
  return;

#else

  if (chunk_size < freeListMaxSize && chunk_size > 0) {
    *(void **)addr =  FreeList[chunk_size];
    FreeList[chunk_size] = addr;
  }
#endif
}

int initMemoryManagement(void);
void deleteChunkChain(char *);
int inChunkChain(void *, void *);
void printChunkChain(void *);

#endif
