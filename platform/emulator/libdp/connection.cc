#include "base.hh"
#include "dpBase.hh"

#include "comObj.hh"
#include "transObj.hh"
#include "tcpTransObj.hh"
#include "dsite.hh"
#include "connection.hh"
#include "tcpConnection.hh"

enum TransportType {
  TRANS_TCP
};

TransportType transportType=TRANS_TCP;

void setTransport(OZ_Term t) {
  OZ_Term tcp=oz_atom("tcp");

  if(oz_eq(t,tcp)) transportType=TRANS_TCP;
}

OZ_Term getTransport() {
  if(transportType==TRANS_TCP)
    return oz_atom("tcp");
  else
    return oz_atom("error");
}

TransController *getTransController(DSite *site) {
  // Based on transportType a TransController is chosen, the DSite
  // will presumeably be used instead in the future.
  switch(transportType) {
  case TRANS_TCP:
    return tcptransController;
  }
}

// Order from comObj to connect.
// Begin with getting a transObj. This will eventually invoke transObjReady
// below.
void doConnect(ComObj *comObj) {
  getTransController(comObj->getSite())->getTransObj(comObj);
}

// The transController delivers a transObj(= a right to use a resource),
// this should now be filled in with a working connection.
void transObjReady(ComObj *comObj,TransObj *transObj) {
  switch(transportType) {
  case TRANS_TCP:
    tcpTransObjReady(comObj,transObj);
  }
}

// The comObj hands back a transObj that it is done with
void handback(ComObj *comObj, TransObj *transObj) {
  void *info=transObj->close();
  switch(transportType) {
  case TRANS_TCP:
    tcpDoDisconnect(info);
  }
  getTransController(comObj->getSite())->transObjFreed(comObj,transObj);
}

// The comObj is informing us that it no longer needs the connection
// it was waiting for.
void comObjDone(ComObj *comObj) {
  getTransController(comObj->getSite())->comObjDone(comObj);
  switch(transportType) {
  case TRANS_TCP:
    tcpComObjDone(comObj);
  }
}


Bool initAccept() {
  switch(transportType) {
  case TRANS_TCP:
    return tcpInitAccept();
  }
}
