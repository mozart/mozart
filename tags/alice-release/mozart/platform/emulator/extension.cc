/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Copyright:
 *    Michael Mehl (1998)
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
#pragma implementation "extension.hh"
#endif

#include "base.hh"
#include "am.hh"
#include "value.hh"

#if defined(__CYGWIN32__) || defined(__MINGW32__)
#  define ozdeclspec __declspec(dllexport)
#else
#  define ozdeclspec
#endif


ozdeclspec OZ_Extension::~OZ_Extension() { 
  OZ_error("invoking destructor ~OZ_Extension()"); 
}

ozdeclspec void OZ_Extension::operator delete(void*,size_t) { 
  OZ_error("invoking OZ_Extension::operator delete(void*,size_t)"); 
}

ozdeclspec OZ_Term OZ_Extension::typeV() { 
  return AtomExtension; 
}

ozdeclspec OZ_Term OZ_Extension::printLongV(int depth, int offset) {
  return OZ_pair2(printV(depth),AtomNewLine); 
}

ozdeclspec OZ_Term OZ_Extension::getFeatureV(OZ_Term f) {
  OZ_Term t;
  return (getFeatureV(f,t)==PROCEED)?t:0;
}

unsigned int oz_newUniqueId() {
  static unsigned int counter=OZ_E_LAST;
  if (counter==0) OZ_error("oz_newUniqueId: counter overflow");
  return counter++;
}

int OZ_isExtension(OZ_Term t)
{
  return oz_isExtension(oz_deref(t));
}

OZ_Extension *OZ_getExtension(OZ_Term t)
{
  return tagged2Extension(oz_deref(t));
}

OZ_Term OZ_extension(OZ_Extension *e)
{
  return makeTaggedExtension(e);
}

unsigned int OZ_getUniqueId(void)
{
  return oz_newUniqueId();
}


void* _OZ_new_OZ_Extension(size_t n) {
 return oz_heapMalloc(n);
}


OZ_Boolean _OZ_isLocal_OZ_Extension(void *inb)
{
  Board *bb=(Board*) inb;
  return bb==0?OZ_TRUE:oz_isCurrentBoard(bb);
}

void * _OZ_currentBoard()
{
  return oz_currentBoard();
}

Bool oz_isChunkExtension(TaggedRef term)
{
  return tagged2Extension(term)->isChunkV();
}

static oz_unmarshalProcType *unmarshalRoutine = 0;
static oz_suspUnmarshalProcType *suspUnmarshalRoutine = 0;
static oz_unmarshalContProcType *unmarshalContRoutine = 0;
static int unmarshalRoutineArraySize = 0;

OZ_Term oz_extension_unmarshal(int type, MarshalerBuffer* bs, Builder *b)
{
  oz_unmarshalProcType f = unmarshalRoutine[type];
  Assert(f);
  return (f(bs, b));
}

//
OZ_Term oz_extension_unmarshal(int type, ByteBuffer *bs, Builder *b,
			       GTAbstractEntity* &arg)
{
  oz_suspUnmarshalProcType f = suspUnmarshalRoutine[type];
  Assert(f);
  return (f(bs, b, arg));
}

//
OZ_Term oz_extension_unmarshalCont(int type, ByteBuffer *bs, Builder *b,
				   GTAbstractEntity *arg)
{
  oz_unmarshalContProcType f = unmarshalContRoutine[type];
  Assert(f);
  return (f(bs, b, arg));
}

void oz_registerExtension(int type,
			  oz_unmarshalProcType u,
			  oz_suspUnmarshalProcType su,
			  oz_unmarshalContProcType suc)
{
  if (unmarshalRoutineArraySize <= type) {
    int newsize = type + 1;
    oz_unmarshalProcType *us = new oz_unmarshalProcType[newsize];
    oz_suspUnmarshalProcType *sus = new oz_suspUnmarshalProcType[newsize];
    oz_unmarshalContProcType *sucs = new oz_unmarshalContProcType[newsize];
    for (int i = unmarshalRoutineArraySize; i--;) {
      us[i] = unmarshalRoutine[i];
      sus[i] = suspUnmarshalRoutine[i];
      sucs[i] = unmarshalContRoutine[i];
    }
    if (unmarshalRoutine) delete [] unmarshalRoutine;
    if (suspUnmarshalRoutine) delete [] suspUnmarshalRoutine;
    if (unmarshalContRoutine) delete [] unmarshalContRoutine;
    unmarshalRoutine = us;
    suspUnmarshalRoutine = sus;
    unmarshalContRoutine = sucs;
    unmarshalRoutineArraySize = newsize;
  }
  unmarshalRoutine[type] = u;
  suspUnmarshalRoutine[type] = su;
  unmarshalContRoutine[type] = suc;
}

void initExtensions() {
  extern void BitString_init();
  extern void ByteString_init();
  extern void Word_init();
  BitString_init();
  ByteString_init();
  Word_init();
}
