/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 1998
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE)
#pragma implementation "DKSNode.hh"
#endif

#include "DKSNode.hh"
#include <math.h>
#include "msl_serialize.hh"
namespace _dss_internal{ 


  // ************************************* ENUMS *************************************************''

  enum DKSNodeMsgs{
    BECOME_NORMAL,
    CHANGE_SUCC_TO,
    JOIN_REQUEST,
    JOIN,
    JOIN_INIT,
    JOIN_FAILED,
    CHANGE_PRED,
    ACK_BECOME_NORMAL,
    RETRY_JOIN_REQUEST,
    LEAVE,
    YOU_CAN_LEAVE,
    BAD_POINTER,
    INSERT,
    ADAPT_PREDLIST,
    ASK_PRED,
    TELL_PRED,
    BROADCAST
  };

  char* DKSNodeStateVec[] = { "DNS_OUTSIDE",
			      "DNS_INSIDE",
			      "DNS_GETTING_OUT",
			      "DNS_GETTING_IN"
			      
  };
  
  char* DKSNodeMsgVec[] = {"BECOME_NORMAL",
			   "CHANGE_SUCC_TO",
			   "JOIN_REQUEST",
			   "JOIN",
			   "JOIN_INIT", 
			   "JOIN_FAILED",
			   "CHANGE_PRED",
			   "ACK_BECOME_NORMAL",
			   "RETRY_JOIN_REQUEST",
			   "LEAVE",
			   "YOU_CAN_LEAVE",
			   "BAD_POINTER",
			   "INSERT",
			   "ADAPT_PREDLIST",
			   "ASK_PRED",
			   "TELL_PRED",
			   "BROAD_CAST"
  };
  
  enum IntervalBounds{
    LEFT_LEFT,
    LEFT_RIGHT,
    RIGHT_RIGHT,
    RIGHT_LEFT
  };
  

  enum RTT_Tokens{
    RttT_ENTRY,
    RttT_SUSP,
    RttT_START,
    RttT_END
  };
  
  const int LE_SIZE = 200; 

  // **************************** INTERNAL CLASSES *************************

    class LevelEntry{
    public:
      int begin;
      int end;
      DksSite rsp;
      LevelEntry():begin(-2), end(-2), rsp(0, NULL){}
      
      void m_print(){
	printf("[%d %d[ => %d", begin, end, rsp.id); 
      }
    };
  



  // ***************************** AUX FUNCTIONS *****************************
  
  char* IntervalBoundsVec[] = { "]]", "][","[[","[]" };

  // {F2I {I2F N}/{I2F {Pow K L}}}
  int f_N_through_K_power_L(int N, int K, int L){
    return   static_cast<int>(static_cast<float>(N) / static_cast<float>(pow(K,L)));
  }
  // f_N_through_K_power_L(N,K,L)

  int f_oplus(int X,  int Y, int N)
  {
    return (X + Y) % N;
  }

  int f_ominus(int X, int Y, int N)
  {
    int diff = X - Y;
    if (diff < 0)
      return (N+diff) % N;
    else
      return diff % N;
  }


  void f_fillMsg_LI(int L, int I, 
		    MsgContainer* msg)
  {

    msg->pushIntVal(L); 
    msg->pushIntVal(I); 
  }
  

  void pushDksSite(MsgContainer *msgC, DksSite s){
    msgC->pushIntVal(s.id); 
    msgC->pushDSiteVal(s.a_site); 
  }

  DksSite popDksSite(::MsgContainer *msg){
    int id = msg->popIntVal(); 
    DSite *site = msg->popDSiteVal();
    return  DksSite(id, site);
  }
  void sendDksMsg(DksSite s, MsgContainer *msgC){
    s.a_site->m_sendMsg(msgC);  
  }
  
  bool f_belongs(int Id, int X, int Y, IntervalBounds Bounds, int N)
  {
    int NX = 0; 
    int NY = f_ominus(Y, X, N);
    int NId = f_ominus(Id, X, N);
    bool Ans; 
    switch(Bounds){
    case RIGHT_RIGHT: 
      Ans =  X == Y || (NId >= NX && NId < NY);
      break; 
    case LEFT_LEFT:
      Ans =  X == Y || (NId > NX && NId <= NY); 
      break; 
    case LEFT_RIGHT:
      Ans =  (X == Y && Id != X) || (NId > NX && NId < NY); 
      break;
    case RIGHT_LEFT:
      Ans = (X == Y && Id == X) || (NId >= NX && NId <= NY); 
      break; 
    default:
      Assert(0);
      printf("Helvete"); 
      return false; 
    }
    return Ans;
  }

  void f_printPredList(DksSite* vec, int f){
    for(int i = 0 ; f > i ; i++)
      {
	printf("%d:%d ", i, vec[i].id); 
     }
    printf("\n"); 
  }

  // *********************** DKS_RoutingTable class ****************************************

  
  int DKS_RoutingTable::m_pos(int l, int i){
    int real_l = l -1; 
    Assert(l >= 0); 
    Assert(i >= 0);
    Assert(real_l < a_L); 
    Assert(i < a_I); 
    return real_l * a_I + i;
  }
  
  DKS_RoutingTable::DKS_RoutingTable(int l, int i){
    a_L = l; 
    a_I = i; 
    a_rtt = new LevelEntry[l*i];
    for(int lctr = 1; lctr <= l ; lctr ++)
      for(int ictr = 0; ictr < i ; ictr ++){
	Assert(a_rtt[m_pos(lctr, ictr)].begin == -2);
	Assert(a_rtt[m_pos(lctr,ictr)].end == -2);
	a_rtt[m_pos(lctr ,ictr)].begin = -1;
	a_rtt[m_pos(lctr, ictr)].end =-1;
      }
  }
  
  void DKS_RoutingTable::m_gc(){
    for(int i = 0; i < a_L*a_I; i++)
      a_rtt[i].rsp.a_site->m_makeGCpreps();
  }

  void DKS_RoutingTable::printTable(){
    for(int l = 1; l <= a_L ; l ++){
      printf("%d:: ", l);
      for(int i = a_I - 1; i >= 0 ; i --){
	printf(" "); 
	a_rtt[m_pos(l, i)].m_print();
      }
      printf("\n"); 
    }
  }

  void DKS_RoutingTable::printLevel(int L){
    for(int i = a_I - 1; i >= 0 ; i --){
      a_rtt[m_pos(L, i)].m_print();
      printf(" "); 
    }
  }
  
  void DKS_RoutingTable::replaceResp(DksSite &oldNode, DksSite &newNode)
  {
    for(int l=1; l<=a_L; l++)
      for(int i = a_I-1; i>0 ; i--)
	{
	  if (a_rtt[m_pos(l,i)].rsp.id == oldNode.id)
	    {
	      set_respons(l, i, newNode);
	    }
	}
    
  }
  
  
  DKS_RoutingTable::~DKS_RoutingTable(){
    if(a_rtt)
      delete [] a_rtt;
    a_rtt = NULL;
  }
  
  int DKS_RoutingTable::begin(int l, int i){
    return a_rtt[m_pos(l, i)].begin;
  }

  int DKS_RoutingTable::end(int l, int i){
    return a_rtt[m_pos(l,i)].end;
  }
  
  DksSite DKS_RoutingTable::respons(int l, int i){
    return a_rtt[m_pos(l,i)].rsp;
  }

  void DKS_RoutingTable::set_respons(int l, int i, DksSite s){
    a_rtt[m_pos(l,i)].rsp = s; 
  }
  
  
  void DKS_RoutingTable::set(int l, int i, int b, int e, DksSite r){
    a_rtt[m_pos(l,i)].begin = b;
    a_rtt[m_pos(l,i)].end   = e;
    a_rtt[m_pos(l,i)].rsp   = r;
  }
  
  
  // *********************** DksSite class ****************************************'

  
  
  void DksSite:: m_makeGCpreps(){
    a_site->m_makeGCpreps();
  }
  
  DksSite* DksSite::copy(){
    return new DksSite(id, a_site); 
  }
  
  DksSite::~DksSite(){
    id = -1; 
    a_site = NULL; 
  }

  
  // *********************** DksSiteVecDct class *****************************************
  
  class DksSiteVecDct:public ExtDataContainerInterface
  {
  private:
    MsgnLayer* a_msl; 
    int a_cur; 
    int a_len;
    DksSite *a_vec;
  private:
    DksSiteVecDct& operator=(const DksSiteVecDct&){ Assert(0); return *this; }
    DksSiteVecDct(const DksSiteVecDct&):a_msl(NULL), a_cur(0), a_len(0), a_vec(NULL){ Assert(0);}
    
  public:
    DksSiteVecDct(DksSite *vec, int len):a_msl(NULL),a_cur(-1), a_len(len),
					 a_vec(new DksSite[len])
    {
      for(int i = 0; i < len; i++)
	a_vec[i] = vec[i]; 
    }

    DksSiteVecDct(MsgnLayer* env):a_msl(env),a_cur(-1), a_len(0),
				       a_vec(NULL)
    {;}
  
    virtual ~DksSiteVecDct(){
      if(a_vec)
	delete [] a_vec;
      a_vec = NULL; 
    }
    
    virtual bool marshal(DssWriteBuffer *bb){
      if(a_cur == -1)
	{
	  gf_MarshalNumber(bb,a_len); 
	  a_cur = 0; 
	}
      for(;bb->canWrite(LE_SIZE+1) &&  a_cur < a_len; a_cur++)
	{
	  gf_Marshal8bitInt(bb,RttT_ENTRY);
	  gf_MarshalNumber(bb, a_vec[a_cur].id); 
	  a_vec[a_cur].a_site->m_marshalDSite(bb);
	}
      if(a_cur == a_len){
	gf_Marshal8bitInt(bb,RttT_END);
	return true;
      }
      gf_Marshal8bitInt(bb,RttT_SUSP);
      return false; 
    }

  virtual bool unmarshal(DssReadBuffer *bb){
     if (a_cur == -1){
       a_len = gf_UnmarshalNumber(bb);
       a_cur = 0 ; 
       a_vec = new DksSite[a_len]; 
     }
     while(true) 
       {
	 switch(gf_Unmarshal8bitInt(bb)){
	 case RttT_ENTRY:
	   a_vec[a_cur].id = gf_UnmarshalNumber(bb);
	   a_vec[a_cur].a_site = a_msl->m_UnmarshalDSite(bb);
	   a_cur ++;
	   break; 
	 case RttT_END:
	   return true; 
	 case RttT_SUSP:
	   return false; 
	 }
       }
  }
  
  virtual void dispose(){
    if(a_vec) 
      delete [] a_vec; 
    a_vec = NULL; 
  }
  
 virtual BYTE getType(){
   return ADCT_DKS_SV;
  }
  virtual void resetMarshaling(){
    a_cur = -1; 
  }
  
    DksSite *getVec(){
    DksSite *ans = a_vec; 
    a_vec = NULL; 
    return ans; 
    }
  };


  ExtDataContainerInterface* createDksSiteVecContainer(MsgnLayer* env){
    return static_cast<ExtDataContainerInterface*>(new DksSiteVecDct(env)); 
  }
  // ************************  RoutingTableDct class ********************************************
  
    
    
  class RoutingTableDct:public ExtDataContainerInterface
  {
    MsgnLayer* a_msl; 
    int a_curPtr;
    DKS_RoutingTable *a_rtt;
    
  private: // just for that darn compiler
    RoutingTableDct& operator=(const RoutingTableDct&){ Assert(0); return *this; }
    RoutingTableDct(const RoutingTableDct&):a_msl(NULL), a_curPtr(0), a_rtt(NULL) {Assert(0); }
  public:
      virtual bool  marshal(DssWriteBuffer *bb){
	int arraySize = a_rtt->a_L * a_rtt->a_I;
	if (a_curPtr == -1)
	  {
	    gf_Marshal8bitInt(bb,RttT_START);
	    gf_MarshalNumber(bb, a_rtt->a_L); 
	    gf_MarshalNumber(bb, a_rtt->a_I);
	    a_curPtr = 0; 
	  }
	// Assuming that we only need LE_SIZE bytes per entry
	for(;bb->canWrite(LE_SIZE+1) &&  a_curPtr < arraySize; a_curPtr++)
	  {
	    //	printf("marshaling entry %d\n", a_curPtr); 
	    gf_Marshal8bitInt(bb,RttT_ENTRY);
	    gf_MarshalNumber(bb, a_rtt->a_rtt[a_curPtr].begin); 
	    gf_MarshalNumber(bb, a_rtt->a_rtt[a_curPtr].end); 
	    gf_MarshalNumber(bb, a_rtt->a_rtt[a_curPtr].rsp.id); 
	    a_rtt->a_rtt[a_curPtr].rsp.a_site->m_marshalDSite(bb);
	  }
	Assert(a_curPtr <= LE_SIZE);
	if(a_curPtr == arraySize){
	  gf_Marshal8bitInt(bb,RttT_END);
	  return true;
	}
	gf_Marshal8bitInt(bb,RttT_SUSP);
	return false;
      }
      
      virtual bool  unmarshal(DssReadBuffer *bb){
	if (a_curPtr == -1){
	  int chk = gf_Unmarshal8bitInt(bb); 
	  Assert(chk == RttT_START);
	  int L = gf_UnmarshalNumber(bb);
	  int I = gf_UnmarshalNumber(bb);
	  //   printf("Unmarshal RT L:%d I:%d\n", L, I); 
	  a_rtt = new DKS_RoutingTable(L, I);
	}
	int arraySize = a_rtt->a_L * a_rtt->a_I;	
	while(true){
	  int chk = gf_Unmarshal8bitInt(bb);
	  if (chk == RttT_ENTRY)
	    {
	      a_curPtr++; 
	      Assert(a_curPtr >= 0); 
	      Assert(a_curPtr < arraySize); 
	      a_rtt->a_rtt[a_curPtr].begin   = gf_UnmarshalNumber(bb);
	      a_rtt->a_rtt[a_curPtr].end = gf_UnmarshalNumber(bb);
	      int id = gf_UnmarshalNumber(bb);
	      DSite *site =  a_msl->m_UnmarshalDSite(bb);
	      a_rtt->a_rtt[a_curPtr].rsp =  DksSite(id, site);
	      continue; 
	    }
	  return chk == RttT_END;
	}
      }
      
      virtual void  dispose(){
	if (a_rtt != NULL)
	  {
	    delete  a_rtt;
	    a_rtt = NULL; 
	  }
	delete this; 
      }
      
    virtual ~RoutingTableDct(){
	;
      }
      
    virtual BYTE getType(){
      return ADCT_DKS_RT;
    }

    virtual void resetMarshaling(){
	a_curPtr = -1; 
      }

    public: 
      DKS_RoutingTable* getRTT(){
	DKS_RoutingTable *tmp = a_rtt;
	a_rtt = NULL; 
	return tmp;
      }
 
    RoutingTableDct(DKS_RoutingTable* rtt): a_msl(NULL), a_curPtr(-1), a_rtt(rtt){;}
    RoutingTableDct(MsgnLayer* env): a_msl(env), a_curPtr(-1), a_rtt(NULL){;}
  };
  
  ExtDataContainerInterface* createDksRoutingTableContainer(MsgnLayer* env){
    return static_cast<ExtDataContainerInterface*>(new RoutingTableDct(env)); 
  }

  // ************************   DksMessageDct class ********************************************


    
  

  // ************************  DKS_Params class ********************************************

  
  void  DKSNode::m_changePredH(DksSite U)
  {
    //printf("adapting to new process\n"); 
    //a_routingTable->printTable();
    //printf("\n OTHER NODE  TABLE \n\n"); 
    DKS_RoutingTable* RTT = m_computeRTFor(U);
    //    RTT->printTable();
    m_adaptTo(U);
    //printf("\n MY NEW TABLE \n\n"); 
    //a_routingTable->printTable();
    if (a_pred.id != a_myId.id)
      {  
	MsgContainer *msgC = m_createDKSMsg();
	msgC->pushIntVal(CHANGE_SUCC_TO); 
	pushDksSite(msgC,U); 
	sendDksMsg(a_pred,msgC);  
      }
    MsgContainer *msgC = m_createDKSMsg();
    msgC->pushIntVal(BECOME_NORMAL); 
    msgC->pushADC(new RoutingTableDct(RTT)); 
    msgC->pushADC(new DksSiteVecDct(a_predList, a_F)); 
    sendDksMsg(U,msgC); 
    a_pred = U;

    m_adaptPredList(0, U); 
    
    a_pPred.a_site = NULL; 
    if (!a_joinQueue.isEmpty())
      {
	m_processQ(); 
      }
  }

  DKS_RoutingTable* DKSNode::m_computeRTFor(DksSite Nj){
    if (a_myId.id == a_pred.id)
      return m_singletonInserter(Nj); 
    return m_nonsingletonInserter(Nj);
  }
  

  void 
  DKSNode::m_adaptPredList(int pos, DksSite site) 
  {
    for(int i = a_F - 1; i > pos ; i--)
      {
	a_predList[i] = a_predList[i-1]; 
      }
    Assert(pos < a_F); 
    a_predList[pos] = site; 
    f_printPredList(a_predList, a_F);
    DksSite succ = a_routingTable->respons(a_log_K_N, 1); 
    pos ++; 
    if (pos < a_F){
      MsgContainer *msgC = m_createDKSMsg(); 
      msgC->pushIntVal(ADAPT_PREDLIST);
      msgC->pushIntVal(pos); 
      pushDksSite(msgC,site); 
      sendDksMsg(succ,msgC); 
    }
  }
  
  
  void DKSNode::m_adaptTo(DksSite NJ){
    int K = a_K;
    for(int L = a_log_K_N ; L >= 1; L--)
      for(int I = 1 ; I <= K-1 ; I ++) 
	{
	  int Xil = a_routingTable->begin(L,I);
	  DksSite R = a_routingTable->respons(L,I);
	  if(f_belongs(Xil,  a_myId.id, NJ.id, LEFT_LEFT, a_N)&&
	     f_belongs(R.id, NJ.id,a_myId.id, LEFT_LEFT, a_N))
	    a_routingTable->set_respons(L,I, NJ); 
	  
	}
  }

  void DKSNode::msgReceived(::MsgContainer *msg, DSite *s_site){
    int s_id = msg->popIntVal();
    DksSite Sender(s_id, s_site);
    int msgType = msg->popIntVal();
    printf("%d [%d %d] In state %s receiving Msg %s from %d\n", a_myId.id,  a_pred.id,a_myId.id, DKSNodeStateVec[a_status], DKSNodeMsgVec[msgType], Sender.id);
    switch(msgType){
    case ASK_PRED: 
      {
	 MsgContainer *msgC = m_createDKSMsg(); 
	 msgC->pushIntVal(TELL_PRED);
	 pushDksSite(msgC, a_predList[0]); 
	 sendDksMsg(Sender, msgC); 
	 break;
      }
    case  TELL_PRED:
      {
	DksSite newPred = popDksSite(msg); 
	printf("from %d receiving new pred %d\n", Sender.id, newPred.id); 
	f_printPredList(a_predList, a_F); 
	for(int i = 0; i < a_F -1 ; i ++){
	  if(a_predList[i].id == Sender.id)
	    {
	      a_predList[i+1] = newPred; 
	    }
	}
	f_printPredList(a_predList, a_F); 
	m_correctPredList(a_myId); 
	break;
	}
    case ADAPT_PREDLIST:
      {
	if (a_status != DNS_GETTING_IN){
	  int e = msg->popIntVal();
	  DksSite NS = popDksSite(msg);
	  m_adaptPredList(e, NS); 
	}
	break;
      }
    case JOIN_REQUEST:
      if (a_status == DNS_INSIDE)
	{
	  m_joinRequestH(Sender); 
	  break;
	}
      if (a_status == DNS_GETTING_OUT)
	{
	  printf("duties\n") ;
	  // duties <- {Append @duties [joinRequest(s:Nj  stat:Stat)]}
	}
      break;
    case JOIN_FAILED:
      {
	printf("Id %d allready taken", a_myId.id);
	a_myId.id = (a_myId.id +3) % a_N; 
	printf("new id %d\n", a_myId.id);
	MsgContainer *msgC = m_createDKSMsg();
	msgC->pushIntVal(JOIN_REQUEST); 
	s_site->m_sendMsg(msgC); 
      }
    case JOIN:
      if (a_status == DNS_INSIDE)
	{
	  DksSite Nj = popDksSite(msg);
	  int L = msg->popIntVal();
	  int I = msg->popIntVal();
	  m_joinH(Sender, Nj, L, I);
	}
      if (a_status == DNS_GETTING_OUT)
	{
	  printf("duties\n") ;
	  Assert(0); 
	}
      break;
    case JOIN_INIT:
      if (a_status == DNS_GETTING_IN)
	{
	  DksSite P = popDksSite(msg);
	  DksMessage *dmsg  =a_callback->popDksMessage(msg); 
	  m_joinInitH(Sender, P, dmsg); 
	}
      if (a_status == DNS_GETTING_OUT)
	{
	  printf("##################panic(joinInitErr\n"); 
	  Assert(0);
	}
      break; 
    case CHANGE_PRED:
      if (a_status == DNS_INSIDE && Sender.id == a_pPred.id)
	m_changePredH(Sender);
      else
	{
	  if (a_status == DNS_GETTING_OUT) 
	    printf("##################panic(changePred(s:U r:Me))\n");
	  else
	    printf("changePredErr"); 
	  Assert(0);
	}
      break;
    case BECOME_NORMAL:
      if (a_status == DNS_GETTING_IN)
	{
	  ExtDataContainerInterface* dcv = msg->popADC();
	  DKS_RoutingTable *RTT = static_cast<RoutingTableDct*>(dcv)->getRTT();
	  dcv = msg->popADC();
	  DksSite *vec = static_cast<DksSiteVecDct*>(dcv)->getVec();
	  m_becomeNormalH(Sender, RTT, vec);
	}
      else
	{
	  printf("Error become_normal\n");
	  Assert(0);
	}
      break;
    case ACK_BECOME_NORMAL:
      if (a_status == DNS_INSIDE)
	{
	  m_ackBecomeNormalH(); 
	}
      else
	{
	  printf("Error ack_become_normal\n");
	  Assert(0);
	}
      break;
    case RETRY_JOIN_REQUEST:
      if (a_status == DNS_GETTING_IN)
	{
	  DksSite P = popDksSite(msg);
	  m_retryJoinRequestH( P); 
	}
      else
	{
	  Assert(0); 
	}
      break;
    case CHANGE_SUCC_TO:
      if (a_status == DNS_INSIDE)
	{
	  DksSite NS = popDksSite(msg);
	  m_changeSuccToH(NS); 
	}
      else
	{
	  printf("Error ack_become_normal\n");
	  Assert(0);
	}
      break;
    case LEAVE:
      if (a_status == DNS_INSIDE)
	{
	  DksSite w = popDksSite(msg);
	  m_leaveH(w);
	}
      else
	{
	  printf("Error, leave\n");
	  Assert(0);
	}
      break;
    case YOU_CAN_LEAVE:
      if (a_status == DNS_GETTING_OUT)
	{
	  m_youCanLeaveH(Sender); 
	}
      else
	{
	  printf("Error, you_cal_leave\n");
	  Assert(0);
	}
      break;
    case INSERT:
      if (a_status == DNS_INSIDE) 
	{
	  int Key = msg->popIntVal();
	  DksMessage *dMsg = a_callback->popDksMessage(msg); 
	  int L = msg->popIntVal();
	  int I = msg->popIntVal();
	  m_insertH(Sender, Key, dMsg, L, I); 
	}
       else
	 Assert(0); 
       break; 
       
    case BAD_POINTER:
      {
	if(a_status == DNS_INSIDE)
	  {
	    DksSite c = popDksSite(msg);
	    m_badPointerH(Sender, c, msg); 
	  }
	else
	  Assert(0); 
	break; 


      }
    case BROADCAST:
      {
	DksBcMessage *bcMsg = a_callback->popDksBcMessage(msg); 
	int limit    = msg->popIntVal();
	int level    = msg->popIntVal();
	int interval = msg->popIntVal();
	m_broadCastH(Sender, bcMsg, level, interval, limit); 
	delete bcMsg; 
	break; 
      }
    default: 
      printf("Unhandled Msg\n");
      Assert(0);
    }
  }


  // Initializes a new dks ring. 
  DKSNode::DKSNode(int N, int K, int F, int Id, 
		   DSite* mySite,
		   DKS_userClass* usr):
    a_pred(Id, mySite),
    a_myId(Id, mySite), 
    a_pPred(), 
    a_callback(usr),
    a_routingTable(NULL),
    a_predList(NULL), 
    a_K(K), a_N(N),  a_F(F), 
    a_log_K_N(static_cast<int>( log(static_cast<float>(N)) / log(static_cast<float>(K)))), 
    a_status(DNS_INSIDE)
  {

    //printf("DKSNode(%d %d %d) => %d log_%d(%d) = %d\n", N, K, F, Id, K, N, a_log_K_N); 
    a_routingTable = m_routingTableForFirstNode();
    //    a_routingTable->printTable();
    a_predList = new DksSite[F];

    for(int i = 0; i < a_F ; i ++){
      a_predList[i] = a_myId; 
    }
  }
  
  // Creates an unconnected member instance to an 
  // allready existing dks ring
  DKSNode::DKSNode(int N, int K, int F, int Id, DSite* mySite):
    a_pred(),
    a_myId(Id, mySite), 
    a_pPred(), 
    a_routingTable(NULL), 
    a_predList(new DksSite[F]),
    a_K(K),a_N(N),  a_F(F), 
    a_log_K_N(static_cast<int>( log(static_cast<float>(N)) / log(static_cast<float>(K)))), 
    a_status(DNS_OUTSIDE)
  { 
    printf("DKSNode(%d %d %d) => %d\n", N, K, F, Id); 
  }
  
  
  DKSNode::~DKSNode(){
    delete [] a_predList; 
    delete a_routingTable; 
  }
    
  
  void DKSNode::m_joinNetwork(DSite *entry){
    if(a_status == DNS_OUTSIDE){
      a_status = DNS_GETTING_IN; 
      MsgContainer *msgC = m_createDKSMsg();
      msgC->pushIntVal(JOIN_REQUEST); 
      entry->m_sendMsg(msgC); 
    }
  }
  


  DKS_RoutingTable* DKSNode::m_routingTableForFirstNode(){
     DKS_RoutingTable *R= new DKS_RoutingTable(a_log_K_N, a_K);
     int N = a_N;
     int K = a_K;
     printf("Creating RT\n{RTfirst %d %d %d %d}\n",a_myId.id,N,K,a_log_K_N);
     for(int L = 1 ; L <= a_log_K_N ; L ++)
       {
	 for(int I = 0; I < a_K ; I ++) 
	   {
	     int tmp = f_N_through_K_power_L(N,K,L);
	     int B = f_oplus(a_myId.id,   I*tmp, N);
	     int E = f_oplus(a_myId.id,  (I+1)*tmp, N); 
	     R->set(L, I, B, E, a_myId);
	   }
       }
     return R; 
   }


   DKS_RoutingTable* DKSNode::m_singletonInserter(DksSite Nj)
   {
     DKS_RoutingTable *NjRT= new DKS_RoutingTable(a_log_K_N, a_K);
     int N = a_N;
     int K = a_K;
     for(int L = a_log_K_N ; L >= 1 ; L--)
       for(int I = 0; I < a_K ; I ++) {
	 int tmp = f_N_through_K_power_L(N, K, L);
	 int B = f_oplus(Nj.id,   I*tmp, N);
	 int E = f_oplus(Nj.id,  (I+1)*tmp, N); 
	 if (I==0){
	   NjRT->set(L,I, B, E, Nj); 
	   continue; 
	 }
	 if (f_belongs(B, Nj.id, a_myId.id, LEFT_LEFT, a_N)){
	   NjRT->set(L,I, B, E, a_myId);
	   continue; 
	 }
	 if (f_belongs(B, a_myId.id , Nj.id, LEFT_RIGHT, a_N)){
	   NjRT->set(L,I, B, E, Nj);
	   continue;
	 }
	 printf("zzzzzzzzzzzzzzzzz singletonInserterErr(%d)\n",B);
       }
     return NjRT;
   }

   DKS_RoutingTable* DKSNode::m_nonsingletonInserter(DksSite Nj)
   {
     DKS_RoutingTable *NjRT= new DKS_RoutingTable(a_log_K_N, a_K);
     int N = a_N;
     int K = a_K;
     for(int L = a_log_K_N ; L >= 1 ; L--)
       for(int I = 0; I < K ; I ++) {
	 int tmp = f_N_through_K_power_L(N,K,L);
	 int B = f_oplus(Nj.id,   I*tmp, N);
	 int E = f_oplus(Nj.id,  (I+1)*tmp, N); 
	 if (I==0){
	   NjRT->set(L,I, B, E, Nj); 
	   continue; 
	 }
	 if (f_belongs(B ,Nj.id , a_myId.id ,LEFT_LEFT, a_N)){
	   NjRT->set(L,I, B, E, a_myId);
	   continue; 
	 }
	 if (f_belongs(B, a_pred.id, Nj.id, LEFT_RIGHT, a_N)){
	   NjRT->set(L,I, B, E, Nj);
	   continue;
	 }
	 if (f_belongs(B, a_myId.id, a_pred.id, LEFT_LEFT, a_N)){
	   NjRT->set(L,I, B, E, m_approxSucc(B));
	   continue; 
	 }
	 printf("zzzzzzzzzzzzzzzzz nonsingletonInserterErr\n");
       }
     return NjRT;
   }


   DksSite DKSNode::m_approxSucc(int S){
     int K = a_K;
     int Me = a_myId.id; 
     int N = a_N; 

     for (int L = a_log_K_N; L >= 1; L--)
       {
	 for(int I = 1; I < K ; K++)
	   {
	     int Xil = f_oplus(Me, I*f_N_through_K_power_L(N,K,L), N);
	     if(f_belongs(Xil,Me , a_pred.id, LEFT_LEFT, a_N))
	       {
		 if(Xil == S) 
		   return a_routingTable->respons(L,I);
		 if(f_belongs(Xil,Me, S, LEFT_RIGHT, a_N))
		   continue; 
		 if(f_belongs(Xil,S ,a_pred.id, LEFT_RIGHT, a_N))
		   return a_routingTable->respons(L,I);
		 printf("zzzzzzzzzzzzzzzzz approxSuccErr1(s:%d xil:%d p:%d)\n", 
			S, Xil, a_pred.id); 
	       }
	     else
	       return  a_pred; 
	   }
       }
     return a_pred; 
   }


   void DKSNode::m_insertInBack(DksSite Nj) 
   {
     a_pPred = Nj; 
     DksMessage* transfer = a_callback->m_divideResp(a_myId.id, Nj.id, a_N);
     if (transfer)
       m_transferResponsability(transfer); 
   }
  
  void DKSNode::m_transferResponsability(DksMessage* transfer)
  {
    MsgContainer *msgC = m_createDKSMsg();
    msgC->pushIntVal(JOIN_INIT);
    pushDksSite(msgC,a_pred); 
    a_callback->pushDksMessage(msgC, transfer); 
    sendDksMsg(a_pPred,msgC); 
  }
  
  void DKSNode::m_insertOrForward(int L,  int I,  DksSite Nj)
  {
    if (Nj.id == a_myId.id){
      MsgContainer *msgC = m_createDKSMsg();
      msgC->pushIntVal(JOIN_FAILED);
      sendDksMsg(Nj, msgC); 
      return; 
    }
    if (f_belongs(Nj.id, a_pred.id, a_myId.id, LEFT_RIGHT, a_N))
      {
	if (a_pPred.a_site == NULL)
	  m_insertInBack(Nj);
	else
	  {
	    printf(" zzzzzzzzzzzzzzzzzzenquing(n:%d nj:%d)\n",a_myId.id,Nj.id);
	    //a_joinQueue.append(Nj); 
	  }
      }
    else
       {
	 MsgContainer *msg =  m_createDKSMsg();
	 msg->pushIntVal(JOIN);
	 pushDksSite(msg, Nj);
	 m_forward(msg, L, Nj.id, a_myId);

       }    
   }

  void DKSNode::m_forward(MsgContainer *msg, int L, int  Nj, DksSite sender)
   {
     int I;
     DksSite nxtNode;
     

     //a_routingTable->printTable();
     do{
       L ++; 
       for(I = 0; I < a_K ; I++) 
	 {
	   int B = a_routingTable->begin(L,I);
	   int E = a_routingTable->end(L,I);
	   if(f_belongs(Nj, B, E, RIGHT_RIGHT, a_N))
	     {
	       nxtNode = a_routingTable->respons(L,I);
	       break; 
	     }
	 }
       
     }
     while(nxtNode.a_site == a_myId.a_site);
     
     if (nxtNode.a_site == NULL) 
       {
	 printf("forward - failed Nj: %d L:%d\n", Nj, L); 
	 a_routingTable->printTable();
	 Assert(0); 
       }
     
       
     msg->pushIntVal(L);
     msg->pushIntVal(I);
     sendDksMsg(nxtNode, msg);
     //     printf("forwarding to: %d\n", nxtNode.id); 
   }

   void DKSNode::m_processQ(){
     DksSite* E = a_joinQueue.pop();
     printf("zzzzzzzzzzzzzzzzzz#dequed\n"); 
     if(f_belongs(E->id, a_pred.id, a_myId.id, LEFT_RIGHT, a_N))
       m_insertInBack(*E); 
     else
       {
	 MsgContainer *msgC = m_createDKSMsg();
	 Assert(0); 
	 // This code will not work. 
	 // Go back to the source and check out the 
	 // semantics for this operation. 
	 msgC->pushIntVal(RETRY_JOIN_REQUEST);
	 sendDksMsg(*E,msgC); 
       }
   }



   //===============================================================
   // H A N D L E R S
   //===============================================================	 



  void DKSNode::m_joinRequestH(DksSite Nj){
     m_insertOrForward(0, 0, Nj); 
   }

   void  DKSNode::m_becomeNormalH(DksSite U, DKS_RoutingTable *RTT, DksSite* vec)
   {
     
     Assert(a_routingTable == NULL); 
     a_routingTable = RTT; 
     a_predList = vec; 
     f_printPredList(a_predList, a_F); 
     a_status = DNS_INSIDE; 
     a_callback->dks_functional();
     MsgContainer *msgC = m_createDKSMsg();
     msgC->pushIntVal(ACK_BECOME_NORMAL); 
     sendDksMsg(U,msgC); 
   }

  void DKSNode::m_ackBecomeNormalH(){
    ; 
  }

  int DKSNode::m_getId(){
    return a_myId.id;
  }

  void DKSNode::m_retryJoinRequestH( DksSite P){
     MsgContainer *msgC = m_createDKSMsg();
     msgC->pushIntVal(JOIN_REQUEST); 
     sendDksMsg(P,msgC);
   }

   void DKSNode::m_changeSuccToH(DksSite NS)
   {
     m_adaptTo(NS); 
   }


   void DKSNode::m_joinH(DksSite U, DksSite Nj, 
			 int L, int I)
   {
     int N = a_N;
     int K = a_K; 
     int Xil = f_oplus(U.id, I * f_N_through_K_power_L(N, K, L), N);
     if (f_belongs(Xil, a_pred.id, a_myId.id, LEFT_LEFT, a_N))
       m_insertOrForward(L, I, Nj);
     else
       {
	 printf("I'm bad(ptr) sender: %d key: %d L. %d %d - %d \n", U.id, Nj.id, L, a_pred.id, a_myId.id);
	 MsgContainer *msg =  m_createDKSMsg();
	 msg->pushIntVal(BAD_POINTER); 
	 pushDksSite(msg,a_pred); 
	 msg->pushIntVal(JOIN);
	 pushDksSite(msg,Nj); 
	 f_fillMsg_LI(L, I,msg); 
	 sendDksMsg(U,msg); 
       }
   } 

  void DKSNode::m_joinInitH(DksSite U, DksSite P, DksMessage *trans )
  {
     Assert(a_routingTable == NULL);
     a_callback->m_newResponsability(P.id, a_myId.id, a_N,  trans); 
     a_pred = P;
     MsgContainer *msg =  m_createDKSMsg();
     msg->pushIntVal(CHANGE_PRED);
     sendDksMsg(U,msg);
   }

   void DKSNode::m_leaveDKSRing(){
     DksSite succ = a_routingTable->respons(a_log_K_N,1);
     a_status = DNS_GETTING_OUT;
     MsgContainer *msg =  m_createDKSMsg();
     msg->pushIntVal(LEAVE);
     pushDksSite(msg,a_myId);
     sendDksMsg(succ,msg);
   }

  void DKSNode::m_leaveH(DksSite W){
    if (W.id == a_pred.id){
       printf("Receiving and installing a lot of items... not done\n");

       MsgContainer *msg =  m_createDKSMsg();
       msg->pushIntVal(YOU_CAN_LEAVE);
       pushDksSite(msg,a_myId);
       sendDksMsg(W,msg);
     }
     else
       {
	 printf("###################leaveHErr(");
	 Assert(0); 
       }
   }

  void DKSNode::m_youCanLeaveH(DksSite U){
    printf("Here we should pass all duties along, we dont right now\n"); 
  }

  void DKSNode::m_forwardInsert(DksMessage *dMsg,
				  int L, 
				  int T,
				  DksSite sender // might not be needed
				)
  {
    MsgContainer *msg =  m_createDKSMsg();
    msg->pushIntVal(INSERT);  
    msg->pushIntVal(T);
    a_callback->pushDksMessage(msg, dMsg);
    m_forward(msg, L, T, sender);  
  }



  DKSRouteRes DKSNode::m_route(int T, DksMessage* msg)
  {
    if (T >= a_N || T < 0 ) return DRR_INVALID_KEY;
    if (a_status == DNS_GETTING_OUT)     return DRR_CLOSING; 
    if (a_status == DNS_GETTING_IN)      return DRR_OPENING;
    if(f_belongs(T, a_pred.id, a_myId.id, LEFT_LEFT, a_N))
      {
	return DRR_DO_LOCAL; 
      }
    else
      {
	m_forwardInsert(msg, 0, T, a_myId);
	return DRR_ROUTING; 
      }
  }
  



  DKSRouteRes DKSNode::m_broadcastRing(DksBcMessage* msg)
  {
    if (a_status == DNS_GETTING_OUT)     return DRR_CLOSING; 
    if (a_status == DNS_GETTING_IN)      return DRR_OPENING;
    m_broadCastH(a_myId, msg, 1,0,a_myId.id);
    return DRR_ROUTING; 
  }
  
  
  
  

  void DKSNode::m_insertH(DksSite U,  int T, DksMessage *dMsg, int L, int I)
  {
    int N = a_N; 
    int K = a_K; 
    if(U.id == a_myId.id){
      m_forwardInsert(dMsg, L, T, a_myId);
      return; 
    }
    int C = a_pred.id; 
    int Xil =  f_oplus(U.id, I*f_N_through_K_power_L(N,K,L), N);
    if (f_belongs(Xil, C, a_myId.id, LEFT_LEFT, N))
      {
	if(f_belongs(T, C, a_myId.id, LEFT_LEFT, N))
	  {
	    //	    printf("The item is mine key:%d myId:%d=>", T, a_myId.id);
	    a_callback->m_receivedRoute(T,dMsg); 
	  }
	else
	  m_forwardInsert(dMsg, L, T, a_myId);
	
      }
    else
      {
	 MsgContainer *msg =  m_createDKSMsg();
	 printf("BadPointer INSERT\n"); 
	 msg->pushIntVal(BAD_POINTER); 
	 pushDksSite(msg,a_pred); 
	 msg->pushIntVal(INSERT);
	 msg->pushIntVal(T); 
	 a_callback->pushDksMessage(msg, dMsg); 
	 f_fillMsg_LI(L, I, msg); 
	 sendDksMsg(U,msg); 
      }
}


  void DKSNode::m_correctPredList(DksSite failedNode){
    // the last non clered entry is asked for its predecessor. 
    int entry; 
    for(entry = a_F -1; entry >=0 && a_predList[entry].a_site == NULL  ; entry --);
    if(entry<a_F -1 && entry >= 0)
      {
	MsgContainer *msgC = m_createDKSMsg(); 
	msgC->pushIntVal(ASK_PRED);
	pushDksSite(msgC,failedNode);  // this will tell the target process that deadNode is dead. 
	sendDksMsg( a_predList[entry], msgC);  
      }
  }
  

    void DKSNode::nodeFailed(DSite* site, FaultState s, MsgContainer* msg){
    DksSite deadNode;  
    
    if (s & FS_PERM == 0) return; 
    
    if (a_status != DNS_INSIDE) return; 
    
    for (int l=a_log_K_N; l>=1; l--) // find DksNode associated with site
      {
	for(int i=1; i<=a_K - 1; i++)
	  {
	    if (a_routingTable->respons(l, i).a_site == site)
	      {
		deadNode = a_routingTable->respons(l, i);
		break; 
	      }
	    
	  }
      }
    
    if(deadNode.a_site != NULL) {
      a_routingTable->printTable();
      printf("Node %i left or failed ",deadNode.id);
	DksSite replacement; 
	for(int i = 0; i < a_F ; i ++)
	  if(a_predList[i].id != deadNode.id){
	    replacement = a_predList[i];
	    break; 
	  }
	if(replacement.a_site == NULL)
	  {
	    printf("Failed to find replacement\n");
	    return ; 
	  }
	
	
	for (int ll=a_log_K_N; ll>=1; ll--)
	  {
	    for(int ii=1; ii<a_K ; ii++)
	      {
		if(f_belongs(a_routingTable->respons(ll,ii).id, deadNode.id,
			     a_myId.id, LEFT_RIGHT, a_N)) {
		  replacement = a_routingTable->respons(ll,ii);
		  printf(", replacing with node %i",replacement.id);
		  break;
		}
		
	      }
	  }
	Assert(replacement.id!=deadNode.id); 
	a_routingTable->printTable();
	printf("Replacing %d with %d\n",deadNode.id,replacement.id); 
	
	a_routingTable->replaceResp(deadNode, replacement); // Swap everywhere in the RT
	
	a_routingTable->printTable();
	for (; msg!=NULL; msg=msg->m_getNext())
	  {
	    printf("Resending  to %d!\n", replacement.id); 

	    //sendDksMsg(replacement, msg);   // resend
	  }

    }
    while(msg!=NULL)
      {
	printf("deleting \n");  



	MsgContainer* tmp = msg; 
	msg = msg->m_getNext();
	delete tmp; 
      }
      
    int entry = -1; 
    for(int i = 0; i < a_F; i ++){
      if (a_predList[i].a_site == site){
	deadNode = a_predList[i];
	entry = i; 
      }
    }
    
    if(entry > -1){
      f_printPredList(a_predList, a_F); 
      for(int ii = entry; ii < a_F -1 ; ii++)
	{
	  a_predList[ii] = a_predList[ii+1]; 
	}
      // a failed entry in the predList(vector) is cleared
      a_predList[a_F -1].a_site = NULL; 
      a_predList[a_F -1].id = -1;
      a_pred = a_predList[0];
      m_correctPredList(deadNode); 
    }
    
  }
  
  void DKSNode::m_badPointerH(DksSite U, DksSite C, MsgContainer *msg)
  {
    if(C.id == a_myId.id)
      {
	printf("################### badCandidate(r:%d c:%d)\n", 
	       a_myId.id, C.id ); 
	Assert(0); 
      }
    else
      {
	printf("Bad pointer found\n"); 
	m_adaptTo(C); 
	printf("Adapted to C\n"); 
	MsgContainer *out_msg =  m_createDKSMsg();
	int type = msg->popIntVal();
	out_msg->pushIntVal(type);
	switch(type){
	case JOIN:{
	  pushDksSite(out_msg,popDksSite(msg));
	  break;
	}
	case INSERT: {
	  printf("found insert\n"); 
	  out_msg->pushIntVal(msg->popIntVal()); 
	  a_callback->pushDksMessage(out_msg, a_callback->popDksMessage(msg));
	  break;
	}
	case BROADCAST: {
	  printf("found broadcast\n"); 
	  a_callback->pushDksBcMessage(out_msg,a_callback->popDksBcMessage(msg));
	  out_msg->pushIntVal(msg->popIntVal()); 
	  break; 
	}
	default:
	  printf("Bad pointer , what %d?!\n", type); 
	  Assert(0); 
	}
	out_msg->pushIntVal(msg->popIntVal());  // L
	out_msg->pushIntVal(msg->popIntVal());  // I
	sendDksMsg(C,out_msg); 
      }
  }
  
  
  void DKSNode::m_broadCastH(DksSite sender, DksBcMessage* load, int level, int interval, int limit)
  {
    int N = a_N;
    int K = a_K; 
    int Xil = f_oplus(sender.id, interval * f_N_through_K_power_L(N, K, level), N);
    printf("received a broadcast, myId: %d level:%d interval:%d limit:%d\n", 
	   a_myId.id, level, interval, limit);
    
    
    if(f_belongs(Xil, a_pred.id, a_myId.id, LEFT_LEFT, a_N))
      {
	if(sender.id != a_myId.id){
	  printf("Deliver the load: %d = ]%d %d]\n", Xil,a_pred.id, a_myId.id); 
	  a_callback->m_receivedBroadcast(load); 
	}
	for(int i = 1; i <= a_log_K_N; i ++)
	  for(int t = K -1 ; t > 0; t --){
	    DksSite resp = a_routingTable->respons(i, t);
	    if(f_belongs(resp.id, a_myId.id, limit, LEFT_RIGHT, a_N))
	      {
		printf("Sending to %d, limit:%d\n", resp.id, limit); 
		MsgContainer *msg = m_createDKSMsg();
		msg->pushIntVal(BROADCAST);
		a_callback->pushDksBcMessage(msg, load);
		msg->pushIntVal(limit);
		msg->pushIntVal(i);
		msg->pushIntVal(t);
		sendDksMsg(resp, msg);
		limit = f_oplus(a_myId.id,t * f_N_through_K_power_L(N, K, i), N); 
	      }
	  }
      }
    else
      {
	MsgContainer *bp_msg =  m_createDKSMsg(); 
	printf("BadPointer BROADCAST %d != ]%d %d]\n", Xil,a_pred.id, a_myId.id); 
	bp_msg->pushIntVal(BAD_POINTER);
	pushDksSite(bp_msg,a_pred); 
	bp_msg->pushIntVal(BROADCAST);
	a_callback->pushDksBcMessage(bp_msg, load);
	bp_msg->pushIntVal(limit); 
	bp_msg->pushIntVal(level); 
	bp_msg->pushIntVal(interval); 
	sendDksMsg(sender, bp_msg); 
      }
  }
  
  void DKSNode::m_gcResources(){
    if(a_pPred.a_site) a_pPred.m_makeGCpreps();
    a_pred.m_makeGCpreps();
    a_routingTable->m_gc();
  }


  DKS_userClass* DKSNode::getCallBackService()
  {
    return a_callback;
  }

  void DKSNode::setCallBackService(DKS_userClass* cb){
    a_callback = cb;
  }
  
}
  

