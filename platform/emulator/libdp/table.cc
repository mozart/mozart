/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
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

#if defined(INTERFACE)
#pragma implementation "table.hh"
#endif

#include "table.hh"
#include "value.hh"
#include "var.hh"
#include "msgbuffer.hh"
#include "dpMarshaler.hh"
#include "state.hh"
#include "fail.hh"
#include "protocolState.hh"

int NetHashTable::hashFunc(NetAddress *na){
  unsigned char *p = (unsigned char*) na;
  int i;
  unsigned h = 0, g;

  for(i=0;i<8; i++,p++) {
    h = (h << 4) + (*p);
    if ((g = h & 0xf0000000)) {
      h = h ^ (g >> 24);
      h = h ^ g;}}
  return (int) h;}

int NetHashTable::findNA(NetAddress *na){
  GenHashNode *ghn;
  int bindex;
  int hvalue=hashFunc(na);
  if(findPlace(hvalue,na,ghn)){
    int bindex= GenHashNode2BorrowIndex(ghn);
    PD((HASH,"borrow index b:%d",bindex));
    return bindex;}
  return -1;}

void NetHashTable::add(NetAddress *na,int bindex){
  int hvalue=hashFunc(na);
  GenHashNode *ghn;
  Assert(!findPlace(hvalue,na,ghn));
  PD((HASH,"adding hvalue=%d net=%d:%d bindex=%d",
	       hvalue,na->site,na->index,bindex));
  GenHashBaseKey* ghn_bk;
  GenHashEntry* ghn_e;
  GenCast(na,NetAddress*,ghn_bk,GenHashBaseKey*);
  GenCast(bindex,int,ghn_e,GenHashEntry*);  
  htAdd(hvalue,ghn_bk,ghn_e);}

void NetHashTable::sub(NetAddress *na){
  int hvalue=hashFunc(na);
  GenHashNode *ghn;
  findPlace(hvalue,na,ghn);
  PD((HASH,"deleting hvalue=%d net=%d:%d bindex=%d",
	       hvalue,na->site,na->index));
  htSub(hvalue,ghn);}

#ifdef DEBUG_PERDIO
void NetHashTable::print(){
  int limit=getSize();
  int used=getUsed();
  int i;
  printf("******************************************************\n");
  printf("************* NetAddress Hash Table *****************\n");
  printf("******************************************************\n");
  printf("Size:%d Used:%d\n",limit,used);
  GenHashNode *ghn;
  NetAddress *na;
  for(i=0;i<limit;i++){
    ghn=getElem(i);
    if(ghn!=NULL){
      na=GenHashNode2NetAddr(ghn);
      printf("<%d> - s%s o:%d\n",i,oz_site2String(na->site),na->index);
      ghn=ghn->getNext();
      while(ghn!=NULL){
	na=GenHashNode2NetAddr(ghn);
	printf("<coll> - s:%s o:%d\n",oz_site2String(na->site),na->index);
	ghn=ghn->getNext();}}}
  printf("-----------------------------------\n");
}
#endif

/* -------------------------------------------------------------------- */

OwnerTable *ownerTable;
BorrowTable *borrowTable;
DSite* creditSiteIn;
DSite* creditSiteOut;

void sendPrimaryCredit(DSite *sd,int OTI,Credit c);

#ifdef DEBUG_PERDIO

void printTables(){
  ownerTable->print();
  borrowTable->print();
  borrowTable->hshtbl->print();}

void resize_hash(){
  borrowTable->hshtbl->print();}

#endif

void OwnerEntry::addCreditExtended(Credit back){
  ReduceCode rc=getOwnerCreditExtension()->addCreditE(back);
  if(rc==CANNOT_REDUCE) return;
  if(rc==CAN_REDUCE_LAST){
    getOwnerCreditExtension()->reduceLast();
    return;}
  Assert(rc==CAN_REDUCE_SINGLE);
  removeExtension();
  removeFlags(PO_EXTENDED);}


void OwnerTable::init(int beg,int end){
  int i=beg;
  while(i<end){
    array[i].makeFree(i+1);
    i++;}
  i--;
  array[i].makeFree(END_FREE);
  nextfree=beg;}

void OwnerTable::compactify()  /* TODO - not tested */
{
  return; // mm2

  Assert(size>=DEFAULT_OWNER_TABLE_SIZE);
  if(size==DEFAULT_OWNER_TABLE_SIZE) return;
  if(no_used/size< TABLE_LOW_LIMIT) return;
  PD((TABLE,"TABLE:owner compactify enter: size:%d no_used:%d",
	       size,no_used));
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  int i=0;
  int used_slot= -1;
  int* base = &nextfree;
  while(TRUE){
    if(i>=size) break;
    if(array[i].isFree()){
      *base=i;
      base=&array[i].uOB.nextfree;}
    else used_slot=i;
    i++;}
  *base=END_FREE;
  int first_free=used_slot+1;
  int newsize= first_free-no_used < TABLE_BUFFER ? 
    first_free-no_used+TABLE_BUFFER : used_slot+1;
  if(first_free < size - TABLE_WORTHWHILE_REALLOC){
    PD((TABLE,"TABLE:owner compactify free slots: new%d",newsize));
    OwnerEntry *oldarray=array; 
    array = (OwnerEntry*) realloc(array,newsize*sizeof(OwnerEntry));
    Assert(array!=NULL);
    if(array!=NULL){
      size=newsize;
      init(first_free,size);
      return;}
    array=oldarray;}
  init(first_free,size);      
  PD((TABLE,"TABLE:owner compactify no realloc"));
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  return;}

void OwnerTable::resize(){
#ifdef BTRESIZE_CRITICAL
  warning("OwnerTable::resize: maybe incorrect");
#endif
  int newsize = ((int) (TABLE_EXPAND_FACTOR *size));
  PD((TABLE,"TABLE:resize owner old:%d no_used:%d new:%d",
		size,no_used,newsize));
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  array = (OwnerEntry*) realloc(array,newsize*sizeof(OwnerEntry));
  if(array==NULL){
    error("Memory allocation: Owner Table growth not possible");}
  init(size, newsize);  
  size=newsize;
  PD((TABLE2,"TABLE:resize owner complete"));
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  return;}

int OwnerTable::newOwner(OwnerEntry *&oe){
  if(nextfree == END_FREE) resize();
  int index = nextfree;
  nextfree = array[index].uOB.nextfree;
  oe = (OwnerEntry *)&(array[index]);
  oe->setCreditOB(START_CREDIT_SIZE);
  PD((TABLE,"owner insert: o:%d",index));
  no_used++;
  return index;}

void OwnerTable::freeOwnerEntry(int i){
  array[i].setFree();
  array[i].uOB.nextfree=nextfree;
  nextfree=i;
  no_used--;
  PD((TABLE,"owner delete o:%d",i));
  return;}

#define PO_getValue(po) \
((po)->isTertiary() ? makeTaggedConst((po)->getTertiary()) : (po)->getRef())

OZ_Term OwnerTable::extract_info(){
  OZ_Term list = oz_nil();
  OZ_Term credit;

  char *str;
  for(int ctr = 0; ctr<size;ctr++){
    OwnerEntry *oe = OT->getEntry(ctr);
    if(oe==NULL){continue;}
    Assert(oe!=NULL);
    if(oe->isExtended()) {
      OwnerCreditExtension *next;
      next = oe->uOB.oExt;
      credit = oz_nil();
      while(next != NULL){
	credit = oz_cons(OZ_recordInit(oz_atom("ext"),
		   oz_cons(oz_pairAI("credit0",next->getCredit(0)),
		   oz_cons(oz_pairAI("credit1",next->getCredit(1)),
		           oz_nil()))), credit);
	next = next->getNext();}
      credit = OZ_recordInit(oz_atom("big"), oz_cons(credit, oz_nil()));}
    else {
      if(oe->uOB.credit == -1)
	credit = oz_atom("persistent");
      else
	credit = oz_int(oe->uOB.credit);
    }
    list=
      oz_cons(OZ_recordInit(oz_atom("oe"),
	oz_cons(oz_pairAI("index", ctr),
	oz_cons(oz_pairAA("type", toC(PO_getValue(oe))),
	oz_cons(oz_pairA("credit", credit), 
		oz_nil())))), list);
  }
  return OZ_recordInit(oz_atom("ot"),
           oz_cons(oz_pairAI("size", size),
           oz_cons(oz_pairA("list", list), oz_nil())));
}

void OwnerTable::print(){
  printf("***********************************************\n");
  printf("********* OWNER TABLE *************************\n");
  printf("***********************************************\n");
  printf("Size:%d No_used:%d \n\n",size,no_used);
  printf("BI\t BORROW\t Credit\n");
  int i;
  for(i=0;i<size;i++){
    if(!(array[i].isFree())){
      OwnerEntry *oe=getOwner(i);
      if(oe->isExtended()) {
	int ctr = 1;
	OwnerCreditExtension *next;
	next = oe->uOB.oExt;
	printf("<%d>\t %s\t", i, toC(PO_getValue(oe)));
	while(next != NULL){
	  printf("ex:%d %ld#%ld ", ctr, next->getCredit(0), 
		 next->getCredit(1));
	  ctr ++;
	  next = next->getNext();}
	printf("\n");}
      else {
	if(oe->uOB.credit == -1)
	  printf("<%d>\t %s\t PERSISTENT\n", i, toC(PO_getValue(oe)));
	else
	  printf("<%d>\t %s\t %ld\n", i, toC(PO_getValue(oe)), 
		 oe->uOB.credit);}
#ifdef XXDEBUG_PERDIO
      getOwner(i)->print();
    } else{
      printf("<%d> FREE: next:%d\n",i,array[i].uOB.nextfree);
#endif
    }
  }
  printf("-----------------------------------------------\n");  
}


void OwnerCreditExtension::init(Credit c){
  Assert(c > 0);
  credit[0]=c;
  credit[1]=START_CREDIT_SIZE;
  PD((CREDIT,"owner credit extension init %d %d %d",c, credit[0], credit[1]));
  next=NULL;}

void OwnerCreditExtension::requestCreditE(Credit req){
  if(credit[0]>=req) {
    credit[0] -= req;
    PD((CREDIT,"request from owner credit extension credit[0] %d %d", credit[0], credit[1]));
    return;}
  if(credit[1]!=0){
    credit[1]--;
    PD((CREDIT,"request from owner credit extension credit[1] %d %d", credit[0],credit[1]));
    credit[0] += START_CREDIT_SIZE - req;
    return;}
  if(next==NULL){
    expand();
    requestCreditE(req);
    return;}
  next->requestCreditE(1);
  credit[1]=START_CREDIT_SIZE;
  requestCreditE(req);}

ReduceCode OwnerCreditExtension::isReducible(){ // called from first
  OwnerCreditExtension* oce=next;
  if(oce==NULL){
    if(credit[1]!=START_CREDIT_SIZE) {return CANNOT_REDUCE;}
    return CAN_REDUCE_SINGLE;}
  while(oce->next!=NULL) {oce=oce->next;}
  if(credit[0]!=START_CREDIT_SIZE) {return CANNOT_REDUCE;}
  if(credit[1]!=START_CREDIT_SIZE) {return CANNOT_REDUCE;}  
  return CAN_REDUCE_LAST;}

ReduceCode OwnerCreditExtension::addCreditE(Credit ret){
  if(credit[0]+ret>START_CREDIT_SIZE){
    credit[0]= credit[0]-START_CREDIT_SIZE+ret;
    if(credit[1]==START_CREDIT_SIZE){
      ReduceCode rc=next->addCreditE(1);
      if(rc==CANNOT_REDUCE) {return CANNOT_REDUCE;}
      return CAN_REDUCE_LAST;}
    credit[1]++;
    if(credit[1]<START_CREDIT_SIZE){return CANNOT_REDUCE;}
    if(next==NULL) {return CAN_REDUCE_SINGLE;}
    return CANNOT_REDUCE;}
  credit[0]+=ret;
  return CANNOT_REDUCE;}

#ifdef MISC_BUILTINS

OZ_BI_define(BIprintOwnerTable,0,0)
{
  OT->print();
  return PROCEED;
} OZ_BI_end

#endif

/* expand */

void BorrowCreditExtension::expandSlave(){
  BorrowCreditExtension* master=newBorrowCreditExtension();
  master->initMaster(slaveGetSecCredit());
  uSOB.bce=master;}

void BorrowCreditExtension::expandMaster(){
  OwnerCreditExtension* oce=newOwnerCreditExtension();
  oce->init(uSOB.secCredit);
  uSOB.oce=oce;
  return;}

/* get secondary credit */

Bool BorrowCreditExtension::getSecCredit_Master(Credit cred){
  Credit c=masterGetSecCredit();
  if(c>cred){ 
    PD((CREDIT,"Got %d SecCred %d left",cred, c-cred));
    masterSetSecCredit(c-cred);
    return NO;}
  PD((CREDIT,"Wanted %d SecCred %d in store",cred, c));
  expandMaster();
  return OK;} 

Bool BorrowCreditExtension::getSmall_Slave(Credit &cred){
  Credit c=slaveGetSecCredit();
  if(c>BORROW_GIVE_CREDIT_SIZE){
    cred=BORROW_GIVE_CREDIT_SIZE;
    slaveSetSecCredit(c-BORROW_GIVE_CREDIT_SIZE);
    PD((CREDIT,"Got %d SecSlaveCred %d left",cred, c-cred));
    return NO;}
  PD((CREDIT,"Wanted %d SecSlaveCred %d in store",cred, c));
  expandSlave();
  return OK;}
  
Bool BorrowCreditExtension::getOne_Slave(){
  Credit c=slaveGetSecCredit();
  if(c>1){
    slaveSetSecCredit(c-1);
    PD((CREDIT,"Got one SecSlaveCred %d left", c-1));
    return NO;}
  PD((CREDIT,"Wanted one SecSlaveCred %d in store", c - 1));
  expandSlave();
  return OK;
}

/* reduce */

Credit BorrowCreditExtension::reduceSlave(Credit more,DSite* &s,Credit &secCredit){
  Assert(msGetPrimCredit()+more < BORROW_HIGH_THRESHOLD);
  s=site;
  secCredit=slaveGetSecCredit();
  return msGetPrimCredit()+more;}

/* add PrimaryCredit */

Credit BorrowCreditExtension::addPrimaryCredit_Master(Credit more){
  Assert(more!=INFINITE_CREDIT);
  Credit c=msGetPrimCredit()+more;
  if(c>BORROW_HIGH_THRESHOLD) { 
    msSetPrimCredit(BORROW_HIGH_THRESHOLD);
    return c-BORROW_HIGH_THRESHOLD;}
  msSetPrimCredit(c);
  return 0;}

void OwnerCreditExtension::expand(){
  OwnerCreditExtension *oce=newOwnerCreditExtension();
  oce->init(0);
  next=oce;}

void OwnerCreditExtension::reduceLast(){
  OwnerCreditExtension *oce=getNext();
  OwnerCreditExtension *tmp;
  while(oce->next->next!=NULL){
    oce=oce->next;}
  tmp=oce->next;
  Assert(tmp->credit[0]==START_CREDIT_SIZE);
  Assert(tmp->credit[1]==START_CREDIT_SIZE);
  Assert(tmp->next==NULL);
  freeOwnerCreditExtension(tmp);
  oce->next=NULL;}

void OwnerEntry::extend(){
  OwnerCreditExtension* oce=newOwnerCreditExtension();
  oce->init(getCreditOB());
  addFlags(PO_EXTENDED|PO_BIGCREDIT);
  setOwnerCreditExtension(oce);}

void OwnerEntry::removeExtension(){
  OwnerCreditExtension *oce=getOwnerCreditExtension();
  setCreditOB(oce->reduceSingle());
  freeOwnerCreditExtension(oce);
  return;}

/* ********************** private **************************** */

void BorrowEntry::initSecBorrow(DSite *cs,Credit c,DSite *s,int i){
  Assert(isFree());
  Assert(c!=INFINITE_CREDIT);
  unsetFree();
  netaddr.set(s,i);
  createSecSlave(c,cs);
}

void BorrowEntry::removeSoleExtension(Credit c){            
  BorrowCreditExtension* bce=getSlave();
  freeBorrowCreditExtension(bce);
  removeFlags(PO_MASTER|PO_SLAVE|PO_EXTENDED);
  setCreditOB(c);}

void BorrowEntry::createSecMaster(){
  BorrowCreditExtension *bce=newBorrowCreditExtension();
  bce->initMaster(getCreditOB());
  setFlags(PO_MASTER|PO_EXTENDED);
  setMaster(bce);}

void BorrowEntry::createSecSlave(Credit cred,DSite *s){
  BorrowCreditExtension *bce=newBorrowCreditExtension();
  bce->initSlave(getCreditOB(),cred,s);
  setFlags(PO_SLAVE|PO_EXTENDED);
  setSlave(bce);}

Bool BorrowEntry::getOnePrimaryCredit_E(){
  Credit c=extendGetPrimCredit();
  if(c>BORROW_MIN){
    extendSetPrimCredit(c-1);
    return OK;}
  return NO;}

Credit BorrowEntry::getSmallPrimaryCredit_E(){
  Credit c=extendGetPrimCredit();
  if(c>BORROW_MIN+BORROW_GIVE_CREDIT_SIZE){
    extendSetPrimCredit(c-BORROW_GIVE_CREDIT_SIZE);
    return BORROW_GIVE_CREDIT_SIZE;}
  if(c>BORROW_MIN+2){
    extendSetPrimCredit(c-2);
    return 2;}
  return 0;}
  
/* ********************** public **************************** */

DSite* BorrowEntry::getSmallSecondaryCredit(Credit &cred){
  while(TRUE){
    switch(getExtendFlags()){
    case PO_EXTENDED|PO_SLAVE|PO_MASTER|PO_BIGCREDIT:{
      cred=OWNER_GIVE_CREDIT_SIZE;
      getSlave()->getMaster()->getBig()->requestCreditE(OWNER_GIVE_CREDIT_SIZE);
      return myDSite;}
    case PO_EXTENDED|PO_SLAVE|PO_MASTER:{
      cred=OWNER_GIVE_CREDIT_SIZE;      
      if(getSlave()->getMaster()->getSecCredit_Master(OWNER_GIVE_CREDIT_SIZE)){
	addFlags(PO_BIGCREDIT);
	break;}
      return myDSite;}      
    case PO_EXTENDED|PO_MASTER:{
      cred=OWNER_GIVE_CREDIT_SIZE;      
      if(getMaster()->getSecCredit_Master(OWNER_GIVE_CREDIT_SIZE)){
	addFlags(PO_BIGCREDIT);
	break;}
      return myDSite;}
    case PO_EXTENDED|PO_MASTER|PO_BIGCREDIT:{
      cred=OWNER_GIVE_CREDIT_SIZE;
      getMaster()->getBig()->requestCreditE(OWNER_GIVE_CREDIT_SIZE);
      return myDSite;}
    case PO_SLAVE:{
      if(getSlave()->getSmall_Slave(cred)){
	addFlags(PO_MASTER);
	break;}
      return getSlave()->getSite();}
    case PO_NONE:{
      createSecMaster();
      break;}
    default:{
      Assert(0);}
    }
  }
}

DSite* BorrowEntry::getOneSecondaryCredit(){
  while(TRUE){
    switch(getExtendFlags()){
    case PO_EXTENDED|PO_SLAVE|PO_MASTER|PO_BIGCREDIT:{
      getSlave()->getMaster()->getBig()->requestCreditE(1);
      return myDSite;}
    case PO_EXTENDED|PO_SLAVE|PO_MASTER:{
      if(getSlave()->getMaster()->getSecCredit_Master(1)){
	addFlags(PO_BIGCREDIT);
	break;}
      return myDSite;}      
    case PO_EXTENDED|PO_MASTER:{
      if(getMaster()->getSecCredit_Master(1)){
	addFlags(PO_BIGCREDIT);
	break;}
      return myDSite;}
    case PO_EXTENDED|PO_MASTER|PO_BIGCREDIT:{
      getMaster()->getBig()->requestCreditE(1);
      return myDSite;}
    case PO_EXTENDED|PO_SLAVE:{
      if(getSlave()->getOne_Slave()){
	addFlags(PO_MASTER);
	break;}
      return getSlave()->getSite();}
    case PO_NONE:{
      createSecMaster();
      break;}
    default:{
      Assert(0);}

    }
  }
  return NULL; // stupid compiler
}

void BorrowEntry::addPrimaryCreditExtended(Credit c){  
  Credit overflow;
  switch(getExtendFlags()){
  case PO_EXTENDED|PO_SLAVE|PO_MASTER|PO_BIGCREDIT:
  case PO_EXTENDED|PO_SLAVE|PO_MASTER:{
    overflow=getSlave()->getMaster()->addPrimaryCredit_Master(c);
    break;}
  case PO_EXTENDED|PO_SLAVE:{
    DSite *s;
    Credit sec;
    overflow=getSlave()->reduceSlave(c,s,sec);
    removeSoleExtension(overflow);
    giveBackSecCredit(s,sec);
    break;}
  case PO_EXTENDED|PO_MASTER|PO_BIGCREDIT:
  case PO_EXTENDED|PO_MASTER:{
    overflow=getMaster()->addPrimaryCredit_Master(c);
    break;
  default:
    Assert(0);}}
  if(overflow>0){
    giveBackCredit(overflow);}
}

OZ_Term BorrowEntry::extract_info(int index) {
  OwnerCreditExtension *next;
  OZ_Term primCred, secCred;
  OZ_Term na=
    OZ_recordInit(oz_atom("netAddress"),
      oz_cons(oz_pairA("site", oz_atom(netaddr.site->stringrep_notype())),
      oz_cons(oz_pairAI("index",(int)netaddr.index), oz_nil())));
/*
  OZ_Term na=
    OZ_recordInit(oz_atom("netAddress"),
      oz_cons(oz_pairA("site",OZ_recordInit(oz_atom("site"),
          oz_cons(oz_pairAI("port",(int)netaddr.site->getPort()),
	  oz_cons(oz_pairAI("timeint",(int)netaddr.site->getTimeStamp()->start),
	  oz_cons(oz_pairA("timestr",oz_atom(
			          ctime(&netaddr.site->getTimeStamp()->start))),
	  oz_cons(oz_pairAI("ipint",(unsigned int)netaddr.site->getAddress()),
	  oz_cons(oz_pairAI("hval",(int)netaddr.site),
		oz_nil()))))))), 
	oz_cons(oz_pairAI("index",(int)netaddr.index), oz_nil())));
*/
  switch(getExtendFlags()){
  case PO_PERSISTENT:
    primCred = oz_atom("persistent");
    secCred = oz_atom("persistent");
    break;
  case PO_EXTENDED|PO_SLAVE|PO_MASTER|PO_BIGCREDIT:
    primCred = OZ_recordInit(oz_atom("slave"),
                 oz_pairII(1, getSlave()->getMaster()->primCredit));
    secCred = oz_nil();
    next = getSlave()->getMaster()->uSOB.oce;
    while(next != NULL){
      secCred = oz_cons(OZ_recordInit(oz_atom("big"),
		    oz_cons(oz_pairAI("credit0",next->getCredit(0)),
		    oz_cons(oz_pairAI("credit1",next->getCredit(1)),
			    oz_nil()))), secCred);
      next = next->getNext();}
    break;
  case PO_EXTENDED|PO_SLAVE|PO_MASTER:
    primCred = OZ_recordInit(oz_atom("slave"),
                 oz_pairII(1, getSlave()->getMaster()->primCredit));
    secCred = oz_int(getSlave()->getMaster()->uSOB.secCredit);
    break;
  case PO_EXTENDED|PO_SLAVE:
    primCred = OZ_recordInit(oz_atom("slave"),
                 oz_pairII(1, getSlave()->getMaster()->primCredit));
    secCred = oz_int(getSlave()->uSOB.secCredit);
    break;
  case PO_EXTENDED|PO_MASTER|PO_BIGCREDIT:
    primCred = OZ_recordInit(oz_atom("slave"),
                 oz_pairII(1, getMaster()->primCredit));
    secCred = oz_nil();
    next = getMaster()->uSOB.oce;
    while(next != NULL){
      secCred = oz_cons(OZ_recordInit(oz_atom("big"),
		    oz_cons(oz_pairAI("credit0",next->getCredit(0)),
		    oz_cons(oz_pairAI("credit1",next->getCredit(1)),
			    oz_nil()))), secCred);
      next = next->getNext();}
    break;
  case PO_EXTENDED|PO_MASTER:
    primCred = oz_int(getMaster()->primCredit);
    secCred = oz_int(getMaster()->uSOB.secCredit);
    break;
  case PO_NONE:
    secCred = oz_int(0);
    primCred = oz_int(uOB.credit);
    break;
  default:
    Assert(0);}

  return OZ_recordInit(oz_atom("be"),
     oz_cons(oz_pairAI("index", index),
     oz_cons(oz_pairAA("type", toC(PO_getValue(this))),
     oz_cons(oz_pairA("na", na),
     oz_cons(oz_pairAI("secCred", secCred),
     oz_cons(oz_pairA("primCred",primCred),
	     oz_nil()))))));
}


/************* reduction  all private *****************/

void BorrowEntry::removeBig(BorrowCreditExtension* master){
  OwnerCreditExtension *oce=master->getBig();
  master->masterSetSecCredit(oce->reduceSingle());
  freeOwnerCreditExtension(oce);
  removeFlags(PO_BIGCREDIT);
  generalTryToReduce();}

void BorrowEntry::removeMaster_SM(BorrowCreditExtension* master){
  Assert(!(getExtendFlags() & PO_BIGCREDIT));
  Assert(master->masterGetSecCredit()==START_CREDIT_SIZE);  
  BorrowCreditExtension *slave=getSlave();
  Credit c=master->msGetPrimCredit();
  slave->slaveSetSecCredit(c);
  removeFlags(PO_EXTENDED|PO_MASTER);
  freeBorrowCreditExtension(master);
  generalTryToReduce();}

void BorrowEntry::removeMaster_M(BorrowCreditExtension* master){
  Assert(!(getExtendFlags() & PO_BIGCREDIT));
  Assert(!(getExtendFlags() & PO_SLAVE));
  Assert(master->masterGetSecCredit()==START_CREDIT_SIZE);
  Credit c=master->msGetPrimCredit();
  removeFlags(PO_EXTENDED|PO_MASTER);
  freeBorrowCreditExtension(master);
  setCreditOB(c);}

void BorrowEntry::removeSlave(){
  BorrowCreditExtension* slave=getSlave();
  Credit c=slave->msGetPrimCredit();
  Assert((c>=BORROW_MIN)||(c==0));
  giveBackSecCredit(slave->getSite(),slave->slaveGetSecCredit());
  removeFlags(PO_SLAVE|PO_EXTENDED);
  freeBorrowCreditExtension(slave);
  setCreditOB(c);}

void BorrowEntry::generalTryToReduce(){
  while(TRUE){
    switch(getExtendFlags()){
    case PO_EXTENDED|PO_SLAVE|PO_MASTER|PO_BIGCREDIT:{
      if(getSlave()->getMaster()->isReducibleBig()){
	removeBig(getSlave()->getMaster());
	PD((CREDIT,"slave+master removing big"));
	break;}
      return;}
  
    case PO_EXTENDED|PO_SLAVE|PO_MASTER:{
      if(getSlave()->getMaster()->isReducibleMaster()){
	removeMaster_SM(getSlave()->getMaster());
	PD((CREDIT,"slave removing master"));
	break;}
      return;}
  
    case PO_EXTENDED|PO_SLAVE:{
      if(getSlave()->isReducibleSlave()){
	PD((CREDIT,"removing slave"));
	removeSlave();}
      return;}
  
    case PO_EXTENDED|PO_MASTER|PO_BIGCREDIT:{
      if(getMaster()->isReducibleBig()){
	removeBig(getMaster()); 
	PD((CREDIT,"master removing big"));
	break;}
      return;}

    case PO_EXTENDED|PO_MASTER:{
      if(getMaster()->isReducibleMaster()){
	PD((CREDIT,"removing master"));
	removeMaster_M(getMaster());      
	break;}
      return;}
    case PO_NONE:{
      return;}
    default:{
      Assert(0);}
    }
  }
}

/************* add secondary *****************/

/* private */

void BorrowEntry::addSecCredit_MasterBig(Credit c,BorrowCreditExtension *master){
  ReduceCode rc=master->getBig()->addCreditE(c);
  if(rc==CANNOT_REDUCE) return;
  if(rc==CAN_REDUCE_LAST){
    master->getBig()->reduceLast();
    return;}
  Assert(rc==CAN_REDUCE_SINGLE);
  generalTryToReduce();}

void BorrowEntry::addSecCredit_Master(Credit c,BorrowCreditExtension *master){
  Credit cx=master->masterGetSecCredit()+c;
  master->masterSetSecCredit(cx);
  if(cx==START_CREDIT_SIZE){
    generalTryToReduce();}}

Bool BorrowEntry::addSecCredit_Slave(Credit c,BorrowCreditExtension *slave){
  Credit cx=slave->slaveGetSecCredit()+c;
  if(cx>START_CREDIT_SIZE){
    return NO;}
  slave->slaveSetSecCredit(cx);
  return OK;}

/* public */

void BorrowEntry::addSecondaryCredit(Credit c,DSite *s){
  switch(getExtendFlags()){
  case PO_EXTENDED|PO_SLAVE|PO_MASTER|PO_BIGCREDIT:{
    if(s==myDSite){
      addSecCredit_MasterBig(c,getSlave()->getMaster());      
      return;}
    if(s==getSlave()->getSite()){
      if(addSecCredit_Slave(c,getSlave())) {return;}}
    break;}
  
  case PO_EXTENDED|PO_SLAVE|PO_MASTER:{
    if(s==myDSite){
      addSecCredit_Master(c,getSlave()->getMaster());
      return;}
    if(s==getSlave()->getSite()){
      if(addSecCredit_Slave(c,getSlave())) {return;}}
    break;}
  
  case PO_EXTENDED|PO_SLAVE:{
    if(s==getSlave()->getSite()){
      if(addSecCredit_Slave(c,getSlave())){return;}}
    break;}
  
  case PO_EXTENDED|PO_MASTER|PO_BIGCREDIT:{
    if(s==myDSite){
      addSecCredit_MasterBig(c,getMaster());
      return;}
    break;}

  case PO_EXTENDED|PO_MASTER:{
    if(s==myDSite){
      addSecCredit_Master(c,getMaster());
      return;}
    break;}
  default:
    break;
  }
  giveBackSecCredit(s,c);
}

void BorrowEntry::copyBorrow(BorrowEntry* from,int i){
  if (from->isTertiary()) {
    mkTertiary(from->getTertiary(),from->getFlags());
    from->getTertiary()->setIndex(i);
  } else if (from->isVar()) {
    mkVar(from->getRef(),from->getFlags());
    tagged2PerdioVar(*(from->getPtr()))->setIndex(i);
  } else {
    Assert(from->isRef());
    mkRef(from->getRef());
  }
  netaddr.set(from->netaddr.site,from->netaddr.index);
}

void BorrowEntry::moreCredit(){
  NetAddress *na = getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  PD((CREDIT,"Asking for more credit %s",na->site->stringrep()));
  getOneMsgCredit();
  marshal_M_ASK_FOR_CREDIT(bs,na->index,myDSite);
  SendTo(na->site,bs,M_ASK_FOR_CREDIT,netaddr.site,netaddr.index); 
}

void BorrowEntry::giveBackCredit(Credit c){
  NetAddress *na = getNetAddress();
  DSite* site = na->site;
  int index = na->index;
  sendPrimaryCredit(site,index,c);
}

void BorrowEntry::giveBackSecCredit(DSite *s,Credit c){
  NetAddress *na = getNetAddress();
  DSite* site = na->site;
  int index = na->index;
  sendSecondaryCredit(s,site,index,c);
}

void BorrowEntry::freeBorrowEntry(){
  Assert(!isExtended());
  if(!isPersistent())
    giveBackCredit(getCreditOB());}

void BorrowEntry::gcBorrowRoot(int i) {
  if (isVar()) {
    PD((GC,"BT1 b:%d variable found",i));
    PerdioVar *pv=tagged2PerdioVar(*getPtr());
    if (pv->gcIsAliveV()) {
      PD((WEIRD,"BT1 b:%d pending unmarked var found",i));
      gcPO();
    }
    return;
  }
}

void BorrowTable::init(int beg,int end)
{
  int i=beg;
  while(i<end){
    array[i].uOB.nextfree=i+1;
    array[i].setFree();
    i++;}
  i--;
  array[i].uOB.nextfree=nextfree;
  nextfree=beg;
}

void BorrowTable::compactify(){
  if(no_used / size >= TABLE_LOW_LIMIT) return;
  Assert(size>=DEFAULT_BORROW_TABLE_SIZE);
  if(size==DEFAULT_BORROW_TABLE_SIZE) return;
  int newsize= no_used+TABLE_BUFFER;
  if(newsize<DEFAULT_BORROW_TABLE_SIZE) newsize=DEFAULT_BORROW_TABLE_SIZE;
  PD((TABLE,"compactify borrow old:%d no_used:%d new:%d",
		size,no_used,newsize));
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  BorrowEntry *oldarray=array;
  array = (BorrowEntry*) malloc(newsize*sizeof(BorrowEntry));
  if(array==NULL){
    PD((TABLE,"compactify borrow NOT POSSIBLE"));
    array=oldarray;
    return;}
  int oldsize=size;
  size=newsize;
  copyBorrowTable(oldarray,oldsize);
  PD((TABLE,"compactify borrow complete"));
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  return;}

void BorrowTable::resize()
{
#ifdef BTRESIZE_CRITICAL
  warning("BorrowTable::resize: maybe incorrect");
#endif
  Assert(no_used==size);
  int newsize = int (TABLE_EXPAND_FACTOR*size);
  PD((TABLE,"resize borrow old:%d no_used:%d new:%d", size,no_used,newsize));
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  BorrowEntry *oldarray=array;
  array = (BorrowEntry*) malloc(newsize*sizeof(BorrowEntry));
  if(array==NULL){
    error("Memory allocation: Borrow Table growth not possible");}
  int oldsize=size;
  size=newsize;
  copyBorrowTable(oldarray,oldsize);
  PD((TABLE,"resize borrow complete"));
  PERDIO_DEBUG_DO1(TABLE2,printTables());
  return;}

int BorrowTable::newSecBorrow(DSite *creditSite,Credit c,DSite * sd,int off){
  if(nextfree == END_FREE) resize();
  int index=nextfree;
  nextfree= array[index].uOB.nextfree;
  BorrowEntry* oe = &(array[index]);
  oe->initSecBorrow(creditSite,c,sd,off);
  PD((HASH2,"<SPECIAL>:net=%x borrow=%x owner=%x hash=%x",
		oe->getNetAddress(),array,ownerTable->array,
		hshtbl->table));
  hshtbl->add(oe->getNetAddress(),index);
  no_used++;
  PD((TABLE,"borrow insert: b:%d",index));
  return index;}

int BorrowTable::newBorrow(Credit c,DSite * sd,int off){
  if(nextfree == END_FREE) resize();
  int index=nextfree;
  nextfree= array[index].uOB.nextfree;
  BorrowEntry* oe = &(array[index]);
  oe->initBorrow(c,sd,off);
  if(c!=PERSISTENT_CRED && c<=BORROW_LOW_THRESHOLD){
    oe->moreCredit();}
  
  PD((HASH2,"<SPECIAL>:net=%x borrow=%x owner=%x hash=%x",
		oe->getNetAddress(),array,ownerTable->array,
		hshtbl->table));
  hshtbl->add(oe->getNetAddress(),index);
  no_used++;
  PD((TABLE,"borrow insert: b:%d",index));
  return index;}

void BorrowTable::maybeFreeBorrowEntry(int index){
  BorrowEntry *b = &(array[index]);
  if(b->isExtended()){
    if(b->getExtendFlags() & PO_MASTER) return;
    Assert(b->getExtendFlags()==PO_SLAVE);
    b->removeSlave();}
  Assert(!b->isExtended());
  b->freeBorrowEntry();  
  hshtbl->sub(getBorrow(index)->getNetAddress());
  b->setFree();
  b->uOB.nextfree=nextfree;
  nextfree=index;
  no_used--;
  PD((TABLE,"borrow delete: b:%d",index));
  return;}

void BorrowTable::copyBorrowTable(BorrowEntry *oarray,int osize){
  hshtbl->clear();
  int oindex=0;
  int nindex= 0;
  BorrowEntry *ob,*nb;
  while(TRUE){
    nb=&(array[nindex]);
    ob= &(oarray[oindex]);
    while(ob->isFree()) {
      oindex++;
      if(oindex>=osize) goto fin;
      ob= &(oarray[oindex]);}
    nb->copyBorrow(ob,nindex);
    hshtbl->add(nb->getNetAddress(),nindex);
    nindex++;
    oindex++;
    if(oindex>=osize) goto fin;}
fin:
  nextfree=END_FREE;
  init(nindex,size);
  free(oarray);
}


void OB_Entry::print() {
  printf("Credit:%ld\n",getCreditOB());
}

void OB_Entry::gcPO() {
  if (isGCMarked())
    return;
  makeGCMark();
  
  if (isTertiary()) {
    PD((GC,"OT tertiary found"));
    Assert(!inToSpace(u.tert));
    u.tert=(Tertiary *)u.tert->gcConstTerm();
  } else {
    Assert(isRef() || isVar());
    PD((GC,"OT var/ref"));
    OZ_collectHeapTerm(u.ref,u.ref);}
}

void BorrowEntry::print() {
  OB_Entry::print();
  NetAddress *na=getNetAddress();
  printf("NA: s:%s o:%d\n",na->site->stringrep(),na->index);
}

void BorrowEntry::print_entry(int nr) {
  int ctr = 1;
  char pC[1000], sC[1000];
  char *temp;
  char *primCred = (char *) pC,  *secCred = (char *) sC;
  OwnerCreditExtension *next;
  switch(getExtendFlags()){
  case PO_PERSISTENT:
    printf("<%d>\t %s\t %d\t PERSISTENT\n", nr, 
	   toC(PO_getValue(this)), netaddr.index);
    return;
  case PO_EXTENDED|PO_SLAVE|PO_MASTER|PO_BIGCREDIT:
    sprintf(primCred, "%ld(slave)", getSlave()->getMaster()->primCredit);
    next = getSlave()->getMaster()->uSOB.oce;
    *secCred = '\0';
    temp = secCred;
    while(next != NULL){
      temp += strlen(temp);
      sprintf(temp, "ex:%d %ld#%ld ", ctr, next->getCredit(0), 
	      next->getCredit(1));
      ctr ++;
      next = next->getNext();}
    break;
  case PO_EXTENDED|PO_SLAVE|PO_MASTER:
    sprintf(primCred, "%ld(slave)", getSlave()->getMaster()->primCredit);
    sprintf(secCred, "%ld", getSlave()->getMaster()->uSOB.secCredit);
    break;
  case PO_EXTENDED|PO_SLAVE:
    sprintf(primCred, "%ld", getSlave()->primCredit);
    sprintf(secCred, "%ld", getSlave()->uSOB.secCredit);
    break;
  case PO_EXTENDED|PO_MASTER|PO_BIGCREDIT:
    sprintf(primCred, "%ld", getMaster()->primCredit);
    next = getMaster()->uSOB.oce;
    *secCred = '\0';
    temp = secCred;
    while(next != NULL){
      temp += strlen(temp);
      sprintf(temp, "ex:%d %ld#%ld ", ctr, next->getCredit(0), 
	      next->getCredit(1));
      ctr ++;
      next = next->getNext();}
    break;
  case PO_EXTENDED|PO_MASTER:
    sprintf(primCred, "%ld", getMaster()->primCredit);
    sprintf(secCred, "%ld", getMaster()->uSOB.secCredit);
    break;
  case PO_NONE:
    secCred = "0";
    sprintf(primCred, "%ld", uOB.credit);
    break;
  default:
    Assert(0);}
  
  printf("<%d>\t %s\t %d\t %s\t\t %s\n", nr, toC(PO_getValue(this)), 
	 netaddr.index, primCred, secCred);
}

void BorrowTable::print(){
  printf("***********************************************\n");
  printf("********* BORROW TABLE *************************\n");
  printf("***********************************************\n");
  printf("Size:%d No_used:%d \n\n",size,no_used);
  printf("BI\t BORROW\t OI\t PrimCredit\t SecCredit\n");
  int i;
  BorrowEntry *b;
  for(i=0;i<size;i++){
    if(!(array[i].isFree())){
      b=getBorrow(i);
      b->print_entry(i);
#ifdef XXDEBUG_PERDIO
      b->print();
    } else {
      printf("<%d> FREE: next:%d\n",i,array[i].uOB.nextfree);
#endif
    }
  }
  printf("-----------------------------------------------\n");  
}

#ifdef MISC_BUILTINS

OZ_BI_define(BIprintBorrowTable,0,0)
{
  BT->print();
  return PROCEED;
} OZ_BI_end

#endif

Bool withinBorrowTable(int i){
  if(i<borrowTable->getSize()) return OK;
  return NO;}


void OwnerTable::gcOwnerTableRoots()
{
  PD((GC,"owner gc"));
  for(int i=0;i<size;i++) {
    OwnerEntry* o = getOwner(i);
    if(!o->isFree() && !o->hasFullCredit()) {
      PD((GC,"OT o:%d",i));
      o->gcPO();
    }
  }
  return;
}

void OwnerTable::gcOwnerTableFinal()
{
  PD((GC,"owner gc"));
  for(int i=0;i<size;i++) {
    OwnerEntry* o = getOwner(i);
    if(!o->isFree()) {
      PD((GC,"OT o:%d",i));
      if(o->hasFullCredit() && !o->isGCMarked()) {
	 freeOwnerEntry(i);
      } else {
	o->gcPO();
	o->removeGCMark();
      }
    }
  }
  compactify();
  return;
}

void BorrowTable::gcBorrowTableRoots()
{
  PD((GC,"borrowTable1 gc"));
  for(int i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);
    if (!b->isFree() && !b->isGCMarked())
      b->gcBorrowRoot(i);
  }
}

void BorrowEntry::gcBorrowUnusedFrame(int i) {
  if(isTertiary() && getTertiary()->isFrame())
    {u.tert= (Tertiary*) u.tert->gcConstTermSpec();}}

void BorrowTable::gcBorrowTableUnusedFrames()
{
  PD((GC,"borrow gc roots"));
  int i;
  for(i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);
    if(!b->isFree()){
      Assert((b->isVar()) || (b->getTertiary()->isFrame()) 
	     || (b->getTertiary()->isProxy()));
      if(!(b->isGCMarked())) {b->gcBorrowUnusedFrame(i);}}}
}

void BorrowTable::gcFrameToProxy(){
  PD((GC,"borrow frame to proxy"));
  int i;
  for(i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);
    if((!b->isFree()) && (!b->isVar())){
      Tertiary *t=b->getTertiary();
      if(t->isFrame()) {
	if((t->getType()==Co_Cell)
	   && ((CellFrame*)t)->getState()==Cell_Lock_Invalid){
	  ((CellFrame*)t)->convertToProxy();}
	else{
	  if((t->getType()==Co_Lock)
	     && ((LockFrame*)t)->getState()==Cell_Lock_Invalid){
	    ((LockFrame*)t)->convertToProxy();}}}}}
}


void maybeUnask(BorrowEntry *be){
  Tertiary *t=be->getTertiary();
  /* PER-HANDLE
  Watcher* w=t->getWatchersIfExist();
  EntityCond ec;
  while(w!=NULL){
    ec=managerPart(w->getWatchCond());
    if(ec!=ENTITY_NORMAL){
      sendUnAskError(t,ec);}
    w=w->getNext();}
  */
}


/* OBSERVE - this must done at the end of other gc */
void BorrowTable::gcBorrowTableFinal()
{
  PD((GC,"borrow gc"));
  int i;
  for(i=0;i<size;i++) {
    BorrowEntry *b=getBorrow(i);

    if (b->isFree())
      continue;

    if(b->isVar()) {
      if(b->isGCMarked()) {
	b->removeGCMark();
	b->getSite()->makeGCMarkSite();
	PD((GC,"BT b:%d mark variable found",i));
      } else{
	PD((GC,"BT b:%d unmarked variable found",i));
	borrowTable->maybeFreeBorrowEntry(i);
      }
    } else {
      Tertiary *t = b->getTertiary();
      if(b->isGCMarked()) {
	b->removeGCMark();
	b->getSite()->makeGCMarkSite();
	PD((GC,"BT b:%d mark tertiary found",i));
	
	if(t->isFrame()) {
	  switch(t->getType()){
	  case Co_Cell:{
	    CellFrame *cf=(CellFrame *)t;
	    if(cf->isAccessBit()){
	      cf->resetAccessBit();
	      if(cf->dumpCandidate()) {
		cellLockSendDump(b);
	      }
	    }
	    break;
	  }
	  case Co_Lock:{
	    LockFrame *lf=(LockFrame *)t;
	    if(lf->isAccessBit()){
	      lf->resetAccessBit();
	      if(lf->dumpCandidate()) {
		cellLockSendDump(b);
	      }
	    }
	    break;
	  }
	  default:
	    Assert(0);
	    break;
	  }
	}
      } else{
	/* PER-HANDLE
	if(t->maybeHasInform() && t->getType()!=Co_Port)
	  maybeUnask(b);
	*/
	Assert(t->isProxy());
	borrowTable->maybeFreeBorrowEntry(i);
      }
    }
  }
  compactify();
  hshtbl->compactify();
}
