/*
 *  Authors:
 *    Andreas Sundstrom <andreas@sics.se>
 *    Kostja Popov (kost@sics.se)
 * 
 *  Contributors:
 *
 *  Copyright:
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

//---------------------------------------------------------------------
// General methods to class Builder in gentraverser.hh
// specialized in fast_builder_methods.hh and robust_builder_methods.hh
//---------------------------------------------------------------------

//
// 'buildValue' is the main actor: it pops tasks;
void buildValue_ROBUST(OZ_Term value) {
  CrazyDebug(incDebugNODES(););
  GetBTFrame(frame);
  GetBTTaskType(frame, type);
  if (type == BT_spointer) {
    GetBTTaskPtr1(frame, OZ_Term*, spointer);
    DiscardBTFrame(frame);
    SetBTFrame(frame);
    *spointer = value;
  } else {
    buildValueOutline_ROBUST(value, frame, type);
  }
}

void buildValueRemember_ROBUST(OZ_Term value, int n) {
  buildValue_ROBUST(value);
  set(value, n);
}

//
void buildRepetition_ROBUST(int n) {
  OZ_Term value = get(n);
  buildValue_ROBUST(value);
}

//
void buildList_ROBUST() {
  LTuple *l = new LTuple();
  buildValue_ROBUST(makeTaggedLTuple(l));
  GetBTFrame(frame);
  EnsureBTSpace(frame, 2);
  PutBTTaskPtr(frame, BT_spointer, l->getRefTail());
  PutBTTaskPtr(frame, BT_spointer, l->getRefHead());
  SetBTFrame(frame);
}
void buildListRemember_ROBUST(int n) {
  LTuple *l = new LTuple();
  OZ_Term list = makeTaggedLTuple(l);
  buildValue_ROBUST(list);
  set(list, n);
  GetBTFrame(frame);
  EnsureBTSpace(frame, 2);
  PutBTTaskPtr(frame, BT_spointer, l->getRefTail());
  PutBTTaskPtr(frame, BT_spointer, l->getRefHead());
  SetBTFrame(frame);
}

//
void buildDictionary_ROBUST(int size) {
  OzDictionary *aux = new OzDictionary(am.currentBoard(), size);
  aux->markSafe();
  //
  buildValue_ROBUST(makeTaggedConst(aux));
  //
  GetBTFrame(frame);
  EnsureBTSpace(frame, size);
  while(size-- > 0) {
    PutBTTaskPtr(frame, BT_dictKey, aux);
  }
  SetBTFrame(frame);
}

//
void buildDictionaryRemember_ROBUST(int size, int n) {
  OzDictionary *aux = new OzDictionary(am.currentBoard(),size);
  aux->markSafe();
  //
  OZ_Term dict = makeTaggedConst(aux);
  buildValue_ROBUST(dict);
  set(dict, n);
  //
  GetBTFrame(frame);
  EnsureBTSpace(frame, size);
  while(size-- > 0) {
    PutBTTaskPtr(frame, BT_dictKey, aux);
  }
  SetBTFrame(frame);
}

void knownChunk_ROBUST(OZ_Term chunkTerm) {
  buildValue_ROBUST(chunkTerm);
  putTask(BT_spointer, &blackhole);
}

void knownClass_ROBUST(OZ_Term classTerm) {
  buildValue_ROBUST(classTerm);
  putTask(BT_spointer, &blackhole); // class features;
}

//
// There is a need for 'knownProc' since a user is not supposed to
// understand the structure of closure's (Oz) terms that are to be
// skipped;
void knownProcRemember_ROBUST(OZ_Term procTerm, int memoIndex) {
  buildValue_ROBUST(procTerm);
  set(procTerm, memoIndex);
  
  //
  Abstraction *pp = (Abstraction *) tagged2Const(procTerm);
  Assert(isAbstraction(pp));
  int gsize = pp->getPred()->getGSize();
  //
  GetBTFrame(frame);
  EnsureBTSpace(frame, gsize+2); // 'name' and 'file' as well;
  for (int i = 0; i < gsize; i++) {
    PutBTTaskPtr(frame, BT_spointer, &blackhole);
  }
  PutBTTaskPtr(frame, BT_spointer, &blackhole); // name;
  PutBTTaskPtr(frame, BT_spointer, &blackhole); // file;
  SetBTFrame(frame);
}
