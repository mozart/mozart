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

#ifndef __EXTVAR__H__
#define __EXTVAR__H__

#include "genvar.hh"

class ExtentedVar : public GenCVariable {
public:
  // gc: copying
  virtual GenCVariable* gcV() = 0;
  // gc: collect entry points
  virtual void          gcRecurseV() = 0;
  // tell
  virtual OZ_Return     unifyV(TaggedRef *, TaggedRef, ByteCode *) = 0;
  virtual OZ_Return     bindV(TaggedRef *, TaggedRef, ByteCode *) = 0;
  // ask
  virtual OZ_Return     validV(TaggedRef) = 0;
  // suspend
  virtual void addSuspV(TaggedRef *vPtr, Suspension susp, int unstable = TRUE)
  {
    addSuspSVar(susp, unstable);
  }
  // printing/debugging
  virtual void          printStreamV(ostream &out,int depth = 10)
  {
    out << "<cvar: " << getType() << ">";
  }
  virtual void          printLongStreamV(ostream &out,int depth = 10,
                                         int offset = 0)
  {
    printStreamV(out,depth); out << endl;
  }
  void                  print(void)
    { printStreamV(cerr); cerr << endl; cerr.flush(); }
  void                  printLong(void)
    { printLongStreamV(cerr); cerr.flush(); }
  virtual OZ_Term       inspectV();
  virtual int           getSuspListLengthV()
    { return getSuspListLengthS(); }
};

#endif
