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

#ifdef DEBUG_PERDIO 

enum DEBUGType {
  SEND_EMIT,
  SEND_DONE,
  DEBT_SEC,
  DEBT_MAIN,
  MSG_RECEIVED,
  MSG_SENT,
  DELAYED_MSG_SENT,
  TABLE,
  TABLE2,
  GC,
  CREDIT,
  LOOKUP,
  GLOBALIZING,
  AUXILLARY,
  DEBT,
  MARSHALL,
  UNMARSHALL,
  PENDLINK,
  HASH,
  HASH2
};

class DebugVector{
public:
  Bool ar[40];
  DebugVector(){
    int i;
    for(i=0;i<40;i++) {ar[i]=FALSE;}}
  Bool on(DEBUGType t){return ar[(int)t];}
  void set(DEBUGType t){ar[(int)t]=TRUE;}
};

extern DebugVector *DV;
extern void resize_hash();

#define PERDIO_DEBUG_DO(X) X
#define PERDIO_DEBUG_DO1(INDEX,Y) {if(DV->on(INDEX)) {Y;}}

#define PERDIO_DEBUG(INDEX,Y) {if(DV->on(INDEX)) \
  {printf(Y);printf("\n");}}

#define PERDIO_DEBUG1(INDEX,Y,A) {if(DV->on(INDEX)) \
  {printf(Y,A);printf("\n");}}

#define PERDIO_DEBUG2(INDEX,Y,A,B) {if(DV->on(INDEX)) \
  {printf(Y,A,B);printf("\n");}}

#define PERDIO_DEBUG3(INDEX,Y,A,B,C) {if(DV->on(INDEX)) \
  {printf(Y,A,B,C);printf("\n");}}

#define PERDIO_DEBUG4(INDEX,Y,A,B,C,D) {if(DV->on(INDEX)) \
  {printf(Y,A,B,C,D);printf("\n");}}



#else

#define PERDIO_DEBUG_DO(X) 
#define PERDIO_DEBUG_DO1(INDEX,Y) 
#define PERDIO_DEBUG(INDEX,Y)
#define PERDIO_DEBUG1(INDEX,Y,A) 
#define PERDIO_DEBUG2(INDEX,Y,A,B) 
#define PERDIO_DEBUG3(INDEX,Y,A,B,C) 
#define PERDIO_DEBUG4(INDEX,Y,A,B,C,D) 

#endif 


#endif
