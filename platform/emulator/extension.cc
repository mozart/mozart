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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE)
#pragma implementation "extension.hh"
#endif

#include "extension.hh"
#include "ozostream.hh"
#include "am.hh"

unsigned int oz_newUniqueId() {
  static unsigned int counter=OZ_E_LAST;
  if (counter==0) error("oz_newUniqueId: counter overflow");
  return counter++;
}

int OZ_isExtension(OZ_Term t)
{
  return oz_isExtension(oz_deref(t));
}

Extension *OZ_getExtension(OZ_Term t)
{
  return oz_tagged2Extension(oz_deref(t));
}

OZ_Term OZ_extension(Extension *e)
{
  return oz_makeTaggedExtension(e);
}

unsigned int OZ_getUniqueId(void)
{
  return oz_newUniqueId();
}

Extension::~Extension()
{
  error("invoking destructor ~Extension()");
}

void Extension::operator delete(void*,size_t)
{
  error("invoking Extension::operator delete(void*,size_t)");
}

void Extension::printStreamV(ostream &out,int depth)
{
  out << "extension";
}

void Extension::printLongStreamV(ostream &out,int depth,
                                 int offset)
{
  printStreamV(out,depth);
  out << endl;
}

OZ_Term Extension::typeV()
{
  return oz_atom("extension");
}

SituatedExtension::SituatedExtension(void)
  : Extension()
{
  board = oz_currentBoard();
}

void SituatedExtension::printStreamV(ostream &out,int depth)
{
  out << "situatedExtension";
}

OZ_Term SituatedExtension::typeV()
{
  return oz_atom("situatedExtension");
}

Bool oz_isChunkExtension(TaggedRef term)
{
  return oz_tagged2Extension(term)->isChunkV();
}

static oz_unmarshalProcType *unmarshalRoutine = 0;
static int unmarshalRoutineArraySize = 0;

OZ_Term oz_extension_unmarshal(int type,MsgBuffer*bs) {
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
