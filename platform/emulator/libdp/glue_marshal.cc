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

// globalize a variable
OzVariable *glue_globalizeOzVariable(TaggedRef *vPtr) {
  printf("--- raph: globalize var %x\n", makeTaggedRef(vPtr));
  Assert(oz_isVar(*vPtr));
  Assert(!(tagged2Var(*vPtr)->isDistributed()));
  Assert(oz_isFree(*vPtr) || oz_isReadOnly(*vPtr));

  // create mediator and globalize
  OzVariable *var = oz_getNonOptVar(vPtr);
  OzVariableMediator *med = new OzVariableMediator(NULL, makeTaggedRef(vPtr));
  var->setDistributed(med);
  med->globalize();
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
  Mediator *me;
  AbstractEntity *ae;
  ProtocolName prot; 
  AccessArchitecture aa; 
  RCalg gc;
  switch(t->getType()) {
  case Co_Cell:
    {
      // retrieve mediator, or create one
      me = mediatorTable->lookup(makeTaggedConst(t));
      if (me == NULL) me = new CellMediator(t);
      static_cast<CellMediator*>(me)->globalize();
      //me->globalize();
      break; 
    }
  case Co_Lock:
    {
      getAnnotations(makeTaggedConst(t),
                     PN_MIGRATORY_STATE,
                     AA_STATIONARY_MANAGER,
                     RC_ALG_WRC,
                     prot, aa, gc);
      ae = dss->m_createMutableAbstractEntity( prot, aa, gc);
      LockMediator *lm = new LockMediator(ae,t);
      me = lm; 
      ae->assignMediator(lm); 
      break;
    }
  case Co_Object:
    {
      getAnnotations(makeTaggedConst(t),
                     PN_MIGRATORY_STATE, 
                     AA_STATIONARY_MANAGER,
                     RC_ALG_WRC,
                     prot, aa, gc);
      ae = dss->m_createMutableAbstractEntity(prot,aa,gc);
      //bmc: Maybe two possibilities here. Create a LazyVarMediator first
      //continuing with the approach of marshaling only the stub in the 
      //beginning, or just go eagerly for the object. We are going to try
      //the eager approach first, and then the optimization.
      ObjectMediator *om = new ObjectMediator(ae,t);
      ae->assignMediator(om);
      me = om;
      
      Object *o = (Object *) t;
      RecOrCell state = o->getState();
      if (!stateIsCell(state)) {
        SRecord *r = getRecord(state);
        Assert(r!=NULL);
        Tertiary *cell = (Tertiary *) tagged2Const(OZ_newCell(makeTaggedSRecord(r)));
        o->setState(cell);
      }
      break;
    }
  case Co_Port:
    {
      getAnnotations(makeTaggedConst(t),
                     PN_SIMPLE_CHANNEL,
                     AA_STATIONARY_MANAGER,
                     RC_ALG_WRC,
                     prot, aa, gc);
      ae = dss->m_createRelaxedMutableAbstractEntity(prot,aa, gc);
      
      // Looks strange, but the asbtarct entity takes only 
      // Mediator interfaces that is an suprclass of the specialized
      // Mediators, but not of the top Mediator.
      PortMediator *pm = new PortMediator(ae,t);
      ae->assignMediator(pm);
      me = pm;
      break; 
    }
  default:
    {
      OZ_error("Globalization of unknown tert type %d\n",t->getType());
      Assert(0);
    }
  }


  // Creating the EMU
  // Instrumenting the proxy to be distributed.
  t->setTertType(Te_Proxy);
  t->setTertIndex(reinterpret_cast<int>(me));
}





///// For the darn threads 
void oz_thread_setDistVal(TaggedRef tr, int i, void* v); 
void* oz_thread_getDistVal(TaggedRef tr, int i); 
///////////////////////////////////////////////////////////////////////////
////  Marshal Array 

void glue_marshalArray(ByteBuffer *bs, ConstTermWithHome *arrayConst)
{
  OzArray *ozA = static_cast<OzArray*>(arrayConst);
  AbstractEntity *ae;
  if(!ozA->isDistributed()) {
    ArrayMediator *am = static_cast<ArrayMediator*>
      (mediatorTable->lookup(makeTaggedConst(arrayConst)));
    if (am == NULL) am = new ArrayMediator(NULL, arrayConst);
      
    am->globalize();
    ae = am->getAbstractEntity();
    Assert(ozA->isDistributed());
  }
  else
    ae=index2AE(ozA->getDist());
  GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer *>(bs); 
  ae->getCoordinatorAssistant()->marshal(gwb, PMF_ORDINARY);
  bs->put(DSS_DIF_ARRAY);
  marshalNumber(bs, ozA->getLow());;
  marshalNumber(bs, ozA->getHigh());;
  
}

///////////////////////////////////////////////////////////////////////////
////  Marshal Dictionary
void glue_marshalDictionary(ByteBuffer *bs, ConstTermWithHome *arrayConst) {
  // Not implemented yet
  Assert(0);
}


///////////////////////////////////////////////////////////////////////////
////  Marshal Unusable 

void glue_marshalUnusable(ByteBuffer *bs, TaggedRef tr) {
  //bmc:
  // If the Unusable has a mediator in the engineTable
  // then, it's already distributed
  AbstractEntity *ae;
  UnusableMediator *me;
  me = reinterpret_cast<UnusableMediator*>(mediatorTable->lookup(tr));
  if ( me == NULL) {
    // Even when we don't distribute this unusable, we have to remember
    // that it has been exported.
    //bmc: I'm not sure yet about the distibution strategy of this entity
    ae = dss->m_createImmutableAbstractEntity(
      PN_IMMUTABLE_LAZY,
      AA_STATIONARY_MANAGER,
      RC_ALG_WRC);
    me = new UnusableMediator(ae, tr);
    ae->assignMediator(me);
  }
  else {
    ae = me->getAbstractEntity();
  }
  CoordinatorAssistantInterface *cai = ae->getCoordinatorAssistant();
  GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer *>(bs);
  cai->marshal(gwb, PMF_ORDINARY);
  bs->put(DSS_DIF_UNUSABLE);
  
  /*
  Mediator *me = taggedref2Me(tr);   
  ProxyInterface *pi;
  if (me  == NULL){
    pi = createEMU(CM_PLACE_HOLDER,
		   PN_SYNC_CHANEL,
		   AA_STATIONARY_MANAGER,
		   RC_ALG_WRC);
    Mediator *me = new UnusableMediator(pi,tr,CM_PLACE_HOLDER);
    int med = reinterpret_cast<int>(me);
    engineTable->insert(me, tr);
  }else{
    pi=me->getProxy();
  }
  GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer *>(bs); 
  pi->marshal(gwb, PMF_ORDINARY);
  bs->put(DSS_DIF_UNUSABLE);
  // ERIK, Add a description? 
  */
  //Assert(0);
}

void glue_marshalOzThread(ByteBuffer *bs, TaggedRef thr)
{
  Mediator *med = reinterpret_cast<Mediator*>(oz_thread_getDistVal(thr, 0)); 
  
  if (med == NULL){
    AbstractEntity *ae = dss->m_createMutableAbstractEntity(PN_SIMPLE_CHANNEL, AA_STATIONARY_MANAGER, RC_ALG_WRC);
    OzThreadMediator *tm = new OzThreadMediator(ae, thr);
    med = tm; 
    ae->assignMediator(tm); 
    oz_thread_setDistVal(thr, 0, reinterpret_cast<void*>(med)); 
  }
  CoordinatorAssistantInterface* cai = med->getCoordinatorAssistant(); 
  GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer *>(bs); 
  cai->marshal(gwb, PMF_ORDINARY);
  bs->put(DSS_DIF_THREAD); 
}


///////////////////////////////////////////////////////////////////////////
////  Marshal Tertiary 


void glue_marshalTertiary(ByteBuffer *bs, Tertiary *t, Bool push)
{
  switch(t->getTertType()){
  case Te_Local:
    globalizeTertiary(t);
    // no break here!
  case Te_Proxy:
    {
      CoordinatorAssistantInterface* cai=index2CAI(t->getTertIndex());
      GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer *>(bs); 
      cai->marshal(gwb, (push)?PMF_PUSH:PMF_ORDINARY);

      switch(t->getType()){
      case Co_Cell:
        bs->put(DSS_DIF_CELL);
        break;
      case Co_Port:
        bs->put(DSS_DIF_PORT);
        break; 
      case Co_Lock:
        bs->put(DSS_DIF_LOCK);
        break;
      case Co_Resource:
        bs->put(DSS_DIF_UNUSABLE);
        break;
      default:
        OZ_error("We dont distribute that SHIT %d", t->getType());
      }
      break; 
    }
  default:
    OZ_error("A tertiary is either a proxy or a manager not a %d",t->getTertType());
  }
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

  LazyVarMediator *me = new LazyVarMediator(ae, val);
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
      Tertiary* tert =  new UnusableResource();
      TaggedRef tr = makeTaggedConst(tert);
      UnusableMediator *um = new UnusableMediator(ae, tr);
      tert->setTertIndex(reinterpret_cast<int>(um));
      ae->assignMediator(um);
      return tr;
    }
    case DSS_DIF_THREAD:{
      TaggedRef oTh=  oz_thread(oz_newThreadSuspended(1));
      OzThreadMediator *me = new OzThreadMediator(ae,oTh); 
      ae->assignMediator(me);
      Mediator *med = static_cast<Mediator*>(me); 
      oz_thread_setDistVal(oTh, 0, reinterpret_cast<void*>(med));
      return oTh; 
    }
    case DSS_DIF_CELL:{
      Tertiary *tert = new CellProxy(); 
      CellMediator *me = new CellMediator(ae, tert); 
      tert->setTertIndex(reinterpret_cast<int>(me));
      ae->assignMediator(me);
      return makeTaggedConst(tert);
    }
    case DSS_DIF_PORT: {
      Tertiary *tert = new PortProxy();        
      PortMediator *me = new PortMediator(ae, tert); 
      tert->setTertIndex(reinterpret_cast<int>(me));
      ae->assignMediator(me);
      return makeTaggedConst(tert);
    }
    case DSS_DIF_LOCK:{
      Tertiary *tert = new LockProxy();
      LockMediator *me = new LockMediator(ae, tert); 
      tert->setTertIndex(reinterpret_cast<int>(me));
      ae->assignMediator(me); 
      return makeTaggedConst(tert);
    }
    case DSS_DIF_ARRAY:{
      int low  =  unmarshalNumber(bs); 
      int high =  unmarshalNumber(bs); 
      ConstTermWithHome *ozc = new ArrayProxy(low, high);
      ArrayMediator* me = new ArrayMediator(ae, ozc); 
      int mediator = reinterpret_cast<int>(me);
      ozc->setDist(mediator);
      Assert(ozc->isDistributed()); 
      Assert(ozc->getDist() == mediator);
      //	printf("Inserting am:&d me:%d\n",(int)me, (int)(Mediator*)me); 
      ae->assignMediator(me);
      return makeTaggedConst(ozc); 
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
  Assert(tagged2Var(*vPtr)->isDistributed());

  // marshal emulator-specific data
  MarshalTag tag = (oz_isReadOnly(*vPtr) ?
		    (hasIndex ? DIF_READONLY_DEF : DIF_READONLY) :
		    (hasIndex ? DIF_VAR_DEF : DIF_VAR));
  bs->put(tag);

  // marshal dss-specific data
  OzVariable *var = tagged2Var(*vPtr);
  Mediator *med = static_cast<Mediator*> (var->getMediator());
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

    med = new OzVariableMediator(ae, ref);
    var->setDistributed(med);
    ae->assignMediator(med);
    return ref;
  }
}
