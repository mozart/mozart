#include "wsock.hh"

#include "base.hh"
#include "dpBase.hh"

// ERIK added as a hack
#include "perdio.hh"
#include "thr_int.hh"

//

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

#ifdef WINDOWS
#define ETIMEDOUT    WSAETIMEDOUT
#define EHOSTUNREACH WSAEHOSTUNREACH
#endif

#define NETWORK_ERROR(Args) {OZ_error Args;}

// Network parameters
unsigned int  ipPortNumber  = OZReadPortNumber;
int  ipIpNumber    = 0; // Zero indicates that the default should be used.

void tcpListenPort(int port, char* nodename){
  // Here should the parameter of the engine be set.

  // Klippt fran createTcpPort
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if(nodename==0) {
    NETWORK_ERROR(("createTcpPort"));
  }
  struct hostent *hostaddr;
  hostaddr=gethostbyname(nodename);
  if (hostaddr==NULL) {
    nodename = "localhost";
    hostaddr=gethostbyname(nodename);
    OZ_warning("Unable to reach the net, using localhost instead\n");
  }

  struct in_addr tmp;
  memcpy(&tmp,hostaddr->h_addr_list[0],sizeof(in_addr));
  ip_address ip=ntohl(tmp.s_addr);

  // Klipp klart

  TimeStamp timestamp(time(0),osgetpid());
  myDSite = makeMyDSite(ip, port, timestamp);
  Assert(myDSite!=NULL);
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
