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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifdef INTERFACE
#pragma implementation "marshaler.hh"
#endif

#include "wsock.hh"
#include "codearea.hh"
#include "indexing.hh"
#include "gname.hh"
#include "var_base.hh"
#include "gc.hh"
#include "dictionary.hh"
#include "urlc.hh"
#include "msgbuffer.hh"
#include "marshaler.hh"
#include "site.hh"
#include "pickle.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>

//
SendRecvCounter dif_counter[DIF_LAST];
SendRecvCounter misc_counter[MISC_LAST];

char *misc_names[MISC_LAST] = {
  "string",
  "gname",
  "site"
};

RefTable *refTable;

/* ************************************************************************ */
/*  SECTION ::  provided to marshaler from perdio.cc */
/* ************************************************************************ */

void addGName(GName*,TaggedRef);
TaggedRef oz_findGName(GName*);
void deleteGName(GName*);

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
            18 Exported to marshalMsg.m4cc
            19 message marshaling

***************************************************************************** */

/* *********************************************************************/
/*   SECTION 1: forward declarations                                  */
/* *********************************************************************/

OZ_Term unmarshalTerm(MsgBuffer *);
void marshalTerm(OZ_Term, MsgBuffer *);
ProgramCounter unmarshalCode(MsgBuffer*,Bool);
SRecord *unmarshalSRecord(MsgBuffer *);
void unmarshalTerm(MsgBuffer *, OZ_Term *);

#define CheckD0Compatibility \
   if (ozconf.perdioMinimal) goto bomb;



RefTrail *refTrail;


/* *********************************************************************/
/*   SECTION 6:  simple ground marshaling/unmarshaling                 */
/* *********************************************************************/

unsigned short unmarshalShort(MsgBuffer *bs){
  unsigned short sh;
  unsigned int i1 = bs->get();
  unsigned int i2 = bs->get();
  sh= (i1 + (i2<<8));
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

class ByteStream;

static
char *getString(MsgBuffer *bs, unsigned int i)
{
  char *ret = new char[i+1];
  if (ret==NULL)
    return NULL;
  for (unsigned int k=0; k<i; k++) {
    if (bs->atEnd()) {
      delete ret;
      return NULL;
    }
    ret[k] = bs->get();
  }
  ret[i] = '\0';
  return ret;
}

char *unmarshalString(MsgBuffer *bs)
{
  misc_counter[MISC_STRING].recv();
  unsigned int i = unmarshalNumber(bs);

  return getString(bs,i);
}

/* a version of unmarshalString that is more stable against garbage input */
char *unmarshalVersionString(MsgBuffer *bs)
{
  unsigned int i = bs->get();
  return getString(bs,i);
}

#define Comment(Args) if (bs->textmode()) {comment Args;}

void comment(MsgBuffer *bs, const char *format, ...)
{
  char buf[10000];
  va_list ap;
  va_start(ap,format);
  vsprintf(buf,format,ap);
  va_end(ap);
  putComment(buf,bs);
}




/* *********************************************************************/
/*   SECTION 7: gname marshaling/unmarshaling                          */
/* *********************************************************************/

void marshalGName(GName *gname, MsgBuffer *bs)
{
  if (gname==NULL)
    return;

  misc_counter[MISC_GNAME].send();

  Comment((bs,"GNAMESTART"));
  gname->site->marshalSiteForGName(bs);
  for (int i=0; i<fatIntDigits; i++) {
    marshalNumber(gname->id.number[i],bs);
  }
  marshalNumber((int)gname->gnameType,bs);
  Comment((bs,"GNAMEEND"));
}

void unmarshalGName1(GName *gname, MsgBuffer *bs)
{
  gname->site=unmarshalSite(bs);
  for (int i=0; i<fatIntDigits; i++) {
    gname->id.number[i] = unmarshalNumber(bs);
  }
  gname->gnameType = (GNameType) unmarshalNumber(bs);
}

GName *unmarshalGName(TaggedRef *ret, MsgBuffer *bs)
{
  misc_counter[MISC_GNAME].recv();
  GName gname;
  unmarshalGName1(&gname,bs);

  TaggedRef aux = oz_findGName(&gname);
  if (aux) {
    if (ret) *ret = aux; // ATTENTION
    return 0;
  }
  return new GName(gname);
}

/* *********************************************************************/
/*   SECTION 9: term marshaling routines                               */
/* *********************************************************************/

inline Bool checkCycle(void *l, MsgBuffer *bs)
{
  int n = refTrail->find(l);
  if (n>=0) {
    marshalDIF(bs,DIF_REF);
    marshalTermRef(n,bs);
    return OK;
  }
  return NO;
}

inline void trailCycle(void *l, MsgBuffer *bs)
{
  int counter = refTrail->trail(l);
  marshalTermDef(counter,bs);
}

void trailCycleOutLine(void *l, MsgBuffer *bs){ trailCycle(l, bs);}
Bool checkCycleOutLine(void *l, MsgBuffer *bs){ return checkCycle(l, bs);}

void marshalSRecord(SRecord *sr, MsgBuffer *bs)
{
  TaggedRef t = oz_nil();
  if (sr) {
    t = makeTaggedSRecord(sr);
  }
  marshalTerm(t,bs);
}

GName *globalizeConst(ConstTerm *t, MsgBuffer *bs)
{
  if (!bs->globalize())
    return 0;

  switch(t->getType()) {
  case Co_Object:      return ((Object*)t)->globalize();
  case Co_Class:       return ((ObjectClass*)t)->globalize();
  case Co_Chunk:       return ((SChunk*)t)->globalize();
  case Co_Abstraction: return ((Abstraction*)t)->globalize();
  default: Assert(0); return NULL;
  }
}

void marshalClass(ObjectClass *cl, MsgBuffer *bs)
{
  marshalDIF(bs,DIF_CLASS);
  GName *gn = globalizeConst(cl,bs);
  trailCycle(cl,bs);
  marshalGName(gn,bs);
  marshalNumber(cl->getFlags(),bs);
  marshalSRecord(cl->getFeatures(),bs);
}

void marshalNoGood(TaggedRef term, MsgBuffer *bs, Bool trail)
{
  if (ozconf.perdioMinimal || bs->getSite()==NULL){
    bs->addNogood(term);
    marshalTerm(NameNonExportable,bs);} // to make bs consistent
  else{
    (*marshalSPP)(term,bs, trail); }
}


/* *********************************************************************/
/*   SECTION 10: ConstTerm and Term marshaling                          */
/* *********************************************************************/

static
void marshalConst(ConstTerm *t, MsgBuffer *bs)
{
  switch (t->getType()) {
  case Co_BigInt:
    marshalDIF(bs,DIF_BIGINT);
    marshalString(toC(makeTaggedConst(t)),bs);
    return;

  case Co_Dictionary:
    {
      OzDictionary *d = (OzDictionary *) t;

      if (!d->isSafeDict()) {
        goto bomb;
      }

      marshalDIF(bs,DIF_DICT);

      int size = d->getSize();
      trailCycle(d,bs);
      marshalNumber(size,bs);

      int i = d->getFirst();
      i = d->getNext(i);
      while(i>=0) {
        marshalTerm(d->getKey(i),bs);
        marshalTerm(d->getValue(i),bs);
        i = d->getNext(i);
        size--;
      }
      return;
    }

  case Co_Builtin:
    {
      Builtin *bi= (Builtin *)t;
      if (bi->isSited())
        goto bomb;

      marshalDIF(bs,DIF_BUILTIN);
      trailCycle(t,bs);

      marshalString(bi->getPrintName(),bs);
      return;
    }
  case Co_Chunk:
    {
      SChunk *ch=(SChunk *) t;
      GName *gname=globalizeConst(ch,bs);
      marshalDIF(bs,DIF_CHUNK);
      trailCycle(t,bs);
      marshalGName(gname,bs);
      marshalTerm(ch->getValue(),bs);
      return;
    }
  case Co_Class:
    {
      ObjectClass *cl = (ObjectClass*) t;
      if (cl->isSited())
        goto bomb;

      globalizeConst(cl,bs);
      marshalClass(cl,bs);
      return;
    }
  case Co_Abstraction:
    {
      Abstraction *pp=(Abstraction *) t;
      if (pp->getPred()->isSited())
        goto bomb;

      GName *gname = globalizeConst(pp,bs);

      marshalDIF(bs,DIF_PROC);
      trailCycle(t,bs);

      marshalGName(gname,bs);
      marshalTerm(pp->getName(),bs);
      marshalNumber(pp->getArity(),bs);
      ProgramCounter pc = pp->getPC();
      int gs = pp->getPred()->getGSize();
      marshalNumber(gs,bs);
      marshalNumber(pp->getPred()->getMaxX(),bs);
      for (int i=0; i<gs; i++) {
        marshalTerm(pp->getG(i),bs);
      }

      marshalCode(pc,bs);
      return;
    }

  case Co_Object:
    {
      CheckD0Compatibility;
      Object *o = (Object*) t;
      if(o->getClass()->isSited())
        goto bomb;
      if (!bs->globalize()) return;
      (*marshalObject)(t, bs, NULL);
      return;
    }

#define HandleTert(string,tag,check)                    \
    if (check) { CheckD0Compatibility; }                \
    if (!bs->globalize()) return;                       \
    if ((*marshalTertiary)((Tertiary *) t,tag,bs)) return;      \
    trailCycle(t,bs);                                   \
    return;

  case Co_Lock: HandleTert("lock",DIF_LOCK,OK);
  case Co_Cell: HandleTert("cell",DIF_CELL,OK);
  case Co_Port: HandleTert("port",DIF_PORT,NO);
  case Co_Resource:HandleTert("resource",DIF_RESOURCE_T,OK);
#undef HandleTert

  default:
    goto bomb;
  }

  Assert(0);

bomb:
  marshalNoGood(makeTaggedConst(t),bs,OK);
  trailCycle(t,bs);
}

void marshalTerm(OZ_Term t, MsgBuffer *bs)
{
  int depth = 0;

loop:

  DEREF(t,tPtr,tTag);
  switch(tTag) {

  case SMALLINT:
    if (bs->visit(t)) {
      marshalDIF(bs,DIF_SMALLINT);
      marshalNumber(smallIntValue(t),bs);
    }
    break;

  case OZFLOAT:
    if (bs->visit(t)) {
      marshalDIF(bs,DIF_FLOAT);
      marshalFloat(tagged2Float(t)->getValue(),bs);
    }
    break;

  case LITERAL:
    {
      Literal *lit = tagged2Literal(t);
      if (checkCycle(lit,bs)) goto exit;

      if (!bs->visit(t))
        break;

      MarshalTag litTag;
      GName *gname = NULL;

      if (lit->isAtom()) {
        litTag = DIF_ATOM;
      } else if (lit->isUniqueName()) {
        litTag = DIF_UNIQUENAME;
      } else if (lit->isCopyableName()) {
        litTag = DIF_COPYABLENAME;
      } else {
        litTag = DIF_NAME;
        if (bs->globalize())
          gname = ((Name*)lit)->globalize();
      }

      marshalDIF(bs,litTag);
      const char *name = lit->getPrintName();
      trailCycle(lit,bs);
      marshalString(name,bs);
      marshalGName(gname,bs);
      break;
    }

  case LTUPLE:
    {
      depth++; Comment((bs,"("));
      LTuple *l = tagged2LTuple(t);
      if (checkCycle(l,bs)) goto exit;
      if (!bs->visit(t))
        break;
      marshalDIF(bs,DIF_LIST);
      trailCycle(l,bs);

      marshalTerm(l->getHead(),bs);

      // tail recursion optimization
      t = l->getTail();
      goto loop;
    }

  case SRECORD:
    {
      depth++; Comment((bs,"("));
      SRecord *rec = tagged2SRecord(t);
      if (checkCycle(rec,bs)) goto exit;
      if (!bs->visit(t))
        break;
      TaggedRef label = rec->getLabel();

      if (rec->isTuple()) {
        marshalDIF(bs,DIF_TUPLE);
        trailCycle(rec,bs);
        marshalNumber(rec->getTupleWidth(),bs);
      } else {
        marshalDIF(bs,DIF_RECORD);
        trailCycle(rec,bs);
        marshalTerm(rec->getArityList(),bs);
      }
      marshalTerm(label,bs);
      int argno = rec->getWidth();

      for(int i=0; i<argno-1; i++) {
        marshalTerm(rec->getArg(i),bs);
      }
      // tail recursion optimization
      t = rec->getArg(argno-1);
      goto loop;
    }

  case EXT:
    {
      if (!checkCycle(oz_tagged2Extension(t),bs) && bs->visit(t)) {
        marshalDIF(bs,DIF_EXTENSION);
        marshalNumber(oz_tagged2Extension(t)->getIdV(),bs);
        if (!oz_tagged2Extension(t)->marshalV(bs)) {
          marshalNoGood(t,bs,NO);
        }
      }
      break;
    }
  case OZCONST:
    {
      if (!checkCycle(tagged2Const(t),bs) && bs->visit(t)) {
        Comment((bs,"("));
        marshalConst(tagged2Const(t),bs);
        Comment((bs,")"));
      }
      break;
    }

  case FSETVALUE:
    {
      CheckD0Compatibility;

      if (!bs->visit(t))
        break;

      OZ_FSetValue * fsetval = tagged2FSetValue(t);
      marshalDIF(bs,DIF_FSETVALUE);
      // tail recursion optimization
      t = fsetval->getKnownInList();
      goto loop;
    }

  case UVAR:
    // FUT
  case CVAR:
    if (!bs->visit(makeTaggedRef(tPtr)))
      break;
    if((*triggerVariable)(tPtr)){
      marshalTerm(makeTaggedRef(tPtr),bs);
      return;}
    if((*marshalVariable)(tPtr, bs, NULL))
      break;
    t=makeTaggedRef(tPtr);
    goto bomb;

  default:
  bomb:
    marshalNoGood(t,bs, NO);
    break;
  }

 exit:
  while(depth--) {
    Comment((bs,")"));
  }
  return;
}

/* *********************************************************************/
/*   SECTION 11: term unmarshaling routines                            */
/* *********************************************************************/

void unmarshalDict(MsgBuffer *bs, TaggedRef *ret)
{
  int refTag = unmarshalRefTag(bs);
  int size   = unmarshalNumber(bs);
  Assert(oz_onToplevel());
  OzDictionary *aux = new OzDictionary(am.currentBoard(),size);
  *ret = makeTaggedConst(aux);
  gotRef(*ret,refTag);

  aux->markSafe();

  while(size-- > 0) {
    TaggedRef key = unmarshalTerm(bs);
    TaggedRef val = unmarshalTerm(bs);
    aux->setArg(key,val);
  }
  return;
}


void unmarshalClass(ObjectClass *cl, MsgBuffer *bs)
{
  int flags = unmarshalNumber(bs);

  SRecord *feat = unmarshalSRecord(bs);

  if (cl==NULL)  return;

  TaggedRef ff = feat->getFeature(NameOoFeat);

  cl->import(feat,
             tagged2Dictionary(feat->getFeature(NameOoFastMeth)),
             oz_isSRecord(ff) ? tagged2SRecord(ff) : (SRecord*)NULL,
             tagged2Dictionary(feat->getFeature(NameOoDefaults)),
             flags);
}

OZ_Term unmarshalTerm(MsgBuffer *bs)
{
  OZ_Term ret;
  unmarshalTerm(bs,&ret);
  return ret;
}

inline
ObjectClass *newClass(GName *gname) {
  Assert(oz_onToplevel());
  ObjectClass *ret = new ObjectClass(NULL,NULL,NULL,NULL,NO,NO,am.currentBoard());
  ret->setGName(gname);
  return ret;
}

SRecord *unmarshalSRecord(MsgBuffer *bs){
  TaggedRef t = unmarshalTerm(bs);
  return oz_isNil(t) ? (SRecord*)NULL : tagged2SRecord(t);
}

/* *********************************************************************/
/*   SECTION 12: term unmarshaling                                     */
/* *********************************************************************/

void unmarshalTerm(MsgBuffer *bs, OZ_Term *ret)
{
loop:
  MarshalTag tag = (MarshalTag) bs->get();

  dif_counter[tag].recv();
  switch(tag) {

  case DIF_SMALLINT:
    *ret = OZ_int(unmarshalNumber(bs));
    return;

  case DIF_FLOAT:
    *ret = OZ_float(unmarshalFloat(bs));
    return;

  case DIF_NAME:
    {
      int refTag = unmarshalRefTag(bs);
      GName *gname;
      char *printname;

      printname = unmarshalString(bs);
      gname     = unmarshalGName(ret,bs);

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
      gotRef(*ret,refTag);
      delete printname;
      return;
    }

  case DIF_COPYABLENAME:
    {
      int refTag      = unmarshalRefTag(bs);
      char *printname = unmarshalString(bs);

      NamedName *aux = NamedName::newCopyableName(ozstrdup(printname));
      *ret = makeTaggedLiteral(aux);
      gotRef(*ret,refTag);
      delete printname;
      return;
    }

  case DIF_UNIQUENAME:
    {
      int refTag      = unmarshalRefTag(bs);
      char *printname = unmarshalString(bs);

      *ret = oz_uniqueName(printname);
      gotRef(*ret,refTag);
      delete printname;
      return;
    }

  case DIF_ATOM:
    {
      int refTag = unmarshalRefTag(bs);
      char *aux  = unmarshalString(bs);
      *ret = OZ_atom(aux);
      gotRef(*ret,refTag);
      delete aux;
      return;
    }

  case DIF_BIGINT:
    {
      char *aux = unmarshalString(bs);
      *ret = OZ_CStringToNumber(aux);
      delete aux;
      return;
    }

  case DIF_LIST:
    {
      LTuple *l = new LTuple();
      *ret = makeTaggedLTuple(l);
      int refTag = unmarshalRefTag(bs);
      gotRef(*ret,refTag);
      unmarshalTerm(bs,l->getRefHead());
      // tail recursion optimization
      ret = l->getRefTail();
      goto loop;
    }
  case DIF_TUPLE:
    {
      int refTag = unmarshalRefTag(bs);
      int argno  = unmarshalNumber(bs);
      TaggedRef label = unmarshalTerm(bs);
      SRecord *rec = SRecord::newSRecord(label,argno);
      *ret = makeTaggedSRecord(rec);
      gotRef(*ret,refTag);

      for(int i=0; i<argno-1; i++) {
        unmarshalTerm(bs,rec->getRef(i));
      }
      // tail recursion optimization
      ret = rec->getRef(argno-1);
      goto loop;
    }

  case DIF_RECORD:
    {
      int refTag = unmarshalRefTag(bs);
      TaggedRef arity = unmarshalTerm(bs);
      TaggedRef sortedarity = arity;
      if (!isSorted(arity)) {
        int len;
        TaggedRef aux = duplist(arity,len);
        sortedarity = sortlist(aux,len);
      }
      TaggedRef label = unmarshalTerm(bs);
      SRecord *rec    = SRecord::newSRecord(label,aritytable.find(sortedarity));
      *ret = makeTaggedSRecord(rec);
      gotRef(*ret,refTag);

      while(oz_isCons(arity)) {
        TaggedRef val = unmarshalTerm(bs);
        rec->setFeature(oz_head(arity),val);
        arity = oz_tail(arity);
      }
      return;
    }

  case DIF_REF:
    {
      int i = unmarshalNumber(bs);
      *ret = refTable->get(i);
      Assert(*ret);
      return;
    }

  case DIF_REF_DEBUG:
    {
      Assert(0); // not yet implemented
      int i          = unmarshalNumber(bs);
      TypeOfTerm tag = (TypeOfTerm) unmarshalNumber(bs);
      *ret = refTable->get(i);
      Assert(*ret);
      Assert(tag==tagTypeOf(*ret));
      return;
    }

  case DIF_OWNER:
  case DIF_OWNER_SEC:
    {
      *ret = (*unmarshalOwner)(bs, tag);
      return;
    }
  case DIF_RESOURCE_T:
  case DIF_PORT:
  case DIF_THREAD_UNUSED:
  case DIF_SPACE:
  case DIF_CELL:
  case DIF_LOCK:
  case DIF_OBJECT:
    {
      *ret = (*unmarshalTertiary)(bs, tag);
      int refTag = unmarshalRefTag(bs);
      gotRef(*ret,refTag);
      return;
    }
  case DIF_RESOURCE_N:
    {
      *ret = (*unmarshalTertiary)(bs, tag);
      return;
    }

  case DIF_CHUNK:
    {
      int refTag   = unmarshalRefTag(bs);
      GName *gname = unmarshalGName(ret,bs);

      SChunk *sc;
      if (gname) {
        Assert(oz_onToplevel());
        sc=new SChunk(am.currentBoard(),0);
        sc->setGName(gname);
        *ret = makeTaggedConst(sc);
        addGName(gname,*ret);
      } else {
        // mm2: share the follwing code DIF_CHUNK, DIF_CLASS, DIF_PROC!
        Assert(oz_isSChunk(oz_deref(*ret)));
        sc = 0;
      }
      gotRef(*ret,refTag);
      TaggedRef value = unmarshalTerm(bs);
      if (sc) sc->import(value);
      return;
    }

  case DIF_CLASS:
    {
      int refTag = unmarshalRefTag(bs);

      GName *gname=unmarshalGName(ret,bs);

      ObjectClass *cl;
      if (gname) {
        cl = newClass(gname);
        *ret = makeTaggedConst(cl);
        addGName(gname,*ret);
      } else {
        Assert(oz_isClass(oz_deref(*ret)));
        cl = 0;
      }
      gotRef(*ret,refTag);
      unmarshalClass(cl,bs);
      return;
    }

  case DIF_VAR:
    {
      *ret = (*unmarshalVar)(bs,FALSE,FALSE);
      return;
    }

  case DIF_FUTURE:
    {
      *ret = (*unmarshalVar)(bs,TRUE,FALSE);
      return;
    }

  case DIF_VAR_AUTO:
    {
      *ret = (*unmarshalVar)(bs,FALSE,TRUE);
      return;
    }

  case DIF_FUTURE_AUTO:
    {
      *ret = (*unmarshalVar)(bs,TRUE,TRUE);
      return;
    }

  case DIF_PROC:
    {
      int refTag    = unmarshalRefTag(bs);
      GName *gname  = unmarshalGName(ret,bs);
      OZ_Term name  = unmarshalTerm(bs);
      int arity     = unmarshalNumber(bs);
      int gsize     = unmarshalNumber(bs);
      int maxX      = unmarshalNumber(bs);

      if (gname) {
        PrTabEntry *pr = new PrTabEntry(name,mkTupleWidth(arity),0,0,0,
                                        oz_nil(), maxX);
        Assert(oz_onToplevel());
        pr->setGSize(gsize);
        Abstraction *pp = Abstraction::newAbstraction(pr,am.currentBoard());
        *ret = makeTaggedConst(pp);
        pp->setGName(gname);
        addGName(gname,*ret);
        gotRef(*ret,refTag);
        for (int i=0; i<gsize; i++) {
          pp->initG(i, unmarshalTerm(bs));
        }
        pr->PC=unmarshalCode(bs,NO);
        pr->patchFileAndLine();
      } else {
        Assert(oz_isAbstraction(oz_deref(*ret)));
        gotRef(*ret,refTag);
        for (int i=0; i<gsize; i++) {
          (void) unmarshalTerm(bs);
        }
        (void) unmarshalCode(bs,OK);
      }
      return;
    }

  case DIF_DICT:
    {
      unmarshalDict(bs,ret);
      return;
    }
  case DIF_ARRAY:
    {
      OZ_warning("unmarshal array not impl");  // mm2
      return;
    }
  case DIF_BUILTIN:
    {
      int refTag = unmarshalRefTag(bs);
      char *name = unmarshalString(bs);
      Builtin * found = string2Builtin(name);

      if (!found) {
        OZ_warning("Builtin '%s' not in table.", name);
        *ret = oz_nil();
        delete name;
        return;
      }

      if (found->isSited()) {
        OZ_warning("Unpickling sited builtin: '%s'", name);
      }

      delete name;
      *ret = makeTaggedConst(found);
      gotRef(*ret,refTag);
      return;
    }

  case DIF_FSETVALUE:
    {
      OZ_Term glb=unmarshalTerm(bs);
      extern void makeFSetValue(OZ_Term,OZ_Term*);
      makeFSetValue(glb,ret);
      return;
    }

  case DIF_EXTENSION:
    {
      int type = unmarshalNumber(bs);
      *ret = oz_extension_unmarshal(type,bs);
      if(*ret == 0 && !ozconf.perdioMinimal)
        *ret = unmarshalTerm(bs);
      return;
    }

  default:
    OZ_error("unmarshal: unexpected tag: %d\n",tag);
    Assert(0);
    *ret = oz_nil();
    return;
  }

  Assert(0);
}

/* *********************************************************************/
/*   SECTION 16: initialization                                       */
/* *********************************************************************/

void initMarshaler()
{
  refTable = new RefTable();
  refTrail = new RefTrail();

  /* do some consistency checks: whenever one of the
   * following tests fails: increase PERDIOMINOR
   */
  Assert(OZERROR == 224);  /* new instruction(s) added? */
  Assert(DIF_LAST == 43);  /* new dif(s) added? */
}




/* *********************************************************************/
/*   SECTION 17: code unmarshaling/marshaling                          */
/* *********************************************************************/

#include "marshalcode.cc"

/* *********************************************************************/
/*   SECTION 18: Exported to marshalMsg.m4cc                         */
/* *********************************************************************/

void marshalTermRT(OZ_Term t, MsgBuffer *bs){
  Assert(refTrail->isEmpty());
  marshalTerm(t, bs);
  refTrail->unwind();}

OZ_Term unmarshalTermRT(MsgBuffer *bs){
  OZ_Term ret;
  refTable->reset();
  Assert(refTrail->isEmpty());
  ret =  unmarshalTerm(bs);
  refTrail->unwind();
  return ret;
}

#include "pickle.cc"
