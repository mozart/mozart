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
  Assert(!comObj->connectgrantrequested);
  DSite *site=comObj->getSite();
  /*  Thread *tt = oz_newThreadToplevel();
      tt->pushCall(defaultConnectionProcedure,
      oz_int(site->getAddress()),oz_int(site->getPort()),
      oz_int((int) comObj));
  */
  // Send connect(Requestor LocalOzState DistOzState)
  // Requestor=requestor(id:SiteId req:comObj)
  OZ_Term Requestor=OZ_recordInit(oz_atom("requestor"),
				  oz_cons(oz_pairAA("id",
						    site->stringrep_notype()),
					  oz_cons(oz_pairAI("req",(int) comObj),
						  oz_nil())));
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

inline OZ_Return parseRequestor(OZ_Term requestor, 
				ComObj *&comObj, OZ_CONST char *&siteid) {
  if(OZ_isRecord(requestor)) {
    SRecord *srequestor = tagged2SRecord(requestor);
    int index = srequestor->getIndex(oz_atom("id"));
    if (index>=0) { 
      OZ_Term t0 = srequestor->getArg(index);
      NONVAR(t0,t);
      if(OZ_isAtom(t))
	siteid=OZ_atomToC(t);
      else return OZ_FAILED;
    }
    else return OZ_FAILED;

    index = srequestor->getIndex(oz_atom("req"));
    if (index>=0) { 
      OZ_Term t0 = srequestor->getArg(index);
      NONVAR(t0,t);
      if(OZ_isInt(t))
	comObj=(ComObj *) oz_intToC(t);
      else return OZ_FAILED;

      // The comObj might be returned and not used which is checked with valid,
      // or it may be reused which is checked by comparing the sites.
      if(comController->valid(comObj)) {
	DSite *site=comObj->getSite();
	if(site==NULL) { // this comObj has been reused for accept
//  	  fprintf(stderr,"reused for accept\n");
	  return OZ_FAILED;
	}
	else if(strcmp(site->stringrep_notype(),siteid)!=0) {
	  // reused for other site 
//  	  fprintf(stderr,"reused for other site\n");
	  return OZ_FAILED;
	}
	// The right one! Go ahead
      }
      else return OZ_FAILED;
    }
    else return OZ_FAILED;
    //    printf("parseR %s %d %s\n",toC(requestor),(int) comObj,siteid);
    return OZ_ENTAILED;
  }
  return OZ_FAILED;
}

OZ_BI_define(BIgetConnGrant,4,0){
  oz_declareNonvarIN(0,requestor);
  OZ_declareTerm(1,type);
  OZ_declareBool(2,canWait);
  OZ_declareTerm(3,var) 

  OZ_Term tcp=oz_atom("tcp");
  OZ_Return ret;
  TransController *transController;
  if(oz_eq(type,tcp))
    transController=tcptransController;
  else
    OZ_error("Unknown transport media");
  if(canWait) {
    // If we can wait, then there must be a valid comobj in the 
    // requestor for queing purposes.
    OZ_CONST char *unused;
    ComObj *comObj;
    ret=parseRequestor(requestor,comObj,unused);
    if(ret!=OZ_ENTAILED)
      return ret;
    if(comObj->getState()!=CLOSED_WF_HANDOVER)
      return OZ_FAILED;
    Assert(!comObj->connectgrantrequested);
    Assert(comObj->connectVar==(OZ_Term) 0x45);
    Assert(comObj->transtype==(OZ_Term) 0x46);
    comObj->connectVar=var;
    comObj->transtype=type;
    comObj->connectgrantrequested=TRUE;
    OZ_protect(&comObj->connectVar); // Protects connectVar from GC, 
                                     // must be unprotected when done
    OZ_protect(&comObj->transtype);
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
  OZ_CONST char *unused;
  ComObj *comObj;
  OZ_Return ret;
  ret=parseRequestor(requestor,comObj,unused);
  if(ret!=OZ_ENTAILED)
    return ret;
  SRecord *sgrant = tagged2SRecord(grant);
  int index = sgrant->getIndex(oz_atom("key"));
  if (index>=0) { 
    OZ_Term t = sgrant->getArg(index);
    TransObj *transObj=(TransObj *) OZ_intToC(t);

    transObj->close(FALSE);
  }
  else
    OZ_error("Unknown grant freed");
  return OZ_ENTAILED;
}OZ_BI_end

// The transController delivers a transObj(= a right to use a resource)
void transObjReady(ComObj *comObj,TransObj *transObj) {
  //  printf("got a transobj, now unifying\n");
  if(comObj->connectgrantrequested) {
    OZ_unify(comObj->connectVar,
	     OZ_recordInit(oz_atom("grant"),
			   oz_cons(oz_pairAI("key",
					     (int) transObj),
				   oz_nil())));
    OZ_unprotect(&comObj->connectVar);
    OZ_unprotect(&comObj->transtype);
    DebugCode(comObj->connectVar=(OZ_Term) 0x45);
    DebugCode(comObj->transtype=(OZ_Term) 0x46);
    comObj->connectgrantrequested=FALSE;
  }
  else
    transObj->close(FALSE);
}

OZ_BI_define(BIhandover,3,0){
  oz_declareNonvarIN(0,requestor);
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
    Bool accepting=FALSE;

    if(oz_eq(requestor,oz_atom("accept"))) {
      accepting=TRUE;
      comObj=comController->newComObj(NULL);
    }
    else {
      OZ_CONST char *siteid;
      OZ_Return ret;
      ret=parseRequestor(requestor,comObj,siteid);
//        printf("bef cmp %s %d %s\n",toC(requestor),(int) comObj,siteid);
      if(ret!=OZ_ENTAILED) {
	return ret;
      }
    }

    TransObj *transObj=(TransObj *) OZ_intToC(t);
    transObj->setUp(comObj->getSite(),comObj,settings);
    if(accepting) {
      comObj->accept(transObj);
      tcptransController->addRunning(comObj);
    }
    else {
      if(comObj->handover(transObj)) 
	tcptransController->addRunning(comObj);
      else 
	transObj->close(FALSE);
    }
  }
  else
    OZ_raiseC("Invalid grant",1);

  return OZ_ENTAILED;
}OZ_BI_end

// The comObj hands back a transObj that it is done with
void handback(ComObj *comObj, TransObj *transObj) {
//    printf("handback %d %x %x\n",getpid(),comObj,transObj);
  transObj->close(TRUE);
}

// The comObj is informing us that it no longer needs the connection
// it was waiting for.
void comObjDone(ComObj *comObj) {
  if(comObj->connectgrantrequested) {
    //    OZ_unify(comObj->connectVar,oz_atom("abort"));
    OZ_Term tcp=oz_atom("tcp");
    TransController *transController;
    if(oz_eq(comObj->transtype,tcp))
      transController=tcptransController;
    else
      OZ_error("Unknown transport media");
    transController->comObjDone(comObj);
    OZ_unprotect(&comObj->connectVar);
    OZ_unprotect(&comObj->transtype);
    comObj->connectgrantrequested=FALSE;
    DebugCode(comObj->connectVar=(OZ_Term) 0x45);
    DebugCode(comObj->transtype=(OZ_Term) 0x46);
  }
  // Requestor=requestor(id:SiteId req:comObj)
  OZ_Term Requestor=OZ_recordInit(oz_atom("requestor"),
				  oz_cons(oz_pairAA("id",
						    comObj->getSite()->stringrep_notype()),
					  oz_cons(oz_pairAI("req",(int) comObj),
						  oz_nil())));
  OZ_Term command=OZ_recordInit(oz_atom("abort"),
				oz_cons(oz_pair2(oz_int(1),
						 Requestor),
					oz_nil()));
  doPortSend(((PortWithStream *) tagged2Const(ConnectPort)), command, NULL);  
}

OZ_BI_define(BIconnFailed,2,0) {
  oz_declareNonvarIN(0,requestor);
  oz_declareNonvarIN(1,reason);

  DSite *site;
  OZ_CONST char *siteid;
  ComObj *comObj;
  OZ_Return ret;
  ret=parseRequestor(requestor,comObj,siteid);
  if(ret!=OZ_ENTAILED)
    return ret;

  site=comObj->getSite();
  if(oz_eq(reason,oz_atom("perm"))) {
    site->discoveryPerm();
    site->probeFault(PROBE_PERM);
  } 
  else if(oz_eq(reason,oz_atom("temp"))) {
    // This could be reported to the comObj, but the comObj also
    // has its own timer to discover this.
    ;
  }
  else { // AN: For now do as for temp. Could go ahead and inform comObj.
//      printf("connFailed due to");
//      printf(" %s\n",OZ_atomToC(reason));
    ;
  }

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
