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

#include "extension.hh"
#include "ozostream.hh"
#include "am.hh"

int oz_newUniqueId() {
  static int counter=OZ_E_LAST;
  return counter++;
}

int OZ_getUniqueId(void)
{
  return oz_newUniqueId();
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

static oz_unmarshalProcType *unmarshalRoutine = 0;
static int unmarshalRoutineArraySize = 0;

OZ_Term oz_extension_unmarshal(int type,MsgBuffer*bs) {
  oz_unmarshalProcType f = unmarshalRoutine[type];
  if (f==0) return 0;
  else return f(bs);
}

void oz_registerConstExtension(int type, oz_unmarshalProcType f)
{
  if (unmarshalRoutineArraySize<type) {
    oz_unmarshalProcType *n=new oz_unmarshalProcType[type+100];
    for (int i=unmarshalRoutineArraySize; i--;) {
      n[i]=unmarshalRoutine[i];
    }
    if (unmarshalRoutine) delete [] unmarshalRoutine;
    unmarshalRoutine = n;
  }
  unmarshalRoutine[type] = f;
}
