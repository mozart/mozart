#include "table.hh"
#include "creditHandler.hh"
#include "dpMarshaler.hh"

/* -------------------------------------------------------------------- */

#define PERSISTENT_CRED            (0-1)

//  #define DEBUG_CREDIT
#ifdef DEBUG_CREDIT

#define START_CREDIT_SIZE        (256)
#define OWNER_GIVE_CREDIT_SIZE   (32)
#define BORROW_MIN               (1)
#define BORROW_GIVE_SWITCH       (1)
#define BORROW_LOW_THRESHOLD     (8)
#define BORROW_HIGH_THRESHOLD    (64)


#else

#define START_CREDIT_SIZE        ((1<<30)-1)
#define OWNER_GIVE_CREDIT_SIZE   ((1<<19))
#define BORROW_MIN               (1)
#define BORROW_GIVE_SWITCH       (2)       // Equals to divide by 2^2=4
#define BORROW_LOW_THRESHOLD     ((1<<4))
#define BORROW_HIGH_THRESHOLD    ((1<<20)) // Twice of OWNER_GIVE_SIZE
#endif

/* -------------------------------------------------------------------- */

/* Flags: possibilities
 *     CH_NONE
 *     CH_EXTENDED | CH_BIGCREDIT                              (owner)
 *     CH_EXTENDED | CH_MASTER                                 (borrow)
 *     CH_EXTENDED | CH_MASTER | CH_BIGCREDIT                  (borrow)
 *     CH_EXTENDED | CH_SLAVE  | CH_MASTER | CH_BIGCREDIT      (borrow)
 */

enum CH_FLAGS{      
  CH_NONE=0,
  CH_EXTENDED=1,
  CH_BIGCREDIT=2,
  CH_MASTER=4,
  CH_SLAVE=8,
  CH_PERSISTENT=16
};

enum ReduceCode{
  CANNOT_REDUCE,
  CAN_REDUCE_LAST,
  CAN_REDUCE_SINGLE};

typedef int int_Credit;

/* -------------------------------------------------------------------- */

// Marshaling ---------------------------------------------------------
void marshalCredit(MarshalerBuffer *buf,Credit c) {
  PD((CREDIT_NEW,"marshalCredit %d %x",c.credit,c.owner));
  Assert(c.credit>0);
  if(c.owner==NULL) {
    buf->put(DIF_PRIMARY);
    marshalNumber(buf, c.credit);
  }
  else {
    buf->put(DIF_SECONDARY);
    marshalNumber(buf, c.credit);
    marshalDSite(buf, c.owner);
  }
}

// These difs assume on one credit and enclose the oti which makes
// is a full reference to the entity at the receiving owner.
void marshalCreditToOwner(MarshalerBuffer *buf,Credit c,int oti) {
  PD((CREDIT_NEW,"marshalToOwner %d %x",c.credit,c.owner));
  Assert(c.credit==1);
  if(c.owner==NULL) {
    buf->put(DIF_OWNER);
    marshalNumber(buf, oti);
  }
  else {
    buf->put(DIF_OWNER_SEC);
    marshalNumber(buf, oti);
    marshalDSite(buf, c.owner);
  }
}

#ifndef USE_FAST_UNMARSHALER

static Credit mkECredit(Credit c)
{
  Assert(0);
  c.credit = (int) 0;
  c.owner = (DSite *) 0;
  return (c);
}

Credit unmarshalCreditRobust(MarshalerBuffer *buf, int *error)
{
  Credit c;
  MarshalTag mt = (MarshalTag) buf->get();
  switch (mt) {
  case DIF_PRIMARY: {
    c.credit = unmarshalNumberRobust(buf, error);
    if (*error) return (mkECredit(c));
    c.owner = NULL;
    break;
  }
  case DIF_SECONDARY: {
    c.credit = unmarshalNumberRobust(buf, error);
    if (*error) return (mkECredit(c));
    c.owner = unmarshalDSiteRobust(buf, error);
    if (*error) return (mkECredit(c));
    break;
  }
  default: {
    *error = TRUE;
    return (mkECredit(c)); 
  }}

  Assert(c.credit>0);

  PD((CREDIT_NEW,"unmarshalCreditRobust %d %x",c.credit,c.owner));

  return c;
}

Credit unmarshalCreditToOwnerRobust(MarshalerBuffer *buf,
				    MarshalTag mt, int &oti,
				    int *error)
{
  Credit c;
  if(mt==DIF_OWNER){
    c.credit=1;
    oti = unmarshalNumberRobust(buf, error);
    if (*error) return (mkECredit(c));
    c.owner = NULL;
    return (c);
  } else {
    Credit tmp;
    tmp.credit = 1;
    Assert(mt == DIF_OWNER_SEC);
    oti = unmarshalNumberRobust(buf, error);
    if (*error) return (mkECredit(c));
    tmp.owner = unmarshalDSiteRobust(buf, error);
    if (*error) return (mkECredit(c));
    sendCreditBack(myDSite, oti, tmp);
    c.owner = ((DSite *) 0);
    c.credit = 0;
  }
  PD((CREDIT_NEW,"unmarshalCreditToOwnerRobust %d %x",c.credit,c.owner));
  return (c);
}

#else
Credit unmarshalCredit(MarshalerBuffer *buf) {
  Credit c;
  MarshalTag mt=(MarshalTag) buf->get();
  switch(mt) {
  case DIF_PRIMARY: {
    c.credit=unmarshalNumber(buf);
    c.owner=NULL;
    break;
  }
  case DIF_SECONDARY: {
    c.credit=unmarshalNumber(buf);
    c.owner=unmarshalDSite(buf);
    break;
  }
  default: {
    Assert(0);
  }}
  PD((CREDIT_NEW,"unmarshalCredit %d %x",c.credit,c.owner));

  Assert(c.credit>0);

  return c;
}

Credit unmarshalCreditToOwner(MarshalerBuffer *buf,MarshalTag mt,
			      int &oti) {
  Credit c;
  if(mt==DIF_OWNER){
    c.credit=1;
    oti=unmarshalNumber(buf);
    c.owner=NULL;
    return c;
  }
  else {
    Credit tmp;
    tmp.credit=1;
    Assert(mt==DIF_OWNER_SEC);
    oti=unmarshalNumber(buf);
    tmp.owner=unmarshalDSite(buf);
    sendCreditBack(myDSite,oti,tmp);
    c.owner=NULL;
    c.credit=0;
  }
  PD((CREDIT_NEW,"unmarshalCreditToOwner %d %x",c.credit,c.owner));
  return c;
}
#endif

// Calculations ------------------------------------------------------
inline int getBorrowGiveSize(int avail) {
  int_Credit give=avail>>BORROW_GIVE_SWITCH;
  Assert(give>=0);
  if(give>BORROW_LOW_THRESHOLD)
    return give;
  else if(give-2>=BORROW_MIN)
    return 2;
  else
    return 0;
}


// Extensions ---------------------------------------------------------

class OwnerCreditExtension{
  friend class OB_Entry;
  int_Credit credit[2];
  OwnerCreditExtension *next;
  
protected:
  
public:
  OwnerCreditExtension(){}
  void init(int_Credit);
  void requestCreditE(int_Credit);
  ReduceCode addCreditE(int_Credit);
  ReduceCode isReducible();
  OwnerCreditExtension* getNext(){return next;}
  void reduceLast();
  int_Credit reduceSingle(){
    Assert(credit[1]==START_CREDIT_SIZE);
    Assert(next==NULL);
    return credit[0];}
  int_Credit getCredit(int Index){
    return credit[Index];}
  void expand();
    
};

inline OwnerCreditExtension* newOwnerCreditExtension(){
  Assert(sizeof(OwnerCreditExtension)<=sizeof(Construct_3));
  return (OwnerCreditExtension*) genFreeListManager->getOne_3();}

inline void freeOwnerCreditExtension(OwnerCreditExtension* oce){
  genFreeListManager->putOne_3((FreeListEntry*)oce);}

void OwnerCreditExtension::init(int_Credit c){
  Assert(c > 0);
  credit[0]=c;
  credit[1]=START_CREDIT_SIZE;
  PD((CREDIT,"owner credit extension init %d %d %d",c, credit[0], credit[1]));
  next=NULL;}

void OwnerCreditExtension::requestCreditE(int_Credit req){
//    printf("requestCreidtE \n");
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

ReduceCode OwnerCreditExtension::addCreditE(int_Credit ret){
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

void OwnerCreditExtension::expand(){
//    printf("OwnerCreditExtension::expand\n");
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

class BorrowCreditExtension{
  friend class BorrowCreditHandler;
  union{
    int_Credit secCredit;          // SecSlave || (SecMaster && ~BigCredit)
    OwnerCreditExtension *oce; // SecMaster && BigCredit
    BorrowCreditExtension *bce; // SecSlave 
  }uSOB;
  int_Credit primCredit; 
  DSite* site;                // non-NULL SecSlave SecMaster

public:
  int_Credit msGetPrimCredit(){return primCredit;}
  int_Credit slaveGetSecCredit(){return uSOB.secCredit;}

protected:

  BorrowCreditExtension* getMaster(){return uSOB.bce;}
  OwnerCreditExtension* getBig(){return uSOB.oce;}
  DSite *getSite(){return site;}

  void initMaster(int_Credit cred){
    uSOB.secCredit=START_CREDIT_SIZE;
    primCredit=cred;
    site=NULL;}

  void initSlave(int_Credit pc,int_Credit sc,DSite *s){
    uSOB.secCredit=sc;
    primCredit=pc;
    site=s;}

  Bool getSecCredit_Master(int_Credit); // called from Master - can expand to Big
  Bool getSmall_Slave(int_Credit &);    // called from Slave - can expand to Master
  Bool getOne_Slave();              // called from Slave - can expand to Master

  int_Credit reduceSlave(int_Credit,DSite* &,int_Credit &); // called from Slave - before removal
  int_Credit addPrimaryCredit_Master(int_Credit); // called from Master
  int_Credit addPrimaryCredit_SlaveMaster(int_Credit); // called from Master



  int_Credit masterGetSecCredit(){return uSOB.secCredit;}

  void msSetPrimCredit(int_Credit c){primCredit=c;}
  void slaveSetSecCredit(int_Credit c){uSOB.secCredit=c;}
  void masterSetSecCredit(int_Credit c){uSOB.secCredit=c;}

  Bool isReducibleSlave() {return (primCredit>BORROW_MIN)||(primCredit==0);}
  Bool isReducibleMaster(){return uSOB.secCredit==START_CREDIT_SIZE;}
  Bool isReducibleBig(){return uSOB.oce->isReducible();}

  void expandSlave();    // called from Slave
  void expandMaster();   // called from Master
  
public:
  BorrowCreditExtension(){}
  void print_entry(int nr);
};

inline BorrowCreditExtension* newBorrowCreditExtension(){
  Assert(sizeof(BorrowCreditExtension)<=sizeof(Construct_3));
  return (BorrowCreditExtension*) genFreeListManager->getOne_3();}

inline void freeBorrowCreditExtension(BorrowCreditExtension* bce){
  genFreeListManager->putOne_3((FreeListEntry*)bce);}

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

Bool BorrowCreditExtension::getSecCredit_Master(int_Credit cred){
  int_Credit c=masterGetSecCredit();
  if(c>cred){ 
    PD((CREDIT,"Got %d SecCred %d left",cred, c-cred));
    masterSetSecCredit(c-cred);
    return NO;}
  PD((CREDIT,"Wanted %d SecCred %d in store",cred, c));
  expandMaster();
  return OK;} 

Bool BorrowCreditExtension::getSmall_Slave(int_Credit &cred){
  int_Credit c=slaveGetSecCredit();
  int_Credit give=getBorrowGiveSize(c);
  if(give>0){
    cred=give;
    slaveSetSecCredit(c-give);
    PD((CREDIT,"Got %d SecSlaveCred %d left",cred, c-cred));
    return NO;}
  PD((CREDIT,"Wanted %d SecSlaveCred %d in store",cred, c));
  expandSlave();
  return OK;}
  
Bool BorrowCreditExtension::getOne_Slave(){
  int_Credit c=slaveGetSecCredit();
  if(c>1){
    slaveSetSecCredit(c-1);
    PD((CREDIT,"Got one SecSlaveCred %d left", c-1));
    return NO;}
  PD((CREDIT,"Wanted one SecSlaveCred %d in store", c - 1));
  expandSlave();
  return OK;
}

/* reduce */

int_Credit BorrowCreditExtension::reduceSlave(int_Credit more,DSite* &s,int_Credit &secCredit){
//    printf("reduce Slave in:%d prim:%d sec:%d\n",more,msGetPrimCredit(),slaveGetSecCredit());
  Assert(msGetPrimCredit()+more < BORROW_HIGH_THRESHOLD);
  s=site;
  secCredit=slaveGetSecCredit();
  return msGetPrimCredit()+more;}

/* add PrimaryCredit */

int_Credit BorrowCreditExtension::addPrimaryCredit_Master(int_Credit more){
  int_Credit c=msGetPrimCredit()+more;
  if(c>BORROW_HIGH_THRESHOLD) { 
    msSetPrimCredit(BORROW_HIGH_THRESHOLD);
    return c-BORROW_HIGH_THRESHOLD;}
  msSetPrimCredit(c);
  return 0;}


/* Implementation methods of CreditHandlers --------------------------------*/

// CreditHandler - parent --------------------------------------------------
Bool CreditHandler::isExtended(){
  if(getFlags() & CH_EXTENDED) return OK;
  Assert(flags==0 || flags & CH_PERSISTENT);
  return NO;}

void CreditHandler::initCreditOB() {
  cu.credit=START_CREDIT_SIZE;
}

void CreditHandler::addCreditOB(int_Credit c) {
  Assert(!isExtended());
  cu.credit += c;
  Assert(cu.credit<=(START_CREDIT_SIZE)); 
}

void CreditHandler::subCreditOB(int_Credit c) {
  Assert(cu.credit>c);
  Assert(!isExtended());
  cu.credit -=c;}

void CreditHandler::setCreditOB(int_Credit c){
  cu.credit=c;}    

int_Credit CreditHandler::getCreditOB(){
  Assert(!isExtended());
  return cu.credit;}

Bool CreditHandler::isPersistent(){
  return (getFlags() & CH_PERSISTENT);}

void CreditHandler::makePersistent(){
  Assert(!isPersistent());
  addFlags(CH_PERSISTENT);
  cu.credit = PERSISTENT_CRED;}

// OwnerCreditHandler ------------------------------------------------------
void OwnerCreditHandler::setUp(int index) {
  oti=index;
  initCreditOB();
  setFlags(CH_NONE);
}

void OwnerCreditHandler::print() {
  if(isExtended()) {
    int ctr = 1;
    OwnerCreditExtension *next;
    next = cu.oExt;
    while(next != NULL){
      printf("ex:%d %d#%d ", ctr, next->getCredit(0), 
	     next->getCredit(1));
      ctr ++;
      next = next->getNext();
    }
  }
  else {
    if(cu.credit == -1)
      printf("PERSISTENT\t");
    else
      printf("%d", cu.credit);
  }

}

OZ_Term OwnerCreditHandler::extract_info() {
  OZ_Term credit;
  if(isExtended()) {
    OwnerCreditExtension *next;
    next = cu.oExt;
    credit = oz_nil();
    while(next != NULL){
      credit = oz_cons(OZ_recordInit(oz_atom("ext"),
		   oz_cons(oz_pairAI("credit0",next->getCredit(0)),
		   oz_cons(oz_pairAI("credit1",next->getCredit(1)),
		           oz_nil()))), credit);
      next = next->getNext();}
    credit = OZ_recordInit(oz_atom("big"), oz_cons(credit, oz_nil()));
  }
  else {
    if(cu.credit == -1)
      credit = oz_atom("persistent");
    else
      credit = oz_int(cu.credit);
  }

  return credit;
}

OwnerCreditExtension* OwnerCreditHandler::getOwnerCreditExtension(){
  Assert(isExtended());
  return cu.oExt;}

void OwnerCreditHandler::setOwnerCreditExtension(OwnerCreditExtension* oce){
  Assert(isExtended());
  cu.oExt=oce;}

void OwnerCreditHandler::extend(){
//    printf("OwnerCreditHandler::extend\n");
  OwnerCreditExtension* oce=newOwnerCreditExtension();
  oce->init(getCreditOB());
  addFlags(CH_EXTENDED|CH_BIGCREDIT);
  setOwnerCreditExtension(oce);}

void OwnerCreditHandler::getCredit(int_Credit req){
  //    printf("oe i:%d %d c:-%d\n",oti,osgetpid(),req);
  if(isExtended()){
    getOwnerCreditExtension()->requestCreditE(req);
    return;}
  int_Credit credit=getCreditOB();
  if(credit<=req){
    extend();
    getCredit(req);
    return;}
  subCreditOB(req);
  return;
}

void OwnerCreditHandler::addCreditExtended(int_Credit back){
  ReduceCode rc=getOwnerCreditExtension()->addCreditE(back);
  if(rc==CANNOT_REDUCE) return;
  if(rc==CAN_REDUCE_LAST){
    getOwnerCreditExtension()->reduceLast();
    return;}
  Assert(rc==CAN_REDUCE_SINGLE);
  removeExtension();
  removeFlags(CH_EXTENDED);}

void OwnerCreditHandler::addCredit(Credit back){
  PD((CREDIT_NEW,"owner::addCredit %d %d %x",oti,back.credit,back.owner));
  if(isExtended()){
    addCreditExtended(back.credit);
    return;}
  addCreditOB(back.credit);
  if (hasFullCredit())
    ownerTable->getOwner(oti)->localize(oti);
  return;}

Bool OwnerCreditHandler::hasFullCredit(){ 
  if(isPersistent()) return NO;
  if(isExtended()) return NO;
  int_Credit c=getCreditOB();
  Assert(c<=START_CREDIT_SIZE);  
  if(c<START_CREDIT_SIZE) return NO;
  return OK;}

Credit OwnerCreditHandler::getCreditBig() {
  Credit c;
  getCredit(OWNER_GIVE_CREDIT_SIZE);
  c.credit=OWNER_GIVE_CREDIT_SIZE;
  c.owner=NULL;
  PD((CREDIT_NEW,"owner::getCreditBig %d %d %x",oti,c.credit,c.owner));
  return c;
}

Credit OwnerCreditHandler::getCreditSmall() {
  Credit c;
  getCredit(1);
  c.credit=1;
  c.owner=NULL;
  PD((CREDIT_NEW,"owner::getCreditSmall %d %d %x",oti,c.credit,c.owner));
  return c;
}

void OwnerCreditHandler::removeExtension(){
  OwnerCreditExtension *oce=getOwnerCreditExtension();
  setCreditOB(oce->reduceSingle());
  freeOwnerCreditExtension(oce);
  return;}

// BorrowCreditHandler -----------------------------------------------------
void BorrowCreditHandler::setUp(Credit c,DSite* s,int i){
  Assert(s!=myDSite);
  setFlags(CH_NONE);
  setCreditOB(0);
  netaddr.set(s,i);
  /* PER-LOOK
     what happends if a probe is released when we ask
     for more credit?
  */
  PD((CREDIT_NEW,"borrow_new::addCredit %x %x %d %x",
      netaddr.site, netaddr.index,
      c.credit,c.owner));
  if (c.owner==NULL) {
    if (c.credit != PERSISTENT_CRED)
      addPrimaryCredit(c.credit);
    else
      Assert(isPersistent());
  } else {
    Assert(!isExtended());
    createSecSlave(c.credit,c.owner);
    Assert(isExtended());
  }
  if(cu.credit<=BORROW_LOW_THRESHOLD){
    moreCredit();
  }
}

void BorrowCreditHandler::setUpPersistent(DSite* s,int i){
  setFlags(CH_NONE);
  setCreditOB(0);
  makePersistent();
  Assert(isPersistent());
  netaddr.set(s,i);
  PD((CREDIT_NEW,"borrow_new::persistent %x %x",
      netaddr.site, netaddr.index));
}

void BorrowCreditHandler::print() {
  int ctr = 1;
  OwnerCreditExtension *next;
  switch(getExtendFlags()){
  case CH_PERSISTENT:
    printf("PERSISTENT\t");
    break;
  case CH_EXTENDED|CH_SLAVE|CH_MASTER|CH_BIGCREDIT:
    printf("%d(slave)\t\t", getSlave()->getMaster()->primCredit);
    next = getSlave()->getMaster()->uSOB.oce;
    while(next != NULL){
      printf("ex:%d %d#%d ", ctr, next->getCredit(0), 
	      next->getCredit(1));
      ctr ++;
      next = next->getNext();}
    break;
  case CH_EXTENDED|CH_SLAVE|CH_MASTER:
    printf("%d(slave)\t\t", getSlave()->getMaster()->primCredit);
    printf("%d", getSlave()->getMaster()->uSOB.secCredit);
    break;
  case CH_EXTENDED|CH_SLAVE:
    printf("%d\t\t", getSlave()->primCredit);
    printf("%d", getSlave()->uSOB.secCredit);
    break;
  case CH_EXTENDED|CH_MASTER|CH_BIGCREDIT:
    printf("%d\t\t", getMaster()->primCredit);
    next = getMaster()->uSOB.oce;
    while(next != NULL){
      printf("ex:%d %d#%d ", ctr, next->getCredit(0), 
	     next->getCredit(1));
      ctr ++;
      next = next->getNext();}
    break;
  case CH_EXTENDED|CH_MASTER:
    printf("%d\t\t", getMaster()->primCredit);
    printf("%d", getMaster()->uSOB.secCredit);
    break;
  case CH_NONE:
    printf("%d\t\t 0", cu.credit);
    break;
  default:
    Assert(0);}
}

void BorrowCreditHandler::extract_info(OZ_Term &primCred,OZ_Term &secCred) {
  OwnerCreditExtension *next;
  switch(getExtendFlags()){
  case CH_PERSISTENT:
    primCred = oz_atom("persistent");
    secCred = oz_atom("persistent");
    break;
  case CH_EXTENDED|CH_SLAVE|CH_MASTER|CH_BIGCREDIT:
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
  case CH_EXTENDED|CH_SLAVE|CH_MASTER:
    primCred = OZ_recordInit(oz_atom("slave"),
                 oz_pairII(1, getSlave()->getMaster()->primCredit));
    secCred = oz_int(getSlave()->getMaster()->uSOB.secCredit);
    break;
  case CH_EXTENDED|CH_SLAVE:
    primCred = OZ_recordInit(oz_atom("slave"),
                 oz_pairII(1, getSlave()->getMaster()->primCredit));
    secCred = oz_int(getSlave()->uSOB.secCredit);
    break;
  case CH_EXTENDED|CH_MASTER|CH_BIGCREDIT:
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
  case CH_EXTENDED|CH_MASTER:
    primCred = oz_int(getMaster()->primCredit);
    secCred = oz_int(getMaster()->uSOB.secCredit);
    break;
  case CH_NONE:
    secCred = oz_int(0);
    primCred = oz_int(cu.credit);
    break;
  default:
    Assert(0);
  }
}

Credit BorrowCreditHandler::getCreditBig() {
  Credit c;
  c.credit = getSmallPrimaryCredit();
  if(c.credit) {
    c.owner=NULL;
  } else {
//      printf("new:getSecondary %d\n",osgetpid());
    c.owner = getSmallSecondaryCredit(c.credit);  
  }
  PD((CREDIT_NEW,"borrow::getCreditBig %x %x %d %x",
      netaddr.site, netaddr.index,c.credit,c.owner));
  return c;
}

Credit BorrowCreditHandler::getCreditSmall() {
  Credit c;
  c.credit=1;
  if (getOnePrimaryCredit()) {
    c.owner=NULL;
  } else {
//      printf("new:getOneSecondary\n");
    c.owner = getOneSecondaryCredit();
  }
  PD((CREDIT_NEW,"borrow::getCreditSmall %x %x %d %x",
      netaddr.site, netaddr.index,c.credit,c.owner));
  return c;
}

void BorrowCreditHandler::addCredit(Credit c) {
  PD((CREDIT_NEW,"borrow::addCredit %x %x %d %x",netaddr.site, netaddr.index,
      c.credit,c.owner));
  if (c.owner==NULL) {
    if (c.credit != PERSISTENT_CRED)
      addPrimaryCredit(c.credit);
    else
      Assert(isPersistent());
  } else {
    addSecondaryCredit(c.credit, c.owner);
  }
}

void BorrowCreditHandler::giveBackAllCredit() {
  PD((CREDIT_NEW,"borrow::giveBackAllCredit %x %x %d",
      netaddr.site, netaddr.index,
      cu.credit));
  Assert(!isExtended());
  Assert(flags==0);
  if(cu.credit>0) { // Other case could occur if gc happens of an entity
                    // that didn't yet have any real credit.
    giveBackCredit(getCreditOB());
    setCreditOB(0);
  }
}

void BorrowCreditHandler::copyHandler(BorrowCreditHandler *from) {
  cu=from->cu;
  flags=from->flags;
  netaddr.set(from->netaddr.site,from->netaddr.index); 
}

NetAddress* BorrowCreditHandler::getNetAddress() {
  return &netaddr;}

Bool BorrowCreditHandler::maybeFreeCreditHandler() {
  if(isExtended()){
    if(getExtendFlags() & CH_MASTER) {
      return FALSE;
    }
    Assert(getExtendFlags() & CH_SLAVE);
    removeSlave();
  }
  Assert(!isExtended());
  return TRUE;
}

Bool BorrowCreditHandler::canBeFreed() {
  if(isExtended() && (getExtendFlags() & CH_MASTER))
    return FALSE;
  else
    return TRUE;
}

BorrowCreditExtension* BorrowCreditHandler::getSlave(){
  Assert(getFlags() & CH_SLAVE);
  return cu.bExt;}

BorrowCreditExtension* BorrowCreditHandler::getMaster(){
  Assert(getFlags() & CH_MASTER);
  Assert(!(getFlags() & CH_SLAVE));
  return cu.bExt;}


void BorrowCreditHandler::setSlave(BorrowCreditExtension* bce){
  Assert(getFlags() & CH_SLAVE);
  cu.bExt = bce;}

void BorrowCreditHandler::setMaster(BorrowCreditExtension* bce){
  Assert(getFlags() & CH_MASTER);
  Assert(!(getFlags() & CH_SLAVE));
  cu.bExt = bce;}


Bool BorrowCreditHandler::getOnePrimaryCredit_E(){
  int_Credit c=extendGetPrimCredit();
  if(c>BORROW_MIN+1) {
    extendSetPrimCredit(c-1);
    return OK;
  }
  else
    return NO;
}

int_Credit BorrowCreditHandler::getSmallPrimaryCredit_E(){
  int_Credit c=extendGetPrimCredit();
  int_Credit give=getBorrowGiveSize(c);
  if(give>0) {
    extendSetPrimCredit(c-give);
    return give;
  }
  else
    return 0;
}

void BorrowCreditHandler::thresholdCheck(int_Credit c){
  if((getCreditOB()+c>BORROW_LOW_THRESHOLD) &&
     (getCreditOB()<=BORROW_LOW_THRESHOLD)){
    Assert(!isExtended());
    moreCredit();}}

void BorrowCreditHandler::removeSoleExtension(int_Credit c){            
  BorrowCreditExtension* bce=getSlave();
  freeBorrowCreditExtension(bce);
  removeFlags(CH_MASTER|CH_SLAVE|CH_EXTENDED);
  setCreditOB(c);}

void BorrowCreditHandler::createSecMaster(){
  BorrowCreditExtension *bce=newBorrowCreditExtension();
  bce->initMaster(getCreditOB());
  setFlags(CH_MASTER|CH_EXTENDED);
  setMaster(bce);}

void BorrowCreditHandler::removeSlave() {
  BorrowCreditExtension* slave=getSlave();
  int_Credit c=slave->msGetPrimCredit();
  Assert((c>=BORROW_MIN)||(c==0));
  giveBackSecCredit(slave->getSite(),slave->slaveGetSecCredit());
  removeFlags(CH_SLAVE|CH_EXTENDED);
  freeBorrowCreditExtension(slave);
  setCreditOB(c);}

void BorrowCreditHandler::createSecSlave(int_Credit cred,DSite *s){
  BorrowCreditExtension *bce=newBorrowCreditExtension();
  bce->initSlave(0,cred,s);
  setFlags(CH_SLAVE|CH_EXTENDED);
  setSlave(bce);}

int_Credit BorrowCreditHandler::extendGetPrimCredit(){
  if(getFlags() & CH_MASTER)
    return getMaster()->msGetPrimCredit();
  return getSlave()->msGetPrimCredit();}
  
void BorrowCreditHandler::extendSetPrimCredit(int_Credit c){
  if(getFlags() & CH_MASTER)
    getMaster()->msSetPrimCredit(c);
  else
    getSlave()->msSetPrimCredit(c);}

void BorrowCreditHandler::generalTryToReduce(){  
  while(TRUE){
    switch(getExtendFlags()){
    case CH_EXTENDED|CH_SLAVE|CH_MASTER|CH_BIGCREDIT:{
      if(getSlave()->getMaster()->isReducibleBig()){
	removeBig(getSlave()->getMaster());
	PD((CREDIT,"slave+master removing big"));
	break;}
      return;}
  
    case CH_EXTENDED|CH_SLAVE|CH_MASTER:{
      if(getSlave()->getMaster()->isReducibleMaster()){
	removeMaster_SM(getSlave()->getMaster());
	PD((CREDIT,"slave removing master"));
	break;}
      return;}
  
    case CH_EXTENDED|CH_SLAVE:{
      if(getSlave()->isReducibleSlave()){
	PD((CREDIT,"removing slave"));
	removeSlave();}
      return;}
  
    case CH_EXTENDED|CH_MASTER|CH_BIGCREDIT:{
      if(getMaster()->isReducibleBig()){
	removeBig(getMaster()); 
	PD((CREDIT,"master removing big"));
	break;}
      return;}

    case CH_EXTENDED|CH_MASTER:{
      if(getMaster()->isReducibleMaster()){
	PD((CREDIT,"removing master"));
	removeMaster_M(getMaster());      
	break;}
      return;}
    case CH_NONE:{
      return;}
    default:{
      Assert(0);}
    }
  }
}

void BorrowCreditHandler::giveBackSecCredit(DSite *s,int_Credit cint){
  NetAddress *na = getNetAddress();
  DSite* site = na->site;
  int index = na->index;
  Credit c;
  c.credit=cint;
  c.owner=s;
  sendCreditBack(site,index,c);
}

void BorrowCreditHandler::removeBig(BorrowCreditExtension* master){
  OwnerCreditExtension *oce=master->getBig();
  master->masterSetSecCredit(oce->reduceSingle());
  freeOwnerCreditExtension(oce);
  removeFlags(CH_BIGCREDIT);
  generalTryToReduce();}

void BorrowCreditHandler::removeMaster_SM(BorrowCreditExtension* master){
  Assert(!(getExtendFlags() & CH_BIGCREDIT));
  Assert(master->masterGetSecCredit()==START_CREDIT_SIZE);  
  BorrowCreditExtension *slave=getSlave();
  int_Credit c=master->msGetPrimCredit();
  slave->slaveSetSecCredit(c);
  removeFlags(CH_MASTER);
  freeBorrowCreditExtension(master);
  generalTryToReduce();}

void BorrowCreditHandler::removeMaster_M(BorrowCreditExtension* master){
  Assert(!(getExtendFlags() & CH_BIGCREDIT));
  Assert(!(getExtendFlags() & CH_SLAVE));
  Assert(master->masterGetSecCredit()==START_CREDIT_SIZE);
  int_Credit c=master->msGetPrimCredit();
  removeFlags(CH_EXTENDED|CH_MASTER);
  freeBorrowCreditExtension(master);
  setCreditOB(c);
  BorrowEntry *be=BT->find(&netaddr);
  if(be->isRef()){  
    BT->maybeFreeBorrowEntry(BT->ptr2Index(be));}
}

void BorrowCreditHandler::addSecCredit_MasterBig(int_Credit c,BorrowCreditExtension *master){
  ReduceCode rc=master->getBig()->addCreditE(c);
  if(rc==CANNOT_REDUCE) return;
  if(rc==CAN_REDUCE_LAST){
    master->getBig()->reduceLast();
    return;}
  Assert(rc==CAN_REDUCE_SINGLE);
  generalTryToReduce();}

void BorrowCreditHandler::addSecCredit_Master(int_Credit c,BorrowCreditExtension *master){
  int_Credit cx=master->masterGetSecCredit()+c;
  master->masterSetSecCredit(cx);
  if(cx==START_CREDIT_SIZE){
    generalTryToReduce();}}

Bool BorrowCreditHandler::addSecCredit_Slave(int_Credit c,BorrowCreditExtension *slave){
  int_Credit cx=slave->slaveGetSecCredit()+c;
  if(cx>START_CREDIT_SIZE){
    return NO;}
  slave->slaveSetSecCredit(cx);
  return OK;}



int BorrowCreditHandler::getExtendFlags(){
  return getFlags();
}

void BorrowCreditHandler::addPrimaryCreditExtended(int_Credit c){  
  int_Credit overflow;
  switch(getExtendFlags()){
  case CH_EXTENDED|CH_SLAVE|CH_MASTER|CH_BIGCREDIT:
  case CH_EXTENDED|CH_SLAVE|CH_MASTER:{
    overflow=getSlave()->getMaster()->addPrimaryCredit_Master(c);
    break;}
  case CH_EXTENDED|CH_SLAVE:{
    DSite *s;
    int_Credit sec;
    overflow=getSlave()->reduceSlave(c,s,sec);
    removeSoleExtension(overflow);
    giveBackSecCredit(s,sec);
    //      printf("reduce Slave fin %d\n",getCreditOB());
    overflow=0; 
    break;}
  case CH_EXTENDED|CH_MASTER|CH_BIGCREDIT:
  case CH_EXTENDED|CH_MASTER:{
    overflow=getMaster()->addPrimaryCredit_Master(c);
    break;
  default:
    OZ_error("secondary credit error");
    Assert(0);}}
  if(overflow>0){
    giveBackCredit(overflow);}
}

void BorrowCreditHandler::addSecondaryCredit(int_Credit c,DSite *s){
  switch(getExtendFlags()){
  case CH_EXTENDED|CH_SLAVE|CH_MASTER|CH_BIGCREDIT:{
    if(s==myDSite){
      addSecCredit_MasterBig(c,getSlave()->getMaster());      
      return;}
    if(s==getSlave()->getSite()){
      if(addSecCredit_Slave(c,getSlave())) {return;}}
    break;}
  
  case CH_EXTENDED|CH_SLAVE|CH_MASTER:{
    if(s==myDSite){
      addSecCredit_Master(c,getSlave()->getMaster());
      return;}
    if(s==getSlave()->getSite()){
      if(addSecCredit_Slave(c,getSlave())) {return;}}
    break;}
  
  case CH_EXTENDED|CH_SLAVE:{
    if(s==getSlave()->getSite()){
      if(addSecCredit_Slave(c,getSlave())){return;}}
    break;}
  
  case CH_EXTENDED|CH_MASTER|CH_BIGCREDIT:{
    if(s==myDSite){
      addSecCredit_MasterBig(c,getMaster());
      return;}
    break;}

  case CH_EXTENDED|CH_MASTER:{
    if(s==myDSite){
      addSecCredit_Master(c,getMaster());
      return;}
    break;}
  default:
    break;
  }
#ifdef DEBUG_CHECK
  if(s==myDSite){
    OZ_warning("sec credit error: please inform andreas@sics.se");
    return;}
#endif
  giveBackSecCredit(s,c);
}


void BorrowCreditHandler::addPrimaryCredit(int_Credit c){
  //      printf("be i:%d %d c:+%d\n",getOTI(),netaddr.site->getTimeStamp()->pid,c);
  if(isExtended()) {
    addPrimaryCreditExtended(c);
    return;}
  int_Credit cur=getCreditOB();
  PD((CREDIT,"borrow add s:%s o:%d add:%d to:%d",
      oz_site2String(getNetAddress()->site),
      getNetAddress()->index,c,cur));
  if(cur>BORROW_HIGH_THRESHOLD){
    giveBackCredit(c+cur-BORROW_HIGH_THRESHOLD);
    setCreditOB(BORROW_HIGH_THRESHOLD);
    return;}
  addCreditOB(c);}

Bool BorrowCreditHandler::getOnePrimaryCredit(){
  PD((CREDIT,"Trying to get one primary credit"));
  if(isPersistent()) {
    PD((CREDIT,"Persistent, no credit needed"));
    PD((CREDIT,"%d credit left", getCreditOB()));
    return OK;}
  if(isExtended()){
    PD((CREDIT,"Structure extended, no primary"));
    return NO;}
  int_Credit tmp=getCreditOB();
  Assert(tmp>0);
  if(tmp-1<BORROW_MIN) {
    PD((CREDIT,"gopc low credit %d",tmp));
    PD((CREDIT,"got no credit"));
//    printf("BorrowCreditHandler::getOnePrimaryCredit got none, not yet extended\n");
    return NO;}
  //      printf("be i:%d %d c:-%d\n",getOTI(),netaddr.site->getTimeStamp()->pid,1);
  subCreditOB(1);
  thresholdCheck(1);
  PD((CREDIT,"Got one credit, %d credit left", tmp-1));
  return OK;}

int_Credit BorrowCreditHandler::getSmallPrimaryCredit(){
  PD((CREDIT,"Trying to get primary credit"));
  if(isPersistent())
    return PERSISTENT_CRED;
  if(isExtended()){
    PD((CREDIT,"Structure extended, no primary"));
    return NO;}      
  int_Credit tmp=getCreditOB();
  Assert(tmp>0);
  int_Credit give=getBorrowGiveSize(tmp);
  if(give>0) {
    subCreditOB(give);
    PD((CREDIT,"Got %d credit, %d credit left", 
	give,tmp-give));
    thresholdCheck(give);
    return give;
  }
  else {
    PD((CREDIT,"got no credit"));
    return 0;
  }
}

DSite *BorrowCreditHandler::getOneSecondaryCredit() {
  while(TRUE){
    switch(getExtendFlags()){
    case CH_EXTENDED|CH_SLAVE|CH_MASTER|CH_BIGCREDIT:{
      getSlave()->getMaster()->getBig()->requestCreditE(1);
      return myDSite;}
    case CH_EXTENDED|CH_SLAVE|CH_MASTER:{
      if(getSlave()->getMaster()->getSecCredit_Master(1)){
	addFlags(CH_BIGCREDIT);
	break;}
      return myDSite;}      
    case CH_EXTENDED|CH_MASTER:{
      if(getMaster()->getSecCredit_Master(1)){
	addFlags(CH_BIGCREDIT);
	break;}
      return myDSite;}
    case CH_EXTENDED|CH_MASTER|CH_BIGCREDIT:{
      getMaster()->getBig()->requestCreditE(1);
      return myDSite;}
    case CH_EXTENDED|CH_SLAVE:{
      if(getSlave()->getOne_Slave()){
	addFlags(CH_MASTER);
	break;}
      return getSlave()->getSite();}
    case CH_NONE:{
      createSecMaster();
      break;} 
    default:{
      OZ_error("secondary credit error");
      Assert(0);}

    }
  }
  return NULL; // stupid compiler
}

DSite *BorrowCreditHandler::getSmallSecondaryCredit(int_Credit &cred){
  while(TRUE){
    switch(getExtendFlags()){
    case CH_EXTENDED|CH_SLAVE|CH_MASTER|CH_BIGCREDIT:{
      cred=OWNER_GIVE_CREDIT_SIZE;
      getSlave()->getMaster()->getBig()->requestCreditE(OWNER_GIVE_CREDIT_SIZE);
      return myDSite;}
    case CH_EXTENDED|CH_SLAVE|CH_MASTER:{
      cred=OWNER_GIVE_CREDIT_SIZE;      
      if(getSlave()->getMaster()->getSecCredit_Master(OWNER_GIVE_CREDIT_SIZE)){
	addFlags(CH_BIGCREDIT);
	break;}
      return myDSite;}      
    case CH_EXTENDED|CH_MASTER:{
      cred=OWNER_GIVE_CREDIT_SIZE;      
      if(getMaster()->getSecCredit_Master(OWNER_GIVE_CREDIT_SIZE)){
	addFlags(CH_BIGCREDIT);
	break;}
      return myDSite;}
    case CH_EXTENDED|CH_MASTER|CH_BIGCREDIT:{
      cred=OWNER_GIVE_CREDIT_SIZE;
      getMaster()->getBig()->requestCreditE(OWNER_GIVE_CREDIT_SIZE);
      return myDSite;}
    case CH_EXTENDED|CH_SLAVE:{
      if(getSlave()->getSmall_Slave(cred)){
	addFlags(CH_MASTER);
	break;} // Now also master, try again.
      return getSlave()->getSite();}
    case CH_NONE:{
      createSecMaster();
      break;}
    default:{
      OZ_error("secondary credit error");
      Assert(0);}
    }
  }
}

void BorrowCreditHandler::giveBackCredit(int_Credit cint){
  //    printf("be i:%d %d c:-%d\n",getOTI(),netaddr.site->getTimeStamp()->pid
  //  	 ,c);
  NetAddress *na = getNetAddress();
  DSite* site = na->site;
  int index = na->index;
  Credit c;
  c.credit=cint;
  c.owner=NULL;
  sendCreditBack(site,index,c);
}

void BorrowCreditHandler::moreCredit(){
  NetAddress *na = getNetAddress();
  PD((CREDIT,"Asking for more credit %s",na->site->stringrep()));

  askForCredit(na->site,na->index);
}


  
