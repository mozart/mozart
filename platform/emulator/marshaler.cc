/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *
 *  Contributors:
 *    Denys Duchier (duchier@ps.uni-sb.de)
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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifdef INTERFACE
#pragma implementation "marshaler.hh"
#endif

#include "wsock.hh"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include "runtime.hh"
#include "codearea.hh"
#include "indexing.hh"

#include "perdio.hh"
#include "perdio_debug.hh"
#include "genvar.hh"
#include "perdiovar.hh"
#include "gc.hh"
#include "dictionary.hh"
#include "urlc.hh"
#include "marshaler.hh"
#include "comm.hh"
#include "msgbuffer.hh"

/* *****************************************************************************
                       ORGANIZATION


            1  forward declarations
            2  global variables
            3  utility routines
            4  class RefTable RefTrail
            5  unmarshalHeader
            6  simple ground marshaling/unmarshaling
            7  gname marshaling/unmarshaling
            8  url marshaling/unmarshaling
            9  marshaling routines
            10 ConstTerm and term marshaling
            11 unmarshaling routines
            12 term unmarshaling
            13 perdiovar special
            14 full object/class marshaling/unmarshaling
            15 statistics
            16 initialization
            17 code marshaling
            18 message marshaling

***************************************************************************** */

/* *********************************************************************/
/*   SECTION 1: forward declarations                                  */
/* *********************************************************************/

OZ_Term unmarshalTerm(MsgBuffer *);
void unmarshalUnsentTerm(MsgBuffer *);
void marshalTerm(OZ_Term, MsgBuffer *);
void marshalCode(ProgramCounter,MsgBuffer*);
ProgramCounter unmarshalCode(MsgBuffer*,Bool);
void marshalVariable(PerdioVar *, MsgBuffer *);
SRecord *unmarshalSRecord(MsgBuffer *);
void unmarshalUnsentSRecord(MsgBuffer *);
void unmarshalTerm(MsgBuffer *, OZ_Term *);

/* *********************************************************************/
/*   SECTION 2: global variables                                       */
/* *********************************************************************/

SendRecvCounter dif_counter[DIF_LAST];
SendRecvCounter misc_counter[MISC_LAST];

char *misc_names[MISC_LAST] = {
  "string",
  "gname",
  "site"
};

char *dif_names[DIF_LAST] = {
  "smallint",
  "bigint",
  "float",
  "atom",
  "name",
  "uniquename",
  "record",
  "tuple",
  "list",
  "ref",
  "owner",
  "owner_sec",
  "port",
  "cell",
  "lock",
  "var",
  "builtin",
  "dict",
  "object",
  "thread",
  "space",
  "chunk",
  "proc",
  "class",
  "url",
  "array",
  "fsetvalue",
  "newname",
  "abstractionentry",
  "primary",
  "secondary",
  "remote",
  "virtual",
  "perm",
  "passive"
};

/* *********************************************************************/
/*   SECTION 3: utility routines                                       */
/* *********************************************************************/

void marshalDIF(MsgBuffer *bs, MarshalTag tag) {
  dif_counter[tag].send();
  bs->put(tag);
}

/* *********************************************************************/
/*   SECTION 4: classes RefTable RefTrail                              */
/* *********************************************************************/

class RefTable {
  OZ_Term *array;
  int size;
  int pos;
public:
  RefTable()
  {
    pos = 0;
    size = 100;
    array = new OZ_Term[size];
  }
  void reset() { pos=0; }
  OZ_Term get(int i)
  {
    Assert(i<size);
    return array[i];
  }
  int set(OZ_Term val)
  {
    if (pos>=size)
      resize(pos);
    array[pos] = val;
    return pos++;
  }
  void resize(int newsize)
  {
    int oldsize = size;
    OZ_Term  *oldarray = array;
    while(size <= newsize) {
      size = (size*3)/2;
    }
    array = new OZ_Term[size];
    for (int i=0; i<oldsize; i++) {
      array[i] = oldarray[i];
    }
    delete oldarray;
  }
  DebugCode(int getPos() { return pos; })
};

RefTable *refTable;

inline void gotRef(MsgBuffer *bs, TaggedRef val)
{
#define XXRS_HACK
#ifdef RS_HACK
  int n1 = unmarshalNumber(bs);
  int n2 = unmarshalNumber(bs);
  int n = unmarshalNumber(bs);
  Assert(n1==27);
#endif
  int counter = refTable->set(val);
  PD((REF_COUNTER,"got: %d",counter));
#ifdef RS_HACK
  Assert(n==counter);
#endif
}

class RefTrail: public Stack {
  int counter;
public:
  RefTrail() : Stack(200,Stack_WithMalloc) { counter=0; }
  int trail(OZ_Term *t)
  {
    push(t);
    push(ToPointer(*t));
    return counter++;
  }
  void unwind()
  {
    while(!isEmpty()) {
      OZ_Term oldval = ToInt32(pop());
      OZ_Term *loc = (OZ_Term*) pop();
      *loc = oldval;
      counter--;
    }
    Assert(counter==0);
  }
};

RefTrail *refTrail;

/* *********************************************************************/
/*   SECTION 5: unmarshalHeader                                        */
/* *********************************************************************/

MessageType unmarshalHeader(MsgBuffer *bs){
  bs->unmarshalBegin();
  refTable->reset();
  MessageType mt= (MessageType) bs->get();
  mess_counter[mt].recv();
  return mt;}

/* *********************************************************************/
/*   SECTION 6:  simple ground marshaling/unmarshaling                 */
/* *********************************************************************/

#define SBit (1<<7)

void marshalNumber(unsigned int i, MsgBuffer *bs)
{
  while(i >= SBit) {
    bs->put((i%SBit)|SBit);
    i /= SBit;}
  bs->put(i);
}

int unmarshalNumber(MsgBuffer *bs)
{
  unsigned int ret = 0, shft = 0;
  unsigned int c = bs->get();
  while (c >= SBit) {
    ret += ((c-SBit) << shft);
    c = bs->get();
    shft += 7;
  }
  ret |= (c<<shft);
  return (int) ret;
}

int unmarshalUnsentNumber(MsgBuffer *bs) // ATTENTION
{
  unsigned int ret = 0, shft = 0;
  unsigned int c = bs->get();
  while (c >= SBit) {
    ret += ((c-SBit) << shft);
    c = bs->get();
    shft += 7;
  }
  ret |= (c<<shft);
  return (int) ret;
}

#undef SBit


const int shortSize = 2;

void marshalShort(unsigned short i, MsgBuffer *bs){
  PD((MARSHAL_CT,"Short %d BYTES:2",i));
  for (int k=0; k<shortSize; k++) {
    bs->put(i&0xFF);
    i = i>>8;}}

unsigned short unmarshalShort(MsgBuffer *bs){
  unsigned short sh;
  unsigned int i1 = bs->get();
  unsigned int i2 = bs->get();
  sh= (i1 + (i2<<8));
  PD((UNMARSHAL_CT,"Short %d BYTES:2",sh));
  return sh;}

class DoubleConv {
public:
  union {
    unsigned char c[sizeof(double)];
    int i[sizeof(double)/sizeof(int)];
    double d;
  } u;
};

Bool isLowEndian()
{
  DoubleConv dc;
  dc.u.i[0] = 1;
  return dc.u.c[0] == 1;
}

const Bool lowendian = isLowEndian();

void marshalFloat(double d, MsgBuffer *bs)
{
  static DoubleConv dc;
  dc.u.d = d;
  if (lowendian) {
    marshalNumber(dc.u.i[0],bs);
    marshalNumber(dc.u.i[1],bs);
  } else {
    marshalNumber(dc.u.i[1],bs);
    marshalNumber(dc.u.i[0],bs);
  }
}

double unmarshalFloat(MsgBuffer *bs)
{
  static DoubleConv dc;
  if (lowendian) {
    dc.u.i[0] = unmarshalNumber(bs);
    dc.u.i[1] = unmarshalNumber(bs);
  } else {
    dc.u.i[1] = unmarshalNumber(bs);
    dc.u.i[0] = unmarshalNumber(bs);
  }
  return dc.u.d;
}

void marshalString(const char *s, MsgBuffer *bs)
{
  misc_counter[MISC_STRING].send();
  marshalNumber(strlen(s),bs);
  PD((MARSHAL_CT,"String BYTES:%d",strlen(s)));
  while(*s) {
    bs->put(*s);
    s++;  }
}

char *unmarshalString(MsgBuffer *bs)
{
  misc_counter[MISC_STRING].recv();
  int i = unmarshalNumber(bs);

  char *ret = new char[i+1];
  int k=0;
  for (; k<i; k++) {
    ret[k] = bs->get();
  }
  PD((UNMARSHAL_CT,"String BYTES:%d",k));
  ret[i] = '\0';
  return ret;
}

void unmarshalUnsentString(MsgBuffer *bs)
{
  misc_counter[MISC_STRING].recv();
  int i = unmarshalNumber(bs);

  int k=0;
  for (; k<i; k++) {bs->get();}
  PD((UNMARSHAL_CT,"String BYTES:%d",k));
}



/* *********************************************************************/
/*   SECTION 7: gname marshaling/unmarshaling                          */
/* *********************************************************************/

void marshalGName(GName *gname, MsgBuffer *bs)
{
  misc_counter[MISC_GNAME].send();
  PD((MARSHAL,"gname: s:%s", gname->site->stringrep()));
  gname->site->marshalPSite(bs);
  for (int i=0; i<fatIntDigits; i++) {
    PD((MARSHAL,"gname: id%d:%u", i,gname->id.number[i]));
    marshalNumber(gname->id.number[i],bs);
  }
  marshalNumber((int)gname->gnameType,bs);
}

void unmarshalGName1(GName *gname, MsgBuffer *bs)
{
  gname->site=unmarshalPSite(bs);
  PD((UNMARSHAL,"gname: s:%s", gname->site->stringrep()));
  for (int i=0; i<fatIntDigits; i++) {
    gname->id.number[i] = unmarshalNumber(bs);
    PD((MARSHAL,"gname: id%d:%u", i, gname->id.number[i]));
  }
  gname->gnameType = (GNameType) unmarshalNumber(bs);
  PD((UNMARSHAL,"gname finish type:%d", gname->gnameType));
}

GName *unmarshalGName(TaggedRef *ret, MsgBuffer *bs)
{
  PD((UNMARSHAL,"gname"));
  misc_counter[MISC_GNAME].recv();
  GName gname;
  unmarshalGName1(&gname,bs);

  TaggedRef aux = findGNameOutline(&gname);
  if (aux) {
    if (ret) *ret = aux; // ATTENTION
    return 0;
  }
  return new GName(gname);
}

/* *********************************************************************/
/*   SECTION 9: term marshaling routines                               */
/* *********************************************************************/

inline Bool checkCycle(OZ_Term t, MsgBuffer *bs)
{
  if (!IsRef(t) && _tagTypeOf(t)==GCTAG) {
    PD((MARSHAL,"circular: %d",t>>tagSize));
    marshalDIF(bs,DIF_REF);
    marshalNumber(t>>tagSize,bs);
    return OK;
  }
  return NO;
}

inline void trailCycle(OZ_Term *t, MsgBuffer *bs,int n)
{
  int counter = refTrail->trail(t);
  PD((REF_COUNTER,"trail: %d",counter));
  *t = ((counter)<<tagSize)|GCTAG;
#ifdef RS_HACK
  marshalNumber(27,bs);
  marshalNumber(n,bs);
  marshalNumber(counter,bs);
#endif
}

void marshalClosure(Abstraction *a,MsgBuffer *bs) {
  PD((MARSHAL,"closure"));
  RefsArray globals = a->getGRegs();
  int gs = globals ? a->getGSize() : 0;
  marshalNumber(gs,bs);
  for (int i=0; i<gs; i++) {
    marshalTerm(globals[i],bs);
  }}

void marshalSRecord(SRecord *sr, MsgBuffer *bs)
{
  TaggedRef t = nil();
  if (sr) {
    t = makeTaggedSRecord(sr);
  }
  marshalTerm(t,bs);
}

void marshalClass(ObjectClass *cl, MsgBuffer *bs)
{
  marshalDIF(bs,DIF_CLASS);
  marshalGName(cl->getGName(),bs);
  trailCycle(cl->getRef(),bs,2);
  marshalSRecord(cl->getFeatures(),bs);
}

void marshalDict(OzDictionary *d, MsgBuffer *bs)
{
  if (!d->isSafeDict()) {
    warning("Marshaling unsafe dictionary, will expire soon!!\n");
  }
  int size = d->getSize();
  marshalNumber(size,bs);
  trailCycle(d->getRef(),bs,3);

  int i = d->getFirst();
  i = d->getNext(i);
  while(i>=0) {
    marshalTerm(d->getKey(i),bs);
    marshalTerm(d->getValue(i),bs);
    i = d->getNext(i);
    size--;
  }
  Assert(size==0);
}

void marshalObject(Object *o, MsgBuffer *bs, GName *gnclass)
{
  if (marshalTertiary(o,DIF_OBJECT,bs)) return;   /* ATTENTION */
  trailCycle(o->getRef(),bs,111);
  marshalGName(gnclass,bs);
}

/* *********************************************************************/
/*   SECTION 10: ConstTerm and Term marshaling                          */
/* *********************************************************************/

void marshalConst(ConstTerm *t, MsgBuffer *bs)
{
  switch (t->getType()) {
  case Co_Dictionary:
    {
      PD((MARSHAL,"dictionary"));
      marshalDIF(bs,DIF_DICT);
      //      addRes(bs,makeTaggedConst(t));
      marshalDict((OzDictionary *) t,bs);
      return;
    }
  case Co_Array:
    {
      PD((MARSHAL,"array"));
      marshalDIF(bs,DIF_ARRAY);
      bs->addRes(makeTaggedConst(t));
      // mm2
      warning("marshal array not impl");
      return;
    }
  case Co_Builtin:
    {
      PD((MARSHAL,"builtin"));
      marshalDIF(bs,DIF_BUILTIN);
      PD((MARSHAL_CT,"tag DIF_BUILTIN BYTES:1"));
      marshalString(((BuiltinTabEntry *)t)->getPrintName(),bs);
      break;
    }
  case Co_Chunk:
    {
      PD((MARSHAL,"chunk"));
      SChunk *ch=(SChunk *) t;
      GName *gname=ch->getGName();
      marshalDIF(bs,DIF_CHUNK);
      marshalGName(gname,bs);
      trailCycle(t->getRef(),bs,4);
      marshalTerm(ch->getValue(),bs);
      return;
    }
  case Co_Class:
    {
      PD((MARSHAL,"class"));
      ObjectClass *cl = (ObjectClass*) t;
      cl->globalize();
      marshalClass(cl,bs);
      return;
    }
  case Co_Abstraction:
    {
      PD((MARSHAL,"abstraction"));
      Abstraction *pp=(Abstraction *) t;
      GName *gname=pp->getGName();

      marshalDIF(bs,DIF_PROC);
      marshalGName(gname,bs);
      TaggedRef names = pp->getPred()->getNames();
      Bool hasNames = !literalEq(names,NameUnit);
      Assert(hasNames==OK || hasNames==NO);
      marshalNumber(hasNames,bs);

      bs->marshaledProcHasNames(names); // change by Per
      marshalTerm(pp->getName(),bs);
      marshalNumber(pp->getArity(),bs);
      ProgramCounter pc = pp->getPC();
      trailCycle(t->getRef(),bs,5);
      marshalClosure(pp,bs);
      PD((MARSHAL,"code begin"));
      marshalCode(pc,bs);
      PD((MARSHAL,"code end"));
      return;
    }

  case Co_Object:
    {
      PD((MARSHAL,"object"));
      Object *o = (Object*) t;
      ObjectClass *oc = o->getClass();
      oc->globalize();
      bs->addRes(makeTaggedConst(t));
      marshalObject(o,bs,oc->getGName());
      return;
    }
  case Co_Lock:
    PD((MARSHAL,"lock"));
    bs->addRes(makeTaggedConst(t));
    if (marshalTertiary((Tertiary *) t,DIF_LOCK,bs)) return;
    break;
  case Co_Thread:
    PD((MARSHAL,"thread"));
    bs->addRes(makeTaggedConst(t));
    if (marshalTertiary((Tertiary *) t,DIF_THREAD,bs)) return;
    break;
  case Co_Space:
    PD((MARSHAL,"space"));
    bs->addRes(makeTaggedConst(t));
    if (marshalTertiary((Tertiary *) t,DIF_SPACE,bs)) return;
    break;
  case Co_Cell:
    PD((MARSHAL,"cell"));
    bs->addRes(makeTaggedConst(t));
    if (marshalTertiary((Tertiary *) t,DIF_CELL,bs)) return;
    break;
  case Co_Port:
    PD((MARSHAL,"port"));
    bs->addRes(makeTaggedConst(t));
    if (marshalTertiary((Tertiary *) t,DIF_PORT,bs)) return;
    break;
  default:
    error("marshalConst(%d) not impl",t->getType());
  }

  trailCycle(t->getRef(),bs,6);
}

void marshalTerm(OZ_Term t, MsgBuffer *bs)
{
loop:
  DEREF(t,tPtr,tTag);
  switch(tTag) {

  case GCTAG: {
    PD((MARSHAL,"gctag for cycle"));
    Bool b = checkCycle(t,bs);
    Assert(b);
    break;}

  case SMALLINT:
    PD((MARSHAL,"small int: %d",smallIntValue(t)));
    marshalDIF(bs,DIF_SMALLINT);
    marshalNumber(smallIntValue(t),bs);
    break;

  case OZFLOAT:
    PD((MARSHAL,"float"));
    marshalDIF(bs,DIF_FLOAT);
    marshalFloat(tagged2Float(t)->getValue(),bs);
    break;

  case BIGINT:
    PD((MARSHAL,"bigint"));
    marshalDIF(bs,DIF_BIGINT);
    marshalString(toC(t),bs);
    break;

  case LITERAL:
    {
      PD((MARSHAL,"literal"));
      Literal *lit = tagged2Literal(t);
      if (checkCycle(*lit->getRef(),bs)) return;

      if (lit->isAtom()) {
        marshalDIF(bs,DIF_ATOM);
        PD((MARSHAL_CT,"tag DIF_ATOM  BYTES:1"));
        marshalString(lit->getPrintName(),bs);
        PD((MARSHAL,"atom: %s",lit->getPrintName()));
      } else if (lit->isUniqueName()) {
        marshalDIF(bs,DIF_UNIQUENAME);
        marshalString(lit->getPrintName(),bs);
        PD((MARSHAL,"unique name: %s",lit->getPrintName()));
      } else {
        if(bs->knownAsNewName(t)){ // change by Per
          marshalDIF(bs,DIF_NEWNAME);
        } else {
          marshalDIF(bs,DIF_NAME);
          GName *gname = ((Name*)lit)->globalize();
          marshalGName(gname,bs);
          marshalString(lit->getPrintName(),bs);
          PD((MARSHAL,"name: %s",lit->getPrintName()));
        }
      }
      trailCycle(lit->getRef(),bs,7);
      break;
    }

  case LTUPLE:
    {
      PD((MARSHAL,"ltuple"));
      LTuple *l = tagged2LTuple(t);
      if (checkCycle(*l->getRef(),bs)) return;
      marshalDIF(bs,DIF_LIST);
      PD((MARSHAL_CT,"tag DIF_LIST BYTES:1"));
      PD((MARSHAL,"list"));

      TaggedRef *args = l->getRef();
      if (!isRef(*args) && isAnyVar(*args)) {
        PerdioVar *pvar = var2PerdioVar(args);
        trailCycle(args,bs,8);
        marshalVariable(pvar,bs);
      } else {
        OZ_Term head = l->getHead();
        trailCycle(args,bs,9);
        marshalTerm(head,bs);
      }
      // tail recursion optimization
      t = l->getTail();
      goto loop;
    }

  case SRECORD:
    {
      PD((MARSHAL,"srecord"));
      SRecord *rec = tagged2SRecord(t);
      if (checkCycle(*rec->getCycleAddr(),bs)) return;
      TaggedRef label = rec->getLabel();

      if (rec->isTuple()) {
        marshalDIF(bs,DIF_TUPLE);
        PD((MARSHAL_CT,"tag DIF_TUPLE BYTES:1"));
        marshalNumber(rec->getTupleWidth(),bs);
      } else {
        marshalDIF(bs,DIF_RECORD);
        PD((MARSHAL_CT,"tag DIF_RECORD BYTES:1"));
        marshalTerm(rec->getArityList(),bs);
      }
      marshalTerm(label,bs);
      trailCycle(rec->getCycleAddr(),bs,10);
      int argno = rec->getWidth();
      PD((MARSHAL,"record-tuple no:%d",argno));

      for(int i=0; i<argno-1; i++) {
        marshalTerm(rec->getArg(i),bs);
      }
      // tail recursion optimization
      t = rec->getArg(argno-1);
      goto loop;
    }

  case OZCONST:
    {
      PD((MARSHAL,"constterm"));
      if (checkCycle(*(tagged2Const(t)->getRef()),bs))
        break;
      marshalConst(tagged2Const(t),bs);
      break;
    }

  case FSETVALUE:
    {
      PD((MARSHAL,"finite set value"));
      OZ_FSetValue * fsetval = tagged2FSetValue(t);
      marshalDIF(bs,DIF_FSETVALUE);
      // tail recursion optimization
      t = fsetval->getKnownInList();
      goto loop;
    }

  case UVAR:
  case SVAR:
  case CVAR:
    {
      PerdioVar *pvar = var2PerdioVar(tPtr);
      if (pvar==NULL) {
        t = makeTaggedRef(tPtr);
        goto bomb;
      }
      bs->addRes(makeTaggedRef(tPtr));
      marshalVariable(pvar,bs);
      break;
    }

  default:
  bomb:
    warning("Cannot marshal %s",toC(t));
    marshalTerm(nil(),bs);
    break;
  }

  return;
}

/* *********************************************************************/
/*   SECTION 11: term unmarshaling routines                            */
/* *********************************************************************/

void unmarshalDict(MsgBuffer *bs, TaggedRef *ret)
{
  int size = unmarshalNumber(bs);
  PD((UNMARSHAL,"dict size:%d",size));
  Assert(am.onToplevel());
  OzDictionary *aux = new OzDictionary(am.currentBoard(),size);
  aux->markSafe();
  *ret = makeTaggedConst(aux);
  gotRef(bs,*ret);

  while(size-- > 0) {
    TaggedRef key = unmarshalTerm(bs);
    TaggedRef val = unmarshalTerm(bs);
    aux->setArg(key,val);
  }
  return;
}

void unmarshalObject(ObjectFields *o, MsgBuffer *bs){
  o->feat = unmarshalSRecord(bs);
  o->state=unmarshalTerm(bs);
  o->lock=unmarshalTerm(bs);}

void fillInObject(ObjectFields *of, Object *o){
  o->setFreeRecord(of->feat);
  o->setState(tagged2Tert(of->state));
  o->setLock(isNil(of->lock) ? (LockProxy*)NULL : (LockProxy*)tagged2Tert(of->lock));}

void unmarshalUnsentObject(MsgBuffer *bs){
  unmarshalUnsentSRecord(bs);
  unmarshalUnsentTerm(bs);
  unmarshalUnsentTerm(bs);}

void unmarshalObjectAndClass(ObjectFields *o, MsgBuffer *bs){
  unmarshalObject(o,bs);
  o->clas = unmarshalTerm(bs);}

void unmarshalUnsentObjectAndClass(MsgBuffer *bs){
  unmarshalUnsentObject(bs);
  unmarshalUnsentTerm(bs);}

void fillInObjectAndClass(ObjectFields *of, Object *o){
  fillInObject(of,o);
  o->setClass(tagged2ObjectClass(of->clas));}

void unmarshalClass(ObjectClass *cl, MsgBuffer *bs)
{
  SRecord *feat = unmarshalSRecord(bs);

  if (cl==NULL)  return;

  TaggedRef ff = feat->getFeature(NameOoUnFreeFeat);
  Bool locking = literalEq(NameTrue,deref(feat->getFeature(NameOoLocking)));

  cl->import(feat,
             tagged2Dictionary(feat->getFeature(NameOoFastMeth)),
             isSRecord(ff) ? tagged2SRecord(ff) : (SRecord*)NULL,
             tagged2Dictionary(feat->getFeature(NameOoDefaults)),
             locking);
}

RefsArray unmarshalClosure(MsgBuffer *bs) {
  int gsize = unmarshalNumber(bs);
  RefsArray globals = gsize==0 ? 0 : allocateRefsArray(gsize);

  for (int i=0; i<gsize; i++) {
    globals[i] = unmarshalTerm(bs);
  }
  return globals;
}

OZ_Term unmarshalTerm(MsgBuffer *bs)
{
  OZ_Term ret;
  unmarshalTerm(bs,&ret);
  return ret;
}

inline
ObjectClass *newClass(GName *gname) {
  Assert(am.onToplevel());
  ObjectClass *ret = new ObjectClass(NULL,NULL,NULL,NULL,NO,am.currentBoard());
  ret->setGName(gname);
  return ret;
}

SRecord *unmarshalSRecord(MsgBuffer *bs){
  TaggedRef t = unmarshalTerm(bs);
  return isNil(t) ? (SRecord*)NULL : tagged2SRecord(t);
}

void unmarshalUnsentSRecord(MsgBuffer *bs){
  unmarshalUnsentTerm(bs);}

/* *********************************************************************/
/*   SECTION 12: term unmarshaling                                     */
/* *********************************************************************/

void unmarshalTerm(MsgBuffer *bs, OZ_Term *ret)
{
loop:
  MarshalTag tag = (MarshalTag) bs->get();
  PD((UNMARSHAL,"term tag:%s",dif_names[(int) tag]));

  dif_counter[tag].recv();
  switch(tag) {

  case DIF_SMALLINT:
    *ret = OZ_int(unmarshalNumber(bs));
    PD((UNMARSHAL,"small int %d",smallIntValue(*ret)));
    return;

  case DIF_FLOAT:
    *ret = OZ_float(unmarshalFloat(bs));
    PD((UNMARSHAL,"float"));
    return;

  case DIF_NEWNAME:
    {
      Assert(am.onToplevel());
      *ret = makeTaggedLiteral(Name::newName(am.currentBoard()));
      PD((UNMARSHAL,"newName"));
      gotRef(bs,*ret);
      return;
    }

  case DIF_NAME:
    {
      GName *gname    = unmarshalGName(ret,bs);
      char *printname = unmarshalString(bs);

      PD((UNMARSHAL,"name %s",printname));

      if (gname) {
        Name *aux;
        if (strcmp("",printname)==0) {
          aux = Name::newName(am.currentBoard());
        } else {
          aux = NamedName::newNamedName(ozstrdup(printname));
        }
        aux->import(gname);
        *ret = makeTaggedLiteral(aux);
        addGName(gname,*ret);
      }
      delete printname;
      gotRef(bs,*ret);
      return;
    }

  case DIF_UNIQUENAME:
    {
      char *printname = unmarshalString(bs);

      PD((UNMARSHAL,"unique name %s",printname));

      *ret = getUniqueName(printname);
      delete printname;
      gotRef(bs,*ret);
      return;
    }

  case DIF_ATOM:
    {
      char *aux = unmarshalString(bs);
      PD((UNMARSHAL,"atom %s",aux));
      *ret = OZ_atom(aux);
      delete aux;
      gotRef(bs,*ret);
      return;
    }

  case DIF_BIGINT:
    {
      char *aux = unmarshalString(bs);
      PD((UNMARSHAL,"big int %s",aux));
      *ret = OZ_CStringToNumber(aux);
      delete aux;
      return;
    }

  case DIF_LIST:
    {
      PD((UNMARSHAL,"list"));
      LTuple *l = new LTuple();
      *ret = makeTaggedLTuple(l);
      gotRef(bs,*ret);
      unmarshalTerm(bs,l->getRefHead());
      // tail recursion optimization
      ret = l->getRefTail();
      goto loop;
    }
  case DIF_TUPLE:
    {
      int argno = unmarshalNumber(bs);
      PD((UNMARSHAL,"tuple no_args:%d",argno));
      TaggedRef label = unmarshalTerm(bs);
      SRecord *rec = SRecord::newSRecord(label,argno);
      *ret = makeTaggedSRecord(rec);
      gotRef(bs,*ret);

      for(int i=0; i<argno-1; i++) {
        unmarshalTerm(bs,rec->getRef(i));
      }
      // tail recursion optimization
      ret = rec->getRef(argno-1);
      goto loop;
    }

  case DIF_RECORD:
    {
      TaggedRef arity = unmarshalTerm(bs);
      TaggedRef sortedarity = arity;
      if (!isSorted(arity)) {
        int len;
        TaggedRef aux = duplist(arity,len);
        sortedarity = sortlist(aux,len);
      }
      PD((UNMARSHAL,"record no:%d",fastlength(arity)));
      TaggedRef label = unmarshalTerm(bs);
      SRecord *rec    = SRecord::newSRecord(label,aritytable.find(sortedarity));
      *ret = makeTaggedSRecord(rec);
      gotRef(bs,*ret);

      while(isLTuple(arity)) {
        TaggedRef val = unmarshalTerm(bs);
        rec->setFeature(head(arity),val);
        arity = tail(arity);
      }
      return;
    }

  case DIF_REF:
    {
      int i = unmarshalNumber(bs);
      PD((UNMARSHAL,"ref: %d",i));
      *ret = refTable->get(i);
      Assert(*ret);
      return;
    }

  case DIF_OWNER:
  case DIF_OWNER_SEC:
    {
      *ret=unmarshalOwner(bs,tag);
      return;
    }
  case DIF_PORT:
  case DIF_THREAD:
  case DIF_SPACE:
  case DIF_CELL:
  case DIF_LOCK:
  case DIF_OBJECT:
    {
      *ret=unmarshalTertiary(bs,tag);
      gotRef(bs,*ret);
      return;}

  case DIF_URL:
    {
      warning("support no longer supported");
      *ret = oz_newVariable();
      return;
    }
  case DIF_CHUNK:
    {
      PD((UNMARSHAL,"chunk"));

      GName *gname=unmarshalGName(ret,bs);

      SChunk *sc;
      if (gname) {
        Assert(am.onToplevel());
        sc=new SChunk(am.currentBoard(),0);
        sc->setGName(gname);
        *ret = makeTaggedConst(sc);
        addGName(gname,*ret);
      } else if (!isSChunk(deref(*ret))) {
        // mm2: share the follwing code DIF_CHUNK, DIF_CLASS, DIF_PROC!
        DEREF(*ret,chPtr,_1);
        PerdioVar *pv;
        if (!isPerdioVar(*ret)) {
          // mm2
          warning("chunk gname mismatch");
          return;
        }
        Assert(am.onToplevel());
        sc=new SChunk(am.currentBoard(),0);
        sc->setGName(pv->getGName());
        *ret=makeTaggedConst(sc);
        SiteUnifyCannotFail(makeTaggedRef(chPtr),*ret);
        // pv->primBind(chPtr,*ret);
      } else {
        sc = 0;
      }
      gotRef(bs,*ret);
      TaggedRef value = unmarshalTerm(bs);
      if (sc) sc->import(value);
      return;
    }

  case DIF_CLASS:
    {
      PD((UNMARSHAL,"class"));

      GName *gname=unmarshalGName(ret,bs);

      ObjectClass *cl;
      if (gname) {
        cl = newClass(gname);
        *ret = makeTaggedConst(cl);
        addGName(gname,*ret);
      } else if (!isClass(deref(*ret))) {
        DEREF(*ret,chPtr,_1);
        PerdioVar *pv;
        if (!isPerdioVar(*ret)) {
          // mm2
          warning("class gname mismatch");
          return;
        }
        cl = newClass(pv->getGName());
        *ret = makeTaggedConst(cl);
        SiteUnifyCannotFail(makeTaggedRef(chPtr),*ret);
        // pv->primBind(chPtr,*ret);
      } else {
        cl = 0;
      }
      gotRef(bs,*ret);
      unmarshalClass(cl,bs);
      return;
    }

  case DIF_VAR:
    {
      *ret=unmarshalVar(bs);
      return;
    }

  case DIF_PROC:
    {
      PD((UNMARSHAL,"proc"));

      GName *gname  = unmarshalGName(ret,bs);
      Bool hasNames = unmarshalNumber(bs);
      Assert(hasNames==NO || hasNames==OK);
      OZ_Term name  = unmarshalTerm(bs);
      int arity     = unmarshalNumber(bs);

      Abstraction *pp;
      if (gname || hasNames) {
        PrTabEntry *pr = new PrTabEntry(name,mkTupleWidth(arity),AtomNil,0);
        Assert(am.onToplevel());
        pp = new Abstraction(pr,0,am.currentBoard());
        *ret = makeTaggedConst(pp);
        if (!hasNames) {
          pp->setGName(gname);
          addGName(gname,*ret);
        }
      } else if (!isAbstraction(deref(*ret))) {
        DEREF(*ret,chPtr,_1);
        PerdioVar *pv;
        if (!isPerdioVar(*ret)) {
          // mm2
          warning("unmarshal proc gname mismatch: %s",toC(makeTaggedRef(ret)));
          return;
        }
        PrTabEntry *pr=new PrTabEntry(name,mkTupleWidth(arity),AtomNil,0);
        Assert(am.onToplevel());
        pp=new Abstraction(pr,0,am.currentBoard());
        pp->setGName(pv->getGName());
        *ret = makeTaggedConst(pp);
        SiteUnifyCannotFail(makeTaggedRef(chPtr),*ret);
        // pv->primBind(chPtr,*ret);
      } else {
        pp=0;
      }

      gotRef(bs,*ret);

      RefsArray globals = unmarshalClosure(bs);
      if (pp) {
        PD((UNMARSHAL,"code begin"));
        pp->import(globals,unmarshalCode(bs,NO));
        pp->getPred()->patchFileAndLine();
        PD((UNMARSHAL,"code end"));
      } else {
        PD((UNMARSHAL,"code begin"));
        (void) unmarshalCode(bs,OK);
        PD((UNMARSHAL,"code end"));
      }
      return;
    }

  case DIF_DICT:
    {
      PD((UNMARSHAL,"dict"));
      unmarshalDict(bs,ret);
      return;
    }
  case DIF_ARRAY:
    {
      PD((UNMARSHAL,"array"));
      warning("mm2: array not impl");
      return;
    }
  case DIF_BUILTIN:
    {
      char *name = unmarshalString(bs); // ATTENTION deletion
      PD((UNMARSHAL,"builtin: %s",name));
      BuiltinTabEntry *found = builtinTab.find(name);

      if (found == htEmpty) {
        warning("Builtin '%s' not in table.", name);
        *ret = nil();
        return;
      }

      *ret = makeTaggedConst(found);
      gotRef(bs,*ret);
      return;
    }

  case DIF_FSETVALUE:
    {
      PD((UNMARSHAL,"finite set value"));
      OZ_Term glb=unmarshalTerm(bs);
      extern void makeFSetValue(OZ_Term,OZ_Term*);
      makeFSetValue(glb,ret);
      return;
    }

  default:
    printf("unmarshal: unexpected tag: %d\n",tag);
    Assert(0);
    *ret = nil();
    return;
  }

  Assert(0);
}

void unmarshalUnsentTerm(MsgBuffer *bs) {
  OZ_Term t=unmarshalTerm(bs);}

/* *********************************************************************/
/*   SECTION 13: perdiovar - special                                  */
/* *********************************************************************/

void marshalVariable(PerdioVar *pvar, MsgBuffer *bs)
{
  if((pvar->isProxy()) || pvar->isManager()) {
    marshalVar(pvar,bs);
    return;}

  if (pvar->isObject()) {
    PD((MARSHAL,"var objectproxy"));
    if (checkCycle(*(pvar->getObject()->getRef()),bs))
      return;
    marshalObject(pvar->getObject(),bs,pvar->getClass()->getGName());
    return;
  }

  Assert(pvar->isObjectGName());

  PD((MARSHAL,"var objectproxy"));
  marshalObject(pvar->getObject(),bs,pvar->getGNameClass());
  return;
}

/* *********************************************************************/
/*   SECTION 14: full object/class unmarshaling/marshaling             */
/* *********************************************************************/

void marshalFullObject(Object *o,MsgBuffer* bs){
  PD((MARSHAL,"full object"));
  marshalSRecord(o->getFreeRecord(),bs);
  marshalTerm(makeTaggedConst(getCell(o->getState())),bs);
  if (o->getLock()) {marshalTerm(makeTaggedConst(o->getLock()),bs);}
  else {marshalTerm(nil(),bs);}}

void marshalFullObjectAndClass(Object *o,MsgBuffer* bs){
  PD((MARSHAL,"full object and class"));
  ObjectClass *oc=o->getClass();
  marshalFullObject(o,bs);
  marshalClass(oc,bs);}

/* *********************************************************************/
/*   SECTION 15: statistics                                            */
/* *********************************************************************/

OZ_C_proc_begin(BIperdioStatistics,1)
{
  OZ_declareArg(0,out);

  OZ_Term dif_send_ar=oz_nil();
  OZ_Term dif_recv_ar=oz_nil();
  int i;
  for (i=0; i<DIF_LAST; i++) {
    dif_send_ar=oz_cons(oz_pairAI(dif_names[i],dif_counter[i].getSend()),
                        dif_send_ar);
    dif_recv_ar=oz_cons(oz_pairAI(dif_names[i],dif_counter[i].getRecv()),
                        dif_recv_ar);
  }
  OZ_Term dif_send=OZ_recordInit(oz_atom("dif"),dif_send_ar);
  OZ_Term dif_recv=OZ_recordInit(oz_atom("dif"),dif_recv_ar);

  OZ_Term misc_send_ar=oz_nil();
  OZ_Term misc_recv_ar=oz_nil();
  for (i=0; i<MISC_LAST; i++) {
    misc_send_ar=oz_cons(oz_pairAI(misc_names[i],misc_counter[i].getSend()),
                         misc_send_ar);
    misc_recv_ar=oz_cons(oz_pairAI(misc_names[i],misc_counter[i].getRecv()),
                         misc_recv_ar);
  }
  OZ_Term misc_send=OZ_recordInit(oz_atom("misc"),misc_send_ar);
  OZ_Term misc_recv=OZ_recordInit(oz_atom("misc"),misc_recv_ar);

  OZ_Term mess_send_ar=oz_nil();
  OZ_Term mess_recv_ar=oz_nil();
  for (i=0; i<M_LAST; i++) {
    mess_send_ar=oz_cons(oz_pairAI(mess_names[i],mess_counter[i].getSend()),
                         mess_send_ar);
    mess_recv_ar=oz_cons(oz_pairAI(mess_names[i],mess_counter[i].getRecv()),
                         mess_recv_ar);
  }
  OZ_Term mess_send=OZ_recordInit(oz_atom("messages"),mess_send_ar);
  OZ_Term mess_recv=OZ_recordInit(oz_atom("messages"),mess_recv_ar);


  OZ_Term send_ar=oz_nil();
  send_ar = oz_cons(oz_pairA("dif",dif_send),send_ar);
  send_ar = oz_cons(oz_pairA("misc",misc_send),send_ar);
  send_ar = oz_cons(oz_pairA("messages",mess_send),send_ar);
  OZ_Term send=OZ_recordInit(oz_atom("send"),send_ar);

  OZ_Term recv_ar=oz_nil();
  recv_ar = oz_cons(oz_pairA("dif",dif_recv),recv_ar);
  recv_ar = oz_cons(oz_pairA("misc",misc_recv),recv_ar);
  recv_ar = oz_cons(oz_pairA("messages",mess_recv),recv_ar);
  OZ_Term recv=OZ_recordInit(oz_atom("recv"),recv_ar);


  OZ_Term ar=oz_nil();
  ar=oz_cons(oz_pairA("send",send),ar);
  ar=oz_cons(oz_pairA("recv",recv),ar);
  return OZ_unify(out,OZ_recordInit(oz_atom("perdioStatistics"),ar));
}
OZ_C_proc_end

BIspec marshalerSpec[]= {
  {"perdioStatistics",  1, BIperdioStatistics, 0},
  {0,0,0,0}
};

/* *********************************************************************/
/*   SECTION 16: initialization                                       */
/* *********************************************************************/

void initMarshaler(){
  refTable = new RefTable();
  refTrail = new RefTrail();
  BIaddSpec(marshalerSpec);}


static
void splitversion(char *vers, char *&major, char*&minor)
{
  major = vers;
  minor = strrchr(vers,'#');
  if (minor) {
    minor++;
  } else {
    minor = "0";
  }
}


Bool unmarshal_SPEC(MsgBuffer* buf,char* &vers,OZ_Term &t){
  PD((MARSHAL_BE,"unmarshal begin: %s s:%s","$1",buf->siteStringrep()));
  refTable->reset();
  Assert(creditSite==NULL);
  Assert(refTrail->isEmpty());
  if(buf->get()==DIF_SECONDARY) {Assert(0);return NO;}
  vers=unmarshalString(buf);
  char *major, *minor;
  splitversion(vers,major,minor);
  if (strncmp(PERDIOMAJOR,major,strlen(PERDIOMAJOR))!=0) {
    return NO;}
  int minordiff = atoi(PERDIOMINOR) - atoi(minor);
  if (minordiff > 1 || /* we only support the last minor */
      minordiff < 0) { /* emulator older than component */
    return NO;
  }
  buf->unmarshallingOld = (minordiff!=0);
  t=unmarshalTerm(buf);
  if (minordiff) {
    warning("unmarshalling old component(%s), needs resaving",toC(t));
  }
  buf->unmarshalEnd();
  refTrail->unwind();
  PD((MARSHAL_BE,"unmarshal end: %s s:%s","$1",buf->siteStringrep()));
  return OK;}


/* *********************************************************************/
/*   SECTION 17: code unmarshaling/marshaling                          */
/* *********************************************************************/

#include "marshalcode.cc"

/* *********************************************************************/
/*   SECTION 18: message marshaling                                    */
/* *********************************************************************/

#include "marshalMsg.cc"
