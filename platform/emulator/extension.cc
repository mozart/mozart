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
  return oz_tagged2Extension(oz_deref(t));
}

OZ_Term OZ_extension(OZ_Extension *e)
{
  return oz_makeTaggedExtension(e);
}

unsigned int OZ_getUniqueId(void)
{
  return oz_newUniqueId();
}

OZ_Extension::~OZ_Extension()
{
  OZ_error("invoking destructor ~OZ_Extension()");
}

void* OZ_Extension::operator new(size_t n) {
  return alignedMalloc(n,sizeof(double));
}

void OZ_Extension::operator delete(void*,size_t)
{
  OZ_error("invoking OZ_Extension::operator delete(void*,size_t)");
}

OZ_Term OZ_Extension::printV(int depth)
{
  return typeV();
}

OZ_Term OZ_Extension::printLongV(int depth, int offset)
{
  return oz_pair2(printV(depth),oz_atom("\n"));
}

OZ_Term OZ_Extension::typeV()
{
  return oz_atom("extension");
}

OZ_Boolean OZ_Extension::isLocal()
{
  Board *bb=(Board*) __getSpaceInternal();
  return bb==0?OZ_TRUE:oz_isCurrentBoard(bb);
}

OZ_SituatedExtension::OZ_SituatedExtension(void)
  : OZ_Extension()
{
  space = oz_currentBoard();
}

OZ_Term OZ_SituatedExtension::typeV()
{
  return oz_atom("situatedExtension");
}

Bool oz_isChunkExtension(TaggedRef term)
{
  return oz_tagged2Extension(term)->isChunkV();
}

static oz_unmarshalProcType *unmarshalRoutine = 0;
static int unmarshalRoutineArraySize = 0;

OZ_Term oz_extension_unmarshal(int type,void*bs) {
  oz_unmarshalProcType f = unmarshalRoutine[type];
  if (f==0) return 0;
  else return f(bs);
}

void oz_registerExtension(int type, oz_unmarshalProcType f)
{
  if (unmarshalRoutineArraySize<type) {
    oz_unmarshalProcType *n=new oz_unmarshalProcType[type+100];
    for (int i=unmarshalRoutineArraySize; i--;) {
      n[i]=unmarshalRoutine[i];
    }
    if (unmarshalRoutine) delete [] unmarshalRoutine;
    unmarshalRoutine = n;
    unmarshalRoutineArraySize = type+100;
  }
  unmarshalRoutine[type] = f;
}

void initExtensions() {
  extern void BitString_init();
  extern void ByteString_init();
  BitString_init();
  ByteString_init();
}
