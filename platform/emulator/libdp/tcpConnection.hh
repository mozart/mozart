#ifndef __TCPCONNECTION_HH
#define __TCPCONNECTION_HH

// TCP specific versions of the corresponding procedures in connection.hh
void tcpTransObjReady(ComObj *comObj,TransObj *transObj);
void tcpComObjDone(ComObj *comObj);
Bool tcpInitAccept();
void tcpDoDisconnect(void *info);

#endif
