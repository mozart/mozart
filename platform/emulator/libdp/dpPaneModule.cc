/*
 *  Authors:
 *    Andreas Sundstroem (andreas@sics.se)
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


#ifndef MODULES_LINK_STATIC

#include "modDPPane-if.cc"

#endif
