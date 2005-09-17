/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    Boris Mejias (bmc@info.ucl.ac.be)
 * 
 *  Copyright:
 *    Erik Klintskog, 2002
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

#if defined(INTERFACE)
#pragma implementation "glue_marshal.hh"
#endif

#include "glue_marshal.hh"
#include "glue_entities.hh"
#include "glue_mediators.hh"
#include "glue_buffer.hh"
#include "glue_interface.hh"
#include "marshalerBase.hh"
#include "glue_tables.hh"
#include "thr_int.hh"
#include "value.hh"
#include "dss_enums.hh"
/* The interafce to the Marshaler for marshaling and unmarshaling 
   of entities. Since the marshaler differentiate between variables
   and tertiaries this separation is keept. */ 
// An index, if any, is marshaled *after* 'marshalTertiary()';

enum DssMarshalDIFs{
  DSS_DIF_CELL, 
  DSS_DIF_PORT,
  DSS_DIF_LOCK,
  DSS_DIF_VAR,
  DSS_DIF_ARRAY,
  DSS_DIF_UNUSABLE,
  DSS_DIF_THREAD
};

///////////////////////////////////////////////////////////////////////////

/* Globalization of entities */
/* Implemented as two different routines depending on special */ 
/* characterestica of the entities. Still, the both routines uses */
/* the same interafces againts the dss and the glue. */ 

#define RC_ALG_MASK (RC_ALG_WRC | RC_ALG_TL |  RC_ALG_RC | RC_ALG_RLV1 | RC_ALG_RLV2 | RC_ALG_IRC ) 


#define ANOT_USE_DEFAULT 0
#define ANOT_PROT_MASK 0xFF
#define ANOT_GC_MASK       0xFF00
#define ANOT_AA_MASK       0xFF0000
#define ANOT_ASYNCH_CHANEL 0x1
#define ANOT_SYNCH_CHANEL  0x2
#define ANOT_MIGRATORY 0x4
#define ANOT_ONCE_ONLY 0x8
#define ANOT_INVALIDATION 0x10
#define ANOT_STATIONARY 0x20
#define ANOT_STATIONARY_MAN 0x10000
#define ANOT_MOBILE_MAN     0x20000

Bool getAnnotation(TaggedRef e, int &a);

ProxyVar* glue_newGlobalizeFreeVariable(TaggedRef *tPtr)
{
  int annotation=0;
  
  (void)   getAnnotation(makeTaggedRef(tPtr), annotation); 
  int gc_an = ((annotation & RC_ALG_MASK)) ? annotation & RC_ALG_MASK : RC_ALG_WRC;
  int aa_an = ((annotation & ANOT_AA_MASK) != ANOT_USE_DEFAULT) ? annotation & ANOT_AA_MASK : AA_STATIONARY_MANAGER;
  //  printf("glob var --"); 
  AbstractEntity *pi=dss->m_createMonotonicAbstractEntity(PN_TRANSIENT,
							  static_cast<AccessArchitecture>(aa_an),
							  static_cast<RCalg>(gc_an)); 

  // Creating the new variable that will be used to identify the
  // distribution property of the original variable. 
  OzVariable *cv = oz_getNonOptVar(tPtr);
  ProxyVar *mv = new ProxyVar(oz_currentBoard(),FALSE);
  extVar2Var(mv)->setSuspList(cv->unlinkSuspList());
  *tPtr = makeTaggedVar(extVar2Var(mv));

  VarMediator *me = new VarMediator(pi, makeTaggedRef(tPtr)); 
  mv->setMediator(me); 
  pi->assignMediator(me);
  return (mv);
}

// globalize a variable (if necessary)
OzVariable *glue_globalizeOzVariable(TaggedRef *vPtr) {
  Assert(oz_isFree(*vPtr) || oz_isReadOnly(*vPtr));

  // get a mediator, and globalize if necessary
  OzVariable *var = oz_getNonOptVar(vPtr);
  OzVariableMediator *med;
  if (var->hasMediator())
    med = static_cast<OzVariableMediator*>(var->getMediator());
  else {
    med = new OzVariableMediator(makeTaggedRef(vPtr), NULL);
    var->setMediator(med);
  }
  if (med->getAbstractEntity() == NULL) med->globalize();
  return var;
}



/* Globalize Tert defines the EMU and settings for the EMU. 
   Further initializations are done by globalizeTertCntl.. */

void getAnnotations(TaggedRef tr, int p_def, int a_def, int g_def, 
		    ProtocolName &p, AccessArchitecture &a, RCalg &g)
{
  int annotation = 0;
  getAnnotation(tr, annotation); 
  g = static_cast<RCalg>(((annotation & RC_ALG_MASK)) ? annotation & RC_ALG_MASK : g_def);
  p = static_cast<ProtocolName>(((annotation & ANOT_PROT_MASK)) ? annotation & ANOT_PROT_MASK : p_def);
  a = static_cast<AccessArchitecture>((annotation & ANOT_AA_MASK) ? annotation & ANOT_AA_MASK : a_def);
} 

void globalizeTertiary(Tertiary *t)
{ 
  Assert(!(t->isDistributed()));
  switch(t->getType()) {
  case Co_Cell:
    {
      OZ_error("Globalization of Cells shouldn't be done as a Tertiary\n");
      Assert(0);
      break; 
    }
  case Co_Lock:
    {
      OZ_error("Globalization of Lock shouldn't be done as a Tertiary\n");
      Assert(0);
      break; 
      // retrieve mediator, or create one
      LockMediator* me = static_cast<LockMediator*>
	(mediatorTable->lookup(makeTaggedConst(t)));
      if (me == NULL) me = new LockMediator(t);
      me->globalize();
      break;
    }
  case Co_Object:
    {
      // retrieve mediator, or create one
      ObjectMediator* me = static_cast<ObjectMediator*>
	(mediatorTable->lookup(makeTaggedConst(t)));
      if (me == NULL) me = new ObjectMediator(t, NULL);
      me->globalize();
      break;
    }
  case Co_Port:
    {
      OZ_error("Globalization of Ports shouldn't be done as a Tertiary\n");
      Assert(0);
      break;
    }
  default:
    {
      OZ_error("Globalization of unknown tert type %d\n",t->getType());
      Assert(0);
    }
  }
}





///// For the darn threads 
void oz_thread_setDistVal(TaggedRef tr, int i, void* v); 
void* oz_thread_getDistVal(TaggedRef tr, int i); 
///////////////////////////////////////////////////////////////////////////
////  Marshal Array 

void glue_marshalArray(ByteBuffer *bs, ConstTermWithHome *arrayConst)
{
  OzArray *ozA = static_cast<OzArray*>(arrayConst);

  if (!ozA->isDistributed()) {
    ArrayMediator *am = static_cast<ArrayMediator*>
      (mediatorTable->lookup(makeTaggedConst(arrayConst)));
    if (am == NULL) am = new ArrayMediator(arrayConst, NULL);
    am->globalize();
  }
  Assert(ozA->isDistributed());

  AbstractEntity *ae = index2AE((int)(ozA->getMediator()));
  GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer *>(bs); 
  ae->getCoordinatorAssistant()->marshal(gwb, PMF_ORDINARY);
  bs->put(DSS_DIF_ARRAY);
  marshalNumber(bs, ozA->getLow());;
  marshalNumber(bs, ozA->getHigh());;
}

///////////////////////////////////////////////////////////////////////////
////  Marshal Dictionary
void glue_marshalDictionary(ByteBuffer *bs, ConstTermWithHome *dictConst) {
  OzDictionary *ozD = static_cast<OzDictionary*>(dictConst);

  if (!ozD->isDistributed()) {
    DictionaryMediator *me = static_cast<DictionaryMediator*>
      (mediatorTable->lookup(makeTaggedConst(dictConst)));
    if (me == NULL) me = new DictionaryMediator(dictConst, NULL);
    me->globalize();
  }
  Assert(ozD->isDistributed());

  // the rest is not implemented yet
  Assert(0);
}

///////////////////////////////////////////////////////////////////////////
////  Marshal Cell 

void glue_marshalCell(ByteBuffer *bs, ConstTermWithHome *cellConst)
{
  OzCell *ozC = static_cast<OzCell*>(cellConst);
  CellMediator *cm;
  if (!ozC->isDistributed()) {
    cm = static_cast<CellMediator*>
      (mediatorTable->lookup(makeTaggedConst(cellConst)));
    if (cm == NULL) cm = new CellMediator(cellConst, NULL);
    cm->globalize();
  } 
  else cm = static_cast<CellMediator*>(ozC->getMediator());
  
  Assert(ozC->isDistributed());

  GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer *>(bs); 
  cm->getCoordinatorAssistant()->marshal(gwb, PMF_ORDINARY);
  bs->put(DSS_DIF_CELL);
}


///////////////////////////////////////////////////////////////////////////
////  Marshal Port 

void glue_marshalPort(ByteBuffer *bs, ConstTermWithHome *portConst)
{
  OzPort *ozP = static_cast<OzPort*>(portConst);
  PortMediator *pm;
  if (!ozP->isDistributed()) {
    pm = static_cast<PortMediator*>
      (mediatorTable->lookup(makeTaggedConst(portConst)));
    if (pm == NULL) pm = new PortMediator(portConst, NULL);
    pm->globalize();
  }
  else pm = static_cast<PortMediator*>(ozP->getMediator());

  Assert(ozP->isDistributed());

  GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer *>(bs); 
  (pm->getCoordinatorAssistant())->marshal(gwb, PMF_ORDINARY);
  bs->put(DSS_DIF_PORT);
}


///////////////////////////////////////////////////////////////////////////
////  Marshal Lock 

void glue_marshalLock(ByteBuffer *bs, ConstTermWithHome *lockConst)
{
  OzLock *ozL = static_cast<OzLock*>(lockConst);
  LockMediator *lm;
  if (!ozL->isDistributed()) {
    lm = static_cast<LockMediator*>
      (mediatorTable->lookup(makeTaggedConst(lockConst)));
    if (lm == NULL) lm = new LockMediator(lockConst, NULL);
    lm->globalize();
  } 
  else lm = static_cast<LockMediator*>(ozL->getMediator());
  
  Assert(ozL->isDistributed());

  GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer *>(bs); 
  lm->getCoordinatorAssistant()->marshal(gwb, PMF_ORDINARY);
  bs->put(DSS_DIF_LOCK);
}


///////////////////////////////////////////////////////////////////////////
////  Marshal Unusable 

void glue_marshalUnusable(ByteBuffer *bs, TaggedRef tr) {
  // bmc: If the Unusable has a mediator in the mediator table, then
  // it's already distributed
  UnusableMediator *me =
    reinterpret_cast<UnusableMediator*>(mediatorTable->lookup(tr));
  if (me == NULL) {
    // create mediator and globalize
    me = new UnusableMediator(tr);
    me->globalize();
  }

  // now go for marshaling
  CoordinatorAssistantInterface *cai = me->getCoordinatorAssistant();
  GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer *>(bs);
  cai->marshal(gwb, PMF_ORDINARY);
  bs->put(DSS_DIF_UNUSABLE);
}

void glue_marshalOzThread(ByteBuffer *bs, TaggedRef thr)
{
  OzThreadMediator *med =
    reinterpret_cast<OzThreadMediator*>(oz_thread_getDistVal(thr, 0)); 
  if (med == NULL) {
    med = new OzThreadMediator(thr, NULL);
    med->globalize();
  }
  CoordinatorAssistantInterface* cai = med->getCoordinatorAssistant(); 
  GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer *>(bs); 
  cai->marshal(gwb, PMF_ORDINARY);
  bs->put(DSS_DIF_THREAD); 
}


///////////////////////////////////////////////////////////////////////////
////  Marshal Object Stub 


void glue_marshalObjectStubInternal(Object* o, ByteBuffer *bs)
{
  ObjectClass *oc = o->getClass();
  GName *gnclass = globalizeConst(oc);
  Assert(gnclass);
  GName *gnobj = globalizeConst(o);
  Assert(o->getGName1());
  Assert(gnobj);
  //  ProxyName pn=tr2Pn(makeTaggedConst(o));

  CoordinatorAssistantInterface *cai=index2CAI(o->getTertIndex());

  GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer *>(bs); 
  cai->marshal(gwb, PMF_ORDINARY);
  marshalGName(bs, gnobj);
  marshalGName(bs, gnclass);

}


///////////////////////////////////////////////////////////////////////////
////  Unmarshal Object Stub 


OZ_Term glue_unmarshalObjectStub(ByteBuffer *bs)
{
  AbstractEntity *ae;
  AbstractEntityName aen;
  OZ_Term obj;
  OZ_Term clas;
  GlueReadBuffer *buf = static_cast<GlueReadBuffer*>(bs);
  DSS_unmarshal_status status = dss->unmarshalProxy(ae,buf,PUF_ORDINARY,aen);
  GName *gnobj   = unmarshalGName(&obj, bs);
  GName *gnclass = unmarshalGName(&clas, bs);

  if (gnobj == NULL) // The object existed 
    return obj;
  if (gnclass == NULL)
    gnclass = ((ObjectClass*)tagged2Const(clas))->getGName(); 

  Assert(!status.exist);
  ObjectVar *var = new ObjectVar(oz_currentBoard(),gnobj,gnclass); 
  TaggedRef val = makeTaggedRef(newTaggedVar(extVar2Var(var)));

  LazyVarMediator *me = new LazyVarMediator(val, ae);
  var->setMediator(me);
  addGName(gnobj, val);
  //PT->setEngineName(p_name,e_name);
  return val; 
}

///////////////////////////////////////////////////////////////////////////
////  UnMarshal ConstTerms, the ones with a distributed behavior 


OZ_Term  glue_unmarshalDistTerm(ByteBuffer *bs)
{
  AbstractEntity *ae;
  AbstractEntityName aen; 

  GlueReadBuffer *buf = static_cast<GlueReadBuffer*>(bs);

  DSS_unmarshal_status stat = dss->unmarshalProxy(ae, buf, PUF_ORDINARY , aen);
   

  if(stat.exist) {
    switch(bs->get()){
    case DSS_DIF_THREAD:
      {
        MutableMediatorInterface* mmi = 
            dynamic_cast<MutableMediatorInterface*>(ae->accessMediator());
        Assert(mmi); 
        OzThreadMediator *me = dynamic_cast<OzThreadMediator *>(mmi);
        Assert(me);
        return me->getEntity();
      }
    case DSS_DIF_CELL:
    case DSS_DIF_LOCK:
      {
        MediatorInterface *mi = ae->accessMediator(); 
        MutableMediatorInterface* mmi = dynamic_cast<MutableMediatorInterface*>(mi);
        Assert(mmi); 
        ConstMediator *me = dynamic_cast<ConstMediator *>(mmi);
        Assert(me);
        return makeTaggedConst(me->getConst());
      }
    case DSS_DIF_PORT: {
        RelaxedMutableMediatorInterface* mmi = dynamic_cast<RelaxedMutableMediatorInterface*>(ae->accessMediator());
        Assert(mmi); 
        ConstMediator *me = dynamic_cast<ConstMediator *>(mmi);
        Assert(me);
        return makeTaggedConst(me->getConst());
      }
    case DSS_DIF_ARRAY: {
        (void) unmarshalNumber(bs); 
        (void) unmarshalNumber(bs); 
        MutableMediatorInterface* mmi = static_cast<MutableMediatorInterface*>(ae->accessMediator());
        ConstMediator *me = dynamic_cast<ConstMediator *>(mmi);
        Assert(me);
        return makeTaggedConst(me->getConst());
      }
    case DSS_DIF_UNUSABLE: {
        Assert(dynamic_cast<UnusableMediator *>(ae->accessMediator()) != NULL);
        ImmutableMediatorInterface* imi = 
            static_cast<ImmutableMediatorInterface*>(ae->accessMediator());
        UnusableMediator* me = dynamic_cast<UnusableMediator *>(imi);
        Assert(me);
        return me->getEntity();
      }

    default: 
      OZ_error("Unknown DSS_DIF");
    }

  } else {
    switch(bs->get()){
    case DSS_DIF_UNUSABLE:{
      UnusableResource* unused = new UnusableResource();
      TaggedRef tr = makeTaggedConst(unused);
      UnusableMediator *um = new UnusableMediator(tr, ae);
      unused->setMediator((void *)um);
      return tr;
    }
    case DSS_DIF_THREAD:{
      TaggedRef oTh = oz_thread(oz_newThreadSuspended(1));
      Mediator *med = new OzThreadMediator(oTh, ae); 
      oz_thread_setDistVal(oTh, 0, reinterpret_cast<void*>(med));
      return oTh; 
    }
    case DSS_DIF_CELL:{
      OzCell *cell = new OzCell(NULL, makeTaggedNULL()); 
      CellMediator *me = new CellMediator(cell, ae); 
      cell->setMediator((void *)me);
      return makeTaggedConst(cell);
    }
    case DSS_DIF_PORT: {
      OzPort *port = new OzPort(oz_currentBoard(),
                                oz_newReadOnly(oz_currentBoard()));
      PortMediator *me = new PortMediator(port, ae); 
      port->setMediator(me);
      return makeTaggedConst(port);
    }
    case DSS_DIF_LOCK:{
      OzLock *lock = new OzLock(oz_currentBoard());
      LockMediator *me = new LockMediator(lock, ae); 
      lock->setMediator((void *)me);
      return makeTaggedConst(lock);
    }
    case DSS_DIF_ARRAY:{
      int low  =  unmarshalNumber(bs); 
      int high =  unmarshalNumber(bs); 
      OzArray *ozA = new OzArray(oz_currentBoard(), low, high, oz_nil());
      ArrayMediator* me = new ArrayMediator(ozA, ae); 
      void *mediator = reinterpret_cast<void*>(me);
      ozA->setMediator(mediator);
      Assert(ozA->isDistributed()); 
      Assert(ozA->getMediator() == mediator);
      //	printf("Inserting am:&d me:%d\n",(int)me, (int)(Mediator*)me); 
      return makeTaggedConst(ozA); 
    }
    default: 
      OZ_error("Unknown DSS_DIF"); 
    }
      
  }
}


///////////////////////////////////////////////////////////////////////////
////  Unmarshal of a Variable


OZ_Term glue_newUnmarshalVar(ByteBuffer* bs, Bool isFuture)
{
  AbstractEntity *ae;
  AbstractEntityName aen;
  VarMediator *me;
  
  
  GlueReadBuffer *buf = static_cast<GlueReadBuffer*>(bs);
  DSS_unmarshal_status stat = dss->unmarshalProxy(ae, buf, PUF_ORDINARY, aen);
  //printf("Used unmarshaling %d\n", used);

  
  if (stat.exist) {
    /* If we find teh variable, return it.*/
    MonotonicMediatorInterface *tmi = static_cast<MonotonicMediatorInterface *>(ae->accessMediator());
    me = dynamic_cast<VarMediator *>(tmi);
    return me->getEntity();
  }
  /* The variable has to be built.*/
  ProxyVar *pvar = new ProxyVar(oz_currentBoard(),isFuture);
  /* Make a taggedref out of the proxy-var*/
  TaggedRef val = makeTaggedRef(newTaggedVar(extVar2Var(pvar)));
  
  me = new VarMediator(ae, val); 
  //PT->setEngineName(p_name,e_name);
  pvar->setMediator(me);
  ae->assignMediator(me);
  return val;
}


///////////////////////////////////////////////////////////////////////////
////  Marshal of a Variable


void ProxyVar::marshal(ByteBuffer *bs, Bool hasIndex, TaggedRef* vRef, Bool push)
{
  Assert(getIdV() == OZ_EVAR_PROXY);
  MarshalTag tag = (isReadOnly() ? 
		    (hasIndex ? DIF_READONLY_DEF : DIF_READONLY) :
		    (hasIndex ? DIF_VAR_DEF : DIF_VAR));
  bs->put(tag);
  //  ProxyName pn=tr2Pn(makeTaggedRef(vRef));

  CoordinatorAssistantInterface  *cai = getMediator()->getCoordinatorAssistant();
  GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer *>(bs); 
  cai->marshal(gwb, (push)?PMF_PUSH:PMF_ORDINARY);

}



////////////////////////////////////////////////////////////////////////
// Marshal/unmarshal a variable

// Note: when marshaling a DEF, the index is marshaled *afterwards* by
// the caller

void glue_marshalOzVariable(ByteBuffer *bs, TaggedRef *vPtr,
			    Bool hasIndex, Bool push) {
  // assumption: the variable is distributed

  // marshal emulator-specific data
  MarshalTag tag = (oz_isReadOnly(*vPtr) ?
		    (hasIndex ? DIF_READONLY_DEF : DIF_READONLY) :
		    (hasIndex ? DIF_VAR_DEF : DIF_VAR));
  bs->put(tag);

  // marshal dss-specific data
  OzVariable *var = tagged2Var(*vPtr);
  Mediator *med = static_cast<Mediator*>(var->getMediator());
  Assert(med->getAbstractEntity());
  CoordinatorAssistantInterface *cai = med->getCoordinatorAssistant();
  GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer*> (bs);
  bool done = cai->marshal(gwb, (push ? PMF_PUSH : PMF_ORDINARY));
  Assert(done);
}

OZ_Term glue_unmarshalOzVariable(ByteBuffer* bs, Bool isReadOnly) {
  AbstractEntity *ae;
  AbstractEntityName aen;
  OzVariableMediator *med;

  // unmarshal dss-specific data
  GlueReadBuffer *buf = static_cast<GlueReadBuffer*>(bs);
  DSS_unmarshal_status stat = dss->unmarshalProxy(ae, buf, PUF_ORDINARY, aen);

  if (stat.exist) {
    // the variable already exists, return it
    MonotonicMediatorInterface *mmi
      = static_cast<MonotonicMediatorInterface*> (ae->accessMediator());
    med = dynamic_cast<OzVariableMediator*> (mmi);
    return med->getEntity();
  } else {
    // the variable does not exist, create it
    TaggedRef ref = (isReadOnly ?
		     oz_newReadOnly(oz_currentBoard()) :
		     oz_newSimpleVar(oz_currentBoard()));
    TaggedRef *ptr = tagged2Ref(ref);
    OzVariable *var = tagged2Var(*ptr);

    var->setMediator(new OzVariableMediator(ref, ae));
    return ref;
  }
}
