/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
*/

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "mem.hh"
#endif

#include <sys/types.h>

#include "am.hh"

// ----------------------------------------------------------------
// heap memory

// allocate 1000 kilo byte chuncks of memory

MemChunks *MemChunks::list = NULL;

unsigned int heapTotalSize;

#ifndef HEAPTOPINTOREGISTER
char *heapTop;
#endif

char *heapEnd;

void initMemoryManagement(void) {
  // init free store list
  for(int i=0; i<freeListMaxSize; i++)
    FreeList[i] = NULL;

  // init heap memory
  heapTotalSize = 0;
  heapTop       = NULL;
   
  // allocate first chunck of memory;
  MemChunks::list = NULL;
  getMemFromOS(0);

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
// mem from os with 2 alternatives
//   USESBRK
//   USEMALLOC

#if defined(SUNOS_SPARC) || defined(SOLARIS_SPARC) || defined(LINUX) || defined(IRIX5_MIPS) 
#define USESBRK
#else
#define USEMALLOC
#endif



#if defined(USEMALLOC)
#include <memory.h>

void ozFree(void *addr) {
  free(addr);
}

void *ozMalloc(size_t size) {
  return malloc(size);
}

#elif defined(USESBRK)

extern "C" void *sbrk(int incr);

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
    void *p = malloc(MB*3);
    malloc(sizeof(long)); /* ensures that following free does not hand back mem to OS */
    free(p);
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
      fprintf(stderr,"Virtual memory exhausted\n");
      osExit(1);
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
 
 non valid OSMEM in mem.cc

 ERROR ERROR

#endif



void MemChunks::deleteChunkChain()
{
  MemChunks *aux = this;
  while (aux) {
#ifdef DEBUG_GC
//    memset(aux->block,0x14,aux->xsize);
    memset(aux->block,-1,aux->xsize);
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
    if ((aux->block <= (char *) value)
	&& ((char*) value <= aux->block + aux->xsize))
      return OK; 
  }
  return NO;
}

/*
 * debugging aids for memory problems
 */
Bool MemChunks::isInHeap(TaggedRef term)
{
  if (isRef(term) && term != makeTaggedNULL() &&
      !list->inChunkChain(tagged2Ref(term))) {
    return NO;
  }
  if (!isRef(term)) {
    switch (tagTypeOf (term)) {
    case UVAR:
    case SVAR:
    case LTUPLE:
    case OZCONST:
      if (isBuiltin(term)) return OK;
      // no break
    case SRECORD:
      if (!list->inChunkChain(tagValueOf(term))) {
	return NO;
      }
      break;
    default:
      break;
    }
  }
  return OK;
}

Bool MemChunks::areRegsInHeap(TaggedRef *regs, int sz)
{
  for (int i=0; i<sz; i++) {
    if (!isInHeap(regs[i])) {
      return NO;
    }
  }
  return OK;
}


void MemChunks::print()
{
  MemChunks *aux = this;
  while (aux) {
    printf(" chunk( from: 0x%x, to: 0x%x )\n",
	   aux->block, aux->block + aux->xsize);
    aux = aux->next;
    if (aux) {
      printf("  --> ");
    }
  }
}


void *heapMallocOutline(size_t chunk_size)
{
  Assert((int) chunk_size <= ozconf.heapBlockSize);

  return heapMalloc(chunk_size);
}


void getMemFromOS(size_t sz) {
  if ((int)sz > ozconf.heapBlockSize) {
    warning("Memory chunk too big (size=%d)\nTry\n\tsetenv OZHEAPBLOCKSIZE <x>\nwhere <x> is greater than %d.\n",sz,ozconf.heapBlockSize);
    osExit(1);
  }

  heapTotalSize += ozconf.heapBlockSize/KB;
  
  if (ozconf.heapMaxSize != -1 && 
      ((gc_is_running == NO) ?
       (heapTotalSize > ((100 + ozconf.heapTolerance) * 
			 (unsigned long) ozconf.heapMaxSize) / 100) :
       (heapTotalSize > (unsigned) ozconf.heapMaxSize))) {
    int newSize = (heapTotalSize * 3) / 2;
    prefixError();
    printf("\n\n*** Heap maxsize exceeded. Increase from %d to %d? (y/n) ",
	   ozconf.heapMaxSize,newSize);
    fflush(stdout);
    char buf[1000];

    if (osfgets(buf, 1000, stdin) == 0 || buf[0] == 'n') 
      am.exitOz(1);

    ozconf.heapMaxSize = newSize;
  }

  heapEnd = (char *) ozMalloc(ozconf.heapBlockSize);
  
  if (heapEnd == NULL) {
    fprintf(stderr,"Virtual memory exceeded\n");
    osExit(1);
  }

  /* align heapEnd to word boundaries */
  while(ToInt32(heapEnd)%WordSize != 0) {
    heapEnd++;
  }
  
  /* initialize with zeros */
  DebugCheckT(memset(heapEnd,0,ozconf.heapBlockSize));

  heapTop = heapEnd+ozconf.heapBlockSize;

  //message("heapTop: 0x%lx\n",heapTop);
  if (tagValueOf(makeTaggedMisc(heapTop)) != heapTop) {
    warning("Oz adress space exhausted\n");
    osExit(1);
  }
  
  MemChunks::list = new MemChunks(heapEnd,MemChunks::list,ozconf.heapBlockSize);
  
  DebugCheck(heapTotalSize > ozconf.heapBlockSize/KB,
	     message("Increasing heap memory to %d kilo bytes\n",heapTotalSize));
  //return OK;
}


#ifdef OUTLINE
#define inline
#include "mem.icc"
#undef inline
#endif

