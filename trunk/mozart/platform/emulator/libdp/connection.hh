#ifndef __CONNECTION_HH
#define __CONNECTION_HH

class DSite;
class ComObj;
class TransController;

// Get a connection(=working transObj) for comObj
void doConnect(ComObj *comObj);
// The transController delivers a transObj(= a right to use a resource)
void transObjReady(ComObj *comObj, TransObj *transObj);
// The comObj hands back a transObj that it is done with
void handback(ComObj *comObj, TransObj *transObj);
// The comObj informs that it is now done and does not need the connection
// it was waiting for.
void comObjDone(ComObj *comObj);

// Init the proper accepthandler.
Bool initAccept();

void changeMaxTCPCacheImpl();

// Used by dpMiscModule to set parameters such as what transport layer to use
void setIPAddress__(int adr);
int  getIPAddress();
void setIPPort__(int port);
int getIPPort();
void setTransport(OZ_Term);
OZ_Term getTransport();

#endif
