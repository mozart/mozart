/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *
 *  Contributors:
 *    Michael Mehl (mehl@dfki.de)
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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */
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
  MSG_RECEIVED, // 0                    // protocol -layer
  MSG_SENT,
  MSG_PREP,
  TABLE,                                        // borrow/owner table events without print
  TABLE2,                                       // borrow/owner table events without print
  GC,           // 5
  CREDIT,
  LOOKUP,
  GLOBALIZING,
  LOCALIZING,
  PD_VAR,       // 10                           variable protocol
  CELL,                                         // cell protocol
  LOCK,                                         // lock protocol
  SITE_OP,
  THREAD_D,

  MARSHAL,       // 15                  // marshaler
  MARSHAL_CT,
  UNMARSHAL,
  UNMARSHAL_CT,
  MARSHAL_BE,                                   // marshal begin/end
  REF_COUNTER,  // 20

  TCP,                                  // communication layer events
  WEIRD,
  TCP_INTERFACE,
  TCPCACHE,
  TCPQUEUE,     // 25
  SITE,
  REMOTE,
  MESSAGE,
  OS,
  BUFFER,       // 30
  READ,
  WRITE,
  CONTENTS,
  HASH,                                 // site/gname hash table
  HASH2,        // 35

  USER,                                 // misc
  SPECIAL,
  ERROR_DET,
  WRT_QUEUE,
  ACK_QUEUE,    // 40
  CELL_MGR,
  PROBES,
  LOCK_MGR,     // 43
  NET_HANDLER,
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

void networkTimer(int);

#define PD(Args) _PD Args

#else

#define PERDIO_DEBUG_DO(X)
#define PERDIO_DEBUG_DO1(INDEX,Y)

#define PD(Args)

#endif

#endif
