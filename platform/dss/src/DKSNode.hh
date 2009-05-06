/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 2003
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

#ifndef __DKS_NODE_HH
#define __DKS_NODE_HH
#include "base.hh"
#include "dss_templates.hh"
#include "dss_comService.hh"


namespace _dss_internal{ 
  
  // Enums
  enum DKSNodeState{
    DNS_OUTSIDE, 
    DNS_INSIDE,
    DNS_GETTING_OUT,
    DNS_GETTING_IN
  };                    
  
  enum DKSRouteRes{  
    DRR_DO_LOCAL,    
    DRR_ROUTING,    
    DRR_OPENING,      
    DRR_CLOSING,      
    DRR_INVALID_KEY 
  };                
                       
                         


  // ****************** DEFINES *****************************

  // these two defines are used to identify the ExtDataConatiners used by the 
  // DKS. It is important that these does not colide with any types defined 
  // by any ExtDataContainer types defined by the DSS(or any other application 
  // that make use of the DKS and the MsgnLayer). 

#define ADCT_DKS_RT 255
#define ADCT_DKS_SV 254
  
  // A process representtaion for the DKS system
  // The class contains a DSite reference plus an id. 
  // The id is not directly derived from the unique identity 
  // of the DSite. To be able to cope with small DKS rings
  // where the chance of identity colisions, the identity 
  // is _derived_ from the DSite id. However, it is first
  // after the insert( acheck is made that the id slot is 
  // free) that the id of a DKS node is fixed. 
  
  class DksSite{
  public:
    int id; 
    DSite *a_site; 
    DksSite(int i, DSite *s):id(i), a_site(s){;}
    DksSite():id(0), a_site(NULL){;}
    ~DksSite();
    void m_makeGCpreps();
    DksSite* copy();

    DksSite(const DksSite& ds):id(ds.id),a_site(ds.a_site){}
    DksSite& operator=(const DksSite& ds){ id = ds.id; a_site = ds.a_site; return *this; }
  private:
    
  };
  
  class LevelEntry; 
  class RoutingTableDct;
  
  class DKS_RoutingTable{
    friend class RoutingTableDct;
  private:
    LevelEntry* a_rtt; 
    int a_L, a_I; 
  public:
    DKS_RoutingTable(int l, int i);
    ~DKS_RoutingTable();

    int m_pos(int, int) ;
    int begin(int l, int i);
    int end(int l, int i);
    DksSite respons(int l, int i);
    void set_respons(int l, int i, DksSite s);
    void replaceResp(DksSite &oldNode, DksSite &newNode);
    void set(int l, int i, int b, int e, DksSite r);
    void printTable();
    void printLevel(int); 
    
    void m_gc();

  private:
    DKS_RoutingTable(const DKS_RoutingTable&):a_rtt(NULL), a_L(0), a_I(0){};
    DKS_RoutingTable& operator=(const DKS_RoutingTable&){ Assert(0); return *this; }
  };
  
  
  // ************************* FOR THE DSS_MSGNLAYERINTERFACE ******

  ExtDataContainerInterface* createDksRoutingTableContainer(MsgnLayer*); 
  ExtDataContainerInterface* createDksSiteVecContainer(MsgnLayer*); 


  // The transported messages, the messages sent over the DKS overlaynetwork by 
  // the application(the user of the DKS), are treated as opaque pointers in the form 
  // of DksMessage classes. The DKS does not know how to push or pop the messages 
  // from/to the msgcontainers. To minimize the definition of interface classes 
  // this is hiden behind interface methods by the DKS_userClass. It is thus 
  // the responsability of the DKS_userClass to implement proper tranportation of
  // afore mentioned messages. 
  class DksMessage{
  };
  

  // A temporary construction
  class DksBcMessage{
  };
  
  
  // The DKSNode needs to interact with the application (e.g. deliver routed messages, divide
  // the responsible domain or insert a new domain responsability). All callbacks used by the 
  // DKSNode is defined by the DKS_userClass. At startup, a DKSNode requires an DKS_useClass 
  // instance. 
  
  class DKS_userClass{
  public:
    virtual void m_receivedRoute(int Key, DksMessage*) = 0;
    virtual void m_receivedRouteNext(int Key, DksMessage*) = 0;
    virtual DksMessage* m_divideResp(int start, int stop, int n) = 0;  
    virtual void m_newResponsability(int begin, int end, int n, DksMessage*) = 0; 
    virtual void dks_functional() = 0; 
    virtual void pushDksMessage(MsgContainer*, DksMessage*) = 0; 
    virtual DksMessage *popDksMessage(MsgContainer*) = 0; 


    virtual void m_receivedBroadcast(DksBcMessage*) = 0; 
    virtual void pushDksBcMessage(MsgContainer*, DksBcMessage*) = 0; 
    virtual DksBcMessage *popDksBcMessage(MsgContainer*) = 0; 
    
  };
  
  

  // Params holder for the DKSNode. It is passed to the node instance at startup 
  // and defines its behavior. 
  
  
  // The allmighty DKSNode! 

  class DKSNode{
  public:  // User interfaces. Used to send messages over the overlay network
    
    // Sends a message to the process responsible for the key. 
    DKSRouteRes m_route(int key, DksMessage*); 
    
    // Sends a message to the next process on the 
    // way towards the node responsible for the key. 
    DKSRouteRes m_routeNext(int key, DksMessage*); 
    
    void m_transferResponsability(DksMessage*);
    
    int  m_getId();
    
    DKSRouteRes m_broadcastRing(DksBcMessage*);
    
  public:     // interfaces uesed by the DSS
    void msgReceived(::MsgContainer *msg, DSite *Sender); 
    
    // Initializes a new dks ring. 
    DKSNode(int N, int K, int F, int Id, 
	     DSite* mySite,
	     DKS_userClass* usr);
    
    // Creates an unconnected member instance to an 
    // allready existing dks ring
    DKSNode( int N, int K, int F, int Id, DSite* mySite);
    
    void m_gcResources();
    
    DKS_userClass* getCallBackService();

    void setCallBackService(DKS_userClass*);
    
    void nodeFailed(DSite*, FaultState, MsgContainer*);
    
    void m_joinNetwork(DSite *entry);
    
    virtual ~DKSNode();
  public: 
    // ************** Redefines *******************************
    virtual MsgContainer *m_createDKSMsg() = 0; 

  protected:
    // ************** Attributes ******************************
    DksSite a_pred;
    DksSite a_myId;
    DksSite a_pPred; 
    DKS_userClass *a_callback; 
    DKS_RoutingTable* a_routingTable;
    SimpleQueue<DksSite*> a_joinQueue;
    DksSite *a_predList;

    int a_K; 
    int a_N; 
    int a_F; 
  private: 
    int a_log_K_N;
    DKSNodeState a_status; 
    
  private: 
    // *************** MISC routines *********************
    DKS_RoutingTable* m_routingTableForFirstNode();
    DksSite m_approxSucc(int S);
    DKS_RoutingTable* m_computeRTFor(DksSite Nj);
    DKS_RoutingTable* m_singletonInserter(DksSite Nj);
    DKS_RoutingTable* m_nonsingletonInserter(DksSite Nj);
    
    void m_forwardOrSilence(DksSite, int,int,int,int,int);
    void m_forwardInsert(DksMessage *entry,
			 int L, 
			 int T,
			 DksSite sender);
    
    void m_findIntervalContaining(int L, DksSite Id, int &new_I, DksSite *&nxtNode);
    void m_adaptPredList(int pos, DksSite site);
    void m_correctPredList(DksSite); 
    void m_insertInBack(DksSite Nj);
    void m_processQ();
    void m_insertOrForward(int L,  int I,  DksSite Nj);
    void m_adaptTo(DksSite Nj);
    void m_forward(MsgContainer *msg, int L, int Nj, DksSite sender);
    
  private:
    // **************** Message handlers *****************
    void m_joinRequestH(DksSite Nj);
    void m_leaveDKSRing();    
    void m_badPointerH(DksSite, DksSite, ::MsgContainer *); 
    void m_joinH(DksSite U, DksSite Nj,int L, int I);
    void m_joinInitH(DksSite sender, DksSite P, DksMessage*);
    void m_changePredH(DksSite  U);
    void m_becomeNormalH(DksSite U, DKS_RoutingTable*, DksSite*);
    void m_retryJoinRequestH(DksSite  P);
    void m_changeSuccToH(DksSite NS);
    void m_ackBecomeNormalH();
    void m_insertH(DksSite U, int Key, DksMessage *,  int L, int I);
    void m_leaveH( DksSite W );
    void m_youCanLeaveH(DksSite U);
    void m_broadCastH(DksSite sender, DksBcMessage* load, int level, int interval, int limit);
    MACRO_NO_DEFAULT_CONSTRUCTORS(DKSNode);
  };
}
#endif
