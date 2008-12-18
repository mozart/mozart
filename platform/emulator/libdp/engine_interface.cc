/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
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
#pragma implementation "engine_interface.hh"
#endif

#include "engine_interface.hh"
#include "builtins.hh"
#include "vprops.hh"
#include "pstContainer.hh"
#include "glue_tables.hh"

#include "cac.hh"
#include "dss_object.hh"
#include "glue_site.hh"
#include "glue_buffer.hh"
#include "os.hh"
#include "glue_base.hh"
#include "glue_mediators.hh"
#include "glue_interface.hh"
#include "glue_entityOpImpl.hh"
#include "glue_suspendedThreads.hh"
#include "glue_ioFactory.hh"

#define HEART_BEAT_RATE 32

Bool glueInitialized = FALSE; 
DSS_Object* dss                    = NULL;
ComService* glue_com_connection    = NULL; 

// GC Routines
// 
// These routines are now defined at Glue level, and not as it used to be
// at DSS level. Most of the coplicated stuff was related to the fact that 
// the DSS was tightly tied to the engine. Most complicated was gc of the 
// marshalers. Since the marshalers now have moved out of the DSS
// (conceptually, they where never there)


// collect a mediator
void gcMediatorImpl(Mediator* med) {
  med->gCollect();
}

/*************************************************************************************************/
/*************************************************************************************************/

void gcGluePrepareImpl()
{
  // Mark all msg containers. Has to be done for the snapshoting
  // mechanism.
  gcPstContainersStart();
}


void gcGlueRootsImpl()
{
  // Now work on that
  gcMediatorTablePrimary();

  // Gcollect the messages stored in the outgoing and 
  // incomming queue
  gcPstContainersRoot();
  
  // mark GlueSites' roots
  gcGlueSiteRoots();

  // GCollect all the suspended operations
  gCollectSuspendedOperations();
}


void gcGlueWeakImpl(){
  gcMediatorTableWeak();
}



void gcGlueFinalImpl()
{
  //Decide what to do and do it for mediators
  gcMediatorTableCleanUp();

  dss->gcDssResources();

  gcGlueSiteFinal();

  gcPstContainersFinish();
}



/************************* Annotations *************************/

bool Annotation::hasMutableProtocol() const {
  switch (pn) {
  case PN_NO_PROTOCOL:
  case PN_SIMPLE_CHANNEL:
  case PN_MIGRATORY_STATE:
  case PN_PILGRIM_STATE:
  case PN_EAGER_INVALID:
  case PN_SITED:
    return true;
  default:
    return false;
  }
}

bool Annotation::hasImmutableProtocol() const {
  switch (pn) {
  case PN_NO_PROTOCOL:
  case PN_SIMPLE_CHANNEL:
  case PN_IMMEDIATE:
  case PN_IMMUTABLE_EAGER:
  case PN_IMMUTABLE_LAZY:
  case PN_SITED:
    return true;
  default:
    return false;
  }
}

bool Annotation::hasTransientProtocol() const {
  switch (pn) {
  case PN_NO_PROTOCOL:
  case PN_TRANSIENT:
  case PN_TRANSIENT_REMOTE:
    return true;
  default:
    return false;
  }
}

bool Annotation::adjoin(const Annotation &a) {
  // check incremental compatibility
  if (a.pn && pn && a.pn != pn) return false;
  if (a.aa && aa && a.aa != aa) return false;
  if (a.rc && rc && a.rc != rc) return false;
  // tell attributes (when present)
  if (a.pn) pn = a.pn;
  if (a.aa) aa = a.aa;
  if (a.rc) rc = a.rc;
  return true;
}

// those macros make the parsing slighly more readable
#define CHECKPROT(VAR,NAME,VALUE)				\
  if (strcmp(VAR, NAME) == 0) {					\
    if (pn != PN_NO_PROTOCOL && pn != VALUE) goto error;	\
    pn = VALUE;							\
    continue;							\
  }
#define CHECHARCH(VAR,NAME,VALUE)				\
  if (strcmp(VAR, NAME) == 0) {					\
    if (aa != AA_NO_ARCHITECTURE && aa != VALUE) goto error;	\
    aa = VALUE;							\
    continue;							\
  }

OZ_Return Annotation::parseTerm(TaggedRef term) {
  Assert(OZ_isList(term, NULL));
  // traverse the list and parse elements
  TaggedRef list;
  for (list = term; oz_isCons(list); list = oz_deref(oz_tail(list))) {
    TaggedRef elem = oz_safeDeref(oz_head(list));
    if (oz_isVarOrRef(elem)) oz_suspendOn(elem);
    Assert(!oz_isVarOrRef(elem));
    if (oz_isAtom(elem)) {
      const char* name = tagged2Literal(elem)->getPrintName();
      // check for consistency protocols
      CHECKPROT(name, "stationary", PN_SIMPLE_CHANNEL);
      CHECKPROT(name, "migratory", PN_MIGRATORY_STATE);
      CHECKPROT(name, "pilgrim", PN_PILGRIM_STATE);
      CHECKPROT(name, "replicated", PN_EAGER_INVALID);
      CHECKPROT(name, "variable", PN_TRANSIENT);
      CHECKPROT(name, "reply", PN_TRANSIENT_REMOTE);
      CHECKPROT(name, "immediate", PN_IMMEDIATE);
      CHECKPROT(name, "eager", PN_IMMUTABLE_EAGER);
      CHECKPROT(name, "lazy", PN_IMMUTABLE_LAZY);
      CHECKPROT(name, "sited", PN_SITED);
      // check for gc protocols
      if (strcmp(name, "persistent") == 0) {
	if (rc & ~RC_ALG_PERSIST) goto error;
	rc = RC_ALG_PERSIST;
	continue;
      }
      if (strcmp(name, "credit") == 0) {
	if (rc & RC_ALG_PERSIST) goto error;
	rc = static_cast<RCalg>(rc | RC_ALG_WRC);
	continue;
      }
      if (strcmp(name, "lease") == 0) {
	if (rc & RC_ALG_PERSIST) goto error;
	rc = static_cast<RCalg>(rc | RC_ALG_TL);
	continue;
      }
    }
    /*
      else if (oz_isTuple(elem)) {
      SRecord* rec = tagged2SRecord(elem);
      const char* label = tagged2Literal(rec->getLabel())->getPrintName();
      if (strcmp(label, "access") == 0 && rec->getWidth() == 1) {
	TaggedRef arg = oz_safeDeref(rec->getArg(0));
	if (oz_isVarOrRef(arg)) oz_suspendOn(arg);
	Assert(!oz_isVarOrRef(arg));
	if (oz_isAtom(arg)) {
	  const char* name = tagged2Literal(arg)->getPrintName();
	  CHECHARCH(name, "stationary", AA_STATIONARY_MANAGER);
	  CHECHARCH(name, "migratory", AA_MIGRATORY_MANAGER);
	}
      }
    }
    */
    // element cannot be parsed
    goto error;
  }
  Assert(oz_isNil(list));
  return PROCEED;

 error:
  return oz_raise(E_SYSTEM, AtomDp, "annotation", 1, term);
}


TaggedRef Annotation::toTerm() {
  TaggedRef list = oz_nil();
  if (rc) {   // push DGC annotation
    if (rc & RC_ALG_PERSIST) list = oz_cons(oz_atom("persistent"), list);
    if (rc & RC_ALG_TL)      list = oz_cons(oz_atom("lease"), list);
    if (rc & RC_ALG_WRC)     list = oz_cons(oz_atom("credit"), list);
  }
  /*
  switch (aa) {   // push access architecture annotation
  case AA_STATIONARY_MANAGER:
    list = oz_cons(OZ_mkTupleC("access", 1, oz_atom("stationary")), list);
    break;
  case AA_MIGRATORY_MANAGER:
    list = oz_cons(OZ_mkTupleC("access", 1, oz_atom("migratory")), list);
    break;
  default:
    break;
  }
  */
  switch (pn) {   // push protocol annotation
  case PN_SIMPLE_CHANNEL:   list = oz_cons(oz_atom("stationary"), list); break;
  case PN_MIGRATORY_STATE:  list = oz_cons(oz_atom("migratory"), list); break;
  case PN_PILGRIM_STATE:    list = oz_cons(oz_atom("pilgrim"), list); break;
  case PN_EAGER_INVALID:    list = oz_cons(oz_atom("replicated"), list); break;
  case PN_TRANSIENT:        list = oz_cons(oz_atom("variable"), list); break;
  case PN_TRANSIENT_REMOTE: list = oz_cons(oz_atom("reply"), list); break;
  case PN_IMMEDIATE:        list = oz_cons(oz_atom("immediate"), list); break;
  case PN_IMMUTABLE_EAGER:  list = oz_cons(oz_atom("eager"), list); break;
  case PN_IMMUTABLE_LAZY:   list = oz_cons(oz_atom("lazy"), list); break;
  case PN_SITED:            list = oz_cons(oz_atom("sited"), list); break;
  default: break;
  }
  return list;
}



/************************* Default Annotations *************************/

// Default annotations are get/set as system properties in module
// Property.  Those properties are implemented as virtual properties,
// see vprops.hh.

class AnnotationProperty : public VirtualProperty {
private:
  GlueTag type;

public:
  AnnotationProperty(const GlueTag tag) : type(tag) {}

  virtual OZ_Term get() {
    return getDefaultAnnotation(type).toTerm();
  }
  virtual OZ_Return set(OZ_Term t) {
    Annotation a;
    OZ_Return ret = a.parseTerm(t);
    if (ret == PROCEED) {
      if (!glue_validProtocol(a.pn, type) || !a.isComplete())
	return oz_raise(E_SYSTEM, AtomDp, "annotation", 1, t);
      setDefaultAnnotation(type, a);
    }
    return ret;
  }
};



/************************* Time stuff *************************/

LongTime lastTime; 

static Bool heartBeat_check(LongTime*, void*){
  return TRUE; 
}

static Bool heartBeat_wake(LongTime* t, void*){
  glue_com_connection->a_msgnLayer->m_heartBeat(*t - lastTime); 
  lastTime = *t;
  return TRUE; 
}
#define HEART_BEAT_ID 1234567

void initHeartBeat(int rate)
{
  lastTime=*(am.getEmulatorClock());
  if(!am.registerTask((void*) HEART_BEAT_ID, 
		      heartBeat_check, heartBeat_wake)) {
    OZ_error("Unable to register HeartBeat");
  }
  am.setMinimalTaskInterval((void *) HEART_BEAT_ID,rate);
}



/************************* DP initialization *************************/

void initDP()
{
  //
  if (glueInitialized) return;
  glueInitialized = OK;
  glue_com_connection = new ComService();
  
  // Allocate the marshalers, and unmarshalers
  DPM_Repository = new DPMarshalers();
  DPM_Repository->dpAllocateMarshalers(100);

  // initialize default annotations
  const AccessArchitecture aa = AA_STATIONARY_MANAGER;
  const RCalg              rc = RC_ALG_WRC;
  setDefaultAnnotation(GLUE_NONE,       Annotation());
  setDefaultAnnotation(GLUE_PORT,       PN_SIMPLE_CHANNEL,  aa, rc);
  setDefaultAnnotation(GLUE_CELL,       PN_MIGRATORY_STATE, aa, rc);
  setDefaultAnnotation(GLUE_LOCK,       PN_MIGRATORY_STATE, aa, rc);
  setDefaultAnnotation(GLUE_OBJECT,     PN_IMMUTABLE_EAGER, aa, rc);
  setDefaultAnnotation(GLUE_OBJECTSTATE, PN_MIGRATORY_STATE, aa, rc);
  setDefaultAnnotation(GLUE_ARRAY,      PN_SIMPLE_CHANNEL,  aa, rc);
  setDefaultAnnotation(GLUE_DICTIONARY, PN_SIMPLE_CHANNEL,  aa, rc);
  setDefaultAnnotation(GLUE_THREAD,     PN_SIMPLE_CHANNEL,  aa, rc);
  setDefaultAnnotation(GLUE_VARIABLE,   PN_TRANSIENT,       aa, rc);
  setDefaultAnnotation(GLUE_READONLY,   PN_TRANSIENT,       aa, rc);
  setDefaultAnnotation(GLUE_UNUSABLE,   PN_SITED,           aa, rc);
  setDefaultAnnotation(GLUE_CHUNK,      PN_IMMEDIATE,       aa, rc);
  setDefaultAnnotation(GLUE_CLASS,      PN_IMMEDIATE,       aa, rc);
  setDefaultAnnotation(GLUE_PROCEDURE,  PN_IMMEDIATE,       aa, rc);

  // make them available in module Property
  (new AnnotationProperty(GLUE_PORT))->add("dp.annotation.port");
  (new AnnotationProperty(GLUE_CELL))->add("dp.annotation.cell");
  (new AnnotationProperty(GLUE_LOCK))->add("dp.annotation.lock");
  (new AnnotationProperty(GLUE_OBJECT))->add("dp.annotation.object");
  (new AnnotationProperty(GLUE_OBJECTSTATE))->add("dp.annotation.state");
  (new AnnotationProperty(GLUE_ARRAY))->add("dp.annotation.array");
  (new AnnotationProperty(GLUE_DICTIONARY))->add("dp.annotation.dictionary");
  // no property for GLUE_THREAD
  (new AnnotationProperty(GLUE_VARIABLE))->add("dp.annotation.variable");
  (new AnnotationProperty(GLUE_READONLY))->add("dp.annotation.readonly");
  (new AnnotationProperty(GLUE_UNUSABLE))->add("dp.annotation.unusable");
  (new AnnotationProperty(GLUE_CHUNK))->add("dp.annotation.chunk");
  (new AnnotationProperty(GLUE_CLASS))->add("dp.annotation.class");
  (new AnnotationProperty(GLUE_PROCEDURE))->add("dp.annotation.procedure");

  // create mediator table
  mediatorTable     = new MediatorTable();

  // distributed operations
  initEntityOperations();

  // GC 
  gCollectGlueStart = gcGluePrepareImpl;
  gCollectGlueWeak  = gcGlueWeakImpl;
  gCollectGlueRoots = gcGlueRootsImpl;
  gCollectGlueFinal = gcGlueFinalImpl;
  gCollectMediator  = gcMediatorImpl;

  // Starting the DSS
  dss = new DSS_Object(glue_com_connection, new MAP());

  initHeartBeat(HEART_BEAT_RATE);
  OzSite_init();
}
