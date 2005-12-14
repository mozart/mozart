/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
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
#pragma implementation "msl_endRouter.hh"
#endif

#include "msl_endRouter.hh"
#include "msl_comObj.hh"
#include "msl_msgContainer.hh"
#include "msl_dct.hh"
#include "msl_interRouter.hh"
#include "msl_timers.hh"
#include "msl_dsite.hh"

namespace _msl_internal{

  const int BYTE_ByteBuffer_CUTOFF=200;
  const int HEADER  = 11;  // msg header length
  const int DAC_HEADER = 54; // DAC msg header length

  //! marshal size is with 64 bytes less in order to avoid splitting the msg  
  const int E_BYTE_DEF_SIZE= 4000-HEADER-DAC_HEADER;



  class EndRouterDeliver: public Event{
  private:
    EndRouter* er; 

    EndRouterDeliver(const EndRouterDeliver&):er(NULL){}
    EndRouterDeliver& operator=(const EndRouterDeliver&){ return *this; }
  public: 
    EndRouterDeliver(EndRouter *e):er(e){}
    void event_execute(MsgnLayerEnv* env){
      if (er) {
	er->writeHandler(); 
	er = NULL; 
      }
    }
    void m_makeGCpreps(){;} // Make sure the endrouter is NOT removed
    virtual ~EndRouterDeliver(){}
  };



  /******************** constructor/destructor ********************/

  EndRouter::EndRouter(MsgnLayerEnv* env) :
    BufferedTransObj(E_BYTE_DEF_SIZE, env),
    a_succ(NULL), a_routeId(0), deliverEvent(NULL)
  {
    a_comObj=NULL;
  }



  /**************************** handling connection ********************/

  VirtualChannelInterface* EndRouter::m_closeConnection(){
    dssLog(DLL_DEBUG,"EndRouter closing down");
    return NULL;
  }

  void EndRouter::deliver() {
    //printf("deliver !!!\n");
    Assert(a_succ != NULL); //Assert: the route has already been established
    if (!deliverEvent){
      deliverEvent = new EndRouterDeliver(this); 
      a_mslEnv->m_appendImmediateEvent(deliverEvent); 
    }
  }
  
  void EndRouter::readyToReceive() { ;
  // do somethig else !!! like setting a boolean or something
  }



  void EndRouter::initRouteSetUp(DSite* dsVec[], int nrSites) {
    Site* succ = static_cast<Site*>(dsVec[0]);

    setSuccessor(succ->m_getComObj());
  
  
    a_routeId = a_mslEnv->a_routeIds++; 
    //printf("setup route:%d\n", a_routeId);
  
    // we register a route at this site in order to be able to terminate the route
    (a_mslEnv->a_interRouter)->registerRoute(a_comObj->getSite(), a_mslEnv->a_mySite, a_routeId, a_comObj->getSite(), a_succ->getSite());
  
    MsgCnt *msgC=new MsgCnt(C_SET_ROUTE ,true);
    msgC->pushSiteVal(a_comObj->getSite());     // dst site reference 
    msgC->pushSiteVal(a_mslEnv->a_mySite);  // src site reference 
    msgC->pushIntVal(a_routeId);    // route id

    msgC->pushIntVal(nrSites-1); // intermediary router

    // push the sites 
    for(int i = 1; i < nrSites; i++) 
      msgC->pushSiteVal(static_cast<Site*>(dsVec[i])); // intermediary site 

    // destroy the site array here
    delete dsVec;

    //printf("send C_SET_ROUTE\n");
    a_succ->m_send(msgC,MSG_PRIO_EAGER);
  }



  // ZACHARIAS: Below seems inefficient, why don't we marshal
  // everything at once??
  //
  // raph: The serialized message is encapsulated into another
  // message.  Apparently we avoid to encapsulate more than one frame.

  void EndRouter::writeHandler() {
    int len, acknum;
    BYTE* pos;
    MsgCnt* msgC;
    deliverEvent = NULL; 

    while (a_marshalBuffer->getFree() <= T_MIN_FOR_HEADER &&
	   (msgC = a_comObj->getNextMsgCnt(acknum))) {
      // marshal message
      marshal(msgC,acknum);

      // encode data to a_writeBuffer
      a_writeBuffer->encode();

      // !! be careful to read all the message at once
      len = a_writeBuffer->getReadBlock(pos);
      Assert(len > 0);
      a_mslEnv->a_OSWriteCounter++;

      DssSimpleDacDct *dac = new DssSimpleDacDct();
      dac->putData(pos, len);

      //printf("EndRouter::writeHandler len:%d\n", len);
      msgC = new MsgCnt(C_ROUTE, true);
    
      msgC->pushSiteVal(a_comObj->getSite());     // dst site reference 
      msgC->pushSiteVal(a_mslEnv->a_mySite);  // src site reference 
      msgC->pushIntVal(a_routeId);   // route id
      msgC->pushDctVal(dac);         // data area containing the msg to route
    
      //printf("send C_ROUTE\n");
  
      a_succ->m_send(msgC, MSG_PRIO_EAGER);
      a_writeBuffer->m_commitRead(len);
    }
  }

  //
  // THIS IMPLEMENTATION DOES NOT WORK WHEN:
  //
  // since we have now flow control data area containers will just be
  // forgotten if they can't deliver the entire payload
  //
  // typical fix: a list of unhandled/unfinished dacs connected to the
  // endrouter, the problem is pobably similar for inter-router

  // !! should also treat the case when the connection is lost
  void EndRouter::readHandler(DssSimpleDacDct *dac) {
    unsigned int len, ret;
    BYTE *pos;

    // read from dac to a_readBuffer (loop twice if necessary)
    do { 
      len = a_readBuffer->getWriteBlock(pos);
      //printf("EndRouter::readHandler dlen:%d, len:%d\n",dlen, len);
      ret = dac->getData(pos, len);       // write max len data to pos
      a_readBuffer->m_commitWrite(ret);   // update buffer
    } while (ret == len && a_readBuffer->getFree() > 0);

    // decode data to a_unmarshalBuffer
    if (a_readBuffer->decode()) {
      // unmarshal as many messages as possible
      while (unmarshal() == U_MORE) {}

    } else {
      // decoding has failed, close this connection
      Assert(0);
      printf(" ERROR IN ROUTE\n");
      a_comObj->m_closeErroneousConnection();
    }
  }



  void EndRouter::routeSetUp(int routeId) {
    ;//printf("the route:%d routeId is set up\n", routeId); //! more to come ...
  }

} //End namespace
