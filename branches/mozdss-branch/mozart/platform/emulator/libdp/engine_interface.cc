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
#include "pstContainer.hh"
#include "glue_entities.hh"
#include "glue_tables.hh"

#include "cac.hh"
#include "dss_object.hh"
#include "glue_siteRepresentation.hh"
#include "glue_buffer.hh"
#include "os.hh"
#include "glue_base.hh"
#include "glue_mediators.hh"
#include "glue_interface.hh"
#include "glue_entityOpImpl.hh"
#include "glue_ozSite.hh"
#include "glue_suspendedThreads.hh"
#include "glue_ioFactory.hh"

#define HEART_BEAT_RATE 32

Bool glueInitialized = FALSE; 
MAP* glue_dss_connection;
DSS_Object* dss;
ComService* glue_com_connection; 
GlueIoFactoryClass* glue_ioFactory;

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
  
  // Gcollect suspended connection atempts. These 
  // suspended threads must be keept alive. 
  gCollectGASreps();

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

  gcPstContainersFinish();
}



/************************* Annotations *************************/

void setAnnotation(TaggedRef entity, const Annotation& a) {
  // take the mediator of entity, and store annotation
  entity = oz_safeDeref(entity);
  glue_getMediator(entity)->setAnnotation(a);
}

Annotation getAnnotation(TaggedRef entity) {
  // create a mediator if necessary
  entity = oz_safeDeref(entity);
  return glue_getMediator(entity)->getAnnotation();
}



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

void initDP(int port, int ip, const char *siteId, int primKey)
{
  //
  if (glueInitialized) return;
  glueInitialized = OK;
  glue_dss_connection = new MAP();
  glue_com_connection = new ComService(ip, port, siteId);
  glue_ioFactory      = new GlueIoFactoryClass();
  
  // Allocate the marshalers, and unmarshalers
  DPM_Repository = new DPMarshalers();
  DPM_Repository->dpAllocateMarshalers(100);

  // initialize default annotations
  const AccessArchitecture aa = AA_STATIONARY_MANAGER;
  const RCalg              rc = RC_ALG_WRC;
  setDefaultAnnotation(GLUE_NONE,       emptyAnnotation);
  setDefaultAnnotation(GLUE_PORT,       PN_SIMPLE_CHANNEL,  aa, rc);
  setDefaultAnnotation(GLUE_CELL,       PN_MIGRATORY_STATE, aa, rc);
  setDefaultAnnotation(GLUE_LOCK,       PN_MIGRATORY_STATE, aa, rc);
  setDefaultAnnotation(GLUE_OBJECT,     PN_MIGRATORY_STATE, aa, rc);
  setDefaultAnnotation(GLUE_ARRAY,      PN_SIMPLE_CHANNEL,  aa, rc);
  setDefaultAnnotation(GLUE_DICTIONARY, PN_SIMPLE_CHANNEL,  aa, rc);
  setDefaultAnnotation(GLUE_THREAD,     PN_SIMPLE_CHANNEL,  aa, rc);
  setDefaultAnnotation(GLUE_VARIABLE,   PN_TRANSIENT,       aa, rc);
  setDefaultAnnotation(GLUE_READONLY,   PN_TRANSIENT,       aa, rc);
  setDefaultAnnotation(GLUE_UNUSABLE,   PN_SIMPLE_CHANNEL,  aa, rc);
  setDefaultAnnotation(GLUE_CHUNK,      PN_IMMEDIATE,       aa, rc);

  // create mediator table
  mediatorTable     = new MediatorTable();

  initEntityOperations();

  // GC 
  gCollectGlueStart = gcGluePrepareImpl;
  gCollectGlueWeak  = gcGlueWeakImpl;
  gCollectGlueRoots = gcGlueRootsImpl;
  gCollectGlueFinal = gcGlueFinalImpl;
  gCollectMediator  = gcMediatorImpl;

  // Starting the DSS
  dss = new DSS_Object(glue_ioFactory, glue_com_connection, glue_dss_connection);

  initHeartBeat(HEART_BEAT_RATE);
  OzSite_init();
}
