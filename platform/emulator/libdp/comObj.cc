#include "comObj.hh"
#include "msgContainer.hh"
#include "transObj.hh"
#include "perdio.hh"
#include "connection.hh"
#include "timers.hh"

#define OPEN_TIMEOUT PERDIO_TIMEOUT
#define CLOSE_TIMEOUT PERDIO_TIMEOUT
#define WF_REMOTE_TIMEOUT PERDIO_TIMEOUT*10

#define MSG_ACK_TIMEOUT 1000
#define MSG_ACK_LENGTH 50

//#define COMOBJ_LOG
//#define COMOBJ_CONNECT_LOG

ComObj::ComObj(DSite *site, int recCtr) {
  init(site, recCtr);
}

void ComObj::init(DSite *site, int recCtr) {
  this->site = site;
  this->transObj = NULL;

  localRef=FALSE; // Will be set true by the first send and false by gc
  remoteRef=FALSE;
  sentclearref=FALSE;
  state=CLOSED;

  lastReceived=0;
  lastSent=0;
  timer=NULL;
  reopentimer=NULL;
  closetimer=NULL;

  probing=FALSE;
  probeFired=FALSE;
  probeIntervalTimer=NULL;
  probeFaultTimer=NULL;

  retryTimeout=ozconf.perdioTempRetryFloor;

  nosm=norm=0;

  queues.init();
  infoelement=(void *) 0x0;
}

void ComObj::send(MsgContainer *msgC,int priority) {
#ifdef COMOBJ_LOG
  printf("send(%s %d %d %d %d)\n",
         mess_names[msgC->getMessageType()],
         myDSite->getTimeStamp()->pid,
         site!=NULL?site->getTimeStamp()->pid:0,
         msgC->getMsgNum(),
         (int) am.getEmulatorClock());
#endif

  PD((TCP_INTERFACE,"---send: %s fr %d to %d",
      mess_names[msgC->getMessageType()],myDSite->getTimeStamp()->pid,
      site!=NULL?site->getTimeStamp()->pid:0-1));

  nosm++;
  if(msgC->getMessageType()<C_FIRST && !localRef) {
    localRef=TRUE;
    sentclearref=FALSE;
  }
  switch(state) {
  case WORKING:
  case ANONYMOUS_WF_NEGOTIATE:
  case OPENING_WF_NEGOTIATE_ANS:
    if(state!=WORKING || !queues.hasQueued())
      transObj->deliver();
    break;
  case CLOSED:
    open();
    break;
  default: // Eventually we will have a channel...
    break;
  }

  queues.enqueue(msgC,priority); // After check queues.hasQueued()
}

Bool comObj_openTimerExpired(void *comObj) {
  return ((ComObj *) comObj)->openTimerExpired();
}

Bool comObj_closeTimerExpired(void *comObj) {
  return ((ComObj *) comObj)->closeTimerExpired();
}

Bool comObj_sendProbePing(void *comObj) {
  return ((ComObj *) comObj)->sendProbePing();
}

Bool comObj_probeFault(void *comObj) {
  return ((ComObj *) comObj)->probeFault();
}

Bool comObj_sendAck(void *comObj) {
  return ((ComObj *) comObj)->sendAck(TRUE);
}

Bool comObj_reopen(void *comObj) {
  return ((ComObj *) comObj)->reopen();
}

void ComObj::open() {
  if(transObj==NULL) { // Otherwise we are not quite closed yet
    Assert(site!=NULL);
    PD((TCP_INTERFACE,"***** Connection to %d (%x) initiated *****",
        site->getTimeStamp()->pid,this));
    timers->setTimer(timer,OPEN_TIMEOUT,
                     comObj_openTimerExpired,(void *) this);
    state=CLOSED_WF_HANDOVER;
    doConnect(this);
  }
}

Bool ComObj::reopen() {
  if(state==CLOSED_PROBLEM ||                     // Problem
     state==CLOSED_WF_REMOTE) {                   // Tired of waiting for rem.
    Assert(!site->isPerm());
    Assert(hasNeed() || remoteRef);
    Assert(transObj==NULL);
    retryTimeout=(int) (((double) retryTimeout) *
                        ((100.0+ozconf.perdioTempRetryFactor)/100.0));
    if(/*hasBeenConnected && To be added...AN*/
       retryTimeout>ozconf.perdioTempRetryCeiling)
      retryTimeout=ozconf.perdioTempRetryCeiling;
    open();

    return TRUE;   // Cannot know yet if the problem is resolved or not
  }
  else {
    reopentimer=NULL;
    return FALSE;  // The problem has been resolved
  }
}

// Called by builtin when this comObj can have its communication
// The returnvalue indicates whether it is wanted or not
Bool ComObj::handover(TransObj *transObj) {
  PD((TCP_INTERFACE,"Connection handover (from %d to %d (%x))",
      myDSite->getTimeStamp()->pid,site->getTimeStamp()->pid,this));
#ifdef COMOBJ_CONNECT_LOG
  printf("handover(%d %d)\n",
         site!=NULL?site->getTimeStamp()->pid:0,
         (int) am.getEmulatorClock());
#endif
  if(state!=CLOSED_WF_HANDOVER) {
    return FALSE;
  }

  state=OPENING_WF_PRESENT;
  this->transObj=transObj;

  if(probing && !probeFired) {
    timers->setTimer(probeFaultTimer,maxrtt,
                     comObj_probeFault,(void *) this);
    timers->setTimer(probeIntervalTimer,probeinterval,
                     comObj_sendProbePing,(void *) this);
  }
  transObj->readyToReceive();
  return TRUE;
}

Bool ComObj::openTimerExpired() {
  PD((TCP_INTERFACE,"openTimerExpired at %d %x",am.getEmulatorClock(),this));
  if(state==CLOSED_WF_HANDOVER ||
     state==OPENING_WF_PRESENT || state==OPENING_WF_NEGOTIATE_ANS) {
    if(hasNeed() || remoteRef) {
      close(CLOSED_PROBLEM);
      timer=NULL;
      return FALSE;
    }
    else {
      PD((TCP_INTERFACE,"opentimerexpired with no need or remoteref %x",this));
      close(CLOSED);
    }
  }
  else if(state==ANONYMOUS_WF_NEGOTIATE) {// Never a problem, anonymous
    close(CLOSED,TRUE);//Use close with true not to be reopened
printf("Anonymous opentimerexpired\n");
    comController->deleteComObj(this);
    return FALSE;
  }
  timer=NULL;
  return FALSE;
}

Bool ComObj::closeTimerExpired() {
  printf("closeTimerExpired state %d\n",state);
  switch(state) {
  case CLOSING_HARD:
    // Close violently, hand back resource and wait in line.
    close(CLOSED_WF_HANDOVER);
    break;
  case CLOSING_WEAK:
    if(hasNeed()) // Weird state that could happen if need has arosen
      close(CLOSED_WF_HANDOVER);
    else          // Close violently, if other site has arosen need he can open
      close(CLOSED);
    break;
  default:
    Assert(0);
  }
  closetimer=NULL;
  return FALSE;
}

void ComObj::accept(TransObj *transObj) {
  PD((TCP_INTERFACE,"***** Connection accepted by %d (%x) *****",
      myDSite->getTimeStamp()->pid,this));
  this->transObj=transObj;
  transObj->setOwner(this);
  transObj->getTransController()->addRunning(this);
  state=ANONYMOUS_WF_NEGOTIATE;
  timers->setTimer(timer,OPEN_TIMEOUT,
                   comObj_openTimerExpired,(void *) this);

  MsgContainer *msgC=msgContainerManager->newMsgContainer(NULL);
  msgC->put_C_PRESENT(PERDIOVERSION,myDSite);
  send(msgC,5);
  transObj->readyToReceive();
}

void ComObj::close(CState statetobe) {
  close(statetobe, FALSE);
}

void ComObj::close(CState statetobe,Bool merging) {
  PD((TCPCACHE,"---Closing a connection at %d (comObj %x) from state %d to %d",
      myDSite->getTimeStamp()->pid,this,state,statetobe));
//    printf("---Closing a connection at %d (comObj %x) from state %d to %d\n",
//       myDSite->getTimeStamp()->pid,this,state,statetobe);

  clearTimers();

  if(transObj!=NULL) {
#ifdef COMOBJ_CONNECT_LOG
    printf("close(%d %d)\n",
           site!=NULL?site->getTimeStamp()->pid:0,
           (int) am.getEmulatorClock());
#endif
    handback(this,transObj);
    transObj=NULL;
  }
  else if(state==CLOSED_WF_HANDOVER &&
          (statetobe==CLOSED || statetobe==CLOSED_PROBLEM))
    // No transObj but yet expecting one => cancel
    comObjDone(this);
  queues.clear5();

  switch(statetobe) {
  case CLOSED_WF_HANDOVER:
    //    state=CLOSED_WF_HANDOVER; happens anyway!
    open();
    break;
  case CLOSED_PROBLEM:
    state=CLOSED_PROBLEM;
    timers->setTimer(reopentimer,retryTimeout,
                     comObj_reopen,(void *) this);
    PD((TCP_INTERFACE,"problem timer set at %d to %d",
        am.getEmulatorClock(),retryTimeout));
    break;
  case CLOSED:
    // It could be that a new message has been "sent" while we or other were
    // closing. In that case reopen.
    if(!merging &&
       (state==CLOSING_WEAK || state==CLOSED) &&
       queues.hasNeed()) {
      open(); // => state=CLOSED_WF_HANDOVER
      break;
    }
    else
      state=CLOSED;
    break;
  default:
    state=statetobe;
  }
}

void ComObj::preemptTransObj() {
  Assert(state==WORKING);
  clearTimers(); // And set a closing timer... AN
  MsgContainer *msgC=msgContainerManager->newMsgContainer(NULL);
  msgC->put_C_CLOSE_HARD();
  send(msgC,5);
  state=CLOSING_HARD; // State change "closes" outgoing channel, do after send
  timers->setTimer(closetimer,CLOSE_TIMEOUT,
                   comObj_closeTimerExpired,(void *) this);
}

//  Bool ComObj::canBeFreed() {
//    localRef=FALSE;
//    if(!hasNeed()) {
//      if(transObj!=NULL) {
//        if(state!=CLOSED) {
//      // It is important at exit (dpExit) not to send too many clearrefs.
//      if(!sentclearref) {
//        MsgContainer *msgC=msgContainerManager->newMsgContainer(NULL);
//        msgC->put_C_CLEAR_REFERENCE();
//        send(msgC,4); // 4 avoids sending when not working
//        sentclearref=TRUE;
//      }
//      if(!remoteRef && state==WORKING) {
//        MsgContainer *msgC2=msgContainerManager->newMsgContainer(NULL);
//        msgC2->put_C_CLOSE_WEAK();
//        send(msgC2,5);
//        state=CLOSING_WEAK; // State change "closes" outgoing channel do
//                            // after send.
//        timers->setTimer(closetimer,CLOSE_TIMEOUT,
//                         comObj_closeTimerExpired,(void *) this);
//      }
//      return FALSE;        // Will hopefully be gc:ed next time!
//        }
//        else
//      return FALSE;
//      }
//      else {
//        // Can we be sure to be removed now?
//        // If so, do clear timers:
//        clearTimers();
//  printf("pid %d canBeFreed by pid %d\n",site->getTimeStamp()->pid,myDSite->getTimeStamp()->pid);
//        return TRUE;
//      }
//    }
//    else
//      return FALSE;
//  }

Bool ComObj::canBeFreed() {
  localRef=FALSE;
  if(hasNeed())
    return FALSE;
  else {
    // It is important at exit (dpExit) not to send too many clearrefs.
    if(!sentclearref) {
      MsgContainer *msgC=msgContainerManager->newMsgContainer(NULL);
      msgC->put_C_CLEAR_REFERENCE();
      send(msgC,4); // 4 avoids sending when not working
      sentclearref=TRUE;
    }
    if(remoteRef)
      return FALSE;
    else {
      switch(state) {
      case WORKING: {
        clearTimers();
        MsgContainer *msgC2=msgContainerManager->newMsgContainer(NULL);
        msgC2->put_C_CLOSE_WEAK();
        send(msgC2,5);
        state=CLOSING_WEAK; // State change "closes" outgoing channel do
                            // after send.
        timers->setTimer(closetimer,CLOSE_TIMEOUT,
                         comObj_closeTimerExpired,(void *) this);
        return FALSE;
      }
      case OPENING_WF_PRESENT:
      case OPENING_WF_NEGOTIATE_ANS:
      case CLOSING_HARD:
      case CLOSING_WEAK:
      case CLOSING_WF_DISCONNECT:
      case ANONYMOUS_WF_NEGOTIATE:
        return FALSE;             // Wait for next gc.
      case CLOSED:
      case CLOSED_WF_HANDOVER:
      case CLOSED_WF_REMOTE:
      case CLOSED_PROBLEM:
        clearTimers();
        return TRUE;
      default:
        OZ_error("ComObject in unknown state at gc");
        return FALSE;
      }
    }
  }
}

void ComObj::errorRec(int mt) {
  OZ_error("msgReceive: illegal message %s received by %d in state %d from %d\n",
           mess_names[mt],myDSite->getTimeStamp()->pid,
           state,site!=NULL?site->getTimeStamp()->pid:0);
}

Bool ComObj::hasNeed() {
  return localRef || queues.hasNeed();
}

inline OZ_Term ComObj::createCI(int bufferSize) {
  int time,length;
  if(probing) // should be also when closing down!
    time=length=0;
  else {
    time=MSG_ACK_TIMEOUT;
    length=MSG_ACK_LENGTH;
  }
  return OZ_recordInit(oz_atom("channelinfo"),
                       oz_cons(oz_pairAI("lastReceived",lastReceived),
                         oz_cons(oz_pairAI("msgAckTimeOut",time),
                           oz_cons(oz_pairAI("msgAckLength",length),
                             oz_cons(oz_pairAI("bufferSize",bufferSize),
                                oz_cons(oz_pairAI("hasNeed",hasNeed()),
                                        oz_nil()))))));
}

inline void ComObj::extractCI(OZ_Term channelinfo,int &bufferSize) {
  int index;
  int remLastReceived;

  if (oz_isRecord(channelinfo)) {
    SRecord *srec = tagged2SRecord(channelinfo);

    index=srec->getIndex(oz_atom("lastReceived"));
    remLastReceived=oz_intToC(srec->getArg(index));
    queues.msgAcked(remLastReceived,TRUE,FALSE);      // Resend/Ack msgs
    lastSent=remLastReceived; // Others requeued

    index=srec->getIndex(oz_atom("msgAckTimeOut"));
    msgAckTimeOut=oz_intToC(srec->getArg(index));

    index=srec->getIndex(oz_atom("msgAckLength"));
    msgAckLength=oz_intToC(srec->getArg(index));

    index=srec->getIndex(oz_atom("bufferSize"));
    bufferSize=oz_intToC(srec->getArg(index));
    Assert(bufferSize==transObj->getBufferSize());

    index=srec->getIndex(oz_atom("hasNeed"));
    remoteRef=oz_intToC(srec->getArg(index));
  }
}

Bool ComObj::msgReceived(MsgContainer *msgC) {
#ifdef COMOBJ_LOG
  printf("received(%s %d %d %d %d)\n",
         mess_names[msgC->getMessageType()],
         myDSite->getTimeStamp()->pid,
         site!=NULL?site->getTimeStamp()->pid:0,
         msgC->getMessageType()<C_FIRST?lastReceived+1:0,
         (int) am.getEmulatorClock());
#endif
  PD((TCP_INTERFACE,"---msgReceived: %s nr:%d from %d",
      mess_names[msgC->getMessageType()],lastReceived+1,
      site!=NULL?site->getTimeStamp()->pid:-1));

  norm++;

  MessageType mt=msgC->getMessageType();
  switch(mt) {
  case C_PRESENT:
    if(state==OPENING_WF_PRESENT) {
      char *version;
      DSite *s1;

      msgC->get_C_PRESENT(version,s1);

      if(strcmp(version,PERDIOVERSION)!=0 || s1!=site) {
        msgContainerManager->deleteMsgContainer(msgC);
        site->discoveryPerm();
        site->probeFault(PROBE_PERM);
        return FALSE;
      }
      else {
        state=OPENING_WF_NEGOTIATE_ANS;
        MsgContainer *newmsgC=msgContainerManager->newMsgContainer(NULL);
        OZ_Term channelinfo;
        channelinfo=createCI(transObj->getBufferSize());
        newmsgC->put_C_NEGOTIATE(PERDIOVERSION,myDSite,channelinfo);
        send(newmsgC,5);
      }
    }
    else errorRec(mt);
    break;
  case C_NEGOTIATE:
    if(state==ANONYMOUS_WF_NEGOTIATE) {
      char *version;
      DSite *s1;
      OZ_Term channelinfo;
      msgC->get_C_NEGOTIATE(version,s1,channelinfo);
      site=s1;
      transObj->setSite(site);
#ifdef COMOBJ_CONNECT_LOG
      printf("accept(%d %d)\n",
             site!=NULL?site->getTimeStamp()->pid:0,
             (int) am.getEmulatorClock());
#endif

      if(strcmp(version,PERDIOVERSION)!=0) {
        msgContainerManager->deleteMsgContainer(msgC);
        site->discoveryPerm();
        site->probeFault(PROBE_PERM);
        return FALSE;
      }
      else {
        // Tell the site about this comObj
        ComObj *other=s1->setComObj(this);
        if (other!=NULL) {
          // This object is an anonymous object and the other
          // is to be used. What transObj to use needs to be
          // decided on.
          PD((TCP_INTERFACE,"Other comObj found in state %d",other->state));
          merge(other,this,channelinfo);
          msgContainerManager->deleteMsgContainer(msgC);
          comController->deleteComObj(this); // Is this consistent? AN
          return TRUE; // !This object may not do anything more, but the trO!
        }
        adoptCI(channelinfo);
        state=WORKING;
        // Whether the channel is ok now or not is to be decided by
        // the future fault-model. For now it has to be for ports etc
        // to start working after temp&resource preemption.
        if(probing) {
          timers->setTimer(probeFaultTimer,maxrtt,
                           comObj_probeFault,(void *) this);
          timers->setTimer(probeIntervalTimer,probeinterval,
                           comObj_sendProbePing,(void *) this);
          if(probeFired) {
//          printf("probeOK (f %d t %d)\n",myDSite->getTimeStamp()->pid,
//                 site!=NULL?site->getTimeStamp()->pid:0-1);
            site->probeFault(PROBE_OK);
            probeFired=FALSE;
          }
        }
        timers->clearTimer(timer);
      }
    }
    else errorRec(mt);
    break;
  case C_NEGOTIATE_ANS:
    if(state==OPENING_WF_NEGOTIATE_ANS) {
      OZ_Term channelinfo;
      int bufferSize;
      msgC->get_C_NEGOTIATE_ANS(channelinfo);
      extractCI(channelinfo,bufferSize);
      //      if (bufferSize<myBufferSize) // Shrink

      state=WORKING;
      // Whether the channel is ok now or not is to be decided by
      // the future fault-model. For now it has to be for ports etc
      // to start working after temp&resource preemption.
      if(probing) {
        timers->setTimer(probeFaultTimer,maxrtt,
                         comObj_probeFault,(void *) this);
        timers->setTimer(probeIntervalTimer,probeinterval,
                         comObj_sendProbePing,(void *) this);
        if(probeFired) {
//        printf("probeOK (f %d t %d)\n",myDSite->getTimeStamp()->pid,
//               site!=NULL?site->getTimeStamp()->pid:0-1);
          site->probeFault(PROBE_OK);
          probeFired=FALSE;
        }
      }
      retryTimeout=ozconf.perdioTempRetryFloor;
      timers->clearTimer(timer);
      if(queues.hasQueued())
        transObj->deliver();
    }
    else errorRec(mt);
    break;
  case C_ACK: // Actually a dummy, since acknum was retrieved during unmarsh.
    break;
  case C_SET_ACK_PROP:
    int time,length;
    msgC->get_C_SET_ACK_PROP(time,length);
    if(time<0 || length<0)
      errorRec(mt);
    else {
      if(!msgAckLength || // avoid division by zero
         lastReceived % msgAckLength < // Length since last ack
         length)
        sendAck(FALSE);
      msgAckTimeOut=time;
      msgAckLength=length;
    }
    break;
  case C_CLOSE_HARD:
    if(state==WORKING) {
      MsgContainer *newmsgC=msgContainerManager->newMsgContainer(NULL);
      newmsgC->put_C_CLOSE_ACCEPT();
      send(newmsgC,5);
      state=CLOSING_WF_DISCONNECT;
    }
    else if(state==CLOSING_HARD || state==CLOSING_WEAK) {
      // This is as good confirmation as C_CLOSE_ACCEPT
      // If we are closing weak and the oponent hard, he must have gotten
      // a need since we decided to close...
      Assert(state==CLOSING_HARD||(state==CLOSING_WEAK && remoteRef));
      close(CLOSED_WF_HANDOVER);
      msgContainerManager->deleteMsgContainer(msgC);
      return FALSE;
    }
    else errorRec(mt);
    break;
  case C_CLOSE_WEAK:
    // If this site still has a need, send close_reject else accept
    if(state==WORKING) {
      MsgContainer *newmsgC=msgContainerManager->newMsgContainer(NULL);
      CState newstate;
      if(hasNeed()) {
        newmsgC->put_C_CLOSE_REJECT();
        newstate=WORKING;
      }
      else {
        newmsgC->put_C_CLOSE_ACCEPT();
        newstate=CLOSED;
      }
      send(newmsgC,5);
      clearTimers();
      state=newstate;  // State change may "close" outgoing channel.
    }
    else if(state==CLOSING_HARD) {
      // This is as good confirmation as C_CLOSE_ACCEPT
      close(CLOSED_WF_HANDOVER);
      msgContainerManager->deleteMsgContainer(msgC);
      return FALSE;
    }
    else if(state==CLOSING_WEAK) {
      // This is as good confirmation as C_CLOSE_ACCEPT
      close(CLOSED);
      msgContainerManager->deleteMsgContainer(msgC);
      return FALSE;
    }
    else errorRec(mt);
    break;
  case C_CLOSE_ACCEPT:
    PD((TCPCACHE,"Got close accept in state %d",state));
    if(state==CLOSING_HARD) {
      close(CLOSED_WF_HANDOVER);
      msgContainerManager->deleteMsgContainer(msgC);
      return FALSE;
    }
    else if(state==CLOSING_WEAK) {
      close(CLOSED);
      msgContainerManager->deleteMsgContainer(msgC);
      return FALSE;
    }
    else errorRec(mt);
    break;
  case C_CLOSE_REJECT:
    if(state==CLOSING_HARD || state==CLOSING_WEAK) {
      remoteRef=TRUE;
      state=WORKING;
      if(queues.hasQueued())
        transObj->deliver();
    }
    else errorRec(mt);
    break;
  case C_CLEAR_REFERENCE: // If this is what we waited for start to close...
    if(state==WORKING||state==CLOSING_WEAK||state==CLOSING_HARD) {//in is open
      remoteRef=FALSE;
      if(state==WORKING && !hasNeed()) {
        MsgContainer *newmsgC=msgContainerManager->newMsgContainer(NULL);
        newmsgC->put_C_CLEAR_REFERENCE();
        send(newmsgC,5);

        // remoteRef==FALSE!
        MsgContainer *newmsgC2=msgContainerManager->newMsgContainer(NULL);
        newmsgC2->put_C_CLOSE_WEAK();
        send(newmsgC2,5);
        state=CLOSING_WEAK; // State change "closes" outgoing channel
                            // after send.
        timers->setTimer(closetimer,CLOSE_TIMEOUT,
                         comObj_closeTimerExpired,(void *) this);
      }
    }
    else errorRec(mt);
    break;
  default:
    if(state==WORKING||state==CLOSING_WEAK||state==CLOSING_HARD) {//in is open
      ++lastReceived;
      remoteRef=TRUE; // Implicitly known since he is sending a message!

      if(site->siteStatus()!=SITE_OK) {
        Assert(0); //Not an error just want to see AN
        break;
      }
      perdio_msgReceived(msgC,NULL);

      if(!msgAckLength || // avoid division with zero
         lastReceived%msgAckLength==0)  // Time for explicit acknowledgement
        sendAck(FALSE);
      else if(timer==NULL)
        timers->setTimer(timer,msgAckTimeOut,
                         comObj_sendAck,(void *) this);
    }
    else errorRec(mt);
    break;
  }

  msgContainerManager->deleteMsgContainer(msgC); // A bit dangerous
                                                 // perdio cannot keep msgC:s
  return TRUE;
}

void ComObj::adoptCI(OZ_Term channelinfo){
  int bufferSize;

  retryTimeout=ozconf.perdioTempRetryFloor;
  // Whether the channel is ok now or not is to be decided by
  // the future fault-model. For now it has to be for ports etc
  // to start working after temp&resource preemption.
  if(probing) {
    timers->setTimer(probeFaultTimer,maxrtt,
                     comObj_probeFault,(void *) this);
    timers->setTimer(probeIntervalTimer,probeinterval,
                     comObj_sendProbePing,(void *) this);
    if(probeFired) {
//        printf("probeOK (f %d t %d)\n",myDSite->getTimeStamp()->pid,
//           site!=NULL?site->getTimeStamp()->pid:0-1);
      site->probeFault(PROBE_OK);
      probeFired=FALSE;
    }
  }

  extractCI(channelinfo,bufferSize);
//    if (bufferSize<myBufferSize)
//     // shrink buffer, set myBuffersize to bufferSize, answer with bufferSize
//    else
//      // answer with myBufferSize
  MsgContainer *newmsgC=msgContainerManager->newMsgContainer(NULL);
  channelinfo=createCI(bufferSize);
  newmsgC->put_C_NEGOTIATE_ANS(channelinfo);
  send(newmsgC,5);
}

void ComObj::merge(ComObj *old,ComObj *anon,OZ_Term channelinfo) {
  switch(old->state) {
  case CLOSED:
    goto adopt_anon;
  case CLOSED_WF_HANDOVER:
  case CLOSED_WF_REMOTE:
  case CLOSED_PROBLEM:
  case OPENING_WF_PRESENT:
    old->close(CLOSED,TRUE);
    goto adopt_anon;
  case OPENING_WF_NEGOTIATE_ANS:
    if(myDSite->compareSites(site)<0) {
      old->close(CLOSED,TRUE);
      goto adopt_anon;
    }
    else {
      anon->close(CLOSED,TRUE);
      return;
    }
  case WORKING:
  case CLOSING_HARD: // Resources are poor, wait in line...?
    anon->close(CLOSED,TRUE);
    return;
  case CLOSING_WF_DISCONNECT:
    goto adopt_anon;
  default:
    printf("PROBLEM");
    Assert(0);
    return;
  }

 adopt_anon:
  anon->transObj->setOwner(old);
  Assert(old->transObj==NULL);
  old->transObj=anon->transObj;
  PD((TCPCACHE,"Switch running anon: %d old: %d",anon->state,old->state));
  transObj->getTransController()->switchRunning(anon,old);
  anon->transObj=NULL;
  old->state=ANONYMOUS_WF_NEGOTIATE; // Being in the correct
                                     // state makes send behave
  old->adoptCI(channelinfo);
  old->state=WORKING;
}

void ComObj::msgAcked(int num) {
  PD((TCP_INTERFACE,"---msgAcked: %d",num));
  int rtt=queues.msgAcked(num,FALSE,probing && state==WORKING);
  if(probing && state==WORKING) {
    if(!probeFired) {
      if(rtt==-1 || (rtt>=minrtt && rtt<=maxrtt)) {
        // rtt==-1: Don't know any rtt right now, but at least the channel is
        // open. Let rtt-calc and not timer decide on fault.
        timers->setTimer(probeFaultTimer,maxrtt,
                         comObj_probeFault,(void *) this);
        //      PD((TCP_INTERFACE,"Probe timer (re)set at %d proc %x",
        //  am.getEmulatorClock(),comObj_probeFault));
      }
      else { // rtt out of bounds, fire probe
        PD((TCP_INTERFACE,"---probe rtt: %d",rtt));
//      printf("RTT (%d) invoked probeFault (f %d t %d acking %d)\n",rtt,
//             myDSite->getTimeStamp()->pid,
//             site!=NULL?site->getTimeStamp()->pid:0-1,num);
        timers->clearTimer(probeFaultTimer);
        site->probeFault(PROBE_TEMP);
        probeFired=TRUE;
      }
    }
    else { // probe was fired
      if(rtt!=-1 && rtt>=minrtt && rtt<=maxrtt) { // Back on track
        timers->setTimer(probeFaultTimer,maxrtt,
                         comObj_probeFault,(void *) this);
        //      PD((TCP_INTERFACE,"Probe timer (re)set at %d proc %x",
        //          am.getEmulatorClock(),comObj_probeFault));
        PD((TCP_INTERFACE,"probe ok"));
//      printf("probeOK (f %d t %d)\n",myDSite->getTimeStamp()->pid,
//             site!=NULL?site->getTimeStamp()->pid:0-1);
        site->probeFault(PROBE_OK);
        probeFired=FALSE;
      }
    }
  }
}

MsgContainer *ComObj::getNextMsgContainer(int &acknum) {
  MsgContainer *msgC=queues.getNext(state==WORKING);

  if(msgC!=NULL && !msgC->checkFlag(MSG_HAS_MARSHALCONT) &&
     msgC->getMessageType()<C_FIRST) {
    //    queues.insertUnacked(msgC);
    msgC->setMsgNum(++lastSent);
  }
#ifdef COMOBJ_LOG
  if(msgC!=NULL)
    printf("transmit(%s %d %d %d %d)\n",
           mess_names[msgC->getMessageType()],
           myDSite->getTimeStamp()->pid,
           site!=NULL?site->getTimeStamp()->pid:0,
           msgC->getMsgNum()==-1?0:msgC->getMsgNum(),
           (int) am.getEmulatorClock());
#endif
  if(probing && msgC!=NULL && msgC->getMessageType()<C_FIRST) {
    msgC->setSendTime(am.getEmulatorClock());
    timers->setTimer(probeIntervalTimer,probeinterval,
                     comObj_sendProbePing,(void *) this);
  }
  if(state==WORKING && timer!=NULL) // Acknowledgement is being sent
    timers->clearTimer(timer);
  acknum=lastReceived;
  return msgC;
}

void ComObj::msgSent(MsgContainer *msgC) {
  if(msgC->getMessageType()<C_FIRST) {
    Assert(msgC->getMsgNum()!=-1);
    queues.insertUnacked(msgC);
  }
  else
    msgContainerManager->deleteMsgContainer(msgC);
}

void ComObj::msgPartlySent(MsgContainer *msgC) {
  queues.requeue(msgC);
}

void ComObj::msgPartlyReceived(MsgContainer *msgC) {
  //  printf("msgPartlyReceived invoked at %d\n",myDSite->getTimeStamp()->pid);
  if(msgC->getMsgNum()==-1)
    msgC->setMsgNum(lastReceived+1);
  queues.putRec(msgC);
}

MsgContainer *ComObj::getMsgContainer() {
  MsgContainer *newm=msgContainerManager->newMsgContainer(NULL);
  return newm;
}

MsgContainer *ComObj::getMsgContainer(int num) {
  return queues.getRec(num);
}

void ComObj::connectionLost(void *info) {
  PD((TCP_INTERFACE,"Connection lost, state=%d %x",state,this));
//    printf("Connection lost, state=%d %x to %d\n",state,(int) transObj,
//       site->getTimeStamp()->pid);
  switch(state) {
  case OPENING_WF_PRESENT:
  case OPENING_WF_NEGOTIATE_ANS:
  case WORKING:
    if(hasNeed() || remoteRef)
      //    if(queues.hasNeed())
      close(CLOSED_PROBLEM);
    else
      close(CLOSED);
    break;
  case ANONYMOUS_WF_NEGOTIATE:
    close(CLOSED,TRUE); // use close with true not to be reopened
    comController->deleteComObj(this); // Anonymous, can do no more
    return;
  case CLOSING_HARD:
    close(CLOSED_WF_HANDOVER); // Not the normal way, but fine
    break;
  case CLOSING_WEAK: // In case the other guy still has need, let him worry
    close(CLOSED);
    break;
  case CLOSING_WF_DISCONNECT: // Now the connection was closed as expected.
    close(CLOSED_WF_REMOTE);
//      PD((TCPCACHE,"Closed remote, setting timer"));
//      timers->setTimer(reopentimer,WF_REMOTE_TIMEOUT,
//                   comObj_reopen,(void *) this);
    break;
  case CLOSED: // We accepted gc initiated by him
    close(CLOSED);
    break;
  default:
//      printf("An unknown connection was lost (%d %x %x %d %d)",
//         state,(int) transObj,this,
//         myDSite->getTimeStamp()->pid,
//         site!=NULL?site->getTimeStamp()->pid:0);
    OZ_error("An unknown connection was lost (%d %x %x %d %d)",
             state,(int) transObj,this,
             myDSite->getTimeStamp()->pid,
             site!=NULL?site->getTimeStamp()->pid:0);
    //  printf("An unknown connection was lost");
    //      comController->deleteComObj(this);
  }
}

void ComObj::gcComObj() {
  queues.gcMsgCs();
}

void ComObj::installProbe(int lowerBound, int higherBound, int interval) {
  PD((TCP_INTERFACE,"Installing probe %d<x<%d, at %d",
      lowerBound,higherBound,interval));
  minrtt=lowerBound;
  maxrtt=higherBound;
  probeinterval=interval;
  if(state==WORKING) {
    timers->setTimer(probeFaultTimer,maxrtt,
                     comObj_probeFault,(void *) this);
    timers->setTimer(probeIntervalTimer,probeinterval,
                     comObj_sendProbePing,(void *) this);
    if(!probing) {
      MsgContainer *msgC=msgContainerManager->newMsgContainer(NULL);
      msgC->put_C_SET_ACK_PROP(0,0);
      send(msgC,5);
    }
  }
  probing=TRUE;
}

Bool ComObj::sendProbePing() {
  MsgContainer *msgC=msgContainerManager->newMsgContainer(NULL);
  msgC->put_M_PING();
  send(msgC,5);
  return TRUE;
}

Bool ComObj::probeFault() {
  Assert(!probeFired);
  if(!probeFired) {
    probeFired=TRUE;
    PD((TCP_INTERFACE,"probe timeout at %d",am.getEmulatorClock()));
//      printf("timerinvoked probeFault (f %d t %d)\n",
//         myDSite->getTimeStamp()->pid,
//         site!=NULL?site->getTimeStamp()->pid:0-1);
    site->probeFault(PROBE_TEMP);
  }
  probeFaultTimer=NULL;
  return FALSE;
}

Bool ComObj::sendAck(Bool timerInvoked) {
  if(!queues.hasQueued()) { // Otherwise an ack will go anyway
    MsgContainer *msgC=msgContainerManager->newMsgContainer(NULL);
    msgC->put_C_ACK();
    send(msgC,5);
  }
  if(timerInvoked)
    timer=NULL;
  else
    timers->clearTimer(timer);
  return FALSE;
}

void ComObj::clearTimers() {
  timers->clearTimer(timer);
  timers->clearTimer(closetimer);
  timers->clearTimer(reopentimer);
  timers->clearTimer(probeIntervalTimer);
  timers->clearTimer(probeFaultTimer);
}

void ComObj::shutDown() {
  queues.clearAll();
  close(CLOSED,TRUE);//Use true not to be reopend if need
}

Bool ComObj::hasQueued() {
  return queues.hasQueued();
}

Bool ComObj::canBeClosed() {
  if(state!=ANONYMOUS_WF_NEGOTIATE)
    PD((TCPCACHE,"CanBeClosed asked. %d (%x) is in state %d (transObj %x)",
        site->getTimeStamp()->pid,this,state,transObj));
  else
    PD((TCPCACHE,"CanBeClosed asked. anon (%x) is in state %d (transObj %x)",
        this,state,transObj));
  //  Assert(state!=1);
  return state==WORKING;
}

int ComObj::getNOSM() {
  int tmp=nosm;
  nosm=0;
  return tmp;
}

int ComObj::getNORM() {
  int tmp=nosm;
  nosm=0;
  return tmp;
}

void comController_acceptHandler(TransObj *transObj) {
  ComObj *comObj = comController->newComObj(NULL,0);
  comObj->accept(transObj);
}

void ComController::gcComObjs() {
  ComObj *tmp=list;
  while(tmp!=NULL) {
    tmp->gcComObj();
    tmp=tmp->next;
  }
}

int ComController::closeDownCount() {
  ComObj *tmp=list,*next;
  int count=0;
  while(tmp!=NULL) {
    next=tmp->next;
    if(tmp->canBeFreed()) {
      DSite *site=tmp->site;
      deleteComObj(tmp);  // Inefficient extra listsearch
      site->dumpRemoteSite(4711); // Is this correct?
    }
    else
      count++;
    tmp=next;
  }
  Assert(count==wc);
  return count;
}

ComObj *ComController::newComObj(DSite *site,int recCtr){
  FreeListEntry *f=getOne();
  ComObj *comObj;
  if(f==NULL) {
    comObj=new ComObj(site,recCtr);
//      printf("new ComObj %x at %d\n",comObj,osgetpid());
  }
  else {
    GenCast(f,FreeListEntry*,comObj,ComObj*);
    comObj->init(site,recCtr);
  }
  ++wc;
  comObj->next=list;
  list=comObj;
  //  printf("cr %x\n",(int) comObj);
  return comObj;
}

void ComController::deleteComObj(ComObj* comObj){
  PD((TCPCACHE,"ComObj being deleted %x",comObj));
  //  printf("dl %x\n",(int) comObj);
  comObj->shutDown();

  FreeListEntry *f;
  --wc;
  GenCast(comObj,ComObj*,f,FreeListEntry*);

  ComObj *prev=NULL;
  ComObj *tmp=list;
  while(tmp!=NULL) {
    if(tmp==comObj) {
      if(prev==NULL)
        list=comObj->next;
      else
        prev->next=comObj->next;
      break;
    }
    prev=tmp;
    tmp=tmp->next;
  }

  if(putOne(f))
    return;
  else
    delete comObj;
  return;
}
