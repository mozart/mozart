/*
 *  Authors:
 *    Anna Neiderud (annan@sics.se)
 *
 *  Contributors:
 *    Erik Klintskog (erik@sics.se)
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

#include "base.hh"
#include "builtins.hh"
#include "dpBase.hh"

#include "comObj.hh"
#include "transObj.hh"
#include "tcpTransObj.hh"
#include "dsite.hh"
#include "connection.hh"
#include "tcpConnection.hh"

// To make call to connectprocedure
#include "perdio.hh"
#include "thr_int.hh"
void doPortSend(PortWithStream *port,TaggedRef val,Board*);
// Order from comObj to connect.

void doConnect(ComObj *comObj) {
  DSite *site=comObj->getSite();
  /*  Thread *tt = oz_newThreadToplevel();
      tt->pushCall(defaultConnectionProcedure,
      oz_int(site->getAddress()),oz_int(site->getPort()),
      oz_int((int) comObj));
  */
  // Send connect(Requestor LocalOzState DistOzState)
  OZ_Term Requestor=oz_int((int) comObj);
  OZ_Term LocalOzState=OZ_recordInit(oz_atom("localstate"),
                                     oz_cons(oz_pairA("connectionFunctor",
                                                      defaultConnectionProcedure),
                                             oz_cons(oz_pairA("localState",
                                                              oz_nil()),
                                                     oz_nil())));
  OZ_Term addrinfo=OZ_recordInit(oz_atom("ip_addr"),
                                 oz_cons(oz_pairAI("addr",(unsigned int)
                                                   site->getAddress()),
                                         oz_cons(oz_pairAI("port",
                                                           site->getPort()),
                                                 oz_nil())));
  OZ_Term DistOzState=OZ_recordInit(oz_atom("diststate"),
                                    oz_cons(oz_pairAA("type","ordinary"),
                                            oz_cons(oz_pairA("parameter",
                                                             addrinfo),
                                                    oz_nil())));

  OZ_Term command = OZ_recordInit(oz_atom("connect"),
                                  oz_cons(oz_pair2(oz_int(1),Requestor),
                                  oz_cons(oz_pair2(oz_int(2),LocalOzState),
                                  oz_cons(oz_pair2(oz_int(3),DistOzState),
                                          oz_nil()))));
  doPortSend(((PortWithStream *) tagged2Const(ConnectPort)), command, NULL);
}

OZ_BI_define(BIgetConnGrant,4,0){
  oz_declareNonvarIN(0,requestor);
  OZ_declareTerm(1,type);
  OZ_declareBool(2,canWait);
  OZ_declareTerm(3,var)

  OZ_Term tcp=oz_atom("tcp");
  TransController *transController;
  if(oz_eq(type,tcp))
    transController=tcptransController;
  else
    OZ_error("Unknown transport media");
  if(canWait) {
    // If we can wait, then there must be a valid comobj in the
    // requestor for queing purposes.
    if(!OZ_isInt(requestor))
      OZ_error("A requestor must be specified to be able to wait");
    ComObj *comObj=(ComObj *) oz_intToC(requestor);
    comObj->connectVar=var;
    transController->getTransObj(comObj);
    // When the transObj is ready var will be bound
  }
  else {
    TransObj *tro=transController->getTransObj();
    // Bind var now
    if(tro!=NULL) {
      return OZ_unify(var,
                      OZ_recordInit(oz_atom("grant"),
                                    oz_cons(oz_pairAI("key",
                                                      (int) tro),
                                            oz_nil())));
    }
    else
      return OZ_unify(var,oz_atom("busy"));
  }
  return OZ_ENTAILED;
}OZ_BI_end

OZ_BI_define(BIfreeConnGrant,2,0){
  oz_declareNonvarIN(0,requestor);
  oz_declareNonvarIN(1,grant);

  ComObj *comObj=(ComObj *) oz_intToC(requestor);
  if(!oz_isSRecord(grant))
    OZ_typeError(1, "record");

  SRecord *sgrant = tagged2SRecord(grant);
  int index = sgrant->getIndex(oz_atom("key"));
  if (index>=0) {
    OZ_Term t = sgrant->getArg(index);
    TransObj *transObj=(TransObj *) OZ_intToC(t);
    handback(comObj,transObj);
  }
  else
    OZ_error("Unknown grant freed");
  return OZ_ENTAILED;
}OZ_BI_end

// The transController delivers a transObj(= a right to use a resource)
void transObjReady(ComObj *comObj,TransObj *transObj) {
  OZ_unify(comObj->connectVar,
           OZ_recordInit(oz_atom("grant"),
                         oz_cons(oz_pairAI("key",
                                           (int) transObj),
                                 oz_nil())));
}

OZ_BI_define(BIhandover,3,0){
  OZ_declareTerm(0,requestor);
  oz_declareNonvarIN(1,grant);
  OZ_declareTerm(2,settings);

  if(!oz_isSRecord(grant))
    OZ_typeError(1, "record");
  if(!OZ_isRecord(settings))
    OZ_typeError(2, "record");

  SRecord *sgrant = tagged2SRecord(grant);
  int index = sgrant->getIndex(oz_atom("key"));
  if (index>=0) {
    OZ_Term t = sgrant->getArg(index);
    ComObj *comObj;
    if(OZ_isInt(requestor))
      comObj=(ComObj *) OZ_intToC(requestor);
    else
      comObj=comController->newComObj(NULL,0);
    TransObj *transObj=(TransObj *) OZ_intToC(t);
    transObj->setUp(comObj->getSite(),comObj,settings);
    if(OZ_isInt(requestor)) {
      if(!comObj->handover(transObj))
        handback(comObj,transObj);
    }
    else
      comObj->accept(transObj);
  }
  else
    OZ_raiseC("Invalid grant",1);

  return OZ_ENTAILED;
}OZ_BI_end

// The comObj hands back a transObj that it is done with
void handback(ComObj *comObj, TransObj *transObj) {
//    printf("handback %d %x %x\n",getpid(),comObj,transObj);
  transObj->close();
}

// The comObj is informing us that it no longer needs the connection
// it was waiting for.
void comObjDone(ComObj *comObj) {
//    printf("comObjDone %d %x\n",getpid(),comObj);
  OZ_Term command=OZ_recordInit(oz_atom("abort"),
                                oz_cons(oz_pair2(oz_int(1),
                                                 oz_int((int) comObj)),
                                        oz_nil()));
  doPortSend(((PortWithStream *) tagged2Const(ConnectPort)), command, NULL);
}

// For now the accept procedure is initiated as a tcp accepter opened in
// c and a thread listening to it
Bool initAccept() {
  return TRUE;
  // return tcpInitAccept();
}

OZ_BI_define(BIconnFailed,2,0) {
  oz_declareNonvarIN(0,requestor);
  oz_declareNonvarIN(1,reason);

  ComObj *comObj=(ComObj *) oz_intToC(requestor);
  DSite *site=comObj->getSite();

  if(oz_eq(reason,oz_atom("perm"))) {
    site->discoveryPerm();
    site->probeFault(PROBE_PERM);
  }
  else if(oz_eq(reason,oz_atom("temp"))) {
    // This could be reported to the comObj, but the comObj also
    // has its own timer to discover this.
    ;
  }
  else
    Assert(0);

  return PROCEED;
}OZ_BI_end

/* Hack to set the address of the Machine, Erik */
OZ_BI_define(BIsetListenPort,2,0)
{
  OZ_declareInt(0,port);
  OZ_declareVirtualString(1,nodename);
  tcpListenPort(port,nodename);
  return PROCEED;
}OZ_BI_end
