/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
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
#include "dpBase.hh"

#include "perdio.hh"
#include "table.hh"
#include "dpMarshaler.hh"

#include "builtins.hh"
#include "os.hh"
#include "space.hh"
#include "newmarshaler.hh"

#define USE_VS_MSGBUFFERS

//
#ifdef USE_VS_MSGBUFFERS
#include "virtual.hh"
#endif

OZ_BI_define(BIcrash,0,0)   /* only for debugging */
{
  initDP();

  exit(1);

  return PROCEED;
} OZ_BI_end


#ifdef DEBUG_PERDIO

OZ_BI_define(BIdvset,2,0)
{
  initDP();

  OZ_declareInt(0,what);
  OZ_declareInt(1,val);

  if (val) {
    DV->set(what);
  } else {
    DV->unset(what);
  }
  return PROCEED;
} OZ_BI_end

#else

OZ_BI_define(BIdvset,2,0)
{
  initDP();

  OZ_declareInt(0,what);
  OZ_declareInt(1,val);
  OZ_warning("has no effect - you must compile with DEBUG_PERDIO");
  return PROCEED;
} OZ_BI_end

#endif


/**********************************************************************/
/*   Misc Builtins                                            */
/**********************************************************************/

OZ_BI_define(BItablesExtract,0,1)
{
  initDP();

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
  initDP();

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
// kost@: who put the following comment???
// note that the marshaler always give you these statistics - should
// be changed
OZ_BI_define(BIperdioStatistics,0,1)
{
  initDP();

  OZ_Term dif_send_ar=oz_nil();
  OZ_Term dif_recv_ar=oz_nil();
  int i;
  for (i=0; i<DIF_LAST; i++) {
    dif_send_ar=oz_cons(oz_pairAI(dif_names[i].name,dif_counter[i].getSend()),
                        dif_send_ar);
    dif_recv_ar=oz_cons(oz_pairAI(dif_names[i].name,dif_counter[i].getRecv()),
                        dif_recv_ar);
  }
  OZ_Term dif_send=OZ_recordInit(oz_atom("dif"),dif_send_ar);
  OZ_Term dif_recv=OZ_recordInit(oz_atom("dif"),dif_recv_ar);

  OZ_Term misc_send_ar=oz_nil();
  OZ_Term misc_recv_ar=oz_nil();
  for (i=0; i<MISC_LAST; i++) {
    misc_send_ar=oz_cons(oz_pairAI(misc_names[i],misc_counter[i].getSend()),
                         misc_send_ar);
    misc_recv_ar=oz_cons(oz_pairAI(misc_names[i],misc_counter[i].getRecv()),
                         misc_recv_ar);
  }
  OZ_Term misc_send=OZ_recordInit(oz_atom("misc"),misc_send_ar);
  OZ_Term misc_recv=OZ_recordInit(oz_atom("misc"),misc_recv_ar);

  OZ_Term mess_send_ar=oz_nil();
  OZ_Term mess_recv_ar=oz_nil();
  for (i=0; i<M_LAST; i++) {
    mess_send_ar=oz_cons(oz_pairAI(mess_names[i],mess_counter[i].getSend()),
                         mess_send_ar);
    mess_recv_ar=oz_cons(oz_pairAI(mess_names[i],mess_counter[i].getRecv()),
                         mess_recv_ar);
  }
  OZ_Term mess_send=OZ_recordInit(oz_atom("messages"),mess_send_ar);
  OZ_Term mess_recv=OZ_recordInit(oz_atom("messages"),mess_recv_ar);


  OZ_Term send_ar=oz_nil();
  send_ar = oz_cons(oz_pairA("dif",dif_send),send_ar);
  send_ar = oz_cons(oz_pairA("misc",misc_send),send_ar);
  send_ar = oz_cons(oz_pairA("messages",mess_send),send_ar);
  OZ_Term send=OZ_recordInit(oz_atom("send"),send_ar);

  OZ_Term recv_ar=oz_nil();
  recv_ar = oz_cons(oz_pairA("dif",dif_recv),recv_ar);
  recv_ar = oz_cons(oz_pairA("misc",misc_recv),recv_ar);
  recv_ar = oz_cons(oz_pairA("messages",mess_recv),recv_ar);
  OZ_Term recv=OZ_recordInit(oz_atom("recv"),recv_ar);


  OZ_Term ar=oz_nil();
  ar=oz_cons(oz_pairA("send",send),ar);
  ar=oz_cons(oz_pairA("recv",recv),ar);
  OZ_RETURN(OZ_recordInit(oz_atom("perdioStatistics"),ar));
} OZ_BI_end


OZ_BI_define(BIslowNet,2,0)
{
  initDP();

  oz_declareIntIN(0,arg0);
  oz_declareIntIN(1,arg1);
#ifdef SLOWNET
  TSC_LATENCY = arg0;
  TSC_TOTAL_A = arg1;
  printf("New slownetvals ms:%d buff:%d \n", TSC_LATENCY, TSC_TOTAL_A);
#else
  printf("Slownet not installed\n");
#endif
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIclose,1,0)
{
  oz_declareIntIN(0,time);
  dpExitWithTimer((unsigned int) time);
  osExit(0);
  return PROCEED;
} OZ_BI_end

//
// kost@: temporary (?) builtin for comparing performances of
// marshalling/unmarshalling stuff. It takes a data structure to be
// tested and a number of times to repeat. Before loop it does it once
// and checks correctness of the operation;
//
// Here is essentially a copy of builtin's 'oz_eqeq()':
inline
Bool oz_eqeq(TaggedRef Ain,TaggedRef Bin)
{
  // simulate a shallow guard
  am.trail.pushMark();
  am.setShallowHeapTop(heapTop);
  OZ_Return ret = oz_unify(Ain,Bin,(ByteCode*)1);
  am.setShallowHeapTop(NULL);

  if (ret == PROCEED) {
    if (am.trail.isEmptyChunk()) {
      am.trail.popMark();
      return (OK);
    }

    oz_reduceTrailOnEqEq();
    return (NO);
  }

  oz_reduceTrailOnFail();
  return (NO);
}

//
OZ_BI_define(BImarshalerPerf,2,0)
{
  initDP();

  oz_declareIN(0, value);
  oz_declareIntIN(1, count);
  OZ_Term result;
  MsgBuffer *buf;
  unsigned int timeNow;
  int heapNow;

  // check marshaler (old one);
#ifdef USE_VS_MSGBUFFERS
  buf = getCoreVirtualMsgBuffer((DSite *) 0);
#else
  buf = getRemoteMsgBuffer((DSite *) 0);
#endif
  buf->marshalBegin();
  marshalTermRT(value, buf);
  buf->marshalEnd();
  buf->unmarshalBegin();
  result = unmarshalTermRT(buf);
  buf->unmarshalEnd();
#ifdef USE_VS_MSGBUFFERS
  dumpVirtualMsgBufferImpl(buf);
#else
  dumpRemoteMsgBuffer(buf);
#endif
  if (!oz_eqeq(value, result))
    return (oz_raise(E_ERROR, E_SYSTEM, "odd old marshaler", 0));
  // ... new one:
#ifdef USE_VS_MSGBUFFERS
  buf = getCoreVirtualMsgBuffer((DSite *) 0);
#else
  buf = getRemoteMsgBuffer((DSite *) 0);
#endif
  buf->marshalBegin();
  newMarshalTerm(value, buf);
  buf->marshalEnd();
  buf->unmarshalBegin();
  result = newUnmarshalTerm(buf);
  buf->unmarshalEnd();
#ifdef USE_VS_MSGBUFFERS
  dumpVirtualMsgBufferImpl(buf);
#else
  dumpRemoteMsgBuffer(buf);
#endif
  if (!oz_eqeq(value, result))
    return (oz_raise(E_ERROR, E_SYSTEM, "odd new marshaler", 0));

  // Now let's spin for a while. First, do marshaling speed:
  timeNow = osUserTime();
  heapNow = getUsedMemory();
  for (int i = 0; i < count; i++) {
#ifdef USE_VS_MSGBUFFERS
    buf = getCoreVirtualMsgBuffer((DSite *) 0);
#else
    buf = getRemoteMsgBuffer((DSite *) 0);
#endif
    buf->marshalBegin();
    marshalTermRT(value, buf);
    buf->marshalEnd();
#ifdef USE_VS_MSGBUFFERS
    dumpVirtualMsgBufferImpl(buf);
#else
    dumpRemoteMsgBuffer(buf);
#endif
  }
  fprintf(stdout, "old marshaler/marshaling:");
  printTime(stdout, "r: ", (osUserTime() - timeNow));
  printMem(stdout, ", h: ", (getUsedMemory() - heapNow)*KB);
  fprintf(stdout, "\n"); fflush(stdout);

  // ... new marshaler:
  timeNow = osUserTime();
  heapNow = getUsedMemory();
  for (int i = 0; i < count; i++) {
#ifdef USE_VS_MSGBUFFERS
    buf = getCoreVirtualMsgBuffer((DSite *) 0);
#else
    buf = getRemoteMsgBuffer((DSite *) 0);
#endif
    buf->marshalBegin();
    newMarshalTerm(value, buf);
    buf->marshalEnd();
#ifdef USE_VS_MSGBUFFERS
    dumpVirtualMsgBufferImpl(buf);
#else
    dumpRemoteMsgBuffer(buf);
#endif
  }
  fprintf(stdout, "new marshaler/marshaling:");
  printTime(stdout, "r: ", (osUserTime() - timeNow));
  printMem(stdout, ", h: ", (getUsedMemory() - heapNow)*KB);
  fprintf(stdout, "\n"); fflush(stdout);

  // unmarshalling:
#ifdef USE_VS_MSGBUFFERS
  buf = getCoreVirtualMsgBuffer((DSite *) 0);
#else
  buf = getRemoteMsgBuffer((DSite *) 0);
#endif
  buf->marshalBegin();
  marshalTermRT(value, buf);
  buf->marshalEnd();
  //
  timeNow = osUserTime();
  heapNow = getUsedMemory();
  for (int i = 0; i < count; i++) {
    buf->unmarshalBegin();
    result = unmarshalTermRT(buf);
    buf->unmarshalEnd();
  }
  fprintf(stdout, "old marshaler/unmarshaling:");
  printTime(stdout, "r: ", (osUserTime() - timeNow));
  printMem(stdout,", h: ", (getUsedMemory() - heapNow)*KB);
  fprintf(stdout, "\n"); fflush(stdout);
#ifdef USE_VS_MSGBUFFERS
  dumpVirtualMsgBufferImpl(buf);
#else
  dumpRemoteMsgBuffer(buf);
#endif

#ifdef USE_VS_MSGBUFFERS
  buf = getCoreVirtualMsgBuffer((DSite *) 0);
#else
  buf = getRemoteMsgBuffer((DSite *) 0);
#endif
  buf->marshalBegin();
  newMarshalTerm(value, buf);
  buf->marshalEnd();
  //
  timeNow = osUserTime();
  heapNow = getUsedMemory();
  for (int i = 0; i < count; i++) {
    buf->unmarshalBegin();
    result = newUnmarshalTerm(buf);
    buf->unmarshalEnd();
  }
  fprintf(stdout, "new marshaler/unmarshaling:");
  printTime(stdout, "r: ", (osUserTime() - timeNow));
  printMem(stdout, ", h: ", (getUsedMemory() - heapNow)*KB);
  fprintf(stdout, "\n"); fflush(stdout);
  //
#ifdef USE_VS_MSGBUFFERS
  dumpVirtualMsgBufferImpl(buf);
#else
  dumpRemoteMsgBuffer(buf);
#endif

  //
  return (PROCEED);
} OZ_BI_end

/*
 * The builtin table
 */

#ifndef MODULES_LINK_STATIC

#include "modDPMisc-if.cc"

#endif
