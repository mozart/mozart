/* -*- C++ -*-
 *  Authors:
 *    Konstantin Popov <kost@sics.se>
 *    Per Brand (perbrand@sics.se)
 * 
 *  Contributors:
 *    Andreas Sundstroem <andreas@sics.se>
 * 
 *  Copyright:
 *    Per Brand, 1998
 *    Konstantin Popov, 2000
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

#ifndef __DPMARSHALER_HH
#define __DPMARSHALER_HH

#include "base.hh"
#include "dpBase.hh"
#include "msgType.hh"
#include "byteBuffer.hh"
#include "perdio.hh"
#include "table.hh"
#include "marshalerBase.hh"
#include "gname.hh"

#ifdef INTERFACE  
#pragma interface
#endif

//
void dpmInit();

//
enum DPMAbstractEntities {
  // literals;
  GT_LiteralSusp,		// generic (marshaler);
  //
  GT_AtomSusp,			// (unmarshaler);
  GT_UniqueNameSusp,
  GT_CopyableNameSusp,
  GT_NameSusp,
  // extensions;
  GT_ExtensionSusp		// generic;
};

//
// Abstract continuation for literals' 'suspendAC':
class DPMarshalerLitSusp : public GTAbstractEntity, 
			   public NMMemoryManager {
private:
  OZ_Term lt;
  int totalSize;
  int index;

  //
public:
  DPMarshalerLitSusp(OZ_Term ltIn, int tsIn)
    : lt(ltIn), totalSize(tsIn), index(0) {
    Assert(oz_isLiteral(ltIn));
    Assert(totalSize >= 0);
  }
  virtual ~DPMarshalerLitSusp() {
    DebugCode(lt = (OZ_Term) -1);
    DebugCode(totalSize = index = -1);
  }

  //
  int getTotalSize() { return (totalSize); }
  int getCurrentSize() { return (totalSize - index); }
  const char* getRemainingString() {
    Assert(index <= totalSize);
    Literal *lit = tagged2Literal(lt);
    const char *pn = lit->getPrintName();
    return (pn + index);
  }
  void incIndex(int inc) {
    index += inc;
    Assert(index < totalSize);
  }

  //
  virtual int getType() { return (GT_LiteralSusp); }
  virtual void gc() {
    Assert(lt);
    oz_gCollectTerm(lt, lt);
  }
};

//
class DPMarshaler : public GenTraverser {
private:
  // Support for lazy protocols: when 'doDone' is set to TRUE, a
  // complete representation of the first object (etc.?) is generated,
  // as opposed to its stub.
  //
  // Note that 'DPMarshaler' is not specialized (as opposed to the
  // 'GenTraverser's spirit, when every single variation of a
  // marshaling activity has its own class). This is because marshaler
  // objects are large and, hence, cannot be re-created all the time.
  // Instead, a pool of standard 'DPMarshaler's is kept.
  Bool doToplevel;

public:
  DPMarshaler() : doToplevel(FALSE) {}

  //
  virtual ~DPMarshaler() {}
  virtual void processSmallInt(OZ_Term siTerm);
  virtual void processFloat(OZ_Term floatTerm);
  virtual void processLiteral(OZ_Term litTerm);
  virtual void processExtension(OZ_Term extensionTerm);
  virtual void processBigInt(OZ_Term biTerm, ConstTerm *biConst);
  virtual void processBuiltin(OZ_Term biTerm, ConstTerm *biConst);
  virtual Bool processObject(OZ_Term objTerm, ConstTerm *objConst);
  virtual void processLock(OZ_Term lockTerm, Tertiary *lockTert);
  virtual Bool processCell(OZ_Term cellTerm, Tertiary *cellTert);
  virtual void processPort(OZ_Term portTerm, Tertiary *portTert);
  virtual void processResource(OZ_Term resTerm, Tertiary *tert);
  virtual void processNoGood(OZ_Term resTerm, Bool trail);
  virtual void processUVar(OZ_Term uv, OZ_Term *uvarTerm);
  virtual OZ_Term processCVar(OZ_Term cv, OZ_Term *cvarTerm);
  virtual void processRepetition(OZ_Term t, int repNumber);
  virtual Bool processLTuple(OZ_Term ltupleTerm);
  virtual Bool processSRecord(OZ_Term srecordTerm);
  virtual Bool processFSETValue(OZ_Term fsetvalueTerm);
  virtual Bool processDictionary(OZ_Term dictTerm, ConstTerm *dictConst);
  virtual Bool processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst);
  virtual Bool processClass(OZ_Term classTerm, ConstTerm *classConst);
  virtual Bool processAbstraction(OZ_Term absTerm, ConstTerm *absConst);
  virtual Bool processArray(OZ_Term arrayTerm, ConstTerm *arrayConst);
  virtual void processSync();

  //
  // Support for lazy protocols: next marshaling will generate a full
  // representation of a top-level object (as opposed to its stub);
  void genFullToplevel() { doToplevel = TRUE; }
  DebugCode(Bool isFullToplevel() { return (doToplevel); })

  //
private:
  Bool marshalObjectStub(OZ_Term objTerm, ConstTerm *objConst);
  Bool marshalFullObject(OZ_Term objTerm, ConstTerm *objConst);
};

//
// Extract variables from a term into a list (former '::digOutVars()'
// business;)
class VariableExcavator : public GenTraverser {
private:
  OZ_Term vars;

  //
private:
  void addVar(OZ_Term v) { vars = oz_cons(v, vars); }

  //
public:
  virtual ~VariableExcavator() {}
  void init() { vars = (OZ_Term) 0; }
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
  virtual void processNoGood(OZ_Term resTerm, Bool trail);
  virtual void processUVar(OZ_Term uv, OZ_Term *uvarTerm);
  virtual OZ_Term processCVar(OZ_Term cv, OZ_Term *cvarTerm);
  virtual void processRepetition(OZ_Term t, int repNumber);
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
  OZ_Term getVars()      { return (vars); }
};


//
// Blocking factor for binary areas: how many Oz values a binary area
// may contain (in fact, modulo a constant factor: code area"s, for
// instance, count instruction fields with Oz values but not values
// themselves);
const int ozValuesBADP = 1024;

//
// That's the corresponding 'CodeAreaProcessor':
Bool dpMarshalCode(GenTraverser *m, void *arg);
// 'traverseCode' is shared with the centralized system (logically,
// yeah?)
Bool traverseCode(GenTraverser *m, void *arg);

//
extern VariableExcavator ve;

//
inline
void dpMarshalerStartBatch(ByteBuffer *bs, DPMarshaler *dpm)
{
  dpm->prepareTraversing((Opaque *) bs);
}
inline
void dpMarshalerFinishBatch(DPMarshaler *dpm)
{
  dpm->finishTraversing();
}

//
static inline
DPMarshaler* dpMarshalerFinishTerm(ByteBuffer *bs, DPMarshaler *dpm)
{
  if (dpm->isFinished()) {
    dpMarshalerFinishBatch(dpm);
    marshalDIF(bs, DIF_EOF);
    return ((DPMarshaler *) 0);
  } else {
    // batch has not been finished yet;
    return (dpm);
  }
}

//
// Interface procedures;
// 'dpMarshalTerm' with three arguments is called when a first frame
// is marshaled, and with two arguments - for subsequent frames;
inline
DPMarshaler* dpMarshalTerm(ByteBuffer *bs, DPMarshaler *dpm, OZ_Term term)
{
  Assert(dpm->isFinished());
  //
  dpMarshalerStartBatch(bs, dpm);
  dpm->traverse(term);
  //
  return (dpMarshalerFinishTerm(bs, dpm));
}
//
inline
DPMarshaler* dpMarshalTerm(ByteBuffer *bs, DPMarshaler *dpm)
{
  Assert(!dpm->isFinished());
  //
  dpm->resume();
  //
  return (dpMarshalerFinishTerm(bs, dpm));
}

//
inline
OZ_Term extractVars(OZ_Term in)
{
  ve.init();
  ve.prepareTraversing((Opaque *) 0);
  ve.traverse(in);
  ve.finishTraversing();
  return (ve.getVars());
}

//
// Unmarshaling;
//

//
class DPUnmarshalerLitSusp : public GTAbstractEntity, 
			     public NMMemoryManager {
private:
  int refTag;
  int nameSize;
  char *printname;
  int pnSize;

  //
public:
  DPUnmarshalerLitSusp(int refTagIn, int nameSizeIn,
		       char *printnameIn, int pnSizeIn)
    : refTag(refTagIn), nameSize(nameSizeIn),
      printname(printnameIn), pnSize(pnSizeIn) {}
  // 'virtual' is added just to fix compiler warning;
  virtual ~DPUnmarshalerLitSusp() {
    delete printname;
    DebugCode(printname = (char *) 0);
    DebugCode(refTag = nameSize = pnSize = -1);
  }

  //
  virtual int getType() = 0;
  virtual void gc() = 0;

  //
  int getRefTag() { return (refTag); }
  int getNameSize() { return (nameSize); }
  char *getPrintname() { return (printname); }
  int getPNSize() { return (pnSize); }

  //
  void appendPrintname(int size, char *app) {
    char *npn = new char[pnSize + size + 1];
    strncpy(npn, printname, pnSize);
    strncpy(npn+pnSize, app, size);
    pnSize += size;
    npn[pnSize] = (char) 0;
    delete printname;
    printname = npn;
  }
};

//
class DPUnmarshalerAtomSusp : public DPUnmarshalerLitSusp {
public:
  DPUnmarshalerAtomSusp(int refTagIn, int nameSizeIn,
			char *printnameIn, int pnSizeIn)
    : DPUnmarshalerLitSusp(refTagIn, nameSizeIn, printnameIn, pnSizeIn) {}
  // 'virtual' is added just to fix compiler warning;
  virtual ~DPUnmarshalerAtomSusp() {}

  //
  virtual int getType() { return (GT_AtomSusp); }
  virtual void gc() {}
};

//
class DPUnmarshalerUniqueNameSusp : public DPUnmarshalerLitSusp {
public:
  DPUnmarshalerUniqueNameSusp(int refTagIn, int nameSizeIn,
			      char *printnameIn, int pnSizeIn)
    : DPUnmarshalerLitSusp(refTagIn, nameSizeIn, printnameIn, pnSizeIn) {}
  // 'virtual' is added just to fix compiler warning;
  virtual ~DPUnmarshalerUniqueNameSusp() {}

  //
  virtual int getType() { return (GT_UniqueNameSusp); }
  virtual void gc() {}
};

//
class DPUnmarshalerCopyableNameSusp : public DPUnmarshalerLitSusp {
public:
  DPUnmarshalerCopyableNameSusp(int refTagIn, int nameSizeIn,
				char *printnameIn, int pnSizeIn)
    : DPUnmarshalerLitSusp(refTagIn, nameSizeIn, printnameIn, pnSizeIn) {}
  // 'virtual' is added just to fix compiler warning;
  virtual ~DPUnmarshalerCopyableNameSusp() {}

  //
  virtual int getType() { return (GT_CopyableNameSusp); }
  virtual void gc() {}
};


//
class DPUnmarshalerNameSusp : public DPUnmarshalerLitSusp {
private:
  OZ_Term value;
  GName *gname;

  //
public:
  DPUnmarshalerNameSusp(int refTagIn, int nameSizeIn, OZ_Term valueIn,
			GName *gnameIn, char *printnameIn, int pnSizeIn)
    : DPUnmarshalerLitSusp(refTagIn, nameSizeIn, printnameIn, pnSizeIn),
      value(valueIn), gname(gnameIn) {}
  // 'virtual' is added just to fix compiler warning;
  virtual ~DPUnmarshalerNameSusp() {}

  //
  virtual int getType() { return (GT_NameSusp); }
  virtual void gc() {
    if (gname) {
      Assert(!value);
      gCollectGName(gname);
    } else {
      Assert(value);
      oz_gCollectTerm(value, value);
    }
  }

  //
  OZ_Term getValue() { return (value); }
  GName *getGName() { return (gname); }
};


//
OZ_Term dpUnmarshalTerm(ByteBuffer *bs, Builder *dpb);

//
inline
void dpUnmarshalerStartBatch(Builder *b)
{
  b->prepareBuild();
}
inline
void dpUnmarshalerFinishBatch(Builder *b) {}

//
class SendRecvCounter;
extern SendRecvCounter mess_counter[];

//
void marshalDSite(MarshalerBuffer *, DSite *);
void marshalVarObject(ByteBuffer *bs, int BTI, GName *gnobj, GName *gnclass);

MessageType unmarshalHeader(MarshalerBuffer *bs);
// '-1' means the action is suspended;

inline void marshalCredit(MarshalerBuffer *bs, Credit credit){  
  Assert(sizeof(Credit)==sizeof(int));
  Assert(sizeof(Credit)==sizeof(unsigned int));
  PD((MARSHAL,"credit c:%d",credit));
  PD((CREDIT,"marshal:credit c:%d",credit));
  marshalNumber(bs, credit);}

#ifdef USE_FAST_UNMARSHALER   
inline Credit unmarshalCredit(MarshalerBuffer *bs){
  Assert(sizeof(Credit)==sizeof(int));
  Credit c=unmarshalNumber(bs);
  PD((UNMARSHAL,"credit c:%d",c));
  PD((CREDIT,"unmarshal:credit c:%d",c));
  return c;}
#else
inline Credit unmarshalCreditRobust(MarshalerBuffer *bs, int *error){
  Assert(sizeof(Credit)==sizeof(int));
  Credit c=unmarshalNumberRobust(bs,error);
  PD((UNMARSHAL,"credit c:%d",c));
  PD((CREDIT,"unmarshal:credit c:%d",c));
  return c;}
#endif

// var.cc
#ifdef USE_FAST_UNMARSHALER   
OZ_Term unmarshalBorrow(MarshalerBuffer *bs,OB_Entry *&ob,int &bi);
#else
OZ_Term unmarshalBorrowRobust(MarshalerBuffer *bs,OB_Entry *&ob,int &bi,int *error);
#endif
void marshalToOwner(MarshalerBuffer *bs, int bi);
void marshalBorrowHead(MarshalerBuffer *bs, MarshalTag tag, int bi);
void marshalOwnHead(MarshalerBuffer *bs, int tag, int i);

//
void marshalTertiary(ByteBuffer *bs, Tertiary *t, MarshalTag tag);
#ifdef USE_FAST_UNMARSHALER   
OZ_Term unmarshalTertiary(MarshalerBuffer *bs, MarshalTag tag);
OZ_Term unmarshalOwner(MarshalerBuffer *bs,MarshalTag mt);
#else
OZ_Term unmarshalTertiaryRobust(MarshalerBuffer *bs, MarshalTag tag,int *error);
OZ_Term unmarshalOwnerRobust(MarshalerBuffer *bs,MarshalTag mt,int *error);
#endif
void marshalObject(MarshalerBuffer *bs, ConstTerm* t);
void marshalSPP(MarshalerBuffer *bs, TaggedRef entity, Bool trail);


//
// The maximal sizes for the marshal* routines are defined as follows:
// aux:
#define MCreditMaxSize MNumberMaxSize 
#define MBaseSiteMaxSize (3*MNumberMaxSize + MShortMaxSize)
#define MVirtualInfoMaxSize (4*MNumberMaxSize + MShortMaxSize)
#define MDSiteMaxSize (DIFMaxSize + MBaseSiteMaxSize + MVirtualInfoMaxSize)
#define MBorrowHeadMaxSize (2*DIFMaxSize + 2*MDSiteMaxSize + MNumberMaxSize + MCreditMaxSize)
#define MOwnHeadMaxSize (2*DIFMaxSize + MDSiteMaxSize + MNumberMaxSize + MCreditMaxSize)
#define MObjectVarMaxSize (MBorrowHeadMaxSize + 2*MGNameMaxSize)
// now:
#define MTertiaryMaxSize MBorrowHeadMaxSize
#define MDistVarMaxSize MObjectVarMaxSize
#define MDistObjectMaxSize MObjectVarMaxSize
#define MDistSPPMaxSize MOwnHeadMaxSize

//
// Top-level management of marshalers (to be used by transport
// objects). 

//
#define MUEmpty          0x0
#define MUMarshalerBusy  0x1
#define MUBuilderBusy    0x2
//
typedef struct {
  int flags;
  DPMarshaler *m;
  Builder *b;
} MU;				// marshaler unit

//
class DPMarshalers {
private:
  MU *mus;
  int musNum;

  //
protected:
  DPMarshalers() : mus((MU *) 0), musNum(0) {}
  ~DPMarshalers() { dpAllocateMarshalers(0); }

  //
  // before any marshalers can be obtained, they must be allocated:
  void dpAllocateMarshalers(int numof);
  int getMNum() { return (musNum); }

  // 
  DPMarshaler* dpGetMarshaler();
  Builder* dpGetUnmarshaler();
  void dpReturnMarshaler(DPMarshaler* dpm);
  void dpReturnUnmarshaler(Builder* dpb);
};

//
extern DPMarshaler **dpms;
extern Builder **dpbs;


#endif
