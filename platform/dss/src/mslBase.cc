/*
 *  Authors:
 *    Erik Klintskog
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
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
 
#if defined(INTERFACE)
#pragma implementation "mslBase.hh"
#pragma implementation "dss_comService.hh"
#endif

#include "mslBase.hh"
#include "dss_templates.hh"
#include "msl_dsite.hh"
#include "msl_msgContainer.hh"
#include "msl_interRouter.hh"

namespace _msl_internal{

  // ********************** Scheduled Events to be run ASAP ****************************
  // 
#ifdef DEBUG_CHECK
  int Event::a_allocated=0;
#endif

  

  class ImmediateEvents{
  private:
    SimpleQueue<Event*> a_queue;
  public:
    void m_appendEvent(Event* const ev){
      a_queue.append(ev);
      ev->event_action();
    }

    ImmediateEvents():a_queue(){ };
    ~ImmediateEvents(){ }

    void executeEvents(MsgnLayerEnv *evn){
      while (!a_queue.isEmpty()){
	Event *ev = a_queue.pop();
	ev->event_execute(evn);
	delete ev;
      }
    }
    void gc(){ t_gcList(a_queue); }
  };
  

  class LoopBackEvent: public Event{
  private:
    MsgCnt* a_msgC;
  public:
    LoopBackEvent(MsgCnt *msg):a_msgC(msg){ };

    virtual void event_execute(MsgnLayerEnv* env) { 
      // Converting the outgoing message to an incomming message
      // This _must_ be done here and not when the event is inserted in 
      // the queue, since the pst field can be uninitialized. 
      a_msgC->m_convert2Rec();
      //MsgCnt *msg = static_cast<MsgCnt*>(msgC); 
      int mt=a_msgC->popIntVal();
      if(mt == C_APPLICATION){
	env->m_AppMessageReceived(a_msgC,env->a_mySite);
	return; 
      }
      if(mt == C_CSC){
	env->m_CscMessageReceived(a_msgC,env->a_mySite);
	return;
      }
      Assert(0); 
      
      // env->m_MessageReceived(msg,env->a_mySite); 
    };
    virtual void m_makeGCpreps() { a_msgC->m_makeGCpreps(); };
    virtual ~LoopBackEvent(){ delete a_msgC; };
    MACRO_NO_DEFAULT_CONSTRUCTORS(LoopBackEvent); 
  }; 
  
  
  
  // ********************************************************************
  // Messaging Layer Environment 
  // ********************************************************************
  void MsgnLayerEnv::m_heartBeat(const int& TimePassedInMs){
    
    // In case something came in during sleep
    a_immediateEvents->executeEvents(this); 
    // I think almost always  but one never know :)
    if(TimePassedInMs > 0)        
      a_timers->m_ticks(TimePassedInMs);
  }
  
  void MsgnLayerEnv::m_loopBack(MsgCnt* msg){
    a_immediateEvents->m_appendEvent(new LoopBackEvent(msg)); 
  }

  void MsgnLayerEnv::m_stateChange(Site *s, const FaultState& state){
    DSite *ds = static_cast<DSite*>(s); 
    a_clbck->m_stateChange(ds, state); 
  }

  void MsgnLayerEnv::m_unsentMessages(Site* s, MsgCnt* msg){
    DSite *ds = static_cast<DSite*>(s); 
    MsgContainer* msgS = static_cast<MsgContainer*>(msg); 
    a_clbck->m_unsentMessages(ds, msgS); 
  }
  void MsgnLayerEnv::m_AppMessageReceived(MsgCnt* msg, Site* s)
  {
    DSite *ds = static_cast<DSite*>(s); 
    MsgContainer* msgR = static_cast<MsgContainer*>(msg); 
    a_clbck->m_MessageReceived(msgR, ds); 
  }


  void MsgnLayerEnv::m_CscMessageReceived(MsgCnt* msg, Site* s)
  {
    DSite *ds = static_cast<DSite*>(s); 
    MsgContainer* msgR = static_cast<MsgContainer*>(msg); 
    ds->m_getCsSiteRep()->receivedMsg(msgR);
  }
  
  void MsgnLayerEnv::m_appendImmediateEvent(Event* e){
    a_immediateEvents->m_appendEvent(e); 
  }
  
  DSS_LongTime* 
  MsgnLayerEnv::m_getCurrTime() {
    return NULL; 
  }


  int MsgnLayerEnv::m_getFirewallReopenTimeout(){
    return 60000 *100; 
  }
  int MsgnLayerEnv::m_getReopenRemoteTimeout(){
    return 60000 *10;
  }

  MsgnLayerEnv::MsgnLayerEnv(::AppMslClbkInterface* clbk, 
			     ::ComServiceInterface* csc, 
			     ::MsgnLayer *msl, 
			     const bool& sec):
    a_destSite(NULL),
    a_srcSite(NULL) ,
    a_mySite(NULL),
    a_comService(csc), 
    a_timers(new Timers()), 
    a_interRouter(NULL),
    a_immediateEvents(new ImmediateEvents()), 
    a_clbck(clbk),   
    a_siteHT(NULL),
    a_OSWriteCounter(0),
    a_OSReadCounter(0),
    a_ContCounter(0),
    a_SendCounter(0),
    a_RecCounter(0),
    a_routeIds(0),
    a_ipIsbehindFW(false)
  {
    a_siteHT = new SiteHT(32, this); 
    a_interRouter = new InterRouter(this); 
    
    // Environment initialized and functions bound
    // after crypto we can generate id (used in every private key)
    randomize_crypto();
    //
    // Create the site for this process
    // 
    RSA_private* key = new RSA_private();
    BYTE* p = key->getStringRep();
    int pk = gf_char2integer(&p[5]);
    a_mySite = new Site(pk, key, this, sec);
    dssLog(DLL_IMPORTANT,"SITE: (%p) - Making myDSite", a_mySite);
    a_siteHT->m_insert(a_mySite);
    
    CsSiteInterface *sa = csc->connectSelfReps(msl, static_cast<DSite*>(a_mySite));
    a_mySite->m_setCsSite(sa);
    a_mySite->m_invalidateMarshaledRepresentation(); // get new rep
  }


  MsgnLayerEnv::~MsgnLayerEnv(){
    free_crypto_mem();
  }

  void 
  MsgnLayerEnv::m_gcSweep(){
    // Collecting all loopback messages. A well desined system
    // should not have any pending messages at all.
    a_immediateEvents->gc();
  
    // Give the communication service the chance to 
    // mark all the DSite objects it is using. 
    a_comService->m_gcSweep();
  
    // Clean the DSite table from unmarsked DSite objects.
    // It is of nessesarity that all data structures that 
    // potentially referes DSIte objects have been 
    // given a chance to mark the DSite objects they refer. 
    a_siteHT->gcSiteTable();
  }
}

//Constructors added to force the inclussion of the symbols in the library.
ComServiceInterface::ComServiceInterface() {}
CsSiteInterface::CsSiteInterface() {}
