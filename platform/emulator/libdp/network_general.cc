/*
 *  Authors:
 *    Andreas Sundstrom <andreas@sics.se>
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *
 *  Copyright:
 *    1999 Andreas Sundstrom
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

#ifdef ROBUST_UNMARSHALER
int tcpPreReadHandlerRobust(int fd,void *r0,int *error){
#else
int tcpPreReadHandler(int fd,void *r0){
#endif
  ReadConnection *r=(ReadConnection *)r0, *old;
  tcpOpenMsgBuffer->unmarshalBegin();
  BYTE *pos = tcpOpenMsgBuffer->getBuf(),header;
  int ackStartNr,maxNrSize,todo;
  DSite *si;
  RemoteSite *rs; 
  PD((TCP_CONNECTIONH,"tcpPreReadHandler invoked r:%x",r));  
  if(smallMustRead(fd,pos,PREREAD_NO_BYTES,PREREAD_HANDLER_TRIES))
    goto tcpPreFailure; 
  pos +=5;
  header =  tcpOpenMsgBuffer->readyForRead(todo);
  
  if(header!=TCP_MYSITE && header!=TCP_MYSITE_ACK && header!=TCP_MYSITE_HANDOVER)
    goto tcpPreFailure;
  if(smallMustRead(fd,pos,todo,PREREAD_HANDLER_TRIES)) 
    goto tcpPreFailure;

#ifdef ROBUST_UNMARSHALER
  int e1,e2,e3;
  ackStartNr = unmarshalNumberRobust(tcpOpenMsgBuffer, &e1);
  maxNrSize= unmarshalNumberRobust(tcpOpenMsgBuffer, &e2);
  si = unmarshalDSiteRobust(tcpOpenMsgBuffer, &e3);
  *error = e1 || e2 || e3;
#else
  ackStartNr = unmarshalNumber(tcpOpenMsgBuffer);
  maxNrSize= unmarshalNumber(tcpOpenMsgBuffer);
  si = unmarshalDSite(tcpOpenMsgBuffer);
#endif

  tcpOpenMsgBuffer->unmarshalEnd();
  rs = si->getRemoteSite();
  if(rs == NULL)
    goto tcpPreFailure;
  
  // The connection established is not intended
  // as a readconnection but as a writeconnection. 
  if(header==TCP_MYSITE_HANDOVER){
    // get rid of the read connection. Its not needed any longer.
    tcpCache->remove(r);
    readConnectionManager->freeConnection(r);
    // There might be a write connection attached to the 
    // RemoteSite alredy....
    WriteConnection *w = rs->getWriteConnection();
    if(w != NULL && (w->isProbing() || w->isMyInitiative() || 
		     w->isHisInitiative() || w->isTmpDwn())){
      //Clear the cause, The writecon is gona be opened.
      tcpCache->remove(w);
      if(w->isProbing())w->clearProbing();
      if(w->isMyInitiative())w->clearMyInitiative();
      if(w->isHisInitiative())w->clearHisInitiative();
      if(w->isTmpDwn())w->clearTmpDwn();
      if(w->testFlag(HIS_MY_TMP))w->clearFlag(HIS_MY_TMP);
      w->setFD(fd);
      w->setOpening();
      w->setFlag(HANDED_OVER);
      tcpCache->add(w);
      printf("A read is transfered into a write\n");
      OZ_registerReadHandler(fd,tcpConnectionHandler,(void *)w);
      return 0;
    }
    if(w ==  NULL){
      rs->handedOver = TRUE; 
      w = writeConnectionManager->allocConnection(rs,fd);
      rs->setWriteConnection(w);
      w->setOpening();
      w->setFlag(HANDED_OVER);
      tcpCache->add(w);
      OZ_registerReadHandler(fd,tcpConnectionHandler,(void *)w);
      return 0;
    }
    else{
      // We alredy got a writeconnection. The allocated fd must be closed. 
      osclose(fd);
    }
  }
  old = si->getRemoteSite()->getReadConnection();
  if(old!=NULL){old->close();}
  
  // To be sure that we have a writeConnection if the other peer is
  // behind a firewall we close all open atempts from that site until
  // there is a proper WriteCon.
  if(si->getRemoteSite()->handedOver && 
     (si->getRemoteSite()->getWriteConnection() == NULL 
      || si->getRemoteSite()->getWriteConnection()->isClosing())){
    goto tcpPreFailure;}
  
  si->getRemoteSite()->setReadConnection(r);
  
  if(header==TCP_MYSITE_ACK) if(!r->resend()) goto tcpPreFailure;

  r->setMaxSizeAck(maxNrSize);
  r->clearOpening();
#ifdef PERDIOLOGLOW
  printf("!!!ok%d\n",si->getTimeStamp()->pid); 
#endif
  OZ_registerReadHandler(fd,tcpReadHandler,(void *)r);  
  return 0;

tcpPreFailure:
#ifdef PERDIOLOGLOW
  printf("!!!oq%d\n",fd); 
#endif
  
  if(fd!=LOST) osclose(fd);
  tcpCache->remove(r);
  readConnectionManager->freeConnection(r);
  return 1;  
}
