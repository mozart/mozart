/*
 *  Authors:
 *    Anna Neiderud (annan@sics.se)
 *
 *  Contributors:
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

#include "comObj.hh"
#include "msgContainer.hh"
#include "transObj.hh"
#include "perdio.hh"
#include "connection.hh"
#include "timers.hh"

#ifdef _MSC_VER
#include <io.h>
#include <time.h>
#else
#include <sys/time.h>
#endif

#define OPEN_TIMEOUT            ozconf.dpOpenTimeout
#define CLOSE_TIMEOUT           ozconf.dpCloseTimeout
#define WF_REMOTE_TIMEOUT       ozconf.dpWFRemoteTimeout
#define WF_REOPEN_TIMEOUT       ozconf.dpFirewallReopenTimeout
// one size fits all - currently cannot have different max sizes
// for different sites;
#define MAX_BUFFER_SIZE         ozconf.dpMaxBufferSize

#define MSG_ACK_TIMEOUT 1000
#define MSG_ACK_LENGTH 200


#define DO_CONNECT_LOG ozconf.dpLogConnectLog
#define DO_MESSAGE_LOG ozconf.dpLogMessageLog



ComObj::ComObj(DSite *site) {
  init(site);
}

void ComObj::init(DSite *site) {
  this->site = site;
  this->transObj = NULL;

  localRef=FALSE; // Will be set true by the first send and false by gc
  remoteRef=FALSE;
  sentclearref=FALSE;
  state=CLOSED;

  lastReceived=0;
  lastSent=0;
  sentLrgMsg = 0;
  receivedLrgMsg = 0;
  timer=NULL;
  reopentimer=NULL;
  closetimer=NULL;

  probing=FALSE;
  probeFired=FALSE;
  probeIntervalTimer=NULL;
  probeFaultTimer=NULL;

  retryTimeout=ozconf.dpRetryTimeFloor;

  nosm=norm=0;
  lastrtt=-1;
  connectgrantrequested=FALSE;
  queues.init();

  DebugCode(next_cache=(ComObj *)0x44);
  DebugCode(connectVar=(OZ_Term) 0x45);
  DebugCode(transtype=(OZ_Term) 0x46);
}

// Specifying priority -1 means accepting the default as in msgFormat.m4 and
// should allways be used.
void ComObj::send(MsgContainer *msgC)
{
  // Erik, hack until the prio thing has been properly reworked.
  int priority = -1;
  globalSendCounter++;

  if(DO_MESSAGE_LOG) {
    fprintf(logfile,"send(%s %d %d %d %s)\n",
            mess_names[msgC->getMessageType()],
            myDSite->getTimeStamp()->pid,
            site!=NULL?site->getTimeStamp()->pid:0,
            msgC->getMsgNum(),
            am.getEmulatorClock()->toString());
  }

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

  // Old priority calculation. If the suplied priority is different from
  // -1 then use it else get the default
  /*

    Assert(priority==-1); // Have a good reason before removing this line
    if(priority==-1) priority=msgC->getPriority();
  */
  // New priority caclualtion.  Dynamicaly defined prio either sets the prio or
  // defines that the orgin should be used.

  if (default_mess_priority[msgC->getMessageType()] == USE_PRIO_OF_SENDER){
    priority = msgC->getPriority();
    //    printf("sender prio results in %d\n", priority);
  }
  else
    priority =  default_mess_priority[msgC->getMessageType()] + 1;

  Assert(msgC->getMessageType()>C_FIRST || msgC->getMessageType()==M_PING
         || (priority<5 && priority>=1));
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

    // Increase time to wait before next try
    retryTimeout=(int) (((double) retryTimeout) *
                        ((100.0+ozconf.dpRetryTimeFactor)/100.0));
    if(retryTimeout>ozconf.dpRetryTimeCeiling)
      retryTimeout=ozconf.dpRetryTimeCeiling;

    open();
  }
  reopentimer=NULL;
  return FALSE;
}

// Called by builtin when this comObj can have its communication
// The returnvalue indicates whether it is wanted or not
Bool ComObj::handover(TransObj *transObj) {
  retryTimeout=ozconf.dpRetryTimeFloor;

  PD((TCP_INTERFACE,"Connection handover (from %d to %d (%x))",
      myDSite->getTimeStamp()->pid,site->getTimeStamp()->pid,this));
  if(DO_CONNECT_LOG) {
    fprintf(logfile,"handover(%d %d %s)\n",
            myDSite->getTimeStamp()->pid,
            site!=NULL?site->getTimeStamp()->pid:0,
            am.getEmulatorClock()->toString());
  }
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
  PD((TCP_INTERFACE,"openTimerExpired at %s %x",
      am.getEmulatorClock()->toString(),this));
  if((state==CLOSED_WF_HANDOVER && !connectgrantrequested) ||
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
    comController->deleteComObj(this);
    return FALSE;
  }
  if(connectgrantrequested)
    return TRUE;
  timer=NULL;
  return FALSE;
}

Bool ComObj::closeTimerExpired() {
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
  //transObj->setOwner(this);
  //  transObj->getTransController()->addRunning(this);
  state=ANONYMOUS_WF_NEGOTIATE;
  timers->setTimer(timer,OPEN_TIMEOUT,
                   comObj_openTimerExpired,(void *) this);

  MsgContainer *msgC=msgContainerManager->newMsgContainer(NULL);
  msgC->put_C_PRESENT(PERDIOVERSION,myDSite);
  send(msgC);
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

  if(DO_CONNECT_LOG) {
    fprintf(logfile,"close(%d %d %s %d %d)\n",
            myDSite->getTimeStamp()->pid,
            site!=NULL?site->getTimeStamp()->pid:0,
            am.getEmulatorClock()->toString(),
            state,statetobe);
  }

  clearTimers();
  lastrtt=-1;

  if(transObj!=NULL) {
    handback(this,transObj);
    transObj=NULL;
  }
  else if(state==CLOSED_WF_HANDOVER &&
          (statetobe==CLOSED || statetobe==CLOSED_PROBLEM))
    // No transObj but yet expecting one => cancel
    comObjDone(this);
  Assert(!connectgrantrequested && connectVar==(OZ_Term) 0x45);
  queues.clear5();
  // Can this be a bug? The level 5 messages are cleared, but not
  // eventual partialy received ad sent messages.... ERIK .
  switch(statetobe) {
  case CLOSED_WF_HANDOVER:
    //    state=CLOSED_WF_HANDOVER; happens anyway!
    open();
    break;
  case CLOSED_PROBLEM:
    state=CLOSED_PROBLEM;
    timers->setTimer(reopentimer,retryTimeout,
                     comObj_reopen,(void *) this);
    PD((TCP_INTERFACE,"problem timer set at %s to %d",
        am.getEmulatorClock()->toString(),retryTimeout));
    break;
  case CLOSED:
    // It could be that a new message has been "sent" while we or other were
    // closing. In that case reopen.
    if(!merging &&
       (state==CLOSING_WEAK || state==CLOSED) &&
       queues.hasNeed()) {
      printf("CLOSED => reopen\n");
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
  clearTimers(); // And set a closing timer below
  MsgContainer *msgC=msgContainerManager->newMsgContainer(NULL);
  msgC->put_C_CLOSE_HARD();
  send(msgC);
  state=CLOSING_HARD; // State change "closes" outgoing channel, do after send
  timers->setTimer(closetimer,CLOSE_TIMEOUT,
                   comObj_closeTimerExpired,(void *) this);
}

Bool ComObj::canBeFreed() {
  localRef=FALSE;
  if(hasNeed())
    return FALSE;
  else if(remoteRef)
    return FALSE;
  else {
    switch(state) {
    case WORKING: {
      // It is important at exit (dpExit) not to send too many clearrefs.
      if(!sentclearref) {
        MsgContainer *msgC=msgContainerManager->newMsgContainer(NULL);
        msgC->put_C_CLEAR_REFERENCE();
        send(msgC);
        sentclearref=TRUE;
      }
      clearTimers();
      MsgContainer *msgC2=msgContainerManager->newMsgContainer(NULL);
      msgC2->put_C_CLOSE_WEAK();
      send(msgC2);
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

void ComObj::errorRec(int mt) {
  OZ_error("msgReceive: illegal message %s received by %d in state %d from %d\n",
           mess_names[mt],myDSite->getTimeStamp()->pid,
           state,site!=NULL?site->getTimeStamp()->pid:0);
}

Bool ComObj::hasNeed() {
  return localRef || queues.hasNeed();
}

inline OZ_Term ComObj::createCI(int bufferSize)
{
  int time,length;
  time=MSG_ACK_TIMEOUT;
  length=MSG_ACK_LENGTH;
  return OZ_recordInit(oz_atom("channelinfo"),
                       oz_cons(oz_pairAI("lastReceived",lastReceived),
                       oz_cons(oz_pairAI("msgAckTimeOut",time),
                       oz_cons(oz_pairAI("msgAckLength",length),
                       oz_cons(oz_pairAI("bufferSize",bufferSize),
                       oz_cons(oz_pairAI("maxBufferSize",MAX_BUFFER_SIZE),
                       oz_cons(oz_pairAI("hasNeed",hasNeed()),
                       oz_nil())))))));
}

inline void ComObj::extractCI(OZ_Term channelinfo,
                              int &bufferSize, int &maxBufferSize)
{
  int index;
  int remLastReceived;

  if (oz_isRecord(channelinfo)) {
    SRecord *srec = tagged2SRecord(channelinfo);

    index=srec->getIndex(oz_atom("lastReceived"));
    remLastReceived=oz_intToC(srec->getArg(index));
    queues.msgAcked(remLastReceived,TRUE,FALSE);      // Resend/Ack msgs
    queues.clearRec();                                // Clear partly received
    queues.clearCont();
    sentLrgMsg=0;
    receivedLrgMsg=0;
    lastSent=remLastReceived; // Others requeued

    index=srec->getIndex(oz_atom("msgAckTimeOut"));
    msgAckTimeOut=oz_intToC(srec->getArg(index));

    index=srec->getIndex(oz_atom("msgAckLength"));
    msgAckLength=oz_intToC(srec->getArg(index));

    index=srec->getIndex(oz_atom("bufferSize"));
    bufferSize=oz_intToC(srec->getArg(index));
    index=srec->getIndex(oz_atom("maxBufferSize"));
    maxBufferSize=oz_intToC(srec->getArg(index));

    index=srec->getIndex(oz_atom("hasNeed"));
    remoteRef=oz_intToC(srec->getArg(index));
  }
}

Bool ComObj::msgReceived(MsgContainer *msgC) {
  mess_counter[msgC->getMessageType()].recv();
  if(DO_MESSAGE_LOG) {
    fprintf(logfile,"received(%s %d %d %d %s)\n",
            mess_names[msgC->getMessageType()],
            myDSite->getTimeStamp()->pid,
            site!=NULL?site->getTimeStamp()->pid:0,
            msgC->getMessageType()<C_FIRST?lastReceived+1:0,
            am.getEmulatorClock()->toString());
  }
  PD((TCP_INTERFACE,"---msgReceived: %s nr:%d from %d",
      mess_names[msgC->getMessageType()],lastReceived+1,
      site!=NULL?site->getTimeStamp()->pid:-1));

  norm++;
  globalRecCounter++;


  MessageType mt=msgC->getMessageType();
  switch(mt) {
  case C_PRESENT:
    if (state==OPENING_WF_PRESENT) {
      char *version;
      DSite *s1;

      msgC->get_C_PRESENT(version,s1);

      if (strcmp(version,PERDIOVERSION)!=0 || s1!=site) {
        msgContainerManager->deleteMsgContainer(msgC);
        site->discoveryPerm();
        site->probeFault(PROBE_PERM);
        return FALSE;
      } else {
        state=OPENING_WF_NEGOTIATE_ANS;
        MsgContainer *newmsgC=msgContainerManager->newMsgContainer(NULL);
        OZ_Term channelinfo;
        channelinfo=createCI(transObj->getBufferSize());
        newmsgC->put_C_NEGOTIATE(PERDIOVERSION,myDSite,channelinfo);
        send(newmsgC);
      }
    } else {
      errorRec(mt);
    }
    break;

  case C_NEGOTIATE:
    if (state == ANONYMOUS_WF_NEGOTIATE) {
      char *version;
      DSite *s1;
      OZ_Term channelinfo;

      msgC->get_C_NEGOTIATE(version,s1,channelinfo);
      site=s1;
      transObj->setSite(site);

      if (DO_CONNECT_LOG) {
        fprintf(logfile,"accept(%d %d %s)\n",
                myDSite->getTimeStamp()->pid,
                site!=NULL?site->getTimeStamp()->pid:0,
                am.getEmulatorClock()->toString());

      }

      if (strcmp(version,PERDIOVERSION)!=0) {
        msgContainerManager->deleteMsgContainer(msgC);
        site->discoveryPerm();
        site->probeFault(PROBE_PERM);
        return FALSE;
      } else {
        // Tell the site about this comObj
        ComObj *other=s1->setComObj(this);
        if (other==(ComObj *) -1) {
          // The comObj is refused since the site is already marked perm.
          // abort the channel and return false meaning closed - don't
          // continue reading.
          msgContainerManager->deleteMsgContainer(msgC);
          comController->deleteComObj(this);
          return FALSE;

          //
        } else if (other!=NULL) {
          // This object is an anonymous object and the other
          // is to be used. What transObj to use needs to be
          // decided on.
          PD((TCP_INTERFACE,"Other comObj found in state %d",other->state));
          Bool ret=merge(other,this,channelinfo);
          msgContainerManager->deleteMsgContainer(msgC);
          Assert(state==CLOSED && transObj==NULL);
          comController->deleteComObj(this);
          return ret; // !This object may not do anything more, but the trO
                      // has to continue if it was adopted.

          //
        } else {
          adoptCI(channelinfo);
          state=WORKING;
          // Whether the channel is ok now or not is to be decided by
          // the future fault-model. For now it has to be for ports etc
          // to start working after temp&resource preemption.
          if (probing) {
            timers->setTimer(probeFaultTimer,maxrtt,
                             comObj_probeFault,(void *) this);
            timers->setTimer(probeIntervalTimer,probeinterval,
                             comObj_sendProbePing,(void *) this);
            if (probeFired) {
//            printf("probeOK (f %d t %d)\n",myDSite->getTimeStamp()->pid,
//                   site!=NULL?site->getTimeStamp()->pid:0-1);
              site->probeFault(PROBE_OK);
              probeFired=FALSE;
            }
          }
        }
        timers->clearTimer(timer);
      }
    } else {
      errorRec(mt);
    }
    break;

  case C_NEGOTIATE_ANS:
    if (state==OPENING_WF_NEGOTIATE_ANS) {
      OZ_Term channelinfo;
      int bufferSize, maxBufferSize;
      int myBufferSize = transObj->getBufferSize();
      msgC->get_C_NEGOTIATE_ANS(channelinfo);
      extractCI(channelinfo, bufferSize, maxBufferSize);

      // Handle the buffer size issue.
      // The biggest of local and remote buffer sizes is choosen, but
      // not exceeding either of 'maxBufferSize's;
      bufferSize = max(bufferSize, myBufferSize);
      bufferSize = min(bufferSize, min(MAX_BUFFER_SIZE, maxBufferSize));
      //
      if (bufferSize != myBufferSize)
        transObj->setBufferSize(bufferSize);

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
      retryTimeout=ozconf.dpRetryTimeFloor;
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
      send(newmsgC);
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
      send(newmsgC);
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
      timers->clearTimer(closetimer);
      state=WORKING;
      if(queues.hasQueued())
        transObj->deliver();
    }
    else errorRec(mt);

    break;

  case C_SEND_PING_PONG:
    {
      int id, ctr;
      msgC->get_C_SEND_PING_PONG(id,ctr);
      ctr--;
      if(ctr == 0){
        if(id>0)ResumeSendJobb(id);
      }
      else{
        MsgContainer *newmsgC=msgContainerManager->newMsgContainer(NULL);
        newmsgC->put_C_SEND_PING_PONG(id,ctr);
        send(newmsgC);
      }
      break;
    }
  case C_CLEAR_REFERENCE: // If this is what we waited for start to close...
    if(state==WORKING||state==CLOSING_WEAK||state==CLOSING_HARD) {//in is open
      remoteRef=FALSE;
      if(state==WORKING && !hasNeed()) {
        MsgContainer *newmsgC=msgContainerManager->newMsgContainer(NULL);
        newmsgC->put_C_CLEAR_REFERENCE();
        send(newmsgC);

        // remoteRef==FALSE!
        MsgContainer *newmsgC2=msgContainerManager->newMsgContainer(NULL);
        newmsgC2->put_C_CLOSE_WEAK();
        send(newmsgC2);
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

      perdio_msgReceived(msgC,site);

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

void ComObj::adoptCI(OZ_Term channelinfo)
{
  int bufferSize, maxBufferSize;
  int myBufferSize = transObj->getBufferSize();

  retryTimeout=ozconf.dpRetryTimeFloor;
  // Whether the channel is ok now or not is to be decided by
  // the future fault-model. For now it has to be for ports etc
  // to start working after temp&resource preemption.
  if (probing) {
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

  //
  extractCI(channelinfo, bufferSize, maxBufferSize);

  // Handle the buffer size issue.
  // The biggest of local and remote buffer sizes is choosen, but not
  // exceeding either of 'maxBufferSize's;
  bufferSize = max(bufferSize, myBufferSize);
  bufferSize = min(bufferSize, min(MAX_BUFFER_SIZE, maxBufferSize));
  //
  if (bufferSize != myBufferSize)
    transObj->setBufferSize(bufferSize);

  //
  MsgContainer *newmsgC=msgContainerManager->newMsgContainer(NULL);
  channelinfo=createCI(bufferSize);
  newmsgC->put_C_NEGOTIATE_ANS(channelinfo);
  send(newmsgC);
}

Bool ComObj::merge(ComObj *old,ComObj *anon,OZ_Term channelinfo) {
  if(DO_CONNECT_LOG) {
    fprintf(logfile,"beginmerge(%d %d %s %d %d)\n",
            myDSite->getTimeStamp()->pid,
            site!=NULL?site->getTimeStamp()->pid:0,
            am.getEmulatorClock()->toString(),
            old->state, anon->state);
  }
  switch(old->state) {
  case CLOSED:
    goto adopt_anon;
  case CLOSED_WF_HANDOVER:
    old->close(CLOSED,TRUE);
    goto adopt_anon;
  case CLOSED_WF_REMOTE:
  case CLOSED_PROBLEM:
  case OPENING_WF_PRESENT:
    old->close(CLOSED,TRUE);
    goto adopt_anon;
  case OPENING_WF_NEGOTIATE_ANS:
    if (myDSite->compare(site) < 0) {
      old->close(CLOSED,TRUE);
      goto adopt_anon;
    } else {
      goto drop_anon;
    }
  case WORKING:
  case CLOSING_HARD: // Resources are poor, wait in line...?
    goto drop_anon;
  case CLOSING_WF_DISCONNECT:
  case CLOSING_WEAK:
    old->close(CLOSED,TRUE);
    goto adopt_anon;
  default:
    DebugCode(printf("PROBLEM (state %d %d)\n",old->state,state);)
    Assert(0);
    return FALSE;
  }

 drop_anon:
  anon->close(CLOSED,TRUE);
  if(DO_CONNECT_LOG) {
    fprintf(logfile,"endmerge(%d %d %s anon_dropped)\n",
            myDSite->getTimeStamp()->pid,
            site!=NULL?site->getTimeStamp()->pid:0,
            am.getEmulatorClock()->toString());
  }
  return FALSE;

 adopt_anon:
  if(DO_CONNECT_LOG) {
    fprintf(logfile,"endmerge(%d %d %s anon_adopted)\n",
            myDSite->getTimeStamp()->pid,
            site!=NULL?site->getTimeStamp()->pid:0,
            am.getEmulatorClock()->toString());
  }
  anon->transObj->setOwner(old);
  transObj->getTransController()->switchRunning(anon,old);
  Assert(old->transObj==NULL);
  old->transObj=anon->transObj;
  PD((TCPCACHE,"Switch running anon: %d old: %d",anon->state,old->state));
  anon->transObj=NULL;
  anon->state=CLOSED;
  old->state=ANONYMOUS_WF_NEGOTIATE; // Being in the correct
                                     // state makes send behave
  old->adoptCI(channelinfo);
  old->state=WORKING;
  return TRUE;
}

void ComObj::msgAcked(int num) {
  PD((TCP_INTERFACE,"---msgAcked: %d",num));
  int rtt=queues.msgAcked(num,FALSE,probing && state==WORKING);
  if(rtt!=-1) lastrtt=rtt;
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

  if(msgC==NULL) return NULL;
  if(!msgC->checkFlag(MSG_HAS_MARSHALCONT) && msgC->getMessageType()<C_FIRST) {
    msgC->setMsgNum(NO_MSG_NUM);
  }
  if(DO_MESSAGE_LOG) {
    if(msgC!=NULL)
      fprintf(logfile,"transmit(%s %d %d %d %s)\n",
              mess_names[msgC->getMessageType()],
              myDSite->getTimeStamp()->pid,
              site!=NULL?site->getTimeStamp()->pid:0,
              msgC->getMsgNum()==-1?0:msgC->getMsgNum(),
              am.getEmulatorClock()->toString());
  }

  if(probing && msgC->getMessageType()<C_FIRST) {
    msgC->setSendTime(am.getEmulatorClock());
    timers->setTimer(probeIntervalTimer,probeinterval,
                     comObj_sendProbePing,(void *) this);
  }
  if(state==WORKING && timer!=NULL) // Acknowledgement is being sent
    timers->clearTimer(timer);
  acknum=lastReceived;

  // Resume any threads that are suspended due to flowcntrl.
  msgC->bindCntrlVar();
  return msgC;
}

void ComObj::msgSent(MsgContainer *msgC)
{
  if (msgC->getMessageType() < C_FIRST) {
    msgC->setMsgNum(++lastSent);
    queues.insertUnacked(msgC);
  } else {
    msgContainerManager->deleteMsgContainer(msgC);
  }
}

void ComObj::msgPartlySent(MsgContainer *msgC) {
  if (msgC->getMsgNum() == NO_MSG_NUM)
    msgC->setMsgNum(++sentLrgMsg);
  queues.requeue(msgC);
}

void ComObj::msgPartlyReceived(MsgContainer *msgC) {
  PD((TCP_INTERFACE,"---msgPartlyReceived: %s nr:%d from %d",
      mess_names[msgC->getMessageType()],lastReceived+1,
      site!=NULL?site->getTimeStamp()->pid:-1));
  if(msgC->getMsgNum()==NO_MSG_NUM)
    msgC->setMsgNum(++receivedLrgMsg);
  queues.putRec(msgC);
}

MsgContainer *ComObj::getMsgContainer() {
  MsgContainer *newm=msgContainerManager->newMsgContainer(NULL);
  return newm;
}

MsgContainer *ComObj::getMsgContainer(int num) {
  return queues.getRec(num);
}

void ComObj::connectionLost() {
  PD((TCP_INTERFACE,"Connection lost, state=%d %x",state,this));
//    printf("Connection lost, state=%d %x to %d\n",state,(int) transObj,
//       site->getTimeStamp()->pid);
  if(DO_CONNECT_LOG)
    fprintf(logfile,"lost(%d %d %s %d)\n",
            myDSite->getTimeStamp()->pid,
            site!=NULL?site->getTimeStamp()->pid:0,
            am.getEmulatorClock()->toString(),
            state);
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
    PD((TCPCACHE,"Closed remote, setting timer"));
    if (ipIsbehindFW)
      timers->setTimer(reopentimer,WF_REOPEN_TIMEOUT,
                       comObj_reopen,(void *) this);
    else
      timers->setTimer(reopentimer,WF_REMOTE_TIMEOUT,
                       comObj_reopen,(void *) this);
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
      send(msgC);
    }
  }
  probing=TRUE;
}

Bool ComObj::sendProbePing() {
  MsgContainer *msgC=msgContainerManager->newMsgContainer(NULL);
  msgC->put_M_PING();
  send(msgC);
  return TRUE;
}

Bool ComObj::probeFault() {
  Assert(!probeFired);
  if(!probeFired) {
    probeFired=TRUE;
    PD((TCP_INTERFACE,"probe timeout at %s",am.getEmulatorClock()->toString()));
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
    send(msgC);
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
  int tmp=norm;
  norm=0;
  return tmp;
}

int ComObj::getLastRTT() {
  return lastrtt;
}

OZ_Term ComObj::getStateStatistics() {
  switch(state){
  case CLOSED: return oz_atom("closed");
  case CLOSED_WF_HANDOVER: return oz_atom("wait for handover");
  case CLOSED_WF_REMOTE: return oz_atom("wait for remote");
  case CLOSED_PROBLEM:return oz_atom("problem");
  case ANONYMOUS_WF_NEGOTIATE:return oz_atom("anonymous");
  case OPENING_WF_PRESENT: return oz_atom("presentation");
  case OPENING_WF_NEGOTIATE_ANS:return oz_atom("negotiate");
  case WORKING:   return oz_atom("connected");
  case CLOSING_HARD: return oz_atom("closing (hard)");
  case CLOSING_WEAK: return oz_atom("closing (weak)");
  case CLOSING_WF_DISCONNECT:return oz_atom("wait for disconnect");
  default:
    return oz_atom("unknown");
  }
}
int ComObj::getQueueStatus() {
  return queues.getQueueStatus();
}

void comController_acceptHandler(TransObj *transObj) {
  ComObj *comObj = comController->newComObj(NULL);
  comObj->accept(transObj);
}

//
void ComController::gcComObjs()
{
  ComObj *tmp = list;
  while (tmp) {
    tmp->gcComObj();
    tmp = tmp->next;
  }
}

//
void ComController::startGCComObjs()
{
  ComObj *tmp = list;
  while (tmp) {
    tmp->startGCComObj();
    tmp = tmp->next;
  }
}

//
void ComController::finishGCComObjs()
{
  ComObj *tmp = list;
  while (tmp) {
    tmp->finishGCComObj();
    tmp = tmp->next;
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
      site->dumpRemoteSite();
    }
    else
      count++;
    tmp=next;
  }
  Assert(count==wc);
  return count;
}

void ComController::closeAll() {
  ComObj *tmp=list;
  while(tmp!=NULL) {
    tmp->shutDown();
    tmp=tmp->next;
  }
}

Bool ComController::valid(ComObj *testObj) {
  ComObj *tmp=list;
  while(tmp!=NULL) {
    if(tmp==testObj)
      return TRUE;
    tmp=tmp->next;
  }
  return FALSE;
}

ComObj *ComController::newComObj(DSite *site){
  FreeListEntry *f=getOne();
  ComObj *comObj;
  if(f==NULL) {
    comObj=new ComObj(site);
//      printf("new ComObj %x at %d\n",comObj,osgetpid());
  }
  else {
    comObj = (ComObj*) f;
    comObj->init(site);
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
  Assert(!comObj->connectgrantrequested);

  FreeListEntry *f;
  --wc;
  f = (FreeListEntry*)(void*) comObj;

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

ComController::~ComController(){
  ComObj *comObj;
  FreeListEntry *f;
  int l=length();
  for(int i=0;i<l;i++) {
    f=getOne();
    Assert(f!=NULL);
    comObj = (ComObj*) f;
    delete comObj;
  }
  Assert(length()==0);
}
