/*
 *  Authors:
 *    Kostja Popov (kost@sics.se)
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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
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

//A returnvalue from unmarshalCodeRobust
#define ERR 123
//
// Constants needed for to check that no overflow occurs in unmarshalNumber()

// Biggest number dividable with 7 and less then sizeof(int)
#define RobustMarshaler_Max_Shift   ((sizeof(int)==4) ? 28 : 16)
// (sizeof(int)-RobustMarshaler_Max_Shift)^2
#define RobustMarshaler_Max_Hi_Byte ((sizeof(int)==4) ? 63 : 2)
// for computation see newmarshaller.cc

#ifdef USE_FAST_UNMARSHALER   
inline int unmarshalRefTag(MsgBuffer *bs)
{
  return unmarshalNumber(bs);
}
#else
inline int unmarshalRefTagRobust(MsgBuffer *bs, Builder *builder, int *error)
{
  int e;
  int rt = unmarshalNumberRobust(bs, &e);
  *error = e || !builder->checkNewIndex(rt); // RefTags are found in order?
  return rt;
}
#endif
OZ_Return oz_export(OZ_Term t);

#ifdef USE_FAST_UNMARSHALER   
GName* unmarshalGName(TaggedRef*,MsgBuffer*);
#else
GName* unmarshalGNameRobust(TaggedRef*,MsgBuffer*,int*);
#endif
void marshalGName(GName *gname, MsgBuffer *bs);

class SendRecvCounter {
private:
  long c[2];
public:
  SendRecvCounter() { c[0]=0; c[1]=0; }
  void send() { c[0]++; }
  long getSend() { return c[0]; }
  void recv() { c[1]++; }
  long getRecv() { return c[1]; }
};

//
extern SendRecvCounter dif_counter[];
extern SendRecvCounter misc_counter[];
//
extern char *misc_names[];

// the names of the difs for statistics 
enum {
  MISC_STRING,
  MISC_GNAME,
  MISC_SITE,
  //
  MISC_LAST
};


//
// init stuff - must be called;
void initNewMarshaler();

//
// Blocking factor for binary areas: how many Oz values a binary area
// may contain (in fact, module a constant factor: code area"s, for
// instance, count instructions with Oz values but not values
// themselves);
const int ozValuesBA = 1024;

//
// Memory management for arguments of marshaler's
// 'BinaryAreaProcessor' and builder's 'OzValueProcessor'.
//
#define NMMM_SIZE	12
//
class NMMemoryManager {
  static int32* freelist[NMMM_SIZE];

  //
public:

  //
  void operator delete(void *obj, size_t size) {
    int index = size / sizeof(int32);
    Assert(index);		// must contain at least one word;
    Assert(index * sizeof(int32) == size);
    Assert(index < NMMM_SIZE);
    //
    *((int32 **) obj) = freelist[index];
    freelist[index] = (int32 *) obj;
  }

  // must be empty;
  NMMemoryManager() {}
  ~NMMemoryManager() {}

  //
  static void init() {
    for (int i = 0; i < NMMM_SIZE; i++)
      freelist[i] = (int32 *) 0;
  }

  //
  void *operator new(size_t size) {
    int index = size / sizeof(int32);
    int32 *ptr;
    Assert(index);		// must contain at least one word;
    Assert(index * sizeof(int32) == size);
    Assert(index < NMMM_SIZE);

    //
    ptr = freelist[index];
    if (ptr) {
      freelist[index] = (int32 *) *ptr;
      return (ptr);
    } else {
      return (malloc(size));
    }
  }

};

//
// 'CodeAreaProcessor' argument: keeps the location of a code area
// being processed;
class MarshalerCodeAreaDescriptor : public NMMemoryManager {
private:
  ProgramCounter start, end, current;
public:
  MarshalerCodeAreaDescriptor(ProgramCounter startIn, ProgramCounter endIn)
    : start(startIn), end(endIn), current(startIn) {}
  //
  ProgramCounter getStart() { return (start); }
  ProgramCounter getEnd() { return (end); }
  ProgramCounter getCurrent() { return (current); }
  void setCurrent(ProgramCounter pc) { current = pc; }
};

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
  virtual Bool processCell(OZ_Term cellTerm, Tertiary *cellTert);
  virtual void processPort(OZ_Term portTerm, Tertiary *portTert);
  virtual void processResource(OZ_Term resTerm, Tertiary *tert);
  virtual void processNoGood(OZ_Term resTerm, Bool trail);
  virtual void processUVar(OZ_Term *uvarTerm);
  virtual OZ_Term processCVar(OZ_Term *cvarTerm);
  virtual Bool processRepetition(int repNumber);
  virtual Bool processLTuple(OZ_Term ltupleTerm);
  virtual Bool processSRecord(OZ_Term srecordTerm);
  virtual Bool processFSETValue(OZ_Term fsetvalueTerm);
  virtual Bool processDictionary(OZ_Term dictTerm, ConstTerm *dictConst);
  virtual Bool processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst);
  virtual Bool processClass(OZ_Term classTerm, ConstTerm *classConst);
  virtual Bool processAbstraction(OZ_Term absTerm, ConstTerm *absConst);
  virtual void processSync();
};

//
// That's the corresponding 'CodeAreaProcessor':
Bool newMarshalCode(GenTraverser *m, void *arg);

//
//
extern Marshaler marshaler;
extern Builder builder;

//
class BuilderCodeAreaDescriptor : public NMMemoryManager {
private:
  ProgramCounter start, end, current;
  CodeArea *code;
public:
  BuilderCodeAreaDescriptor(ProgramCounter startIn, ProgramCounter endIn,
			    CodeArea *codeIn)
    : start(startIn), end(endIn), current(startIn), code(codeIn) {}
  //
  ProgramCounter getStart() { return (start); }
  ProgramCounter getEnd() { return (end); }
  ProgramCounter getCurrent() { return (current); }
  void setCurrent(ProgramCounter pc) { current = pc; }
  //
  CodeArea* getCodeArea() { return (code); }
};

//
// Interface procedures. These are equivalent to former '...RT()'
// versions;
inline
void newMarshalTerm(OZ_Term term, MsgBuffer *bs)
{
  marshaler.traverse(term, (Opaque *) bs);
  marshalDIF(bs, DIF_EOF);
}

inline
void newMarshalerStartBatch(MsgBuffer *bs)
{
  marshaler.prepareTraversing((Opaque *) bs);
}
//
inline
void newMarshalTermInBatch(OZ_Term term)
{
  marshaler.traverseOne(term);
  marshalDIF((MsgBuffer *) marshaler.getOpaque(), DIF_EOF);
}  
//
inline
void newMarshalerFinishBatch()
{
  marshaler.finishTraversing();
}

//
OZ_Term newUnmarshalTermInternal(MsgBuffer *);

//
// Interface procedures. 
#ifdef USE_FAST_UNMARSHALER   
inline
OZ_Term newUnmarshalTerm(MsgBuffer *bs)
{
  return newUnmarshalTermInternal(bs);
}
#else
inline
OZ_Term newUnmarshalTermRobust(MsgBuffer *bs)
{
  builder.resetIndexChecker();
  OZ_Term n = newUnmarshalTermInternal(bs);
  return n;
}
#endif

inline
void newUnmarshalerStartBatch()
{
  builder.resetIndexChecker();
}

#endif
