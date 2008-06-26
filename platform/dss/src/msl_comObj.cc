/*
 *  Authors:
 *    Anna Neiderud (annan@sics.se)
 *
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 *
 *  Copyright:
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
#pragma implementation "msl_comObj.hh"
#endif


#include <string.h> //strcmp
#include "dss_comService.hh"

#include "msl_comObj.hh"
#include "msl_msgContainer.hh"
#include "msl_transObj.hh"
#include "msl_timers.hh"

#include "msl_endRouter.hh"
#include "msl_interRouter.hh"
#include "msl_dsite.hh"
#include "dss_comService.hh"
#include "msl_dct.hh"
//#include "dss_serialize.hh"
#include "msl_tcpTransObj.hh"
#include "msl_prioQueues.hh"
#include "msl_timers.hh"


#ifdef _MSC_VER
#include <io.h>
#include <time.h>
#else
#include <sys/time.h>
#endif

namespace _msl_internal { //Start namespace


  char *msl_mess_names[C_LAST] = {
    "conn_first_NOT_A_VALID_MESSAGETYPE",
    "conn_anon_present",
    "conn_init_present",
    "conn_anon_negotiate",
    "conn_init_negotiate",
    "conn_anon_negotiate_answer",
    "conn_ack",
    "conn_close_hard",
    "conn_close_weak",
    "conn_close_accept",
    "conn_close_reject",
    "conn_clear_reference",
    "conn_set_ack_prop",
    "disc_route_discovery",
    "route_set_route",
    "route_target_touched",
    "route_route",
    "direct_msg",
    "ping"
  };

#define DSS_VERSION "0#7"



  const BYTE dss_version[4] = DSS_VERSION;


  // The same as DSS_Environment::m_anonymousChannelEstablished() but for a route.
  ComObj*
  anonymousRouteEstablished(TransObj *transObj, Site *srcSite,
                            Site *nxtSite, int routeId, MsgnLayerEnv* env) {

    EndRouter *endRouter = static_cast<EndRouter *>(transObj);
    ComObj *comObj = new ComObj(srcSite, env);

    endRouter->setOwner(comObj);
    endRouter->setRouteId(routeId);

    endRouter->setSuccessor(nxtSite->m_getComObj());

    // store a reference of the new created comObj for the route
    srcSite->m_setComObj(comObj);

    return comObj;
  }

  const int MSG_ACK_TIMEOUT = 100;
  const int MSG_ACK_LENGTH  = 200;

  // ***************************************************************************
  //
  //  The com-object, the abstraction for a logical communication channel
  //
  //

#ifdef DEBUG_CHECK
  int ComObj::a_allocated=0;
#endif


  ComObj::ComObj(Site *site, MsgnLayerEnv* env):
    a_mslEnv(env),
    a_transObj(NULL),
    a_site(site),
    a_sec(CSecuritySettings()),
    a_queues(new PrioQueues(env->a_timers)),
    a_lastSent(0),
    a_lastReceived(0),
    a_sentLrgMsg(0),
    a_receivedLrgMsg(0),
    a_reopentimer(NULL),
    a_maxrtt(0),
    a_ackCanceled(true),
    a_ackExpiration(DSS_LongTime()),
    a_ackTimer(NULL),
    a_probeIntervalTimer(NULL),
    e_timers(env->a_timers),
    a_msgAckTimeOut(0xFFFFFFFF),
    a_msgAckLength(0xFFFFFFFF),
    a_lastrtt(-1),
    a_state(CLOSED),
    a_closeHardFlag(false),
    a_probing(false),
    a_msgSentDuringProbeInterval(false),
    a_msgReceivedDuringProbeInterval(false),
    a_localRef(false), // Will be set true by the first send and false by gc
    a_remoteRef(false),
    a_sentClearRef(false), // Introduced to correspond to the same variable as in the old DL
    a_pred(NULL){
    a_sec.a_ticket = 0;
    dssLog(DLL_ALL,"COMMUNICATION: ComObject created");
    //printf("ComObject %p created\n",static_cast<void*>(this));
    DebugCode(a_allocated++);
  }

  ComObj::~ComObj() {
    DebugCode(a_allocated--);
    MsgCnt *tmp, *msg= a_queues->clearAll();
    while(msg){
      tmp = msg->a_next;
      delete msg;
      msg = tmp;
    }
    if(!(m_inState(CLOSED)))
      m_close();
    delete a_queues;
  }

  MsgCnt* ComObj::m_clearQueues()
  {
    // this call will violate all assumtions of
    // reliable  in order communication. Should ONLY
    // be called when the comObject is removed due to
    // perm fail.

    MsgCnt *app_msgs = NULL, *msgs = a_queues->clearAll();
    while(msgs!=NULL) {
      msgs->resetCounter(); // "rewind"
      MslMessageType mt = static_cast<MslMessageType>(msgs->popIntVal());
      MsgCnt *nxt = msgs->a_next;

      if(mt == C_APPLICATION){
        msgs->a_next = app_msgs;
        app_msgs = msgs;
      } else
        delete msgs;

      msgs = nxt;
    }
    return app_msgs;
  }

  void ComObj::m_close()
  {
    clearTimers();
    a_lastrtt=-1;
    if(a_transObj){
      DssChannel* ch = a_transObj->m_closeConnection();
      ch->close();          // the channel object can be freed
      delete a_transObj;
      a_transObj=NULL;

    }
    a_queues->clear5();
    m_setCState(CLOSED);
  }

  void ComObj::m_closeDownConnection() { // ZACHARIAS: Is this code correct??
    if(m_inState(OPENING_WF_PRESENT | OPENING_WF_NEGOTIATE | CLOSING_WEAK))
      a_closeHardFlag = true;
    else if(m_inState(WORKING))
      m_WORKING_2_CLOSING_HARD();
  }

  void ComObj::m_closeErroneousConnection(){
    // What about the queues?:
    //  a_queues->clearAll();
    Assert(0); dssError("DISCONNECT DUE TO SEC. VIOLATIONS");
    m_close(); // force a close

  } // perhaps pass from where we came

  unsigned int ComObj::sendProbePing(){
    if (a_msgReceivedDuringProbeInterval) {
      // We have received a msg, and it was within the rtt boundary.
      // So continue probing, and make sure that at least one message
      // is exchanged during the next probe interval.  Send a ping if
      // necessary.
      a_msgSentDuringProbeInterval = false;
      a_msgReceivedDuringProbeInterval = false;
      if (!a_queues->hasNeed()) {   // possibly no communication
        m_send(new MsgCnt(C_PING, false), MSG_PRIO_LAZY);
      }
      return a_maxrtt;
    }

    // Nothing has been received within the rtt boundary, so stop
    // probing, and report the timeout.
    a_probeIntervalTimer = NULL;   // timer will be deleted after return
    a_probing = false;
    a_site->m_getCsSite()->reportTimeout(a_maxrtt);
    return 0;
  }

  inline bool ComObj::hasNeed() {
    return (a_localRef || a_queues->hasNeed());
  }


  void ComObj::m_setLocalRef(){
    a_localRef = true;
  }

  unsigned int ComObj::sendAckTimer() {
    if(a_ackCanceled){ // If, canceled we shouldn't keep it alive
      a_ackTimer = NULL;
      return 0;
    } else {
      int diff = (a_ackExpiration - (e_timers->currTime()));
      if(diff > 0){ //shouldn't fire yet
        return diff;
      } else {
        if(!a_queues->hasQueued()) { // Otherwise an ack will go anyway
          MsgCnt *msg = new MsgCnt(C_ACK, true);
          m_send(msg, MSG_PRIO_EAGER);
        }
        a_ackTimer = NULL;
        a_ackCanceled = true;
        return 0;
      }
    }
  }




  // ***********************************************************
  //
  // Free functions
  //

  static unsigned int if_comObj_sendProbePing(void *comObj) {
    return static_cast<ComObj *>(comObj)->sendProbePing();
  }

  static unsigned int if_comObj_sendAckTimer(void *comObj) {
    return static_cast<ComObj *>(comObj)->sendAckTimer();
  }

  static unsigned int if_comObj_reopen(void *comObj) {
    static_cast<ComObj *>(comObj)->reopen();
    return 0;
  }


  // ZACHARIAS: Below looks more than funny, check the boolean assignments...

  // raph: Those assignments reflect a "clean" state for the probe.
  // Moreover they will force to exchange at least one message, which
  // is necessary for the probing mechanism to work.

  // Specials to opt imer management
  inline void ComObj::setProbeIntervalTimer(){
    a_msgSentDuringProbeInterval     = false;
    a_msgReceivedDuringProbeInterval = true;
    sendProbePing();
    if(a_probeIntervalTimer == NULL){
      e_timers->setTimer(a_probeIntervalTimer,a_maxrtt,
                         if_comObj_sendProbePing, this);
    }
  }

  // set ack timer to msgAckTimeout
  inline void ComObj::setAckTimer(){
    a_ackCanceled   = false;
    a_ackExpiration = (e_timers->currTime());
    a_ackExpiration.increaseTime(a_msgAckTimeOut);
    if (a_ackTimer == NULL)
      e_timers->setTimer(a_ackTimer,a_msgAckTimeOut,
                         if_comObj_sendAckTimer, this);

  }

  void ComObj::reopen() {
    if(a_state==CLOSED_WF_REMOTE) { // Tired of waiting for rem.
      Assert(a_transObj==NULL);
      m_setCState(CLOSED);
      m_CLOSED_2_OPENING_WF_HANDOVER();
    }
    a_reopentimer=NULL;
  }

  // ************************ END OF FREE **********************



  // "clear the ack", now we can actually reuse the thing since it
  // will stay in the shadows, just blocked
  inline void ComObj::clearAckTimer(){
    a_ackCanceled   = true;
  }


  // Specifying priority -1 means accepting the default as in msgFormat.m4 and
  // should allways be used.
  void ComObj::m_send(MsgCnt *msgC, int priority){
    if(msgC == NULL)
      return ;

    bool toDeliver=false;
    a_mslEnv->a_SendCounter++;
    MslMessageType mt = static_cast<MslMessageType>(msgC->peekMslMessageType());

    if(mt<C_FIRST && !a_localRef) {
      a_localRef=true;
      a_sentClearRef=false;
    }

    switch(a_state) {
    case WORKING:
      if(a_queues->hasQueued())
        break;
    case ANONYMOUS_WF_PRESENT:
    case ANONYMOUS_WF_NEGOTIATE:
    case OPENING_WF_NEGOTIATE:
    case OPENING_WF_PRESENT:
      toDeliver = true;
      break;
    case CLOSED:
      m_open();
      break;
    default: // Eventually we will have a channel...
      break;
    }
    /*
      if (default_mess_priority[msgC->getMessageType()] == USE_PRIO_OF_SENDER){
      priority = msgC->getPriority();
      }
      else
      priority =  default_mess_priority[msgC->getMessageType()] + 1;
    */

    a_queues->enqueue(msgC,priority); // After check queues.hasQueued

    if(toDeliver) {//!!new fashion to support synchronous deliver
      a_transObj->deliver();
    }
  }

  // ERIK: check who is calling this one
  void ComObj::m_open() {
    if(a_transObj==NULL)  // Otherwise we are not quite closed yet
      m_CLOSED_2_OPENING_WF_HANDOVER();
  }


  // Called by builtin when this comObj can have its communication
  // The returnvalue indicates whether it is wanted or not
  void ComObj::handover(DssChannel* tc) {
    if(a_state!=OPENING_WF_HANDOVER) {
      tc->close();     // the channel object can be freed
      return;
    }
    m_OPENING_WF_HANDOVER_2_OPENING_WF_PRESENT(tc);
  }

  void ComObj::handoverRoute(DSite *succSite[], int len) {
    if (getState() != OPENING_WF_HANDOVER) {
      printf("Ooops, route not taken, aborted by com\n");
      return ;
    }

    a_transObj = new EndRouter(a_mslEnv);
    a_transObj->setOwner(this);


    m_setCState(OPENING_WF_PRESENT);

    // !!! maybe we need an other state ???

    a_transObj->readyToReceive();

    EndRouter *endRouter = static_cast<EndRouter *>(a_transObj);
    endRouter->initRouteSetUp(succSite, len);
  }


  void ComObj::m_acceptAnonConnection(TransObj *transObj) {
    m_CLOSED_2_ANONYMOUS_WF_PRESENT(transObj);
  }

  bool ComObj::canBeFreed() {
    if(a_queues->hasNeed())
      return false;
    if(a_remoteRef) {
      if(!a_sentClearRef && (a_state==WORKING)) {
        MsgCnt *msg = new MsgCnt(C_CLEAR_REFERENCE, true);
        m_send(msg, MSG_PRIO_EAGER);
        a_sentClearRef=true;
      }
      return false;
    }
    if(a_localRef) {
      if(!a_sentClearRef && (a_state==WORKING)) {
        MsgCnt *msg = new MsgCnt(C_CLEAR_REFERENCE, true);
        m_send(msg, MSG_PRIO_EAGER);
        a_localRef=false;
        a_sentClearRef=true;
      }
    }
    switch(a_state) {
    case WORKING:
      m_WORKING_2_CLOSING_WEAK();
      return false;

    case OPENING_WF_PRESENT:
    case OPENING_WF_NEGOTIATE:
    case CLOSING_HARD:
    case CLOSING_WEAK:
    case CLOSING_WF_DISCONNECT:
    case ANONYMOUS_WF_NEGOTIATE:
    case ANONYMOUS_WF_PRESENT:
      return false;             // Wait for next gc.
    case CLOSED:
    case OPENING_WF_HANDOVER:
    case CLOSED_WF_REMOTE:
      clearTimers();
      return true;
    default:
      dssError("ComObject in unknown state at gc");
      return false;
    }
  }


  //ZACHARIAS: TODO, this one should terminate the connection
  inline void ComObj::errorRec(int mt) {
    printf("msgReceive: illegal message %s state %d", msl_mess_names[mt], a_state);
  }


  bool ComObj::msgReceived(MsgCnt *msgC) {
    a_mslEnv->a_RecCounter++;

    int mt=msgC->popIntVal();
    switch(mt) {
    case C_CSC:
    case C_APPLICATION:
      if(m_inState(WORKING | CLOSING_WEAK | CLOSING_HARD)) {//in is open
        ++a_lastReceived;
        a_remoteRef=true; // Implicitly known since he is sending a message!

        if (mt == C_APPLICATION)
          a_mslEnv->m_AppMessageReceived(msgC,a_site);
        else
          a_mslEnv->m_CscMessageReceived(msgC,a_site);

        if(!a_msgAckLength || // avoid division with zero
           a_lastReceived%a_msgAckLength==0)  // Time for explicit acknowledgement
          sendAckExplicit();
        else if(a_ackCanceled) // Want to keep the period
          setAckTimer();
      }
      else
        {
          printf("App message received in non working comObject state, silently discarded\n");
        }
      // raph: Don't return here: msgC must be freed from memory.
      // return true;
      break;
    case C_PING:
      if (m_inState(WORKING | CLOSING_WEAK | CLOSING_HARD)) { // open
        a_lastReceived++;
        a_remoteRef = true;
        sendAckExplicit();     // the remote site asked for it
      }
      break;
    case C_ACK: // Actually a dummy, since acknum was retrieved during unmarsh.
      break;
    case C_ANON_PRESENT:
      if(a_state== OPENING_WF_PRESENT && m_OPENING_WF_PRESENT_2_OPENING_WF_NEGOTIATE(msgC))
        break;
      else
        goto FAILURE;
    case C_INIT_PRESENT:
      if(a_state == ANONYMOUS_WF_PRESENT && m_ANONYMOUS_WF_PRESENT_2_ANONYMOUS_WF_NEGOTIATE(msgC))
        break;
      else
        goto CLOSE;
    case C_ANON_NEGOTIATE:
      if(a_state == OPENING_WF_NEGOTIATE && m_OPENING_WF_NEGOTIATE_2_WORKING(msgC))
        break;
      else
        goto FAILURE;
    case C_INIT_NEGOTIATE:
      if(a_state==ANONYMOUS_WF_NEGOTIATE && m_ANONYMOUS_WF_NEGOTIATE_2_WORKING(msgC))
        break;
      else
        goto CLOSE;
    case C_SET_ACK_PROP:
      {
        int time = msgC->popIntVal();
        int length = msgC->popIntVal();
        if(time<0 || length<0)
          errorRec(mt);
        else {
          if(!a_msgAckLength || // avoid division by zero
             a_lastReceived % a_msgAckLength < // Length since last ack
             length)
            sendAckExplicit();
          a_msgAckTimeOut=time;
          a_msgAckLength=length;
        }
      }
      break;
    case C_CLOSE_HARD:
      if(a_state == WORKING) {
        m_WORKING_2_CLOSING_WF_DISCONNECT();
      } else if(m_inState(CLOSING_HARD | CLOSING_WEAK)) {
        // This is as good confirmation as C_CLOSE_ACCEPT
        // If we are closing weak and the oponent hard, he must have gotten
        // a need since we decided to close...
        Assert(a_state==CLOSING_HARD||(a_state==CLOSING_WEAK && a_remoteRef));
        goto FAILURE_OPEN;
      } else
        errorRec(mt);
      break;
    case C_CLOSE_WEAK:
      // If this site still has a need, send close_reject else accept
      a_remoteRef = false;
      if(a_state==WORKING) {
        CState newstate;
        if(hasNeed()) {
          MsgCnt *newmsgC=new MsgCnt(C_CLOSE_REJECT, true);
          newstate=WORKING;
          m_send(newmsgC, MSG_PRIO_EAGER);
        } else {
          MsgCnt *newmsgC=new MsgCnt(C_CLOSE_ACCEPT, true);
          newstate=CLOSED;
          m_send(newmsgC, MSG_PRIO_EAGER);
        }
        clearTimers();
        m_setCState(newstate);  // State change may "close" outgoing channel.
      }
      break;
    case C_CLOSE_ACCEPT:
      if(m_inState(CLOSING_HARD))
        // This is as good confirmation as C_CLOSE_ACCEPT
        goto FAILURE_OPEN;
      if(m_inState(CLOSING_WEAK)) {
        m_CLOSING_WEAK_2_CLOSED();
        goto FAILURE;
      }
      errorRec(mt);
      break;
    case C_CLOSE_REJECT:
      if(m_inState(CLOSING_HARD | CLOSING_WEAK)) {
        a_remoteRef=true;
        //CLOSE TIMER REMOVED: e_timers->clearTimer(a_closetimer);
        m_setCState(WORKING);
        if(a_queues->hasQueued())
          a_transObj->deliver();
      } else
        errorRec(mt);
      break;
    case C_CLEAR_REFERENCE: // If this is what we waited for start to close...
      if(m_inState(WORKING | CLOSING_HARD | CLOSING_WEAK)) {//in is open
        a_remoteRef=false;
        if(a_state==WORKING && !hasNeed()) {
          m_WORKING_2_CLOSING_WEAK();
        }
      } else
        errorRec(mt);
      break;
    case C_ROUTE_DISCOVERY: // mayby part of the discovery protocol
      {
        //PstInContainerInterface *load = msgC->popPstVal();
        //a_mslEnv->a_comService->ext_receiveMsg(a_site->getCsSite(), load);
        Assert(0);
      }
      break;
    case C_DIRECT_MSG: // direct msg send between sites at Oz level
      {
        //PstInContainerInterface* load = msgC->popPstVal();
        //a_mslEnv->a_comService->ext_receiveMsg(a_site->m_getCsSite(), load);
        Assert(0);
      }
      break;
    case C_SET_ROUTE:
      {
        Site *srcSiteId = msgC->popSiteVal();
        Site *dstSiteId = msgC->popSiteVal();
        int routeId = msgC->popIntVal();
        int nrSites = msgC->popIntVal();

        //printf("receive C_SET_ROUTE\n");

        // the message has to be forwarded
        if (nrSites > 0) {
          //printf("forward C_SET_ROUTE\n");
          MsgCnt *newmsgC = new MsgCnt(C_SET_ROUTE, true);
          newmsgC->pushSiteVal(srcSiteId);
          newmsgC->pushSiteVal(dstSiteId);
          newmsgC->pushIntVal(routeId);
          nrSites = nrSites-1;
          newmsgC->pushIntVal(nrSites);

          Site *nxtSiteId = msgC->popSiteVal();
          a_mslEnv->a_interRouter->registerRoute(srcSiteId, dstSiteId, routeId, a_site, nxtSiteId);

          // copy the route path (less this site) to the msg to be forward
          for (int i=0; i < nrSites; i++) {
            newmsgC->pushSiteVal(msgC->popSiteVal());
          }
          (nxtSiteId->m_getComObj())->m_send(newmsgC,MSG_PRIO_EAGER);
        }
        // C_SET_ROUTE reached the end of the path
        else {
          //printf("target touched\n");
          TransObj *transObj = new EndRouter(a_mslEnv);
          ComObj* newcom =
            anonymousRouteEstablished(transObj, dstSiteId, a_site, routeId, a_mslEnv);

          ((a_mslEnv)->a_interRouter)->registerRoute(srcSiteId, dstSiteId, routeId, a_site, dstSiteId);

          MsgCnt *newmsgC = new MsgCnt(C_TARGET_TOUCHED, true);
          newmsgC->pushSiteVal(dstSiteId);  // src
          newmsgC->pushSiteVal(srcSiteId);  // dst
          newmsgC->pushIntVal(routeId);     // routeId
          m_send(newmsgC,MSG_PRIO_EAGER);
          //printf("send C_TARGET_TOUCHED\n");

          // initiate the Open connection protocol
          newcom->m_acceptAnonConnection(transObj);
        }
      }
      break;
    case C_TARGET_TOUCHED:
      {
        //printf("receive C_TARGET_TOUCHED\n");

        Site *srcSiteId = msgC->popSiteVal();
        Site *dstSiteId = msgC->popSiteVal();
        int routeId = msgC->popIntVal();

        Site *nxtSiteId =
          ((a_mslEnv)->a_interRouter)->getRouteSite(srcSiteId, dstSiteId, routeId);

        //printf("route id:%d found for src:%p dst:%p through nxt:%p\n", routeId, srcSiteId, dstSiteId, nxtSiteId);

        //!! for the moment, if no route is found, just 'skip'
        if (nxtSiteId == NULL)
          break;

        if (nxtSiteId == dstSiteId) {
          EndRouter *endRouter =
            static_cast<EndRouter *>((nxtSiteId->m_getComObj())->getTransObj());
          //printf("end target_touched\n");
          endRouter->routeSetUp(routeId);
          //((nxtSiteId->getThisComObj())->getTransObj())->routeSetUp(routeId);
        }
        else {
          //printf("forward C_TARGET_TOUCHED\n");
          MsgCnt *newmsgC = new MsgCnt(C_TARGET_TOUCHED, true);
          newmsgC->pushSiteVal(srcSiteId);  // src
          newmsgC->pushSiteVal(dstSiteId);  // dst
          newmsgC->pushIntVal(routeId);     // routeId
          (nxtSiteId->m_getComObj())->m_send(newmsgC,MSG_PRIO_EAGER);
        }
      }
      break;
    case C_ROUTE:
      {
        //printf("receive C_ROUTE\n");

        Site *srcSiteId       = msgC->popSiteVal();
        Site *dstSiteId       = msgC->popSiteVal();
        int routeId            = msgC->popIntVal();
        DssCompoundTerm *dct = static_cast<DssCompoundTerm*>(msgC->m_popDropVal()); // free from deletion
        DssSimpleDacDct *tdac = static_cast<DssSimpleDacDct*>(dct);
        Site *nxtSiteId =
          ((a_mslEnv)->a_interRouter)->getRouteSite(srcSiteId, dstSiteId, routeId);

        //printf("route id:%d found for src:%p dst:%p through nxt:%p\n", routeId, srcSiteId, dstSiteId, nxtSiteId);

        // if no route found, just 'skip'
        if (nxtSiteId == NULL)
          break;

        if (nxtSiteId == dstSiteId){
          EndRouter *endRouter =
            static_cast<EndRouter *>((nxtSiteId->m_getComObj())->getTransObj());
          endRouter->readHandler(tdac);
        }
        else {
          //printf ("forward C_ROUTE\n");
          MsgCnt *newmsgC = new MsgCnt(C_ROUTE, true);
          newmsgC->pushSiteVal(srcSiteId);  // src
          newmsgC->pushSiteVal(dstSiteId);  // dst
          newmsgC->pushIntVal(routeId);     // routeId
          newmsgC->pushDctVal(tdac);         // msg to be routed
          (nxtSiteId->m_getComObj())->m_send(newmsgC,MSG_PRIO_HIGH);
        }
      }
      break;
    default:
      //Shouldn't be here
      dssError("Illegal message type found - check buffer handling");
      break;
    }


    delete msgC; // A bit dangerous
    return true;
  FAILURE_OPEN:
    m_close();
    m_open();
  CLOSE:
  FAILURE:
    delete msgC; // A bit dangerous
    return false;
  }


  bool ComObj::m_merge(ComObj *old) {
    Assert(a_state == ANONYMOUS_WF_NEGOTIATE || a_state == ANONYMOUS_WF_PRESENT);
    switch(old->a_state) {
    case WORKING:
    case CLOSING_HARD: // Resources are poor, wait in line!
      return false;
    case OPENING_WF_PRESENT:
    case OPENING_WF_NEGOTIATE:
      if(*a_site < *(a_mslEnv->a_mySite))
        return false;
    case OPENING_WF_HANDOVER:
    case CLOSED_WF_REMOTE:
    case CLOSING_WF_DISCONNECT:
    case CLOSING_WEAK:
      old->m_close();
    case CLOSED:
      return true;
    default:
      dssError("Severe - connection in illegal state"); // memory problems....
      return false;
    }
  }

  void ComObj::msgAcked(int num) {
    int rtt = a_queues->msgAcked(num, false, a_probing && a_state==WORKING);
    if (rtt != -1) {
      a_lastrtt = rtt;
      a_site->m_getCsSite()->reportRTT(t_max(rtt, MSG_ACK_TIMEOUT));
    }
    if (a_probing && a_state==WORKING) {   // we are probing
      a_msgReceivedDuringProbeInterval = true;
    }
  }

  MsgCnt *
  ComObj::getNextMsgCnt(int &acknum) {
    MsgCnt *msgC=a_queues->getNext(a_state==WORKING);
    if(msgC!=NULL){ // ZACHARIAS: during profiling, check the common case and switch
      if(!msgC->checkFlag(MSG_HAS_MARSHALCONT) && !msgC->m_isInternalMsg()) {
        msgC->setMsgNum(NO_MSG_NUM);
      }

      if(a_probing && !msgC->m_isInternalMsg()) {
        msgC->setSendTime(e_timers->currTime());
        a_msgSentDuringProbeInterval = true;
      }
      if(a_state==WORKING && !a_ackCanceled) // Acknowledgement is being sent
        clearAckTimer();
      acknum=a_lastReceived;
    }

    return msgC; // returns the message or NULL if none found
  }

  void ComObj::msgSent(MsgCnt *msgC){
    if (msgC->m_isInternalMsg()) {
      delete msgC;
    }
    else{
      msgC->setMsgNum(++a_lastSent);
      a_queues->insertUnacked(msgC);
    }
  }

  void ComObj::msgPartlyReceived(MsgCnt *msgC) {
    if(msgC->getMsgNum()==NO_MSG_NUM)
      msgC->setMsgNum(++a_receivedLrgMsg);
    a_queues->putRec(msgC);
  }

  void ComObj::msgPartlySent(MsgCnt *msgC){
    if (msgC->getMsgNum() == NO_MSG_NUM) msgC->setMsgNum(++a_sentLrgMsg);
    a_queues->requeue(msgC);
  }

  MsgCnt *ComObj::getMsgCnt(){ return new MsgCnt(); }

  MsgCnt *ComObj::getMsgCnt(int num){ return a_queues->getRec(num);}

  bool ComObj::isConnected() {
    return (m_inState(OPENING_WF_PRESENT | OPENING_WF_NEGOTIATE | WORKING) &&
            a_transObj != NULL &&
            a_transObj->getTransportMedium() != TM_ROUTE);
  }

  int ComObj::getTransportMedium() {
    return (a_transObj != NULL) ? a_transObj->getTransportMedium() : -1;
  }

  void ComObj::connectionLost() {
    dssLog(DLL_ALL,"COMMUNICATION (%p): Connection lost, state=%d",this, a_state);
    switch(a_state) {
    case CLOSING_HARD:
    case OPENING_WF_PRESENT:
    case OPENING_WF_NEGOTIATE:
    case WORKING:
      m_close();
      m_open();
      break;
    case ANONYMOUS_WF_NEGOTIATE:
      m_close(); // We're connected to a Site..
      break;
    case ANONYMOUS_WF_PRESENT:
      delete this; // Anonymous, can do no more
      break;
    case CLOSING_WEAK: // In case the other guy still has need, let him worry
      m_CLOSING_WEAK_2_CLOSED();
      break;
    case CLOSING_WF_DISCONNECT: // Now the connection was closed as expected.
      m_CLOSING_WF_DISCONNECT_2_CLOSING_WF_REMOTE();
      break;
    case CLOSED: // We accepted gc initiated by him
      if(a_queues->hasNeed())
        m_open();
      break;
    case CLOSED_WF_REMOTE:
    default:
      dssError("COMMUNICATION (%p):An unknown connection was lost (%d %p)", this, a_state,a_transObj);
      break;
    }
  }

  void ComObj::installProbe(int maxrtt) {
    if (maxrtt > 0) {
      a_maxrtt = maxrtt;
      if (!a_probing) {
        setProbeIntervalTimer();
        a_probing = true;
      }
    }
  }

  int ComObj::getQueueStatus(){return a_queues->getQueueStatus();}

  void ComObj::m_makeGCpreps(){ a_queues->gcMsgCs(); }

  bool ComObj::hasQueued() { return a_queues->hasQueued(); }

  void ComObj::sendAckExplicit(){
    if(!a_queues->hasQueued()) // Otherwise an ack will go anyway
      m_send(new MsgCnt(C_ACK,true), MSG_PRIO_EAGER);

    clearAckTimer();
  }


  void ComObj::clearTimers() {
    //printf("clearTimers! %p, %p ,%p, %p\n", a_reopentimer, a_ackTimer ,a_probeIntervalTimer, a_probeFaultTimer );
    e_timers->clearTimer(a_reopentimer);

    // here we could destroy the expiration times too.
    a_ackCanceled = true;
    e_timers->clearTimer(a_ackTimer);

    a_probing = false;
    e_timers->clearTimer(a_probeIntervalTimer);
  }


  // ************************** COMMUNICATION BOOTSTRAP ***************************
  //
  // TODO: General for all connections are that they rely on the
  // marshaling structure which is not safe - we should unmarshal here
  // (in the methods) instead, from a specialized buffer which is
  // controlled and requires certain sizes to assume itself correct.
  //
  // An idead is to have a safe serializer/deserializer that only
  // takes a message type and dct_buffer of fixed sizes. If one set
  // the transport layer in 'upstart' mode it should unmarshal using
  // this safer routine. We can be assure of that we have enough data,
  // just not whether it is ok or not.
  //
  // The

  inline DssSimpleReadBuffer*
  verifyEncryptedMessage(Site* enc_site, MsgCnt* msgC){
    if(msgC->m_getFT() == FT_DCT) { // continue
      DssCompoundTerm *dct = static_cast<DssCompoundTerm*>(msgC->popDctVal());
      DssSimpleDacDct* dsrd = dynamic_cast<DssSimpleDacDct*>(dct);
      // should check that msgC is empty now...

      if (dsrd != NULL)
        return enc_site->m_decrypt(dsrd);
    }
    return NULL;
  }

  inline void ComObj::createCI(DssSimpleWriteBuffer* dswb, int bufferSize) {
    int time,length;
    time=MSG_ACK_TIMEOUT;
    length=MSG_ACK_LENGTH;
    dswb->m_putInt(a_lastReceived);
    dswb->m_putInt(time);
    dswb->m_putInt(length);
    dswb->m_putInt(bufferSize);
    dswb->putByte(hasNeed());
  }

  inline bool ComObj::extractCI(DssSimpleReadBuffer* dsrb,int& bufferSize) {
    Assert(dsrb->availableData() == 17);
    if(dsrb->availableData() == 17){
      int remLastReceived = dsrb->m_getInt();
      a_msgAckTimeOut     = dsrb->m_getInt();
      a_msgAckLength      = dsrb->m_getInt();
      bufferSize          = dsrb->m_getInt();
      //    Assert(a_bufferSize==a_transObj->getBufferSize());

      a_remoteRef=((dsrb->getByte()) != 0);

      a_queues->msgAcked(remLastReceived,true,false);      // Resend/Ack msgs
      a_queues->clearRec();                                // Clear partly received
      a_queues->clearCont();
      a_sentLrgMsg=0;
      a_receivedLrgMsg=0;

      a_lastSent=remLastReceived; // Others requeued
      return true;
    } else
      return false;
  }



  //
  // *************** CONNECTING SIDE - THE INITIATING *****************
  //

  // Pure channel allocation - only thing we can be sure of is that if
  // someone answer the adress leaads somewhere i.e. sure of nothing
  void ComObj::m_CLOSED_2_OPENING_WF_HANDOVER(){
    Assert(a_state==CLOSED);
    a_closeHardFlag = false;
    m_setCState(OPENING_WF_HANDOVER);
    a_closeHardFlag = false;
    DssChannel *tc;
    tc = (a_site->m_getCsSite())->establishConnection();
    if (tc)
      m_OPENING_WF_HANDOVER_2_OPENING_WF_PRESENT(tc);
  }


  // The channel is allocated -
  // 1) create a transportation layer for the channel
  // 2) assume it is the site we want to talk to, if not we will
  //    detect that later (presentation phae)
  void ComObj::m_OPENING_WF_HANDOVER_2_OPENING_WF_PRESENT(DssChannel* tc){
    Assert(a_state == OPENING_WF_HANDOVER);


    // Check the typr of the channel...
    // Choose a transport object that can handle the
    // type of channel, voila TCPtransobj can do that!

    //switch(tc->getType())
    //case VC_STREAM:
    {
      TCPTransObj *tcpT = new TCPTransObj(a_mslEnv);
      tcpT->setChannel(tc);
      a_transObj = tcpT;
    }
    // case VC_FIFO_PACKET:
    // case VC_UNRELIABLE_PACKET:

    a_transObj->setOwner(this);
    m_setCState(OPENING_WF_PRESENT);
    a_transObj->readyToReceive();
  }


  bool ComObj::m_OPENING_WF_PRESENT_2_OPENING_WF_NEGOTIATE(MsgCnt *msgC){
    //printf("m_OPENING_WF_PRESENT_2_OPENING_WF_NEGOTIATE\n");
    Assert(a_state==OPENING_WF_PRESENT);
    //Assert(0);
    DssSimpleReadBuffer* dsrb = verifyEncryptedMessage(a_site, msgC);

    if(dsrb != NULL && dsrb->availableData() == 7){
      BYTE version2[3];
      dsrb->m_readOutBuffer(version2,3);
      u32 ticket = dsrb->m_getInt();
      Assert(a_sec.a_ticket == 0);

      if(strncmp((char*) version2, (char*) dss_version, 3) == 0){ // ok send our data

        m_setCState(OPENING_WF_NEGOTIATE);

        DssSimpleWriteBuffer dswb(new BYTE[256], 256);

        // *********** COLLECT INFO *********
        dswb.writeToBuffer(dss_version,3); // 3
        // site + site-version number
        a_mslEnv->a_mySite->m_marshalDSite(&dswb); // X size

        // For the other side to verify that we are the owner of the key pair
        DssSimpleWriteBuffer dswb2(new BYTE[96], 96); // 49 || 9 + 17 (CI)
        dswb2.m_putInt(ticket);                          // 4
        dswb2.m_putInt(a_sec.a_ticket);                  // 4

        Assert(a_site != NULL);
        if(a_site->m_useSecureChannel()){
          dswb2.putByte(true);                           // 1
          generate_garbage(a_sec.a_key,32);
          a_sec.a_iv1 = random_u32();
          a_sec.a_iv2 = random_u32();

          dswb2.writeToBuffer(a_sec.a_key,32); // 32
          dswb2.m_putInt(a_sec.a_iv1);      // 4
          dswb2.m_putInt(a_sec.a_iv2);      // 4
          //printf("INITIATING SECURE MODE\n");
        } else {
          dswb2.putByte(false);
          //printf("INITIATING UNSECURE MODE\n");
        }
        createCI(&dswb2,a_transObj->getBufferSize());
        int   inLen = dswb2.getUsed();
        Assert(inLen < 96);
        BYTE* plain = dswb2.unhook();
        int retLen; BYTE* cipher;

        a_mslEnv->a_mySite->m_encrypt(retLen,cipher,inLen,plain);
        dswb.m_putInt(retLen);                           // 4
        dswb.writeToBuffer(cipher,retLen);               // Y
        delete [] plain;
        delete [] cipher;
        // ********** INFO COLLECTED ********

        Assert(dswb.getUsed() < 256);
        MsgCnt *newmsgC  = new MsgCnt(C_INIT_PRESENT, true);
        newmsgC->pushDctVal(a_site->m_encrypt(&dswb));

        m_send(newmsgC,MSG_PRIO_EAGER);
        delete dsrb;
        return true;
      }
    }
    dssLog(DLL_BEHAVIOR,"comObj found a non-correct buffer and closes the connection");
    m_closeErroneousConnection();
    delete dsrb;
    return false;
  }



  // Negotiation done - the other side accept us.
  bool ComObj::m_OPENING_WF_NEGOTIATE_2_WORKING(MsgCnt* msg){
    //printf("m_OPENING_WF_NEGOTIATE_2_WORKING\n");
    int bufferSize;
    u32 ticket;

    DssSimpleReadBuffer* dsrb = verifyEncryptedMessage(a_mslEnv->a_mySite, msg);

    if(dsrb != NULL && dsrb->availableData() == 25){

      ticket = dsrb->m_getInt();
      if(ticket == a_sec.a_ticket){

        ticket = dsrb->m_getInt();
        a_sec.a_ticket = random_u32();

        if(extractCI(dsrb,bufferSize)){
          //      if (bufferSize<myBufferSize) // Shrink

          if(!a_closeHardFlag){
            if(a_site->m_useSecureChannel()){
              a_transObj->m_EncryptReadTransport( a_sec.a_key, 32, a_sec.a_iv1, a_sec.a_iv2);
              a_transObj->m_EncryptWriteTransport(a_sec.a_key, 32, a_sec.a_iv1, a_sec.a_iv2);
            }
            DssSimpleWriteBuffer dswb(new BYTE[32], 32);
            MsgCnt* newmsgC = new MsgCnt(C_INIT_NEGOTIATE, true);
            dswb.m_putInt(ticket);
            dswb.m_putInt(a_sec.a_ticket);
            newmsgC->pushDctVal(a_site->m_encrypt(&dswb));
            m_send(newmsgC,MSG_PRIO_EAGER);
            m_setCState(WORKING);
          } else {
            m_setCState(WORKING);
            m_WORKING_2_CLOSING_HARD();
          }

          delete dsrb;
          return true;
        }
      }
    }
    m_closeErroneousConnection();
    delete dsrb;
    return false;
  }


  // *************** ANONYMOUS SIDE - THE RESPONDING *****************

  // A connection has been received, tell them who we are and our
  // security requirements.

  // store:

  // ticket for this session "respond_ticket", a so called nouns or
  // capability

  // - proposed security parameters: no-encrypt | encrypt + key

  // TODO: when closing an anonymous connection make sure that comobj removes itself

  void ComObj::m_CLOSED_2_ANONYMOUS_WF_PRESENT(TransObj *transObj){
    //printf("m_CLOSED_2_ANONYMOUS_WF_PRESENT\n");
    Assert(a_state == CLOSED);
    a_transObj=transObj;

    m_setCState(ANONYMOUS_WF_PRESENT);

    MsgCnt *msgC= new MsgCnt(C_ANON_PRESENT, true);
    DssSimpleWriteBuffer* dswb = new DssSimpleWriteBuffer(new BYTE[32], 32);

    // *********** COLLECT INFO *********

    // DSS version
    dswb->writeToBuffer(dss_version,3); // 3

    // crypto settings
    Assert(a_sec.a_ticket == 0);
    a_sec.a_ticket = random_u32();

    dswb->m_putInt(a_sec.a_ticket);
    // ********** INFO COLLECTED ********
    msgC->pushDctVal(a_mslEnv->a_mySite->m_encrypt(dswb));

    m_send(msgC, MSG_PRIO_EAGER);
    transObj->readyToReceive();
    delete dswb;
  }



  bool ComObj::m_ANONYMOUS_WF_PRESENT_2_ANONYMOUS_WF_NEGOTIATE(MsgCnt *msg){
    //printf("m_ANONYMOUS_WF_PRESENT_2_ANONYMOUS_WF_NEGOTIATE\n");
    Assert(a_state == ANONYMOUS_WF_PRESENT);

    // HUGE stack
    int retLen, len, bufferSize;
    u32 ticket;
    BYTE *plain, *cipher, version2[3];
    bool sec;
    ComObj* other;
    Site*  s1;

    MsgCnt *newmsgC;

    DssSimpleReadBuffer *dsrb = verifyEncryptedMessage(a_mslEnv->a_mySite,msg);
    DssSimpleReadBuffer  dsrb2;

    if(dsrb != NULL && dsrb->canRead(1+3)){
      dsrb->m_readOutBuffer(version2,3);

      if(strncmp((char*) version2, (char*) dss_version, 3) == 0 &&
         (dsrb->canRead(1+4)) && // SIZE OF DSITE
         ((s1 = a_mslEnv->a_siteHT->m_unmarshalSite(dsrb)) != NULL) &&
         (s1->m_getFaultState() != FS_GLOBAL_PERM)
         ){
        len = dsrb->m_getInt();
        if(dsrb->availableData() == len){
          cipher = new BYTE[len];
          dsrb->readFromBuffer(cipher,len);
          dsrb->commitRead(len);

          if(s1->m_decrypt(retLen, plain, len, cipher)){ // if the data checks out when decrypted
            dsrb2.hook(plain, retLen);
            if(dsrb2.canRead(9)){ // tickets + sec type
              ticket = dsrb2.m_getInt();
              if(ticket == a_sec.a_ticket){
                ticket = dsrb2.m_getInt(); // other side
                a_sec.a_ticket = random_u32();
                sec = dsrb2.getByte();
                if (sec){
                  if(dsrb2.canRead(40)){
                    dsrb2.m_readOutBuffer(a_sec.a_key,32);
                    a_sec.a_iv1  = dsrb2.m_getInt();
                    a_sec.a_iv2  = dsrb2.m_getInt();
                  } else
                    {
                      goto FAILURE;
                    }
                }
                // TODO: ON ELSE WE SHOULD DECIDE

                other  = s1->m_getComObj();
                a_site = s1;

                if((other != NULL) && (a_transObj->getTransportMedium() != TM_ROUTE)){
                  // This object is an anonymous object and the other
                  // is to be used. What connection to cut needs to
                  // be decided on.
                  if(m_merge(other)) {
                    // Force close of other, get his queue
                    t_swap((other->a_queues), a_queues);
                    other->m_close();
                  } else {
                    // Not the same as failure. Failure can have other
                    // implications. Zacharias, I close the connection by
                    // m_close, have a look. Erik
                    delete dsrb; // Z_CHANGE
                    m_close();
                    return false;
                  }
                }

                // run with this connection
                if(extractCI(&dsrb2,bufferSize)){
                  // NORMAL CASE
                  a_site->m_setComObj(this);
                  DssSimpleWriteBuffer dswb(new BYTE[64], 64);
                  dswb.m_putInt(ticket);
                  dswb.m_putInt(a_sec.a_ticket); // TODO: last ticket for now
                  createCI(&dswb,  bufferSize);

                  newmsgC = new MsgCnt(C_ANON_NEGOTIATE, true);
                  newmsgC->pushDctVal(a_site->m_encrypt(&dswb));
                  m_send(newmsgC, MSG_PRIO_EAGER);
                  m_setCState(ANONYMOUS_WF_NEGOTIATE);
                  if(sec){
                    a_site->m_useSecureChannel(true);
                    a_transObj->m_EncryptReadTransport(a_sec.a_key, 32, a_sec.a_iv1, a_sec.a_iv2);
                  } else {
                    a_site->m_useSecureChannel(false);
                  }
                  delete dsrb;
                  return true;
                }
                else {
                  ;
                }
              }
            }
          }
        }
      }
    }
 FAILURE:
    delete dsrb;
    m_closeErroneousConnection();
    // TODO: close everything
    return false;
  }


  bool ComObj::m_ANONYMOUS_WF_NEGOTIATE_2_WORKING(MsgCnt *msg){
    //printf("m_ANONYMOUS_WF_NEGOTIATE_2_WORKING\n");
    DssSimpleReadBuffer* dsrb = verifyEncryptedMessage(a_mslEnv->a_mySite, msg);

    if(dsrb != NULL && dsrb->availableData() == 8){

      u32 ticket = dsrb->m_getInt();
      if(ticket == a_sec.a_ticket){
        a_sec.a_ticket = dsrb->m_getInt();
        m_setCState(WORKING);
        a_transObj->readyToReceive();

        if(a_site->m_useSecureChannel())
          a_transObj->m_EncryptWriteTransport(a_sec.a_key, 32, a_sec.a_iv1, a_sec.a_iv2);
        delete dsrb;

        // We can have queued items if a merge happened.
        // The comObject opened from this process can have had
        // messages that where transfered to the anonymous object
        // (this object...).
        if(a_queues->hasQueued())
          a_transObj->deliver();
        return true;
      }
    }
    m_closeErroneousConnection();
    delete dsrb;
    return false;
  }

  void ComObj::m_WORKING_2_CLOSING_HARD(){
    Assert(m_inState(WORKING));
    clearTimers(); // And set a closing timer below,  ZACHARIAS: SEE no timer??
    m_send(new MsgCnt(C_CLOSE_HARD, true), MSG_PRIO_EAGER);
    m_setCState(CLOSING_HARD);
  }

  void ComObj::m_WORKING_2_CLOSING_WEAK(){
    Assert(m_inState(WORKING));
    clearTimers();
    m_send(new MsgCnt(C_CLOSE_WEAK, true), MSG_PRIO_EAGER);
    m_setCState(CLOSING_WEAK); // State change "closes" outgoing channel
  }


  void ComObj::m_WORKING_2_CLOSING_WF_DISCONNECT(){
    Assert(m_inState(WORKING));
    m_send(new MsgCnt(C_CLOSE_ACCEPT, true), MSG_PRIO_EAGER);
    m_setCState(CLOSING_WF_DISCONNECT);
  }
  //

  void ComObj::m_CLOSING_WEAK_2_CLOSED(){
    m_close();
    if(a_queues->hasNeed())
      m_open();
  }

  void ComObj::m_CLOSING_WF_DISCONNECT_2_CLOSING_WF_REMOTE(){
    m_close();
    m_setCState(CLOSED_WF_REMOTE);
    dssLog(DLL_ALL,"COMMUNICATION (%p): Closed remote, setting timer", this);
    e_timers->setTimer(a_reopentimer,
                       (a_mslEnv->a_ipIsbehindFW)?a_mslEnv->m_getFirewallReopenTimeout():a_mslEnv->m_getReopenRemoteTimeout(),
                       if_comObj_reopen, this);
  }

} //End namespace
