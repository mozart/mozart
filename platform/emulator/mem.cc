/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
*/

#ifdef __GNUC__
#pragma implementation "mem.hh"
#endif

#include <sys/types.h>

#include "mem.hh"
#include "tagged.hh"
#include "am.hh"
#include "io.hh"

extern "C" void *sbrk(int incr);

// ----------------------------------------------------------------
// heap memory

// allocate 2000 kilo byte chuncks of memory
const heapMaxSize = 2 * MB;

MemChunks *MemChunks::list = NULL;

int heapTotalSize;

#ifndef HEAPTOPINTOREGISTER
char *heapTop;
#endif

char *heapEnd;

int initMemoryManagement(void)
{
  // init free store list
  for(int i=0; i<freeListMaxSize; i++)
    FreeList[i] = NULL;

  // init heap memory
  heapTotalSize = 0;
  heapTop       = NULL;
   
  // allocate first chunck of memory;
  MemChunks::list = NULL;
  getMemFromOS(0);

  return 4711;
}


// ----------------------------------------------------------------
// free list memory

void *FreeList[freeListMaxSize];


void heapFree(void * /* addr */)
{
//  error("Heap Free: called: @ %d\n",addr);
}

// count bytes free in FreeList memory
unsigned int getMemoryInFreeList()
{
  unsigned int sum = 0;
  void *ptr;

  for (int i=0; i<freeListMaxSize; i++) {
    ptr = FreeList[i];
    while(ptr != NULL) {
      sum += i;
      ptr = *(void **)ptr;
    }
  }
  return sum;
}



// ----------------------------------------------------------------
// mem from os with 3 alternative
//   USESBRK
//   USEMMAP   does not work --> rs
//   USEMALLOC

#if defined(ULTRIX_MIPS) || defined(HPUX_700)
#define USEMALLOC
#else
#define USESBRK
#endif



#if defined USEMALLOC
#include <memory.h>

void ozFree(void *addr) {
  free(addr);
}

void *ozMalloc(size_t size) {
  return malloc(size);
}

#elif defined USEMMAP
#include <sys/mman.h>

void *ozMalloc(size_t size)
{
  int fd, i;
  int *array;
  size += sizeof(int);

  if ((fd = open("/dev/zero",O_RDWR)) == -1) {
    ozperror("open /dev/zero failed");
  }

  caddr_t ret = mmap((((int)sbrk(0)/getpagesize())+1)*getpagesize(),
		     size,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_FIXED,fd,0);
  if ((int)ret == -1) {
    ozperror("ozMalloc: mmap failed");
  }
  close(fd);

  *(int *) ret = size;

  return (void *) ((int*)ret+1);
}

void ozFree(void *block)
{
  caddr_t retBlock = (caddr_t) ((int*)block-1);
  int size = *(int*)retBlock;

  if (munmap(retBlock,size) == -1) {
    ozperror("ozFree: munmap failed");
  }
}

#elif defined USESBRK

/* have to define this, that gcc is quiet */

extern "C"  int brk(void *incr);


class SbrkMemory {
 public:
  /* a list containing all free blocks in ascending order */
  static SbrkMemory *freeList;

  void *oldBrk;   /* brk value before allocation of this block */
  void *newBrk;   /* new brk value after allocation of this block */
  int size;         /* size of this block including size of this class */
  SbrkMemory *next; /* next free block */

  void print() {
    if (this != NULL) {
      printf("new = %d\nsize = %d\nnext = 0x%x\n\n\n",
	     newBrk,size,next);
      next->print();
    }
  }

  /* add a block to the free list, ascending order */
  SbrkMemory *add(SbrkMemory*elem) 
  {
    if (this == NULL) {
      elem->next = NULL;
      return elem;
    } else {
      if (elem->newBrk < newBrk) {
	elem->next = this;
	return elem;
      } else {
	next = next->add(elem);
	return this;
      }
    }
  }


  /* release all blocks, that are on top of our UNIX processes heap */

  SbrkMemory *shrink() 
  {
    if (this == NULL) 
      return NULL;

    next = next->shrink();

    if (newBrk == sbrk(0)) {
#ifdef DEBUG_TRACEMEM
      printf("*** Returning %d bytes to the operating system\n",size);
#endif
      int ret = brk(oldBrk);
      if (ret == -1) {
	error("*** Something wrong when shrinking memory");
      }
      return NULL;
    }
 
    return this;
  }


  /* find a free block with size sz and bind it to ptr,
     return new free list */

  SbrkMemory *find(int sz, void *&ptr) 
  {
    SbrkMemory* ret;
    if (this == NULL) {
      ret = NULL;
    } else {

      if (sz == size) {
	ptr = (void *) (this +1);
#ifdef DEBUG_TRACEMEM
	printf("*** Reusing %d bytes from free list\n", size);
#endif
	ret = next;
      } else {
	next = next->find(sz,ptr);
	ret = this;
      }
    }
    return ret;
  }
};


SbrkMemory *SbrkMemory::freeList = NULL;

/* allocate memory via sbrk, first see if there is
   a block in free list */

void *ozMalloc(int chunk_size)
{
  static int firstCall = 1;


  /* first we allocate space via malloc and release it directly: this means
     that future malloc's will use this area. In this way the heaps, that are
     allocated via sbrk, will be rather surely on top of the UNIX process's
     heap, so we can release this space again!
     */

  if (firstCall == 1) {
    firstCall = 0;
    free(malloc(MB*2));
  }

  chunk_size += sizeof(SbrkMemory);
  void *ret = NULL;
  SbrkMemory::freeList = SbrkMemory::freeList->find(chunk_size,ret);
  if (ret != NULL) {
    return ret;
  } else {
#ifdef DEBUG_TRACEMEM
    printf("*** Allocating %d bytes\n",chunk_size);
#endif
    void *old = sbrk(0);
    void *ret_val = sbrk(chunk_size);
    if (ret_val == (caddr_t) - 1) {
      message("Virtual memory exhausted");
      IO::exitOz(1);
    }
    
    SbrkMemory *newMem = (SbrkMemory *) ret_val;
    newMem->oldBrk = old;
    newMem->newBrk = sbrk(0);
    newMem->size   = chunk_size;
    newMem->next   = NULL;
    
    return (newMem + 1);
  }
}



/* free via sbrk */

void ozFree(void *p) 
{
  SbrkMemory::freeList =
    (SbrkMemory::freeList->add(((SbrkMemory *)p)-1))->shrink();
}

#else

 ERROR ERROR
 
 non valid OSMEM in memory.C 

 ERROR ERROR

#endif



void MemChunks::deleteChunkChain()
{
  MemChunks *aux = this;
  while (aux) {
#ifdef DEBUG_GC
//    memset(aux->block,0x14,aux->size);
    memset(aux->block,-1,aux->size);
#endif
    ozFree(aux->block);

    MemChunks *aux1 = aux;    
    aux = aux->next;
    delete aux1;
  }
}

// 'inChunkChain' returns 1 if value points into chunk chain otherwise 0.
Bool MemChunks::inChunkChain(void *value)
{
  for (MemChunks *aux = this; aux != NULL; aux = aux->next) {
    if ((aux->block <= (char *) value) && ((char*) value <= aux->block + aux->size))
      return OK; 
  }
  return NO;
}

void MemChunks::print()
{
  for (MemChunks *aux = this; aux != NULL; aux = aux->next) {
    printf("chunk( from: 0x%x, to: 0x%x )\n", aux->block, aux->block + aux->size);    
  }
}


void *heapMallocOutline(size_t chunk_size)
{
  return heapMalloc(chunk_size);
}


void getMemFromOS(size_t sz)
{
  if (sz > heapMaxSize) {
    message("memory exhausted: required chunk bigger thank max size");
    message(" hint: look for an endless recursion");
    IO::exitOz(1);
  }

  heapTotalSize += heapMaxSize;

  heapTop = (char *) ozMalloc(heapMaxSize);
  
  if (!heapTop) {
    error("heapMalloc: Virtual memory exceeded"); 
  }

  /* initialize with zeros */
  DebugCheckT(memset(heapTop,0,heapMaxSize));

  heapEnd = &heapTop[heapMaxSize];

  //  message("heapEnd: 0x%x\n maxPointer: 0x%x\n",heapEnd,maxPointer+1);
  if (heapEnd > (char*)maxPointer) {
    message("Virtual memory exhausted");
    IO::exitOz(1);
  }
  
  MemChunks::list = new MemChunks(heapTop,MemChunks::list,heapMaxSize);
  
  DebugCheck(heapTotalSize > heapMaxSize,
	     message("Increasing heap memory to %d kilo bytes\n", 
		     heapTotalSize / KB));
}



int inTospace(void *p)
{
  for(MemChunks *help=MemChunks::list; help!=NULL; help=help->next) {
    char *ptr = help->block;
    if ((char *) p <= ptr+help->size && (char*) p >= ptr) {
      return 1;
    }
  }
  
  return 0;      
}

#ifdef OUTLINE
#define inline
#include "mem.icc"
#undef inline
#endif

