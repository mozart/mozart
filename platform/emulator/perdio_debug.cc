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
  GC,
  CREDIT,
  LOOKUP,
  GLOBALIZING,
  AUXILLARY,
  DEBT,
  MARSHALL,
  UNMARSHALL,
  PENDLINK
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

#define PERDIO_DEBUG(INDEX,Y)
#define PERDIO_DEBUG1(INDEX,Y,A) 
#define PERDIO_DEBUG2(INDEX,Y,A,B) 
#define PERDIO_DEBUG3(INDEX,Y,A,B,C) 
#define PERDIO_DEBUG4(INDEX,Y,A,B,C,D) 

#endif 


/* **************************************************************** */

#ifdef DEBUG_PERDIO

DebugVector *DV = new DebugVector();

void dvset(){
  DV->set(SEND_DONE);
  DV->set(SEND_EMIT);
  DV->set(DEBT_SEC);
  DV->set(DEBT_MAIN);
  DV->set(MSG_RECEIVED);
  DV->set(MSG_SENT);
  DV->set(DELAYED_MSG_SENT);
  DV->set(TABLE);
  DV->set(GC);
  DV->set(CREDIT);
  DV->set(LOOKUP);
  DV->set(GLOBALIZING);
  DV->set(AUXILLARY);
  DV->set(DEBT);
  DV->set(MARSHALL);
  DV->set(UNMARSHALL);
  DV->set(PENDLINK);
}

#endif




