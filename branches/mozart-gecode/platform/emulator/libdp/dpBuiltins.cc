/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#include "base.hh"
#include "perdio.hh"
#include "table.hh"
#include "os.hh"
#include "builtins.hh"
#include "wsock.hh"
#include "fail.hh"
#include "port.hh"
#include "network.hh"

#ifndef WINDOWS
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#endif
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

/*************************************************************/
/* Port interface to Gate                                    */
/*************************************************************/

//
// Trigger holder;
static int perdioTrigger = 1;

static OZ_Term GateStream;

//
// Interface method, BTW
Bool isPerdioInitialized() {
  return (!perdioTrigger);
}

void perdioInitLocal()
{
  //
#ifdef DEBUG_CHECK
  // fprintf(stderr, "Waiting 10 secs... hook up (pid %d)!\n", osgetpid());
  // fflush(stderr);
  // sleep(10);
#endif

  if(perdioTrigger==0)
    return;

  //
  perdioTrigger = 0;
  initPerdio();

  //
  // The gate is implemented as a Port reciding at location 0 in
  // the ownertable. The gateStream is keept alive, the Connection 
  // library will fetch it later.
  // The port is made persistent so it should not disapear.
  //
  GateStream = oz_newVariable();
  OZ_protect(&GateStream);
  {
    Tertiary *t=(Tertiary*)new PortWithStream(oz_currentBoard(),GateStream);
    globalizeTert(t);
    int ind = t->getIndex();
    Assert(ind == 0);
    OwnerEntry* oe=OT->getOwner(ind);
    oe->makePersistent();
  }
  //
  // potentially - kost@, Per - 
  // link perdio interface in perdioInit();
}

OZ_BI_define(BIGetPID,0,1)
{
  // pid = pid(host:String port:Int time:Int#Int)

  //
  perdioInitLocal();

  char *nodename = oslocalhostname();
  if(nodename==NULL) { return oz_raise(E_ERROR,E_SYSTEM,"getPidUname",0); }
  struct hostent *hostaddr=gethostbyname(nodename);
  free(nodename);
  struct in_addr tmp;
  memcpy(&tmp,hostaddr->h_addr_list[0],sizeof(in_addr));

  OZ_Term host = oz_pairA("host",oz_string(inet_ntoa(tmp)));
  OZ_Term port = oz_pairA("port",oz_int(myDSite->getPort()));
  OZ_Term time = 
    oz_pairA("time",
	     OZ_pair2(oz_unsignedLong((unsigned long) myDSite->getTimeStamp()->start),
		      oz_int(myDSite->getTimeStamp()->pid)));
  // NOTE: converting time_t to an unsigned long, maybe a [long] double!

  OZ_Term l = oz_cons(host,oz_cons(port,oz_cons(time,oz_nil())));
  OZ_RETURN(OZ_recordInit(OZ_atom("PID"),l));
} OZ_BI_end

OZ_BI_define(BIReceivedPID,1,0)
{
  oz_declareIN(0,stream);
  //
  perdioInitLocal();
  return oz_unify(GateStream,stream);
} OZ_BI_end


OZ_BI_define(BITicket2Port,4,1)
{
  oz_declareVirtualStringIN(0,host);
  oz_declareIntIN(1,port);
  oz_declareNonvarIN(2,timeV);
  oz_declareIntIN(3,pid);

  //
  perdioInitLocal();

  time_t time;
  if (oz_isSmallInt(timeV)) {
    int i = oz_IntToC(timeV);
    if (i <= 0) goto tbomb;
    time = (time_t) i;
  } else if (oz_isBigInt(timeV)) {
    unsigned long i = tagged2BigInt(timeV)->getUnsignedLong();
    if (i==0 && i == OzMaxUnsignedLong) goto tbomb;
    time = (time_t) i;
  } else {
  tbomb:
    return oz_raise(E_ERROR,E_SYSTEM,"PID.send",2,
		    OZ_atom("badTime"),OZ_in(2));
  }
    
  struct hostent *hostaddr = gethostbyname(host);
  if (!hostaddr) {
    return oz_raise(E_ERROR,E_SYSTEM,"PID.send",2,
		    OZ_atom("gethostbyname"),OZ_in(0));
  }
  struct in_addr tmp;
  memcpy(&tmp,hostaddr->h_addr_list[0],sizeof(in_addr));
  ip_address addr;
  addr = ntohl(tmp.s_addr);
  TimeStamp ts(time,pid);
  DSite *site = findDSite(addr, port, ts);

  if (!site) {
    return oz_raise(E_ERROR,E_SYSTEM,"Ticket2Port",4,
		    OZ_atom("findDSite"),OZ_in(0),OZ_in(1),
		    OZ_in(2));}
  
  OZ_RETURN(getGatePort(site));
} OZ_BI_end

//
OZ_BI_define(BIsetNetBufferSize,1,0)
{
  OZ_Term s = OZ_in(0);
  DEREF(s,_1,tagS);
  int size = 0;
  if (isSmallIntTag(tagS))
    size = smallIntValue(s);
  if(size < 0)
    oz_raise(E_ERROR,E_KERNEL,
	     "NetBufferSize must be of type int and larger than 0",0);
  PortSendTreash = size * 20000;
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIgetNetBufferSize,0,1)
{
  OZ_RETURN(oz_int(PortSendTreash / 20000));
} OZ_BI_end


OZ_BI_define(BIcrash,0,0)   /* only for debugging */
{
  exit(1);  

  return PROCEED;
} OZ_BI_end

Bool isWatcherEligible(Tertiary *c){
  switch(c->getType()){
  case Co_Object:
  case Co_Cell:
  case Co_Lock:
  case Co_Port: return TRUE;
  default: return FALSE;}
  Assert(0);
  return FALSE;
}
    
OZ_BI_define(BIhwInstall,3,0){
  OZ_Term e0        = OZ_in(0);
  OZ_Term c0        = OZ_in(1);
  OZ_Term proc      = OZ_in(2);  
  

  NONVAR(c0, c);
  SRecord  *condStruct;
  if(oz_isSRecord(c)) condStruct = tagged2SRecord(c);
  else return IncorrectFaultSpecification;

  if(isVariableSpec(condStruct)){ 
    DEREF(e0,vs_ptr,vs_tag);
    if(!isVariableTag(vs_tag)) return PROCEED;  // mm3
    VarKind vk=classifyVar(vs_ptr);            //mm3 - follow
    if(vk == VAR_KINDED) return IncorrectFaultSpecification;
    return varWatcherInstall(vs_ptr,condStruct,proc);}
  
  NONVAR(e0, e);
  Tertiary* tert;
  if(!oz_isConst(e)) return IncorrectFaultSpecification;
  tert = tagged2Tert(e);
  if(!isWatcherEligible(tert))return IncorrectFaultSpecification;
  return WatcherInstall(tert,condStruct,proc);

}OZ_BI_end


OZ_BI_define(BIhwDeInstall,3,0){
  OZ_Term e0        = OZ_in(0);
  OZ_Term c0        = OZ_in(1);
  OZ_Term proc      = OZ_in(2);  
  
  NONVAR(c0, c);
  SRecord  *condStruct;
  if(oz_isSRecord(c)) condStruct = tagged2SRecord(c);
  else return IncorrectFaultSpecification;
    
  if(isVariableSpec(condStruct)){ 
    DEREF(e0,vs_ptr,vs_tag);
    if(!isVariableTag(vs_tag)) return PROCEED;  // mm3
    VarKind vk=classifyVar(vs_ptr);            //mm3 - follow
    if((vk==VAR_KINDED) || (vk==VAR_FREE) || 
       (vk==VAR_READONLY)) return IncorrectFaultSpecification;
    return varWatcherDeinstall(vs_ptr,condStruct,proc);} //mm3 - follow

  NONVAR(e0, e);
  if(!oz_isConst(e)) return IncorrectFaultSpecification;
  Tertiary* tert;
  tert = tagged2Tert(e);
  if(!isWatcherEligible(tert)) return IncorrectFaultSpecification;
  return WatcherDeInstall(tert,condStruct,proc);

}OZ_BI_end

OZ_BI_define(BIgetEntityCond,2,1)
{
OZ_Term e0 = OZ_in(0);
OZ_Term v0 = OZ_in(1);
  
  NONVAR(v0, v);
  if(!isAtom(v)) return IncorrectFaultSpecification;
  if(v==AtomVar) {
    DEREF(e0,vs_ptr,vs_tag);
    if(!isVariableTag(vs_tag)) goto normal;     // mm3
    VarKind vk=classifyVar(vs_ptr);             //mm3 - follow
    if((vk==VAR_KINDED) || (vk==VAR_FREE) || 
       (vk==VAR_READONLY)) goto normal;
    getEntityCondVar(makeTaggedRef(v));}         // mm3 - follow

  if(v!=AtomNonVar) return IncorrectFaultSpecification;

  NONVAR(e0, e);
  if(!oz_isConst(e)) return IncorrectFaultSpecification;

  Tertiary *tert = tagged2Tert(entity);
  if(!isWatcherEligible(tert)) return IncorrectFaultSpecification;
  EntityCond ec = getEntityCond(tert);
  if(ec!= ENTITY_NORMAL){
    OZ_RETURN(listifyWatcherCond(ec));}

 normal:
  OZ_RETURN(oz_cons(AtomNormal,oz_nil()));

}OZ_BI_end


/**********************************************************************/
/*   Misc Builtins                                            */
/**********************************************************************/

OZ_BI_define(BItablesExtract,0,1)
{
  perdioInitLocal();

  OZ_Term borrowlist = oz_nil();
  int bt_size=BT->getSize();
  for(int ctr=0; ctr<bt_size; ctr++){
    BorrowEntry *be = BT->getEntry(ctr);
    if(be==NULL){continue;}
    Assert(be!=NULL);
    borrowlist = oz_cons(be->extract_info(ctr), borrowlist);}
  OZ_RETURN(oz_cons(OZ_recordInit(oz_atom("bt"),
                oz_cons(oz_pairAI("size", bt_size),
                oz_cons(oz_pairA("list", borrowlist), oz_nil()))),
              oz_cons(OT->extract_info(), oz_nil())));
} OZ_BI_end

OZ_BI_define(BIsiteStatistics,0,1)
{
  perdioInitLocal();

  int indx;
  DSite* found;
  GenHashNode *node = getPrimaryNode(NULL, indx);
  OZ_Term sitelist = oz_nil(); 
  int sent, received;
  Bool primary = TRUE;
  while(node!=NULL){
    GenCast(node->getBaseKey(),GenHashBaseKey*,found,DSite*);  
    if(found->remoteComm() && found->isConnected()){
      received = getNORM_RemoteSite(found->getRemoteSite());
      sent     = getNOSM_RemoteSite(found->getRemoteSite());}
    else{
      received = 0;
      sent = 0;}
    TimeStamp *ts = found->getTimeStamp();
    sitelist=
      oz_cons(OZ_recordInit(oz_atom("site"),
      oz_cons(oz_pairA("siteString", oz_atom(found->stringrep_notype())),
      oz_cons(oz_pairAI("port",(int)found->getPort()),
      oz_cons(oz_pairAI("timeint",(int)ts->start),
      oz_cons(oz_pairA("timestr",oz_atom(ctime(&ts->start))),
      oz_cons(oz_pairAI("ipint",(unsigned int)found->getAddress()),
      oz_cons(oz_pairAI("hval",(int)found),
      oz_cons(oz_pairAI("sent",sent),
      oz_cons(oz_pairAI("received",received),
      oz_cons(oz_pairA("table", oz_atom(primary?"p":"s")),
      oz_cons(oz_pairAI("strange",ts->pid),
      oz_cons(oz_pairAI("type",(int)found->getTypeStatistics()),
	      oz_nil())))))))))))),sitelist);
    if(primary){
      node = getPrimaryNode(node,indx);
      if(node!=NULL) {
	continue;}
      else primary = FALSE;}
    node = getSecondaryNode(node,indx);}
  OZ_RETURN(sitelist);
  
} OZ_BI_end


//
#ifndef VIRTUALSITES

/**********************************************************************/
/*   Virtual Site Builtins         KOST-LOOK                          */
/**********************************************************************/

//
// Builtins for virtual sites - only two of them are needed:
// (I) Creating a new mailbox (at the master site):
OZ_BI_define(BIVSnewMailbox,0,1)
{
  return oz_raise(E_ERROR, E_SYSTEM,
		  "VSnewMailbox: virtual sites not configured", 0);
} OZ_BI_end

//
// (II) Initializing a virtual site given its mailbox (which contains
// also the parent's id);
OZ_BI_define(BIVSinitServer,1,0)
{
  return oz_raise(E_ERROR, E_SYSTEM,
		  "VSinitServer: virtual sites not configured", 0);
} OZ_BI_end

//
//
OZ_BI_define(BIVSremoveMailbox,1,0)
{
  return oz_raise(E_ERROR, E_SYSTEM,
		  "VSremoveMailbox: virtual sites not configured", 0);
} OZ_BI_end

#endif // VIRTUALSITES






