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
 *    Ralf Scheidhauer, 1998
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

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "mem.hh"
#endif

#include "mem.hh"
#include "os.hh"
#include "am.hh"

#ifdef CS_PROFILE
Bool across_chunks;
#endif


MemChunks *MemChunks::list = NULL;

unsigned int heapTotalSize;
unsigned int heapTotalSizeBytes;


char * _oz_heap_cur;
char * _oz_heap_end;

#ifdef TRACE_ALOVER
size_t _oz_alover;
#endif


void initMemoryManagement(void)
{
  // VERY IMPORTANT: The memory allocator's and the engine's ideas
  // about alignment requirements must coincide!
  Assert(OZ_HEAPALIGNMENT == (1 << (STAG_BITS)));

  // Force the 'mmap'-based allocation for Oz heap chunks, if we can:
#if defined(M_MMAP_THRESHOLD)
  (void) mallopt(M_MMAP_THRESHOLD, (HEAPBLOCKSIZE - 1));
#endif

  // init heap memory
  heapTotalSizeBytes = heapTotalSize = 0;
  _oz_heap_cur       = NULL;
  _oz_heap_end       = NULL;
#ifdef TRACE_ALOVER
  _oz_alover = 0;
#endif

  // allocate first chunk of memory;
  MemChunks::list = NULL;
  _oz_getNewHeapChunk(1);

  // init free memory list
  FL_Manager::init();
}

//
// There are 4 alternatives for obtaining memorry from os:
// - MALLOC with separated (mmap"ed) large objects area
// - MMAP
// - SBRK (somewhat prone to memory fragmentation)
// - MALLOC unoptimized (very prone to memory fragmentation);
// These alternatives are choosen by default in this order.
//
// Now, handling of heap memory is greatly simplified: memory chunks
// can be placed arbitrarily. So, all the troubles with 'mmap()'
// (deprecated usage of fixed allocation addresses, incompatibility
// between OSes and glitches in them) go away.

#if defined(USE_MMAP)

// kost@ : Linux has MAP_ANON(YMOUS), while e.g. Solaris does not.
#if defined(MAP_ANON) && !defined(MAP_ANONYMOUS)
#define	MAP_ANONYMOUS	MAP_ANON
#elif defined(MAP_ANONYMOUS) && !defined(MAP_ANON)
#define	MAP_ANON	MAP_ANONYMOUS
#endif

#include <sys/mman.h>
#include <fcntl.h>

//
void ozFree(char *addr, size_t size) 
{
  if (munmap(addr, size)) {
    ozperror("munmap");
  }
}

//
#if defined(LINUX_I486)
// Potentially, the mmap-based regions could clash with the expanding
// bss. We had a fix for that:
#if defined(BSS_ALLOC_WA)
extern char _end;
// linux kernel (at least the 2.0.33 i'm currently using) seems to
// reserve around 8Mb for potential expantion of bss. Which is not
// always sufficient...
static void *lowNoMap  = &_end;
static void *highNoMap = &_end + MEM_C_HEAP_SIZE;
#endif
#endif

//
void* ozMalloc(size_t size) 
{
  static size_t const pagesize = sysconf(_SC_PAGESIZE);
#if !defined(MAP_ANONYMOUS)
  static int devZeroFD = -1;

  //
  if (devZeroFD == -1) {
    devZeroFD = open("/dev/zero", O_RDWR);
    if (devZeroFD < 0) { ozperror("mmap: open /dev/zero"); }
  }
#endif

  // line up the size of a chunk, as mmap requires;
  if (size % pagesize != 0)
    size = ((size - 1) / pagesize) * pagesize + pagesize;

  //
#if defined(MAP_ANONYMOUS)
  void *ret = mmap((caddr_t) 0, size, (PROT_READ|PROT_WRITE),
		   (MAP_PRIVATE|MAP_ANONYMOUS), 
		   -1, (off_t) 0);
#if defined(BSS_ALLOC_WA)
  // 
  // kost@ : make sure that there is some place for C heap.
  // For that, check where _end_start points to, and skip some
  // MEM_C_HEAP_SIZE bytes, round it up, and try it again...
  if (ret >= lowNoMap && ret < highNoMap) { // implies ret != MAP_FAILED
    if (munmap((char *) ret, size))
      ozperror("munmap");
    char *baseAddress = (char *) highNoMap;
    ret = mmap((caddr_t) baseAddress, size, (PROT_READ|PROT_WRITE),
	       (MAP_PRIVATE|MAP_ANONYMOUS), -1, (off_t) 0);
    if (ret >= lowNoMap && ret < highNoMap)
      OZ_warning("Using C heap with mmap\"s!\n");
  }
#endif // defined(BSS_ALLOC_WA)
#else  // defined(MAP_ANONYMOUS)
  void *ret = mmap((caddr_t) 0, size, (PROT_READ|PROT_WRITE),
		   (MAP_PRIVATE), devZeroFD, (off_t) 0);
#endif // defined(MAP_ANONYMOUS)

  if (ret == MAP_FAILED) { ozperror("mmap"); }
#ifdef DEBUG_CHECK
  //
  // make test of the created page: write, read, write, read!
  for (char *addr = (char *) ret; addr < ((char *) ret) + size; addr++)
    *addr = (char) 0;
  for (char *addr = (char *) ret; addr < ((char *) ret) + size; addr++)
    if (*addr != (char) 0) 
      OZ_error ("mmap check: failed to read zeros");
  for (char *addr = (char *) ret; addr < ((char *) ret) + size; addr++)
    *addr = (char) 0x4f;
  for (char *addr = (char *) ret; addr < ((char *) ret) + size; addr++)
    if (*addr != (char) 0x4f)
      OZ_error ("mmap check: failed to read values");
#endif

  //  
  return (ret);  
}

#elif defined(USE_SBRK)  // defined(USE_MMAP)

/* remember the last sbrk(0), if it changed --> malloc needs more
 * memory, so call fakeMalloc
 */
static void *lastBrk = 0;

//
class SbrkMemory {
 public:
  /* a list containing all free blocks in ascending order */
  static SbrkMemory *freeList;

  void *oldBrk;   /* brk value before allocation of this block */
  void *newBrk;   /* new brk value after allocation of this block */
  int size;         /* size of this block including size of this class */
  SbrkMemory *next; /* next free block */

  void print();

  /* add a block to the free list, ascending order */
  SbrkMemory *add(SbrkMemory*elem);

  /* release all blocks, that are on top of our UNIX processes heap */
  SbrkMemory *shrink();

  /* find a free block with size sz and bind it to ptr,
     return new free list */
  SbrkMemory *find(int sz, void *&ptr);
};


void SbrkMemory::print()
{
  if (this != NULL) {
    printf("new = 0x%p\nsize = %d\nnext = 0x%p\n\n\n",
	   newBrk,size,next);
    next->print();
  }
}

/* add a block to the free list, ascending order */
SbrkMemory* SbrkMemory::add(SbrkMemory*elem) 
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
SbrkMemory* SbrkMemory::shrink() 
{
  if (this == NULL) 
    return NULL;

  next = next->shrink();

  if (newBrk == sbrk(0)) {
#ifdef DEBUG_TRACEMEM
    printf("*** Returning %d bytes to the operating system\n",size);
#endif
#if defined(NETBSD) || defined(__FreeBSD__) || defined(OSF1_ALPHA) || defined(OPENBSD)
    int ret = (int)brk((char*)oldBrk);
#else
    int ret = brk(oldBrk);
#endif
    lastBrk = sbrk(0);
    if (ret == -1) {
      OZ_error("*** Something wrong when shrinking memory");
    }
    return NULL;
  }

  return this;
}


/* find a free block with size sz and bind it to ptr,
   return new free list */
SbrkMemory* SbrkMemory::find(int sz, void *&ptr) 
{
  if (this == NULL)
    return NULL;

  if (sz <= size) {
    ptr = (void *) (this +1);
#ifdef DEBUG_TRACEMEM
    printf("*** Reusing %d bytes from free list\n", size);
#endif
    return next;
  } 

  next = next->find(sz,ptr);
  return this;
}


SbrkMemory *SbrkMemory::freeList = NULL;

/* allocate memory via sbrk, first see if there is
   a block in free list */


/* first we allocate space via malloc and release it directly: this means
 * that future malloc's will use this area. In this way the heaps, that are
 * allocated via sbrk, will be rather surely on top of the UNIX process's
 * heap, so we can release this space again!
 * Note: linux allocates small chunks from a different area than large ones
 *   so we allocate in many small pieces.
 */

static void fakeMalloc(int sz)
{
  /* not needed under Windows, since we there use VirtualAlloc */
#ifndef WINDOWS
  int chunksz = 1024;
  void **array = new void*[sz/chunksz + 1];
  int i=0;
  while (sz>0) {
    array[i++]= malloc(chunksz);
    sz -= chunksz;
  }
  // ensures that following free does not hand back mem to OS;
  sbrk(sizeof(long));

  while(--i>=0) {
    free(array[i]);
  }
  delete array;
#endif
}

void *ozMalloc(int chunk_size)
{
  static int firstCall = 1;

  if (firstCall == 1) {
    firstCall = 0;
    fakeMalloc(3*MB);
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
    if (lastBrk && old != lastBrk) {
      // DebugCheckT(message("fakeMallocing 2MB\n"));
      fakeMalloc(2*MB);
      old = sbrk(0);
    }
    void *ret_val = sbrk(chunk_size);
    lastBrk = sbrk(0);
    if (ret_val == (caddr_t) - 1) {
      fprintf(stderr,"Mozart: Virtual memory exhausted\n");
      am.exitOz(1);
    }

    SbrkMemory *newMem = (SbrkMemory *) ret_val;
    newMem->oldBrk = old;
    newMem->newBrk = lastBrk;
    newMem->size   = chunk_size;
    newMem->next   = NULL;

    return (newMem + 1);
  }
}



/* free via sbrk */

void ozFree(char *p, size_t ignored) 
{
  SbrkMemory::freeList =
    (SbrkMemory::freeList->add(((SbrkMemory *)p)-1))->shrink();
}

#elif defined(USE_MALLOC)  // defined(USE_SBRK)
#if defined(WINDOWS)

/* need windows.h: */
#include "wsock.hh"

void ozFree(char *ptr, int sz)
{
  if (ptr && VirtualFree(ptr,0,MEM_RELEASE) != TRUE)
    OZ_warning("free(0x%p) failed: %d\n",ptr,GetLastError());
}

char *ozMalloc0(int sz)
{
  return ((char *) VirtualAlloc(0, sz, (MEM_RESERVE|MEM_COMMIT),
				PAGE_READWRITE));
}


char *ozMalloc(int sz)
{
  char *aux = ozMalloc0(sz);
  return aux;
}

#else  // defined(WINDOWS) // malloc

void ozFree(char *addr, size_t ignored)
{
  free(addr);
}

void *ozMalloc(size_t size)
{
  return malloc(size);
}

#endif  // defined(WINDOWS)
#else   // defined(USE_MALLOC)
#error BOOOOOM			// an undefined allocation scheme!!
#endif  // defined(USE_MALLOC)



void MemChunks::deleteChunkChain()
{
  MemChunks *aux = this;
  while (aux) {

#ifdef DEBUG_GC
    memset(aux->block,-1,aux->xsize);
#endif

    ozFree(aux->block,aux->xsize);

    MemChunks *aux1 = aux;    
    aux = aux->next;
    delete aux1;
  }
}


#if defined(DEBUG_MEM)

// 'inChunkChain' returns 1 if value points into chunk chain otherwise 0.
Bool MemChunks::inChunkChain(void *value)
{
  for (MemChunks *aux = this; aux != NULL; aux = aux->next) {
    if (aux->isInBlock((char*)value))
      return OK;
  }
  return NO;
}

//
extern Bool isCollecting;

//
Bool MemChunks::isInHeap(TaggedRef term)
{
  if (isCollecting)
    return (OK);

 iterate:
  switch (tagged2stag(term)) {
  case STAG_REF0:
  case STAG_REF1:
    if (term != makeTaggedNULL()) {
      TaggedRef *ref = tagged2Ref(term);
      if (!list->inChunkChain(ref)) {
	return (NO);
      } else {
	term = *ref;
	goto iterate;
      }
      Assert(0);
    }
    break;

  case STAG_MARK:
    // generic traverser also uses it;
    break;

  case STAG_VAR:
    if (!list->inChunkChain(tagged2Var(term)))
      return (NO);
    break;

  case STAG_CONST:
    if (oz_isBuiltin(term))
      break;
    if (!list->inChunkChain(tagged2Const(term)))
      return (NO);
    break;

  case STAG_LTUPLE:
    if (!list->inChunkChain(tagged2LTuple(term)))
      return (NO);
    break;

  case STAG_SRECORD:
    if (!list->inChunkChain(tagged2SRecord(term)))
      return (NO);
    break;

  default:
    break;
  }

  return (OK);
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
    printf(" chunk( from: 0x%p, to: 0x%p )\n",
	   aux->block, aux->block + aux->xsize);
    aux = aux->next;
    if (aux) {
      printf("  --> ");
    }
  }
}

#endif  // defined(DEBUG_MEM)

// #define HEAP_SAFETY_FOR_ALIGN 32

// kost@ : the result is supposed to be just OZ_HEAPALIGMENT-aligned.
// Within a [addr, addr+OZ_HEAPALIGNMENT-1] interval we have at least
// one OZ_HEAPALIGN"ed address. So, we have:
#define HEAP_ALIGN_SAFETY	(OZ_HEAPALIGNMENT - 1)
#define HEAP_ALIGN_SAFETY_MASK	(-OZ_HEAPALIGNMENT)

void _oz_getNewHeapChunk(const size_t raw_sz)
{
  Assert(raw_sz > 0);
  size_t sz =
    (raw_sz + HEAP_ALIGN_SAFETY) & HEAP_ALIGN_SAFETY_MASK;
  size_t thisBlockSz =		// in chunks of HEAPBLOCKSIZE;
    (((sz - 1) / ((size_t) HEAPBLOCKSIZE)) * ((size_t) HEAPBLOCKSIZE)) +
    ((size_t) HEAPBLOCKSIZE);
  // kost@ : these are the invariants now:
  Assert(thisBlockSz >= HEAPBLOCKSIZE);
  Assert(thisBlockSz%OZ_HEAPALIGNMENT == 0);
  Assert(thisBlockSz%HEAPBLOCKSIZE == 0);

  heapTotalSize      += thisBlockSz/KB;
  heapTotalSizeBytes += thisBlockSz;

  size_t malloc_size  = thisBlockSz;
  char * malloc_block = (char *) ozMalloc(malloc_size);

  _oz_heap_end = malloc_block;

  if (_oz_heap_end == NULL) {
    OZ_warning("Mozart: virtual memory exhausted.\n");
    am.exitOz(1);
  }

  // kost@ : _oz_heap_end does NOT have to be "heap-aligned";
  _oz_heap_cur = _oz_heap_end + thisBlockSz;
  while (!isSTAligned(_oz_heap_cur)) {
    thisBlockSz--;
    _oz_heap_cur--;
  }
  Assert(isSTAligned(_oz_heap_cur));

  //
  MemChunks::list =
    new MemChunks(malloc_block, MemChunks::list, malloc_size);
#ifdef CS_PROFILE
  across_chunks = OK;
#endif
}


/*
 * Free List Management
 *
 */

FL_Small * FL_Manager::smmal[FL_SizeToIndex(FL_MaxSize) + 1];
FL_Large * FL_Manager::large;

void FL_Manager::init(void)
{
  large    = (FL_Large *) NULL;
  smmal[0] = NULL;

  for (int i = FL_SizeToIndex(FL_MaxSize); i>0; i--) {
    FL_Small * f = (FL_Small *) oz_heapMalloc(FL_IndexToSize(i));
    f->setNext(NULL);
    smmal[i] = f;
  }
}

#define FL_RFL_Small 32
#define FL_RFL_N1    4
#define FL_RFL_N2    32

void FL_Manager::refill(void)
{
  register size_t sz = FL_MinSize;

  while (smmal[FL_SizeToIndex(sz)])
    sz += FL_MinSize;
  
  register char * block;
  register size_t n;

  /*
   * Take a large block, if available
   *
   */

  if (large != NULL) {
    FL_Large * l = large;
    large = l->getNext();
    block = (char *) l;
    n     = l->getSize();
  } else {
    n     = ((sz > FL_RFL_Small) ? FL_RFL_N1 : FL_RFL_N2) * sz;
    block = (char *) oz_heapMalloc(n);
  }

  smmal[FL_SizeToIndex(sz)] = (FL_Small *) block;

  n -= sz;
  
  while (n >= sz) {
    block += sz;
    n     -= sz;
    ((FL_Small *) (block-sz))->setNext((FL_Small *) block);
  }
  
  ((FL_Small *) block)->setNext(NULL);

  if (n > 0)
    free(block+sz,n);
  
}

unsigned int FL_Manager::getSize(void)
{
  unsigned int s = 0;

  for (int i = 1; i <= FL_SizeToIndex(FL_MaxSize); i++) {
    FL_Small * f = smmal[i];
    while (f) {
      s += FL_IndexToSize(i); f = f->getNext();
    }
  }

  FL_Large * f = large;

  while (f) {
    s += f->getSize(); f = f->getNext();
  }

  return s;
}


#if defined(DEBUG_CHECK)
extern "C" {
void *__real_malloc(size_t sz);
// 
void *__wrap_malloc(size_t sz)
{
  return (__real_malloc(sz));
}
};
#endif // DEBUG_CHECK
