#include "mozart.h"
#include "mozart_task.hh"
#include <string.h>

class OzTask_Connect : public OzTask {
private:
  int fd;
  int ihost;
  char host[256];
  int port;
  int status;
public:
  OzTask_Connect(int f,int ih,int p)
    : OzTask(),fd(f),ihost(ih),port(p),status(0)
  {}
  OzTask_Connect(int f,char* s,int p)
    : OzTask(),fd(f),ihost(0),port(p),status(0)
  { strcpy(host,s); }
  virtual void execute(void);
  virtual void finish(void);
  virtual void gc(void){}
};

void OzTask_Connect::execute(void)
{
  struct sockaddr_in addr;

  if (ihost) {
    addr.sin_addr.s_addr=htonl(ihost);
    addr.sin_family = AF_INET;
    addr.sin_port = htons ((unsigned short) port);
  } 
  else {
    struct hostent *hostaddr;
    if ((hostaddr = gethostbyname(host)) == NULL) {
      status = -1;
      return;
    }
    memset((char *)&addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr,hostaddr->h_addr_list[0],sizeof(addr.sin_addr));
    addr.sin_port = htons ((unsigned short) port);
  }
  if (connect(fd,(struct sockaddr *) &addr,sizeof(addr))<0)
    status = errno;
}
