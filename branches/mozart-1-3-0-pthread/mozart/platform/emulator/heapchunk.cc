/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Michael Mehl (mehl@dfki.de)
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

#include "builtins.hh"

class HeapChunk: public OZ_Extension {
private:
  size_t chunk_size;
  char * chunk_data;
  char * allocate(int size) {
    return (char *) oz_heapMalloc(size);
  }
  char * copyChunkData(void) {
    return (char *) memcpy(allocate(chunk_size), chunk_data, chunk_size);
  }
public:
  HeapChunk(int size)
    : OZ_Extension(), chunk_size(size), chunk_data(allocate(size)) 
  {
  }

  virtual
  OZ_Extension * gCollectV(void) {
    HeapChunk * ret = new HeapChunk(chunk_size);
    ret->chunk_data = copyChunkData();
    return ret;
  }
  virtual
  OZ_Extension * sCloneV(void) {
    HeapChunk * ret = new HeapChunk(chunk_size);
    ret->chunk_data = copyChunkData();
    return ret;
  }

  virtual void sCloneRecurseV(void) {}
  virtual void gCollectRecurseV(void) {}

  virtual
  int getIdV() { return OZ_E_HEAPCHUNK; }

  virtual
  OZ_Term typeV() { return oz_atom("heapChunk"); }

  virtual
  OZ_Term printV(int depth) {
    return oz_pair2(oz_atom("<HeapChunk "),
		    oz_pair2(oz_int((int)chunk_size),
			     oz_pair2(oz_atom(" @"),
				      oz_pair2(oz_int((int)this),
					       oz_atom(">")))));
/*
  char * data = chunk_data;
  for (int i = 0; i < chunk_size; i += 1)
    stream << "chunk_data[" << i << "]@" << &data[i] << "="
	   << data[i] << endl;
	   */
  }

  size_t getChunkSize(void) { return chunk_size; }
  void * getChunkData(void) { return (void *)chunk_data; }

  int peek(unsigned int i) {
    if (i>=chunk_size) { return -1; }
    return oz_char2uint(chunk_data[i]);
  }
  int poke(unsigned int i,char v) {
    if (i>=chunk_size) { return NO; }
    chunk_data[i]=v;
    return OK;
  }
};


// heap chunks
inline
Bool oz_isHeapChunk(TaggedRef term)
{
  return oz_isExtension(term)
    && tagged2Extension(term)->getIdV() == OZ_E_HEAPCHUNK;
}

HeapChunk *tagged2HeapChunk(TaggedRef term)
{
  Assert(oz_isHeapChunk(term));
  return (HeapChunk *) tagged2Extension(term);
}

int OZ_isHeapChunk(OZ_Term t)
{
  return oz_isHeapChunk(oz_deref(t));
}

OZ_Term OZ_makeHeapChunk(int s)
{
  HeapChunk * hc = new HeapChunk(s);
  return makeTaggedExtension(hc);
}

#define NotHeapChunkWarning(T, F, R)                                        \
if (! OZ_isHeapChunk(T)) {                                                  \
  OZ_warning("Heap chunk expected in %s. Got 0x%x. Result undetermined.\n", \
             #F, T);                                                        \
  return R;                                                                 \
}

int OZ_getHeapChunkSize(TaggedRef t)
{
  NotHeapChunkWarning(t, OZ_getHeapChunkSize, 0);
  
  return tagged2HeapChunk(oz_deref(t))->getChunkSize();
}

void * OZ_getHeapChunkData(TaggedRef t)
{
  NotHeapChunkWarning(t, OZ_getHeapChunk, NULL);
  
  return tagged2HeapChunk(oz_deref(t))->getChunkData();
}

#define oz_declareHeapChunkIN(ARG,VAR)		\
HeapChunk *VAR;					\
{						\
  oz_declareNonvarIN(ARG,_VAR);			\
  if (!oz_isHeapChunk(oz_deref(_VAR))) {	\
    oz_typeError(ARG,"HeapChunk");		\
  } else {					\
    VAR = tagged2HeapChunk(oz_deref(_VAR));	\
  }						\
}

OZ_BI_define(BIHeapChunk_new,1,1)
{
  oz_declareIntIN(0,size);
  OZ_RETURN(makeTaggedExtension(new HeapChunk(size)));
} OZ_BI_end

OZ_BI_define(BIHeapChunk_is,1,1)
{
  oz_declareNonvarIN(0,x);
  OZ_RETURN(oz_isHeapChunk(oz_deref(x))? OZ_true(): OZ_false());
} OZ_BI_end

OZ_BI_define(BIHeapChunk_poke,3,0)
{
  oz_declareHeapChunkIN(0,hc);
  oz_declareIntIN(1,i);
  oz_declareIntIN(2,v);
  if (!hc->poke(i,v)) {
    return oz_raise(E_ERROR,E_KERNEL,"HeapChunk.index",2,
		    OZ_in(0),OZ_in(1));
  }
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIHeapChunk_peek,2,1)
{
  oz_declareHeapChunkIN(0,hc);
  oz_declareIntIN(1,i);
  int v=hc->peek(i);
  if (v<0) {
    return oz_raise(E_ERROR,E_KERNEL,"HeapChunk.index",2,
		    OZ_in(0),OZ_in(1));
  }
  OZ_RETURN(oz_int(v));
} OZ_BI_end

