#ifdef INTERFACE
#pragma interface
#endif

class VirtualInfo;
class RemoteSite;
class VirtualSite;
class Site;
class SiteManager;
class SiteExtensionManager;

typedef unsigned short port_t;
typedef unsigned long ip_address;
typedef unsigned int FaultInfo;

/**********************************************************************/
/*   SECTION ::  global variables                                     */
/**********************************************************************/

extern Site * mySite;

/**********************************************************************/
/*   SECTION ::  used here defined in comm.cc                         */
/**********************************************************************/

void siteZeroActiveRef(Site *);
void siteZeroPassiveRef(Site *);

/**********************************************************************/
/*   SECTION :: return codes                                          */
/**********************************************************************/

enum MonitorReturn{
    MONITOR_OK,
    SIZE_THRESHOLD_REACHED,
    NO_MSGS_THRESHOLD_REACHED,
    MONITOR_ALREADY_EXISTS,
    NO_MONITOR_EXISTS,
    MONITOR_PERM
};

enum ProbeReturn{
  PROBE_INSTALLED,
  PROBE_ALREADY_INSTALLED,
  PROBE_DEINSTALLED,
  PROBE_NONEXISTENT,
  PROBE_OF_DIFFERENT_KIND,
    PROBE_PERM,
    PROBE_TEMP
};

enum GiveUpInput{
  ALL_GIVEUP,         // debug purpose only
    TEMP_GIVEUP
  };

enum GiveUpReturn{
    GIVES_UP,
    SITE_NOW_NORMAL,
    SITE_NOW_PERM
  };

enum ProbeType{
  PROBE_TYPE_ALL,
    PROBE_TYPE_PERM
};

enum SiteStatus{
  SITE_OK,
  SITE_PERM,
  SITE_TEMP
};

// sendTo return
#define ACCEPTED       0
#define PERM_NOT_SENT ~1
// TEMP_NOT_SENT  >0


/**********************************************************************/
/*   SECTION :: provided by communication layer                       */
/**********************************************************************/

Site * unmarshalPSite(MsgBuffer*);
Site * unmarshalSite(MsgBuffer*);

/**********************************************************************/
/*   SECTION :: provided by network communication layer               */
/**********************************************************************/

RemoteSite* createRemoteSite(Site*,int readCtr);

void zeroRefsToRemote(RemoteSite *);
void nonZeroRefsToRemote(RemoteSite *);
int sendTo_RemoteSite(RemoteSite*,MsgBuffer*,MessageType,Site*, int);
int discardUnsentMessage_RemoteSite(RemoteSite*,int);
int getQueueStatus_RemoteSite(RemoteSite*,int &noMsgs);  // return size in bytes
SiteStatus siteStatus_RemoteSite(RemoteSite*);
MonitorReturn monitorQueue_RemoteSite(RemoteSite*,int size,int no_msgs,void*);
MonitorReturn demonitorQueue_RemoteSite(RemoteSite*);
ProbeReturn installProbe_RemoteSite(RemoteSite*,int frequency);
ProbeReturn deinstallProbe_RemoteSite(RemoteSite*);
ProbeReturn probeStatus_RemoteSite(RemoteSite*,ProbeType &pt,int &frequncey,void* &storePtr);
GiveUpReturn giveUp_RemoteSite(RemoteSite*);
void discoveryPerm_RemoteSite(RemoteSite*);

void initNetwork();

/**********************************************************************/
/*   SECTION :: provided by virtual site communication layer          */
/**********************************************************************/

VirtualSite* createVirtualSite(Site*,int);

void unmarshalUselessVirtualInfo(MsgBuffer*); // discard marshalled virtual info (hit or unsent msg)
VirtualInfo* unmarshalVirtualInfo(MsgBuffer*);
void marshalVirtualInfo(VirtualInfo*,MsgBuffer*);

void zeroRefsToVirtual(VirtualSite *);
void nonZeroRefsToVirtual(VirtualSite *);

Bool inMyGroup(Site*,VirtualInfo*);

int sendTo_VirtualSite(VirtualSite*,MsgBuffer*,MessageType,Site*, int);
int discardUnsentMessage_VirtualSite(VirtualSite*,int);
int getQueueStatus_VirtualSite(VirtualSite*,int &noMsgs);  // return size in bytes
SiteStatus siteStatus_VirtualSite(VirtualSite*);
MonitorReturn monitorQueue_VirtualSite(VirtualSite*,int size,int no_msgs,void*);
MonitorReturn demonitorQueue_VirtualSite(VirtualSite*);
ProbeReturn installProbe_VirtualSite(VirtualSite*,ProbeType,int frequency,void*);
ProbeReturn deinstallProbe_VirtualSite(VirtualSite*,ProbeType);
ProbeReturn probeStatus_VirtualSite(VirtualSite*,ProbeType &pt,int &frequncey,void* &storePtr);
GiveUpReturn giveUp_VirtualSite(VirtualSite*);
void discoveryPerm_VirtualSite(VirtualSite*);

void dumpVirtualInfo(VirtualInfo*);

/**********************************************************************/
/*   SECTION :: class BaseSite                                       */
/**********************************************************************/

class BaseSite{
friend class Site;
friend class SiteManager;
friend class SiteHashTable;
private:
protected:
  ip_address address;
  time_t timestamp;
  port_t port;
  unsigned short flags;
  int refCtr;

  void init(ip_address ip,port_t p,time_t t){
    address=ip;
    timestamp=t;
    port=p;
    flags=0;}

  void init(ip_address a,port_t p,time_t t,unsigned short ty){
    init(a,p,t);
    flags=ty;}

  ip_address getAddress(){return address;}
  port_t getPort(){return port;}

  unsigned short getType(){return flags;}

public:
  BaseSite(){}
  BaseSite(ip_address a,port_t p,time_t t):address(a),port(p),timestamp(t){}

  time_t getTimeStamp(){return timestamp;} // ATTENTION
  int hashPrimary();
  int hashSecondary();

  char* stringrep();

  void marshalBaseSite(MsgBuffer* buf){
    marshalNumber(address,buf);
    marshalShort(port,buf);
    marshalNumber(timestamp,buf);}

  void unmarshalBaseSite(MsgBuffer* buf){
    address=unmarshalNumber(buf);
    PD((UNMARSHAL,"base site address %d",address));
    port=unmarshalShort(buf);
    PD((UNMARSHAL,"base site port %d",port));
    timestamp=unmarshalNumber(buf);
    PD((UNMARSHAL,"base site timestamp %d",timestamp));}


  int compareSitesNoTimestamp(BaseSite *s){
    if(address<s->address) return 0-1;
    if(s->address<address) return 1;
    if(port< s->port) return 0-1;
    if(s->port< port) return 1;
    return 0;}

  int checkTimeStamp(time_t t){
    if(t==timestamp) return 0;
    if(t<timestamp) return 1;
    return 0-1;}

  int compareSites(BaseSite *s){
    if(address<s->address) return 0-1;
    if(s->address<address) return 1;
    if(port< s->port) return 0-1;
    if(s->port< port) return 1;
    if(timestamp < s->timestamp) return 0-1;
    if(s->timestamp < timestamp) return 1;
    return 0;}
};

/**********************************************************************/
/*   SECTION :: Site                                                  */
/**********************************************************************/


#define REMOTE_SITE           1
#define VIRTUAL_SITE          2
#define CONNECTED             4
#define PERM_SITE             8
#define SECONDARY_TABLE_SITE 16
#define MY_SITE              32


/* Sites -14  possibilities


                    (REMOTE_SITE)              1  (REMOTE_SITE|CONNECTED)               5
                    (VIRTUAL_SITE)             2  (VIRTUAL_SITE|CONNECTED)              6
                    (VIRTUAL_SITE|REMOTE_SITE) 3  (VIRTUAL_SITE|REMOTE_SITE|CONNECTED)  7

                    ()                                               0  (passive)
                    (PERM_SITE)                                      8
                    (PERM_SITE|SECONDARY_TABLE_SITE)                24

                    (REMOTE_SITE|PERM_SITE|CONNECTED)               13
                    (VIRTUAL_SITE|PERM_SITE|CONNECTED)              14
                    (VIRTUAL_SITE|REMOTE_SITE|CONNECTED|PERM_SITE)  15
                    last 3 transitory
                    (MY_SITE)                                       32
                    (MY_SITE|VIRTUAL_SITE)                          34

*/

/**********************************************************************/
/*   SECTION :: class SiteExtension                                   */
/**********************************************************************/

class SiteExtension{
friend class Site;
friend class SiteExtensionManager;
  VirtualInfo *info;
  unsigned int extension; // RemoteSite* or VirtualSite* or readCtr;

protected:
  SiteExtension(){}

  void init(VirtualInfo *vi){
    info=vi;
    extension= (unsigned int) 0;}

  void init(){
    init(NULL);}

  VirtualInfo* getVirtualInfoSE(){return info;}
  void putVirtualInfoSE(VirtualInfo* vi){info=vi;}

  VirtualSite* getVirtualSiteSE(){return (VirtualSite*) extension;}
  void putVirtualSiteSE(VirtualSite* vs){extension = (unsigned int) vs;}
  void dumpVirtualSiteSE(int readCtr){extension=(unsigned int) readCtr;}

  RemoteSite* getRemoteSiteSE(){return (RemoteSite*) extension;}
  void putRemoteSiteSE(RemoteSite* vs){extension= (unsigned int) vs;}
  void dumpRemoteSiteSE(int readCtr){extension=(unsigned int)readCtr;}

  int getReadCtrSE(){return (int) extension;}
  void putReadCtrSE(int i){extension= (unsigned int) i;}
};

/**********************************************************************/
/*   SECTION :: class Site                                     */
/**********************************************************************/

enum FaultCode{
    COMM_FAULT_PERM_NOT_SENT,
    COMM_FAULT_PERM_MAYBE_SENT,
    COMM_FAULT_TEMP_NOT_SENT,
    COMM_FAULT_TEMP_MAYBE_SENT
  };

class Site: public BaseSite{
friend class SiteHashTable;
friend class RemoteSite;
friend class SiteManager;
private:
  unsigned int extension;

  void setType(unsigned int i){flags=i;}

  void disconnectInPerm();

  void disconnect(){
    flags &= (~CONNECTED);
    if(flags & PERM_SITE){
      disconnectInPerm();
      return;}
    return;}

  Bool connect(){
    unsigned int t=getType();
    PD((SITE,"connect, the type of this site: %d",t));
    if(t & CONNECTED) return OK;
    if(t & (PERM_SITE)) return NO;
    if(t & REMOTE_SITE){
      RemoteSite *rs=createRemoteSite(this,getSiteExtension()->getReadCtrSE());
      Assert(rs!=NULL);
      getSiteExtension()->putRemoteSiteSE(rs);
      PD((SITE,"connect; not connected yet, connecting to remote %d",rs));}
    else{
      VirtualSite *vs=createVirtualSite(this,getSiteExtension()->getReadCtrSE());
      getSiteExtension()->putVirtualSiteSE(vs);
      PD((SITE,"connect; not connected yet, connecting to virtual %d",vs));}
    flags |= CONNECTED;
    return OK;}

  void zeroActive(){
    if(getType() & CONNECTED){
      if(getType() & REMOTE_SITE){
        //      zeroRefsToRemote(getRemoteSite());
        return;}
      zeroRefsToVirtual(getVirtualSite());
      return;}
    siteZeroActiveRef(this);}

  void zeroPassive(){return;}
    //    siteZeroPassiveRef(this);}

  void makePermConnected(){
      flags |= PERM_SITE;
      return;}

  void makePerm(){
    flags |= PERM_SITE;
    disconnectInPerm();}

  Bool ActiveSite(){
    if(getType() & (REMOTE_SITE|VIRTUAL_SITE)) return OK;
    return NO;}

  void putPassiveRefCtr(int i){
    Assert(!(ActiveSite()));
    extension = (unsigned int) i;}

  int getPassiveRefCtr(){
    Assert(!(ActiveSite()));
    return (int) extension;}

  void incPassiveRefCtr(){
    putPassiveRefCtr(getPassiveRefCtr()+1);}

protected:

  unsigned short getType(){ return flags;}

  VirtualInfo* getVirtualInfo(){
    Assert(getType() & VIRTUAL_SITE);
    return getSiteExtension()->getVirtualInfoSE();}

  int getRefCtrS(){return refCtr;}
  void putRefCtrS(int i){ refCtr=i;}

public:

  Site(){}
  Site(ip_address a,port_t p,time_t t):BaseSite(a,p,t){
    extension=0;
    setType(MY_SITE);}

 Bool remoteComm(){
   if(getType() & REMOTE_SITE) return OK;
   if(getType() & PERM_SITE) return OK;      // ATTENTION
   return NO;}

  Bool commVirtual(){
    if(getType() & REMOTE_SITE) return OK;
    return NO;}

  SiteExtension * getSiteExtension(){
    return (SiteExtension*) extension;}

  void passiveToPerm(){
    Assert(!(ActiveSite()));
    flags |= PERM_SITE;
    return;}

  void putInSecondary(){setType(getType()|SECONDARY_TABLE_SITE);}

  Bool isInSecondary(){
    if(getType() & SECONDARY_TABLE_SITE) return OK;
    return NO;}

  void inc() {
    if(ActiveSite()){
      int aux=getRefCtrS();
      Assert(aux>=0);
      if(aux!=0){
        putRefCtrS(aux+1);
        return;}
      putRefCtrS(1);
      if(getType() & REMOTE_SITE){
        nonZeroRefsToRemote(getRemoteSite());
        return;}
      nonZeroRefsToVirtual(getVirtualSite());
      return;}
    incPassiveRefCtr();}

  void dec(){
    if(ActiveSite()){
      int aux=getRefCtrS();
      putRefCtrS(aux-1);
      if(aux>1){return;}
      zeroActive();
      return;}
    int aux=getPassiveRefCtr();
    putPassiveRefCtr(aux-1);
    if(aux>1) return;
    zeroPassive();}

  void refCheck(){
    if(ActiveSite()){
      if(getRefCtrS()==0) {
        zeroActive();
        return;}
      return;}
    if(getPassiveRefCtr()==0){
      zeroPassive();}
    return;}

  Bool isPerm(){return getType() & PERM_SITE;}

  void initMySiteR(SiteExtension *se){
    refCtr=1; // can never be reclaimed
    se->init(NULL);
    extension = (unsigned int)se;
    setType(MY_SITE);}

  void initMySiteV(SiteExtension *se,VirtualInfo *v){
    refCtr=1; // can never be reclaimed
    se->init(v);
    extension = (unsigned int)se;
    setType(MY_SITE|VIRTUAL_SITE);}

  void initVirtual(SiteExtension *se,VirtualInfo *vi){
    refCtr=0;
    se->init(vi);
    extension = (unsigned int)se;
    setType(VIRTUAL_SITE);}

  void initVirtualRemote(SiteExtension* se,VirtualInfo *vi){
    refCtr=0;
    extension = (unsigned int)se;
    se->init(vi);
    setType(VIRTUAL_SITE|REMOTE_SITE);}

  void initRemote(SiteExtension* se){
    refCtr=0;
    extension = (unsigned int)se;
    se->init();
    setType(REMOTE_SITE);}

  void initPerm(){
    refCtr=0;
    extension=0;
    setType(PERM_SITE);}

  void initPassive(){
    refCtr=0;
    extension=0;
    setType(0);}

// provided to network-comm

  RemoteSite* getRemoteSite() {
    if(!connect()) {PD((SITE,"getRemoteSite not connected"));return NULL;}
    Assert(getType() & CONNECTED);
    Assert(getType() & REMOTE_SITE);
    RemoteSite *rs = getSiteExtension()->getRemoteSiteSE();
    PD((SITE,"getRemoteSite returning the remote %d",rs));
    Assert(rs!=NULL);
    return  rs;}

  void dumpRemoteSite(int readCtr){
    Assert(getType() & CONNECTED);
    Assert(getType() & REMOTE_SITE);
    disconnect();
    getSiteExtension()->dumpRemoteSiteSE(readCtr);
    refCheck();}

// provided to virtual-comm

  VirtualSite* getVirtualSite(){
    if(!connect()) {return NULL;}
    Assert(getType() & CONNECTED);
    Assert(getType() & VIRTUAL_SITE);
    Assert(!(getType() & REMOTE_SITE));
    return getSiteExtension()->getVirtualSiteSE();}

  void dumpVirtualSite(int readCtr){
    Assert(getType() & CONNECTED);
    Assert(getType() & VIRTUAL_SITE);
    Assert(!(getType() & VIRTUAL_SITE));
    disconnect();
    getSiteExtension()->dumpVirtualSiteSE(readCtr);
    refCheck();}

  // for use by the network-comm and virtual-comm
  // ASSUMPTION: network-comm has reclaimed RemoteSite
//               virtual-comm has reclaimed VirtualSite



  // for use by the protocol-layer

  int sendTo(MsgBuffer *buf,MessageType mt,Site* storeSite,int storeIndex){
    PD((MSG_SENT,"to_site:%s type:%s",this->stringrep(),mess_names[(int) mt]));
    if(connect()){
      if(getType() & REMOTE_SITE){
        return sendTo_RemoteSite(getRemoteSite(),buf,mt,storeSite,storeIndex);}
      return sendTo_VirtualSite(getVirtualSite(),buf,mt,storeSite,storeIndex);}
    return PERM_NOT_SENT;}

  int discardUnsentMessage(int msgNum){
    if(getType()==(VIRTUAL_SITE|CONNECTED)){
      return discardUnsentMessage_VirtualSite(getVirtualSite(),msgNum);}
    Assert(getType() & REMOTE_SITE);
    Assert(getType() & CONNECTED);
    return discardUnsentMessage_RemoteSite(getRemoteSite(),msgNum);}

  int getQueueStatus(int &noMsgs){
    unsigned short t=getType();
    if(!(t & CONNECTED)){
      noMsgs=0;
      return 0;}
    if(t & REMOTE_SITE){
      return getQueueStatus_RemoteSite(getRemoteSite(),noMsgs);}
    return getQueueStatus_VirtualSite(getVirtualSite(),noMsgs);}

  SiteStatus siteStatus(){
    unsigned short t=getType();
    if(!(t & CONNECTED)) {
      if(t & PERM_SITE) {return SITE_PERM;}
      return SITE_OK;}
    if(t & REMOTE_SITE){
      return siteStatus_RemoteSite(getRemoteSite());}
    return siteStatus_VirtualSite(getVirtualSite());}

  MonitorReturn demonitorQueue(){
    unsigned short t=getType();
    if(!(t & CONNECTED)) {
      return NO_MONITOR_EXISTS;}
    if(t & REMOTE_SITE){
      return demonitorQueue_RemoteSite(getRemoteSite());}
    return demonitorQueue_VirtualSite(getVirtualSite());}

  MonitorReturn monitorQueue(int size,int noMsgs,void *storePtr){
    if(connect()){
      if(getType() & REMOTE_SITE){
        return monitorQueue_RemoteSite(getRemoteSite(),size,noMsgs,storePtr);}
      return monitorQueue_VirtualSite(getVirtualSite(),size,noMsgs,storePtr);}
    return MONITOR_PERM;}

  ProbeReturn installProbe(int frequency){
    if(connect()){
      if(getType() & REMOTE_SITE){
        return installProbe_RemoteSite(getRemoteSite(),frequency);}
      return installProbe_VirtualSite(getVirtualSite(),PROBE_TYPE_ALL,frequency,NULL);}
    return PROBE_PERM;}

  ProbeReturn deinstallProbe(){
    unsigned short t=getType();
    if(t & CONNECTED){
      if(t & REMOTE_SITE){
        return deinstallProbe_RemoteSite(getRemoteSite());}
      return deinstallProbe_VirtualSite(getVirtualSite(),PROBE_TYPE_ALL);}
    return PROBE_NONEXISTENT;}

  ProbeReturn probeStatus(ProbeType &pt,int &frequency,void* &storePtr){
    unsigned short t=getType();
    if(t & CONNECTED){
      if(t & REMOTE_SITE){
        return probeStatus_RemoteSite(getRemoteSite(),pt,frequency,storePtr);}
      return probeStatus_VirtualSite(getVirtualSite(),pt,frequency,storePtr);}
    return PROBE_NONEXISTENT;}

  GiveUpReturn giveUp(GiveUpInput flag){
    unsigned short t=getType();
    if(t & PERM_SITE) {return SITE_NOW_PERM;}
    if(flag==ALL_GIVEUP){
      if(t & CONNECTED){
        makePermConnected();
        if(t & REMOTE_SITE){
          return giveUp_RemoteSite(getRemoteSite());}
        return giveUp_VirtualSite(getVirtualSite());}
      makePerm();}
    if(siteStatus()==SITE_OK){ return SITE_NOW_NORMAL;}
    if(t & CONNECTED){
      makePermConnected();
      if(t & REMOTE_SITE){
        return giveUp_RemoteSite(getRemoteSite());}
      return giveUp_VirtualSite(getVirtualSite());}
    makePerm();
    return GIVES_UP;}

  void marshalSite(MsgBuffer *);
  void marshalPSite(MsgBuffer *buf);

  void discoveryPerm(){
    unsigned short t=getType();
    if(t==0) {
      flags |= PERM_SITE;
      return;}
    Assert(!(t & SECONDARY_TABLE_SITE));
    if(t & PERM_SITE) return;
    if(t & CONNECTED){
      makePermConnected();
      if(t & REMOTE_SITE){
        discoveryPerm_RemoteSite(getRemoteSite());
        return;}
      discoveryPerm_VirtualSite(getVirtualSite());
      return;}
    makePerm();}

// provided for network and virtual site comm-layers

  void communicationProblem(MessageType mt,Site*
                            storeSite,int storeIndex
                            ,FaultCode fc,FaultInfo fi);

  void probeFault(ProbeReturn pr);


  void sitePermProblem(){
    if(getType() & PERM_SITE) {return;}
    setType(getType()|PERM_SITE);
    disconnectInPerm();}

  void monitorInvoke(MonitorReturn mt,int size,int noMsgs){
    Assert(0);
    error("not implemented");
    return;}


  void msgReceived(MsgBuffer *);

  char* toString();
};



Site* initMySite(ip_address,port_t,time_t);
Site* initMySiteVirtual(ip_address,port_t,time_t,VirtualInfo*);
