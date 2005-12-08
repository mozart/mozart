#ifndef __ENDROUTE_HH
#define __ENDROUTE_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "msl_transObj.hh"
#include "msl_buffer.hh"
#include "msl_dct.hh"


// #include "dssBase.hh"



enum unmarshalReturn {
  U_MORE,
  U_WAIT,
  U_CLOSED
};

namespace _msl_internal{ //Start namespace 

  class RouteTransController;
  class DataAreaContainer;
  class EndRouterDeliver;
  
  class EndRouter: public TransObj {
    friend class RouteTransController;
  private:
    static const int E_MIN_FOR_HEADER = 100;// Minimal size available to even consider marshaling
    static const int CF_FIRST=0;
    static const int CF_CONT =1;
    static const int CF_FINAL=2;
  protected:
    DssReadByteBuffer*  a_readBuffer;
    DssWriteByteBuffer* a_writeBuffer;
  private:
    // DssTransportChannel *a_channel; // it is not actually used in this class
    int a_minSend;
    ComObj *a_succ;  // the successor ComObj
    int a_routeId;   // the route Id
    
    EndRouterDeliver* deliverEvent; 
    
    void marshal(MsgCnt *msgC, int acknum);
    unmarshalReturn unmarshal();

    EndRouter(const EndRouter&):TransObj(0,NULL),a_readBuffer(NULL),a_writeBuffer(NULL), a_minSend(0), a_succ(NULL), a_routeId(0), deliverEvent(NULL){}
    EndRouter& operator=(const EndRouter&){ return *this; }
    
  public:

    
    EndRouter(MsgnLayerEnv*); 
    virtual ~EndRouter();

    VirtualChannelInterface *m_closeConnection();
    void deliver();
    void readyToReceive();
    
    //    void setChannel(DssTransportChannel*);  // actually, this method is not used

    // Set the next comObj to communicate through
    void setSuccessor(ComObj *succ) { a_succ = succ; }

    void setRouteId(int routeId) { a_routeId = routeId; }
    
    // Init the route set up procedure, by sending C_SET_ROUTE
    void initRouteSetUp(DSite *succ[], int nrSites);

    void writeHandler();
    
    // Read handler for transparent DAC messages.
    void readHandler(DssSimpleDacDct *dac);
    
    // Called by the succ ComObj when receiving C_TARGET_TOUCHED.
    // It specifies that the route routeId was set up.
    // !!!to be continued ...
    void routeSetUp(int routeId);

    TransMedium getTransportMedium() {return TM_ROUTE;}
    virtual void m_EncryptReadTransport(BYTE* const key, const u32& keylen,
					const u32& iv1,  const u32& iv2);
    virtual void m_EncryptWriteTransport(BYTE* const key, const u32& keylen,
					 const u32& iv1,  const u32& iv2);

  };



} //End namespace
#endif


