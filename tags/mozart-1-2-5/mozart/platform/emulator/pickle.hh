/*
 *  Authors:
 *    Ralf Scheidhauer <Ralf.Scheidhauer@ps.uni-sb.de>
 *    Kostja Popov <kost@sics.se>
 * 
 *  Contributors:
 *    Andreas Sundstroem <andreas@sics.se>
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

#ifndef __PICKLE_H
#define __PICKLE_H

#if defined(INTERFACE)
#pragma interface
#endif

#include "base.hh"
#include "pickleBase.hh"
#include "var_base.hh"

//
// init stuff - must be called.  Actually, the pickling initializer
// initializes also certain generic marshaling stuff (memory
// management and robust marshaler's constants);
void initPickleMarshaler();

//
class Pickler : public GenTraverser {
private:
  Bool cc;			// cloneCells;

public:
  virtual ~Pickler() {}
  void init(Bool ccIn) {
    cc = ccIn;
  }

  //
  virtual void processSmallInt(OZ_Term siTerm);
  virtual void processFloat(OZ_Term floatTerm);
  virtual void processLiteral(OZ_Term litTerm);
  virtual void processExtension(OZ_Term extensionTerm);
  virtual void processBigInt(OZ_Term biTerm, ConstTerm *biConst);
  virtual void processBuiltin(OZ_Term biTerm, ConstTerm *biConst);
  virtual void processLock(OZ_Term lockTerm, Tertiary *lockTert);
  virtual void processPort(OZ_Term portTerm, Tertiary *portTert);
  virtual void processResource(OZ_Term resTerm, Tertiary *tert);
  virtual Bool processNoGood(OZ_Term resTerm, Bool trail);
  virtual void processVar(OZ_Term cv, OZ_Term *varTerm);
  virtual void processRepetition(OZ_Term t, OZ_Term *tPtr, int repNumber);
  virtual Bool processLTuple(OZ_Term ltupleTerm);
  virtual Bool processSRecord(OZ_Term srecordTerm);
  virtual Bool processFSETValue(OZ_Term fsetvalueTerm);
  virtual Bool processDictionary(OZ_Term dictTerm, ConstTerm *dictConst);
  virtual Bool processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst);
  virtual Bool processClass(OZ_Term classTerm, ConstTerm *classConst);
  virtual Bool processObject(OZ_Term objTerm, ConstTerm *objConst);
  virtual Bool processCell(OZ_Term cellTerm, Tertiary *cellTert);
  virtual Bool processAbstraction(OZ_Term absTerm, ConstTerm *absConst);
  virtual Bool processArray(OZ_Term arrayTerm, ConstTerm *arrayConst);
  virtual void processSync();

  //
  Bool cloneCells() { return (cc); }
};

//
// Extract resources & nogoods from a term into lists;
class ResourceExcavator : public GenTraverser {
private:
  Bool cc;			// cloneCells;
  OZ_Term resources;
  OZ_Term nogoods;

  //
private:
  void addResource(OZ_Term r) { resources = oz_cons(r, resources); }
  void addNogood(OZ_Term ng) { nogoods = oz_cons(ng, nogoods); }

  //
public:
  virtual ~ResourceExcavator() {}
  void init(Bool ccIn) {
    cc = ccIn;
    resources = nogoods = oz_nil();
  }

  //
  virtual void processSmallInt(OZ_Term siTerm);
  virtual void processFloat(OZ_Term floatTerm);
  virtual void processLiteral(OZ_Term litTerm);
  virtual void processExtension(OZ_Term extensionTerm);
  virtual void processBigInt(OZ_Term biTerm, ConstTerm *biConst);
  virtual void processBuiltin(OZ_Term biTerm, ConstTerm *biConst);
  virtual void processLock(OZ_Term lockTerm, Tertiary *lockTert);
  virtual void processPort(OZ_Term portTerm, Tertiary *portTert);
  virtual void processResource(OZ_Term resTerm, Tertiary *tert);
  virtual Bool processNoGood(OZ_Term resTerm, Bool trail);
  virtual void processVar(OZ_Term cv, OZ_Term *varTerm);
  virtual void processRepetition(OZ_Term t, OZ_Term *tPtr, int repNumber);
  virtual Bool processLTuple(OZ_Term ltupleTerm);
  virtual Bool processSRecord(OZ_Term srecordTerm);
  virtual Bool processFSETValue(OZ_Term fsetvalueTerm);
  virtual Bool processDictionary(OZ_Term dictTerm, ConstTerm *dictConst);
  virtual Bool processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst);
  virtual Bool processClass(OZ_Term classTerm, ConstTerm *classConst);
  virtual Bool processObject(OZ_Term objTerm, ConstTerm *objConst);
  virtual Bool processCell(OZ_Term cellTerm, Tertiary *cellTert);
  virtual Bool processAbstraction(OZ_Term absTerm, ConstTerm *absConst);
  virtual Bool processArray(OZ_Term arrayTerm, ConstTerm *arrayConst);
  virtual void processSync();

  // (from former MarshalerBuffer's 'visit()' business;)
  Bool cloneCells() { return (cc); }
  OZ_Term getResources()      { return (resources); }
  OZ_Term getNoGoods()        { return (nogoods); }
};

//
// Blocking factor for binary areas: how many Oz values a binary area
// may contain (in fact, modulo a constant factor: code area"s, for
// instance, count instruction fields with Oz values but not values
// themselves);
const int ozValuesBAPickles = 1024;
//
// These are the 'CodeAreaProcessor'"s for the pickling and plain
// traversing of code areas:
Bool pickleCode(GenTraverser *m, GTAbstractEntity *arg);
Bool traverseCode(GenTraverser *m, GTAbstractEntity *arg);

//
extern Pickler pickler;
extern ResourceExcavator re;
extern Builder unpickler;

//
inline
void extractResources(OZ_Term in, Bool cloneCells,
		      OZ_Term &resources, OZ_Term &nogoods)
{
  re.init(cloneCells);
  re.prepareTraversing((Opaque *) 0);
  re.traverse(in);
  re.finishTraversing();
  resources = re.getResources();
  nogoods = re.getNoGoods();
}

//
// Interface procedures;
inline
void pickleTerm(PickleMarshalerBuffer *bs, OZ_Term term, Bool cloneCells)
{
  pickler.init(cloneCells);
  pickler.prepareTraversing((Opaque *) bs);
  pickler.traverse(term);
  pickler.finishTraversing();
  marshalDIF(bs, DIF_EOF);
}

//
OZ_Term unpickleTermInternal(PickleMarshalerBuffer *);

//
// Interface procedures. 
inline
#ifdef USE_FAST_UNMARSHALER   
OZ_Term unpickleTerm(PickleMarshalerBuffer *bs)
#else
OZ_Term unpickleTermRobust(PickleMarshalerBuffer *bs)
#endif
{
  unpickler.prepareBuild();
  return unpickleTermInternal(bs);
}

#endif /* __PICKLE_H */
