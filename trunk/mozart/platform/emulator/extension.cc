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
  static int counter=0;
  return counter++;
}

int OZ_getUniqueId(void)
{
  return oz_newUniqueId();
}

SituatedExtension::SituatedExtension(void)
  : ConstTermWithHome(oz_currentBoard(), Co_SituatedExtension) {}

void SituatedExtension::printStreamV(ostream &out,int depth)
{
  out << "situatedExtension";
}

void SituatedExtension::printLongStreamV(ostream &out,int depth,
					 int offset)
{
  printStreamV(out,depth);
  out << endl;
}

OZ_Term SituatedExtension::inspectV()
{
  return oz_atom("situatedExtension");
}

void ConstExtension::printStreamV(ostream &out,int depth)
{
  out << "constExtension";
}

void ConstExtension::printLongStreamV(ostream &out,int depth,
					 int offset)
{
  printStreamV(out,depth);
  out << endl;
}

OZ_Term ConstExtension::inspectV()
{
  return oz_atom("constExtension");
}
