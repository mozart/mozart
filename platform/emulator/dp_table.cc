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

#include "dp_table.hh"
#include "value.hh"
#include "perdiovar.hh"
#include "msgbuffer.hh"

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

void sendPrimaryCredit(Site *sd,int OTI,Credit c);
void SendTo(Site *toS,MsgBuffer *bs,MessageType mt,Site *sS,int sI);

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

void OwnerTable::print(){
  printf("***********************************************\n");
  printf("********* OWNER TABLE *************************\n");
  printf("***********************************************\n");
  printf("Size:%d No_used:%d \n",size,no_used);
  int i;
  for(i=0;i<size;i++){
    if(!(array[i].isFree())){
      OwnerEntry *oe=getOwner(i);
      printf("<%d> OWNER: %s\n",i,toC(PO_getValue(oe)));
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
  PD((CREDIT,"owner credit extension init %d",c));
  credit[0]=c;
  credit[1]=START_CREDIT_SIZE;
  next=NULL;}

void OwnerCreditExtension::requestCreditE(Credit req){
  if(credit[0]>=req) {
    PD((CREDIT,"request from owner credit extension credit[0]"));
    credit[0] -= req;
    return;}
  if(credit[1]!=0){
    credit[1]--;
    PD((CREDIT,"request from owner credit extension credit[1]"));
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
    masterSetSecCredit(c-cred);
    return NO;}
  expandMaster();
  return OK;} 

Bool BorrowCreditExtension::getSmall_Slave(Credit &cred){
  Credit c=slaveGetSecCredit();
  if(c>BORROW_GIVE_CREDIT_SIZE){
    cred=BORROW_GIVE_CREDIT_SIZE;
    slaveSetSecCredit(c-BORROW_GIVE_CREDIT_SIZE);
    return NO;}
  expandSlave();
  return OK;}
  
Bool BorrowCreditExtension::getOne_Slave(){
  Credit c=slaveGetSecCredit();
  if(c>1){
    slaveSetSecCredit(c-1);
    return NO;}
  expandSlave();
  return OK;
}

/* reduce */

Credit BorrowCreditExtension::reduceSlave(Credit more,Site* &s,Credit &secCredit){
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

void BorrowEntry::initSecBorrow(Site *cs,Credit c,Site *s,int i){
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

void BorrowEntry::createSecSlave(Credit cred,Site *s){
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

Site* BorrowEntry::getSmallSecondaryCredit(Credit &cred){
  while(TRUE){
    switch(getExtendFlags()){
    case PO_EXTENDED|PO_SLAVE|PO_MASTER|PO_BIGCREDIT:{
      cred=OWNER_GIVE_CREDIT_SIZE;
      getSlave()->getMaster()->getBig()->requestCreditE(OWNER_GIVE_CREDIT_SIZE);
      return mySite;}
    case PO_EXTENDED|PO_SLAVE|PO_MASTER:{
      cred=OWNER_GIVE_CREDIT_SIZE;      
      if(getSlave()->getMaster()->getSecCredit_Master(OWNER_GIVE_CREDIT_SIZE)){
	addFlags(PO_BIGCREDIT);}
      return mySite;}      
    case PO_EXTENDED|PO_MASTER:{
      cred=OWNER_GIVE_CREDIT_SIZE;      
      if(getMaster()->getSecCredit_Master(OWNER_GIVE_CREDIT_SIZE)){
	addFlags(PO_BIGCREDIT);}
      return mySite;}
    case PO_EXTENDED|PO_MASTER|PO_BIGCREDIT:{
      cred=OWNER_GIVE_CREDIT_SIZE;
      getMaster()->getBig()->requestCreditE(OWNER_GIVE_CREDIT_SIZE);
      return mySite;}
    case PO_SLAVE:{
      if(getSlave()->getSmall_Slave(cred)){
	addFlags(PO_MASTER);}
      return getSlave()->getSite();}
    case PO_NONE:{
      createSecMaster();
      break;}
    default:{
      Assert(0);}
    }
  }
}

Site* BorrowEntry::getOneSecondaryCredit(){
  while(TRUE){
    switch(getExtendFlags()){
    case PO_EXTENDED|PO_SLAVE|PO_MASTER|PO_BIGCREDIT:{
      getSlave()->getMaster()->getBig()->requestCreditE(1);
      return mySite;}
    case PO_EXTENDED|PO_SLAVE|PO_MASTER:{
      if(getSlave()->getMaster()->getSecCredit_Master(1)){
	addFlags(PO_BIGCREDIT);
	break;}
      return mySite;}      
    case PO_EXTENDED|PO_MASTER:{
      if(getMaster()->getSecCredit_Master(1)){
	addFlags(PO_BIGCREDIT);
	break;}
      return mySite;}
    case PO_EXTENDED|PO_MASTER|PO_BIGCREDIT:{
      getMaster()->getBig()->requestCreditE(1);
      return mySite;}
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
    Site *s;
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
	break;}
      return;}
  
    case PO_EXTENDED|PO_SLAVE|PO_MASTER:{
      if(getSlave()->getMaster()->isReducibleMaster()){
	removeMaster_SM(getSlave()->getMaster());
	break;}
      return;}
  
    case PO_EXTENDED|PO_SLAVE:{
      if(getSlave()->isReducibleSlave()){
	removeSlave();}
      return;}
  
    case PO_EXTENDED|PO_MASTER|PO_BIGCREDIT:{
      if(getMaster()->isReducibleBig()){
	removeBig(getMaster()); 
	break;}
      return;}

    case PO_EXTENDED|PO_MASTER:{
      if(getMaster()->isReducibleMaster()){
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

void BorrowEntry::addSecondaryCredit(Credit c,Site *s){
  switch(getExtendFlags()){
  case PO_EXTENDED|PO_SLAVE|PO_MASTER|PO_BIGCREDIT:{
    if(s==mySite){
      addSecCredit_MasterBig(c,getSlave()->getMaster());      
      return;}
    if(s==getSlave()->getSite()){
      if(addSecCredit_Slave(c,getSlave())) {return;}}
    break;}
  
  case PO_EXTENDED|PO_SLAVE|PO_MASTER:{
    if(s==mySite){
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
    if(s==mySite){
      addSecCredit_MasterBig(c,getMaster());
      return;}
    break;}

  case PO_EXTENDED|PO_MASTER:{
    if(s==mySite){
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
  Assert(!isExtended());
  NetAddress *na = getNetAddress();
  MsgBuffer *bs=msgBufferManager->getMsgBuffer(na->site);
  PD((CREDIT,"Asking for more credit %s",na->site->stringrep()));
  marshal_M_ASK_FOR_CREDIT(bs,na->index,mySite);
  SendTo(na->site,bs,M_ASK_FOR_CREDIT,netaddr.site,netaddr.index); 
}

void BorrowEntry::giveBackCredit(Credit c){
  NetAddress *na = getNetAddress();
  Site* site = na->site;
  int index = na->index;
  sendPrimaryCredit(site,index,c);
}

void BorrowEntry::giveBackSecCredit(Site *s,Credit c){
  NetAddress *na = getNetAddress();
  Site* site = na->site;
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
    if (pv->getSuspList() || (pv->isProxy() && pv->hasVal())) {
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

int BorrowTable::newSecBorrow(Site *creditSite,Credit c,Site * sd,int off){
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

int BorrowTable::newBorrow(Credit c,Site * sd,int off){
  if(nextfree == END_FREE) resize();
  int index=nextfree;
  nextfree= array[index].uOB.nextfree;
  BorrowEntry* oe = &(array[index]);
  oe->initBorrow(c,sd,off);
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

void BorrowTable::print(){
  printf("***********************************************\n");
  printf("********* BORROW TABLE *************************\n");
  printf("***********************************************\n");
  printf("Size:%d No_used:%d \n",size,no_used);
  int i;
  BorrowEntry *b;
  for(i=0;i<size;i++){
    if(!(array[i].isFree())){
      b=getBorrow(i);
      printf("<%d> BORROW: %s\n",i,toC(PO_getValue(b)));
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
