/*
 *  Authors:
 *    Kostja Popov (kost@sics.se)
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $Date$
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

#ifndef __NEWMARSHALER_H
#define __NEWMARSHALER_H

#if defined(INTERFACE)
#pragma interface
#endif

#include "base.hh"

#include "gentraverser.hh"
int gopa;
//
class Marshaler : public GenTraverser {
public:
  virtual ~Marshaler() {}
  virtual void processSmallInt(OZ_Term siTerm);
  virtual void processFloat(OZ_Term floatTerm);
  virtual void processLiteral(OZ_Term litTerm);
  virtual void processExtension(OZ_Term extensionTerm);
  virtual void processBigInt(OZ_Term biTerm, ConstTerm *biConst);
  virtual void processBuiltin(OZ_Term biTerm, ConstTerm *biConst);
  virtual void processObject(OZ_Term objTerm, ConstTerm *objConst);
  virtual void processLock(OZ_Term lockTerm, Tertiary *lockTert);
  virtual void processCell(OZ_Term cellTerm, Tertiary *cellTert);
  virtual void processPort(OZ_Term portTerm, Tertiary *portTert);
  virtual void processResource(OZ_Term resTerm, Tertiary *resTert);
  virtual void processNoGood(OZ_Term resTerm);
  virtual void processUVar(OZ_Term uvarTerm);
  virtual void processCVar(OZ_Term cvarTerm);
  virtual Bool processRepetition(OZ_Term term, int repNumber);
  virtual Bool processLTuple(OZ_Term ltupleTerm);
  virtual Bool processSRecord(OZ_Term srecordTerm);
  virtual Bool processFSETValue(OZ_Term fsetvalueTerm);
  virtual Bool processDictionary(OZ_Term dictTerm, ConstTerm *dictConst);
  virtual Bool processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst);
  virtual Bool processClass(OZ_Term classTerm, ConstTerm *classConst);
  virtual Bool processAbstraction(OZ_Term absTerm, ConstTerm *absConst);
};

//
//
extern Marshaler marshaler;
extern Builder builder;

//
// Interface procedures. These are equivalent to former '...RT()'
// versions;
inline
void newMarshalTerm(OZ_Term term, MsgBuffer *bs)
{
  marshaler.traverse(term, (Opaque *) bs);
  marshalDIF(bs, DIF_EOF);
}  
OZ_Term newUnmarshalTerm(MsgBuffer *);

#endif
