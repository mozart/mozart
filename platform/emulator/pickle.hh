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
#define ValuesITInitSize	2048
#define LocationsITInitSize	256

//
class Pickler : public GenTraverser {
private:
  MarshalerDict *vIT;		// shared with the resource excavator;
  // index table for (C heap) locations;
  AddressHashTableO1Reset *lIT;
  //
  Bool cc;			// cloneCells;

  // 
public:
  void resetPickler() {
    DebugCode(cc = TRUE;);
    lIT->mkEmpty();
  }
  //
  Pickler(MarshalerDict *vITin)
    : vIT(vITin) {
    lIT = new AddressHashTableO1Reset(LocationsITInitSize);
    resetPickler();
  }
  ~Pickler() {}

  //
  void setCloneCells(Bool ccIn) { cc = ccIn; }

  //
  void processSmallInt(OZ_Term siTerm);
  void processFloat(OZ_Term floatTerm);
  void processLiteral(OZ_Term litTerm);
  void processExtension(OZ_Term extensionTerm);
  void processBigInt(OZ_Term biTerm);
  void processBuiltin(OZ_Term biTerm, ConstTerm *biConst);
  void processLock(OZ_Term lockTerm, ConstTerm *lockConst);
  void processPort(OZ_Term portTerm, ConstTerm *portConst);
  void processResource(OZ_Term resTerm, ConstTerm *resConst);
  void processNoGood(OZ_Term resTerm);
  Bool processVar(OZ_Term cv, OZ_Term *varTerm);
  Bool processLTuple(OZ_Term ltupleTerm);
  Bool processSRecord(OZ_Term srecordTerm);
  Bool processFSETValue(OZ_Term fsetvalueTerm);
  Bool processDictionary(OZ_Term dictTerm, ConstTerm *dictConst);
  Bool processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst);
  Bool processClass(OZ_Term classTerm, ConstTerm *classConst);
  Bool processObject(OZ_Term objTerm, ConstTerm *objConst);
  Bool processCell(OZ_Term cellTerm, ConstTerm *cellConst);
  Bool processAbstraction(OZ_Term absTerm, ConstTerm *absConst);
  Bool processArray(OZ_Term arrayTerm, ConstTerm *arrayConst);
  void processSync();

  //
  void doit();			// actual processor;
  //
  void traverse(OZ_Term t);
  void resume(Opaque *o);
  void resume();

  //
  Bool cloneCells() { return (cc); }
};

//
#define	TRAVERSERCLASS	Pickler
#include "gentraverserLoop.hh"
#undef	TRAVERSERCLASS

//
// Extract resources & nogoods from a term into lists;
class ResourceExcavator : public GenTraverser {
private:
  // index table for (Oz) values (but not for locations: we don't
  // marshal anything now). It is shared with the pickler;
  MarshalerDict *vIT;
  //
  Bool cc;			// cloneCells;
  OZ_Term resources;
  OZ_Term nogoods;

  //
private:
  void addResource(OZ_Term r) { resources = oz_cons(r, resources); }
  void addNogood(OZ_Term ng) { nogoods = oz_cons(ng, nogoods); }

  //
public:
  void resetResourceExcavator() {
    DebugCode(cc = TRUE;);
    resources = nogoods = oz_nil();
    vIT->mkEmpty();
  }
  //
  ResourceExcavator(MarshalerDict *vITin)
    : vIT(vITin) {
    resetResourceExcavator();
  }
  ~ResourceExcavator() {}

  //
  void setCloneCells(Bool ccIn) { cc = ccIn; }

  //
  void processSmallInt(OZ_Term siTerm);
  void processFloat(OZ_Term floatTerm);
  void processLiteral(OZ_Term litTerm);
  void processExtension(OZ_Term extensionTerm);
  void processBigInt(OZ_Term biTerm);
  void processBuiltin(OZ_Term biTerm, ConstTerm *biConst);
  void processLock(OZ_Term lockTerm, ConstTerm *lockConst);
  void processPort(OZ_Term portTerm, ConstTerm *portConst);
  void processResource(OZ_Term resTerm, ConstTerm *resConst);
  void processNoGood(OZ_Term resTerm);
  Bool processVar(OZ_Term cv, OZ_Term *varTerm);
  Bool processLTuple(OZ_Term ltupleTerm);
  Bool processSRecord(OZ_Term srecordTerm);
  Bool processFSETValue(OZ_Term fsetvalueTerm);
  Bool processDictionary(OZ_Term dictTerm, ConstTerm *dictConst);
  Bool processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst);
  Bool processClass(OZ_Term classTerm, ConstTerm *classConst);
  Bool processObject(OZ_Term objTerm, ConstTerm *objConst);
  Bool processCell(OZ_Term cellTerm, ConstTerm *cellConst);
  Bool processAbstraction(OZ_Term absTerm, ConstTerm *absConst);
  Bool processArray(OZ_Term arrayTerm, ConstTerm *arrayConst);
  void processSync();

  //
  void doit();			// actual processor;
  //
  void traverse(OZ_Term t);
  void resume(Opaque *o);
  void resume();

  // (from former MarshalerBuffer's 'visit()' business;)
  Bool cloneCells() { return (cc); }

  //
  OZ_Term getResources()      { return (resources); }
  OZ_Term getNoGoods()        { return (nogoods); }
};

//
#define	TRAVERSERCLASS	ResourceExcavator
#include "gentraverserLoop.hh"
#undef	TRAVERSERCLASS

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
  // reset both the resource excavator and the pickler;
  re.resetResourceExcavator();
  pickler.resetPickler();
  //
  re.setCloneCells(cloneCells);
  re.prepareTraversing((Opaque *) 0);
  re.traverse(in);
  re.finishTraversing();
  //
  resources = re.getResources();
  nogoods = re.getNoGoods();
}

//
// Interface procedures;
inline
void pickleTerm(OZ_Term term, PickleMarshalerBuffer *bs, Bool cloneCells)
{
  pickler.setCloneCells(cloneCells);
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
OZ_Term unpickleTerm(PickleMarshalerBuffer *bs)
{
  unpickler.prepareBuild();
  return unpickleTermInternal(bs);
}

#endif /* __PICKLE_H */
