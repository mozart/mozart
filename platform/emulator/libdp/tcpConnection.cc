#include "wsock.hh"

#include "base.hh"
#include "dpBase.hh"

#ifndef WINDOWS
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <netdb.h>
#endif
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "os.hh"
#include "comObj.hh"
#include "transObj.hh"
#include "tcpTransObj.hh"
#include "dsite.hh"
#include "connection.hh"
#include "tcpConnection.hh"

#define OZReadPortNumber  9000
#define OZWritePortNumber 9500
#define OZStartUpTries    490

#ifdef WINDOWS
/* connect under windows is done in blocking mode */
#define OZConnectTries    10
void osSetNonBlocking(int fd, Bool onoff) {
  u_long dd = onoff;
  int ret = ioctlsocket(fd,FIONBIO,&dd);
  if (ret<0) 
    message("ioctlsocket(%d,FIONBIO,%d) failed: %d\n",fd,ossockerrno(),onoff);
}
#define ETIMEDOUT    WSAETIMEDOUT
#define EHOSTUNREACH WSAEHOSTUNREACH
#else
#define OZConnectTries    200
#endif

#define NETWORK_ERROR(Args) {OZ_error Args;}

// Network parameters
unsigned int  ipPortNumber  = OZReadPortNumber;
int  ipIpNumber    = 0; // Zero indicates that the default should be used.

class MySiteInfo{
public:
  int        tcpFD;    // the file descriptor of the port
  int        maxNrAck;
  int        maxSizeAck;
}mySiteInfo;

class InfoElement{
public:
  ComObj *comObj;
  TransObj *transObj;
  int fd;
};

int connectionFixReadHandler(int fd,void *info) {
  BYTE b;
  int ret;

  Assert(((InfoElement *) info)->fd == fd);
  Assert(info == ((InfoElement *) info)->comObj->infoelement);

  ComObj *comObj=((InfoElement *) info)->comObj;
  TransObj *transObj=((InfoElement *) info)->transObj;
  comObj->infoelement=(void *) 0x0;
  delete (InfoElement *) info;

  ret = osread(fd,&b,1);
  PD((TCP_INTERFACE,"fixReadHandler %d",fd));
  if(ret<=0) {
    OZ_unregisterRead(fd);
    osclose(fd);
    handback(comObj,transObj);

    DSite *site=comObj->getSite();
    site->discoveryPerm();
    site->probeFault(PROBE_PERM);

    return 0;
  }
  else {
    transObj->setSite(comObj->getSite());
    transObj->setOwner(comObj);
    tcptransController->addRunning(comObj);
    ((TCPTransObj *) transObj)->setFD(fd);
    if(comObj->handover(transObj))
      return 0; // handover generates another register read on this fd
                // return 1 deregisters that one...
    else {
      handback(comObj,transObj);

      return 0;
    }
  }
}

void tcpTransObjReady(ComObj *comObj,TransObj *transObj) {
  DSite *site=comObj->getSite();
  port_t aport=site->getPort();
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr=htonl(site->getAddress());
  addr.sin_port = htons(aport);
  
  int tries = OZConnectTries;
  int fd    = -1;
  int one   =  1;
  
retry:
  fd=ossocket(PF_INET,SOCK_STREAM,0);
  if (fd < 0) {
    if(ossockerrno()==ENOBUFS){
      goto  ipOpenNoAnswer;
    }
    printf("No socket\n");
    handback(comObj,transObj);
    site->discoveryPerm();
    site->probeFault(PROBE_PERM);
    return;
  }
  
  if(setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char*) &one,sizeof(one))<0){
    goto  ipOpenNoAnswer;
  }
  
#ifndef WINDOWS
  fcntl(fd,F_SETFL,O_NDELAY);
#endif
  if(osconnect(fd,(struct sockaddr *) &addr,sizeof(addr))==0 
     || ossockerrno()==EINPROGRESS) {

#ifdef WINDOWS
    // do this later on Windows otherwise it doesn't work
    osSetNonBlocking(fd,OK);
#endif
    //    printf("open success p:%d fd:%d\n",aport,fd);
    InfoElement *ie=new InfoElement();
    ie->transObj=transObj;
    ie->comObj=comObj;
    ie->fd=fd;
    comObj->infoelement=(void *) ie;
    OZ_registerReadHandler(fd,connectionFixReadHandler,(void *) ie);
    return;
  }
  
  tries--;
  if(tries<=0) {
    goto  ipOpenNoAnswer;
  }
  if((ossockerrno() == EADDRNOTAVAIL) || (ossockerrno() == ECONNREFUSED)){
//      printf("Connection failed\n");
    handback(comObj,transObj);
    site->discoveryPerm();
    site->probeFault(PROBE_PERM);
    return;
  }
  addr.sin_port = htons(aport); 
  goto retry;
  
 ipOpenNoAnswer:
  printf("ipOpenNoAnswer\n");
  if(fd >= 0) osclose(fd); 
  handback(comObj,transObj);
  site->discoveryPerm();
  site->probeFault(PROBE_PERM);
  return;
}

void tcpComObjDone(ComObj *comObj) {
  InfoElement *ie=(InfoElement *) comObj->infoelement;
  if(ie!=NULL) { // Waiting for fixreadhandler
      TransObj *transObj=ie->transObj;
      OZ_unregisterRead(ie->fd);
      osclose(ie->fd);
      comObj->infoelement=(void *) 0x0;
      delete ie;
      handback(comObj,transObj);
  }
}

int acceptHandler(int fd,void *unused) {
  TCPTransObj *tcptransObj=(TCPTransObj *) tcptransController->getTransObj();
  if(tcptransObj==NULL) {
    PD((TCP_INTERFACE,"No transobj avail for accept\n"));
    return 0;
  }

  struct sockaddr_in from;
  int fromlen = sizeof(from);
  int newFD=osaccept(fd,(struct sockaddr *) &from, &fromlen);
  if (newFD < 0) {
    tcptransController->transObjFreed(NULL,tcptransObj);
    return 0;
  }
  
  int one=1;
  if(setsockopt(newFD,IPPROTO_TCP,TCP_NODELAY,(char*) &one,sizeof(one))<0){
    tcptransController->transObjFreed(NULL,tcptransObj);
    return 0;
  }

#ifdef WINDOWS
  osSetNonBlocking(newFD,OK);
#else
  fcntl(newFD,F_SETFL,O_NDELAY);
#endif
  
  // FIX
  BYTE b=0xff;
  if(oswrite(newFD,&b,1)<0) {
    printf("CONNECTION weird error SHOULD NOT HAPPEN\n");
    tcptransController->transObjFreed(NULL,tcptransObj);
    return 0;
  }

  tcptransObj->setFD(newFD);
  comController_acceptHandler(tcptransObj);
  
  return 0 ;
}

inline Bool createTcpPort_RETRY(){
  if(ossockerrno()==ENOENT) return TRUE;
  if(ossockerrno()==EADDRINUSE) return FALSE;
  return FALSE;
}
  
static int createTcpPort(int port,ip_address &ip,port_t &oport,
			      int &fd, Bool takeIp) {
  int smalltries=OZConnectTries;
  int bigtries=OZStartUpTries;
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
  if(takeIp){
    char *nodename = oslocalhostname();
    if(nodename==0) {
      NETWORK_ERROR(("createTcpPort"));
    }
    struct hostent *hostaddr;
    hostaddr=gethostbyname(nodename);
    free(nodename);
    if (hostaddr==NULL) {
      nodename = "localhost";
      hostaddr=gethostbyname(nodename);
      OZ_warning("Unable to reach the net, using localhost instead\n");
    }
    
    struct in_addr tmp;
    memcpy(&tmp,hostaddr->h_addr_list[0],sizeof(in_addr));
    ip=ntohl(tmp.s_addr);
  }
 retry:
  if(bigtries<0){
    return -1;
  } // ATTENTION - major fault
  fd=ossocket(PF_INET,SOCK_STREAM,0);
  if (fd < 0) {
    if(ossockerrno()==ENOBUFS) 
      return -1;
    if (ossockerrno() == EINTR) {
      bigtries--;
      goto retry;
    }
    NETWORK_ERROR(("system:socket %d\n",ossockerrno()));
  }
  addr.sin_port = htons(port);

  if(bind(fd,(sockaddr *) &addr,sizeof(struct sockaddr_in))<0) {
    osclose(fd);
    smalltries--;
    if(createTcpPort_RETRY() && smalltries>0) goto retry;
    bigtries--;
    port++;
    smalltries=OZConnectTries;
    goto retry;
  }
  if (listen(fd,5)<0) {
    NETWORK_ERROR(("listen %d\n",ossockerrno()));
  }

  struct sockaddr_in addr1;
#if __GLIBC__ == 2
  unsigned int length = sizeof(addr1);
#else
  int length = sizeof(addr1);
#endif
  if (getsockname(fd, (struct sockaddr *) &addr1, &length) < 0) {
    NETWORK_ERROR(("getsockname %d\n",ossockerrno()));
  }
  oport=ntohs(addr1.sin_port);

  return 0;
}

Bool tcpInitAccept() { 
  /* RS: on Windows there seems to be a bug: reusing a port too
   * quickly leads to ECONNREFUSED. So we try to use a port number
   * randomly choosen between 9000 and 1000.
   * EK: if a fixed number is to be chosen no offset is added.
   */
  ip_address ip;
  port_t p;
  int tcpFD;
  int portno = ipPortNumber;

  if (ipPortNumber == OZReadPortNumber) portno += (osgetpid()%1000);
  int ret=createTcpPort(portno,ip,p,tcpFD,ipIpNumber==0);
  if (ret<0){
    NETWORK_ERROR(("Unable to bind port started at %d and ended at %d.",
		   portno,p));
    return FALSE;
  }
  TimeStamp timestamp(time(0),osgetpid());
  mySiteInfo.tcpFD=tcpFD;
  mySiteInfo.maxNrAck = 100;
  mySiteInfo.maxSizeAck = 10000;
  Assert(myDSite==NULL);
  if(ipIpNumber!=0) 
    ip = ipIpNumber;
  else
    ipIpNumber = ip;
  ipPortNumber = p;
  myDSite = makeMyDSite(ip, p, timestamp);
  Assert(myDSite!=NULL);  
  OZ_registerAcceptHandler(tcpFD,acceptHandler,NULL);
  return TRUE;
}

void tcpDoDisconnect(void *info) {
  if(((int) info)!=-1)
    osclose((int) info);
}


void changeMaxTCPCacheImpl() {
  tcptransController->changeNumOfResources();
}


// Used by dpMiscModule
void setIPAddress__(int adr) {
  ipIpNumber = adr;
}
int  getIPAddress(){return ipIpNumber;}
void setIPPort__(int port) {
  ipPortNumber = port;
}
int getIPPort(){return ipPortNumber;}
