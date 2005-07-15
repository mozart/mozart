/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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
void gcMediatorImpl(void *m) {
  static_cast<Mediator*>(m)->gCollect();
}

/*************************************************************************************************/
/*************************************************************************************************/

void gcGluePrepareImpl()
{
  // Mark all msg containers. Has to be done for the 
  // snapshoting mechanism. 
  gcPstContainersStart();
}


void gcGlueRootsImpl()
{
  // Now work on that
  gcEngineTablePrimary();

  // Gcollect the messages stored in the outgoing and 
  // incomming queue
  gcPstContainersRoot();
  
  // Gcollect suspended connection atempts. These 
  // suspended threads must be keept alive. 
  gCollectGASreps();

  // GCollect all the suspended threads 
  gCollectSuspThreads();
}


void gcGlueWeakImpl(){
  gcEngineTableWeak();
}



void gcGlueFinalImpl()
{
  //Decide what to do and do it for mediators
  gcEngineTableCleanUp();

  dss->gcDssResources();
  // The annotations are weak pointers, and must then be 
  // keept only if the entities themselfs are collected. 
  gcAddress2InfoTables();

  gcPstContainersFinish();
}


AddressHashTableO1Reset *Address2Info;
AddressHashTableO1Reset *Address2InfoBackup;

class InfoClassNode
{
public:
  int annotation; 
  InfoClassNode(int a){
    annotation = a; 
  }
}; 


void annotateEntity(TaggedRef entity,int a){
  TaggedRef e = oz_deref(entity);
  void *in = (InfoClassNode *)Address2Info->htFind((void *)e);
  //printf("Setting Annotation %s key:%x val:%d\n",toC(entity), (int)e, a);
  if(in == htEmpty) 
    Address2Info->htAdd((void *)e, (void*) new InfoClassNode(a));
  else
    ((InfoClassNode*)in)->annotation = a; 
}

Bool getAnnotation(TaggedRef e, int &a)
{
  TaggedRef entity = oz_deref(e);
  //printf("Reading Annotation %s key:%x\n",toC(entity),(int)entity);
  void *in = (InfoClassNode *)Address2Info->htFind((void *)entity);
  if(in==htEmpty){
    printf("---- nothing\n");
    return FALSE;}
  a = ((InfoClassNode*)in)->annotation; 
  printf("++++ found: %d!\n",a);
  return TRUE;
}

// We keep the addresses in two open-hash-tables.
// At each gc, we read all entries out of the used
// ht and insert them into the unused. By this schema
// we swicth between the two ht's. 

void gcAddress2InfoTables()
{
  for(AHT_HashNodeCnt *node = Address2Info->getFirst();node!=NULL;node = Address2Info->getNext(node))
    {
      TaggedRef t = (TaggedRef)node->getKey();
      if (isGCMarkedTerm(t))
	{
	  //printf("Keeping ENtity");
	  oz_gCollectTerm(t,t);
	  Address2InfoBackup->htAdd((void*)t,(void*)node->getValue());
	}
      //else
      //printf("Dropping Node\n");
    }
  // Switch and clean...
  // The current ht is cleared and the hts switch place..
  AddressHashTableO1Reset *tmp=Address2InfoBackup;
  Address2Info->mkEmpty();
  Address2InfoBackup = Address2Info;
  Address2Info = tmp;
}



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
  
  // The two tables used for annotations 
  Address2Info = new AddressHashTableO1Reset(100);
  Address2InfoBackup = new AddressHashTableO1Reset(100); 


  initEntityOperations();
  

  // GC 
  gCollectGlueStart = gcGluePrepareImpl;
  gCollectGlueWeak  = gcGlueWeakImpl;
  gCollectGlueRoots = gcGlueRootsImpl;
  gCollectGlueFinal = gcGlueFinalImpl;
  gCollectMediator  = gcMediatorImpl;

  // Starting the DSS
  engineTable      = new EngineTable(100);

  //resourceTable    = new ResourceHashTable(RESOURCE_HASH_TABLE_DEFAULT_SIZE);
  dss = new DSS_Object(glue_ioFactory, glue_com_connection, glue_dss_connection);

  initHeartBeat(HEART_BEAT_RATE);
  OzSite_init();
}
