/*
  Perdio Project, DFKI & SICS,
  Universit"at des Saarlandes
  Postfach 15 11 59, D-66041 Saarbruecken, Phone (+49) 681 302-5312
  SICS
  Box 1263, S-16428 Sweden, Phone (+46) 8 7521500
  Author: perbrand, scheidhr, mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#ifndef __PERDIO_DEBUG_HH
#define __PERDIO_DEBUG_HH

enum DEBUGType {
  SEND_EMIT,    // 0
  SEND_DONE,
  DEBT_SEC,
  DEBT_MAIN,
  MSG_RECEIVED,
  MSG_SENT,     // 5
  DELAYED_MSG_SENT,
  TABLE,
  TABLE2,
  GC,
  CREDIT,       // 10
  LOOKUP,
  GLOBALIZING,
  AUXILLARY,
  DEBT,
  MARSHALL,     // 15
  MARSHALL_CT,
  UNMARSHALL,
  UNMARSHALL_CT,
  MARSHALL_BE,
  PENDLINK,     // 20
  HASH,
  HASH2,
  USER,
  TCP,
  WEIRD,        // 25
  TCP_INTERFACE,
  TCPCACHE,
  TCPQUEUE,
  SITE,
  REMOTE,       // 30
  MESSAGE,
  OS,
  BUFFER,
  READ,
  WRITE,        // 35
  CONTENTS,
  SPECIAL,
  PD_VAR,
  CELL,
  SITE_OP,     // 40
  MSG_QUEUED,
  LAST
};

#ifdef DEBUG_PERDIO

#include <stdarg.h>

extern char *debugTypeStr[];

class DebugVector{
public:
  Bool ar[LAST];
  DebugVector(){
    int i;
    for(i=0;i<LAST;i++) {ar[i]=FALSE;}}
  Bool on(int t){return ar[t];}
  void set(int t){ar[t]=TRUE;}
  void unset(int t){ar[t]=FALSE;}
};

extern DebugVector *DV;
extern void resize_hash();

#define PERDIO_DEBUG_DO(X) X
#define PERDIO_DEBUG_DO1(INDEX,Y) {if(DV->on(INDEX)) {Y;}}

inline
void _PD(int i,char *format,...)
{
  if (DV->on(i)) {
    printf("%s: ",debugTypeStr[i]);
    va_list ap;
    va_start(ap,format);

    vprintf(format,ap);
    printf("\n");
  }
}

#define PD(Args) _PD Args

#else

#define PERDIO_DEBUG_DO(X)
#define PERDIO_DEBUG_DO1(INDEX,Y)

#define PD(Args)

#endif

#endif
