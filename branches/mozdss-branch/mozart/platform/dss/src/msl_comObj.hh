/*
 *  Authors:
 *    Anna Neiderud (annan@sics.se)
 * 
 *  Contributors:
 * 
 *  Copyright:
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

#ifndef __MSL_COMOBJ_HH
#define __MSL_COMOBJ_HH

#ifdef INTERFACE
#pragma interface
#endif



/* SEC-TODO: 1) Verify initial messages: Maybe it is an idea to have the
 * trans buffer in a safe mode with limits on buffer size for C_
 * messages and safe message de/serialization
 *
 * 2) introduce a new state where all data is effectively pushed out
 * and then new session keys are generated using Diffie-Hellman.
 * 
 *
 *
 */

#include "mslBase.hh"
#include "msl_timers.hh"
#include "msl_dsite.hh"

namespace _msl_internal{ //Start namespace

  

  const int NO_MSG_NUM = -1;

  class TransObj;
  class EndRouter;
  class Site;
  class TimerElement;
  class MsgnLayerEnv; 
  class PrioQueues;
  class DssSimpleWriteBuffer;
  class DssSimpleReadBuffer;
  

  enum CState {
    CLOSED                  = 0x001,  // CLOSED means no transObj (=no connection)
    CLOSED_WF_REMOTE        = 0x002,  //  waiting for incoming connection
    ANONYMOUS_WF_PRESENT    = 0x004,  // State one at recieving side
    ANONYMOUS_WF_NEGOTIATE  = 0x008,  // State two at recieving side
    OPENING_WF_HANDOVER     = 0x010,  // State one at active side
    OPENING_WF_PRESENT      = 0x020,  // State two at active side
    OPENING_WF_NEGOTIATE    = 0x040,  // State three at active side
    WORKING                 = 0x080,
    CLOSING_HARD            = 0x100,  // Forcing a close
    CLOSING_WEAK            = 0x200,  // Initating a negotiated close
    CLOSING_WF_DISCONNECT   = 0x400   // Accepted a close, waiting for channel lost
  };

  typedef struct CSecuritySettings {
    u32  a_ticket;  // current noun/capability to continue in the protocol
    BYTE a_key[32]; // store the key to use for init of crypto alg
    u32  a_iv1;     // part one of init vector for chain block cipher
    u32  a_iv2;     // part two
  } CSecuritySettings;


  // TODO: Introduce a state in ComObj where the transObj is emptied
  // and only session messages can be sent


  class ComObj{
    friend class ComController;
  private:
    MsgnLayerEnv * a_mslEnv;
    TransObj *a_transObj;
    Site    *a_site;
    CSecuritySettings a_sec;

    // Storage for MsgContainers
    PrioQueues* a_queues;

    // Numbers for messages and acking-scheme
    int a_lastSent;
    int a_lastReceived;

    int a_sentLrgMsg;
    int a_receivedLrgMsg;

    // One timer to be used for opening/closing/acking (one at a time)
    TimerElement *a_reopentimer;

    // Special timers for probing and acking
    int  a_minrtt;
    int  a_maxrtt;
    bool a_ackCanceled;
    // If this time is greater than the timer then it shouldn't fire
    // CAUTION: no kill probe is defined so one have to update the
    // probe expiration whenever the timer is set/updated.
    DSS_LongTime  a_ackExpiration;

    TimerElement *a_ackTimer;
    TimerElement *a_probeIntervalTimer;
    Timers* const e_timers;  //Fast access to the timers

    int  a_msgAckTimeOut;
    int  a_msgAckLength;

    // Statistics
    int a_lastrtt;

    CState a_state;    
    bool a_closeHardFlag;
   // For probing
    bool a_probing;
    bool a_msgSentDuringProbeInterval;
    bool a_msgReceivedDuringProbeInterval; 
    bool a_localRef;
    bool a_remoteRef;
    bool a_sentClearRef;

    EndRouter* a_pred; // the transObj preceding this comObj.!!! change it
    
    inline void m_setCState(const CState& s){
      a_state = s;
      if (s == WORKING && a_site->m_getCsSite())
	// notify the upper layer that the connection is now working,
	// hence application monitors can be applied.
	a_site->m_getCsSite()->monitor();
    }
    inline CState m_getCState() const { return a_state; }

    // throw a State vector to check if in one of a set of states -> (WORKING | CLOSED | ... )
    inline bool m_inState(const unsigned int& states) const { return((a_state & states) != 0); }

    // ******* Special timer opts ********
    // remember that if we explicitly clear the timer it won't fire so
    // we don't have to worry about that in our 'fire' function
    inline void setAckTimer();
    inline void clearAckTimer();
    inline void setProbeIntervalTimer();
    inline void setProbeFaultTimer();


    // private methods
    void m_open();
    void errorRec(int);
    inline bool hasNeed();
    inline void createCI(DssSimpleWriteBuffer*, int);
    inline bool extractCI(DssSimpleReadBuffer*,int&);
    inline bool adoptCI(DssSimpleReadBuffer*);


    bool m_merge(ComObj *old);

    // ANONYMOUS
    void m_CLOSED_2_ANONYMOUS_WF_PRESENT(TransObj *transObj);
    bool m_ANONYMOUS_WF_PRESENT_2_ANONYMOUS_WF_NEGOTIATE(MsgCnt *msg);
    bool m_ANONYMOUS_WF_NEGOTIATE_2_WORKING(MsgCnt *msg);
    

    // INITIATING
    void m_CLOSED_2_OPENING_WF_HANDOVER();
    void m_OPENING_WF_HANDOVER_2_OPENING_WF_PRESENT(DssChannel* tc);
    bool m_OPENING_WF_PRESENT_2_OPENING_WF_NEGOTIATE(MsgCnt* msg);
    bool m_OPENING_WF_NEGOTIATE_2_WORKING(MsgCnt* msg); 

    // DIFFERENT CLOSE
    void m_WORKING_2_CLOSING_HARD();
    void m_WORKING_2_CLOSING_WF_DISCONNECT();
    void m_CLOSING_WF_DISCONNECT_2_CLOSING_WF_REMOTE();
    
    void m_WORKING_2_CLOSING_WEAK();
    void m_CLOSING_WEAK_2_CLOSED();

    
    ComObj(const ComObj&);
    ComObj& operator=(const ComObj&){ return *this; }

  protected:
    void clearTimers();
    void m_close(); // internal close

  public:
#ifdef DEBUG_CHECK
    static int a_allocated;
#endif

    void m_closeDownConnection();
    void m_closeErroneousConnection();

    ComObj(Site *site, MsgnLayerEnv* env);
    ~ComObj();

    MsgCnt* m_clearQueues(); 
    
    Site *getSite()  const {return a_site;}
    CState getState() const {return a_state;}
    void m_setLocalRef();

    void setTransObj(TransObj *transObj) { a_transObj = transObj; }
    TransObj *getTransObj() const { return a_transObj; }

    void m_send(MsgCnt *, int prio);
    void installProbe(int lowerBound, int higherBound);  
    // Should this be moved to the comController?
    bool canBeFreed(); // A question that implicitly tells the comObj
    // that no local references exist.
    int getQueueStatus();
    void m_makeGCpreps();
    bool hasQueued();

    // For TransObj:
    MsgCnt *getNextMsgCnt(int &); // Provides the TransObj 
    // with the MsgCnt of the next message to be 
    // sent. The current acknowledgement number is given by 
    // the int &.
    void msgSent(MsgCnt *);
    // Store away a message to be continued later.
    void msgAcked(int num);

    void msgPartlySent(MsgCnt *);
    void msgPartlyReceived(MsgCnt *);

    void sendAckExplicit();

    // Gives a new clean MsgCnt to be filled with an incomming message. 
    MsgCnt *getMsgCnt();

    // Gives the priviously stored MsgCnt for message num to be continued.
    MsgCnt *getMsgCnt(int num);

 
    bool msgReceived(MsgCnt *); // A full message was received and is 

    // Simply return true if the connection is open.
    bool isConnected();

    // Return the transport medium of the associated transObj. Return -1 if
    // there is no transObj.
    int getTransportMedium();

   // now handed up. Return: continue?
    void connectionLost();

    // For connection procedure
    void handover(DssChannel*);
    // The same as handover() but it uses EndRouter instead of TCPTransObj
    void handoverRoute(DSite *vec[], int); 
    void m_acceptAnonConnection(TransObj *);

    // Statistics
    inline int getLastRTT() { return a_lastrtt; }

    // Extras for internal use (must be public anyway)
    inline unsigned int sendProbePing();
    inline unsigned int probeFault();
    unsigned int sendAckTimer();

    void reopen();
  };


} //End namespace
#endif // __COMOBJ_HH
