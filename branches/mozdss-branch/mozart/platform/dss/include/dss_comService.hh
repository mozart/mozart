/*
 *  Authors:
 *   Zacharias El Banna
 *   Erik Klintskog 
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __DSS_COMSERVICE_HH
#define __DSS_COMSERVICE_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "dss_enums.hh"
#include <stddef.h> // size_t

// Forwarders

class MsgContainer; 
class MsgnLayer; 
class CsSiteInterface;
// Using only one unsigned long for time (in ms) gives a maximum lifetime
// of 49 days for an emulator. Therefore, use two unsigned longs in an ADT.


class DSS_LongTime {
private:
  
  unsigned long low;
  unsigned long high;
  
public:
  DSS_LongTime();
  void increaseTime(const unsigned int& interval);
  
  bool operator<=(const DSS_LongTime &t2) ;
  bool operator>(const DSS_LongTime &t2) ;
  bool operator!=(const DSS_LongTime &t2) ;
  
  // This is assumed to be used only to compare times that are rather close
  // to each other and thus fit in an int.
  int operator-(const DSS_LongTime &t2);
  char *stringrep();
};

// ******************************  SECTION 6 - Buffer Routines ***************************
// Exported by the DSS


class  DssReadBuffer{
public:
  virtual int availableData() const = 0; 

  // Read out data which should be deserialized. Assumes available
  // data has been checked before
  virtual void readFromBuffer(BYTE* ptr, size_t wanted) = 0;
  virtual void commitRead(size_t read) = 0; 

  virtual const BYTE   getByte() = 0; 
};

class  DssWriteBuffer{
public:
  virtual int availableSpace() const = 0; 

  // ZACHARIAS: Check available space before writing, I know it did
  // that before but it is somewhat unecessary to do it twice
  virtual void writeToBuffer(const BYTE* ptr, size_t write) = 0; 

  virtual void         putByte(const BYTE&) = 0; 
};


// ************** BUFFER METHODS **************

const int SIZE_INT = 4;

inline void putInt(DssWriteBuffer* const buf, int i){
  for (int k=0; k<SIZE_INT; k++) { 
    buf->putByte((i & 0xFF)); 
    i = i>>8;
  }
}

inline int getInt(DssReadBuffer* const buf){
  int i = 0;
  for (int k=0; k < SIZE_INT; ++k) {
    i = i + ((buf->getByte())<<(k*8));
  }
  return static_cast<int>(i);
}



const int MSG_PRIO_EAGER  = 4;
const int MSG_PRIO_LAZY   = 0;
const int MSG_PRIO_HIGH   = 3;
const int MSG_PRIO_MEDIUM = 2;
const int MSG_PRIO_LOW    = 1;


enum DSiteState{
  DSite_OK         = 0xAA, 
  DSite_TMP        = 0xBB, 
  DSite_GLOBAL_PRM = 0xCC,
  DSite_LOCAL_PRM  = 0xDD
};



class VirtualChannelInterface{
};



class DSite{
public:
  //*************** General Methods *****************************
  virtual void m_marshalDSite(DssWriteBuffer*) = 0; 
  virtual void m_makeGCpreps() = 0;
  virtual char*  m_stringrep() = 0; 
  virtual bool m_sendMsg(MsgContainer*) = 0; 
  virtual bool operator<(const DSite&) = 0; 
  virtual unsigned int m_getShortId() = 0; 
  
  //***************  APP methods  *******************************
  virtual DSiteState  m_getFaultState() const = 0; 
  
  //***************  CSC methods: ******************************'
  virtual void m_connectionEstablished(VirtualChannelInterface* con) = 0; 
  virtual void m_virtualCircuitEstablished( int len, DSite *route[]) = 0; 
  virtual int  m_installRTmonitor( int lowLimit, int highLimit) = 0; 
  virtual void m_stateChange(DSiteState newState) = 0; 
  virtual void m_takeDownConnection() = 0; 
  virtual ConnectivityStatus  m_getChannelStatus() = 0;
  // instruct DSite that CsSite has changed M-R
  virtual void m_invalidateMarshaledRepresentation() = 0; 
  // return a newly allocated string of size 'len' containing the
  // identity of the site
  virtual unsigned char* m_getId(int &len) = 0;
  virtual CsSiteInterface* m_getCsSiteRep() = 0; 
  
};




class ExtDataContainerInterface{
public:
  virtual BYTE getType() = 0; 
  virtual bool marshal(DssWriteBuffer *bb)=0;
  virtual bool unmarshal(DssReadBuffer *bb)=0;
  virtual void dispose()=0;
  virtual void resetMarshaling() = 0; 
};


class TimerElementInterface{
};

typedef unsigned int (*TimerWakeUpProc)(void *);


class MsgContainer{
public:
  virtual void pushDSiteVal(DSite*) = 0;
  virtual void pushIntVal(int) = 0; 
  virtual void pushADC(ExtDataContainerInterface*) = 0; 
  virtual void pushSDC(ExtDataContainerInterface*) = 0; 
  virtual void pushMsgC(MsgContainer*) = 0; 
  
  virtual DSite* popDSiteVal() = 0; 
  virtual int popIntVal() = 0; 
  virtual ExtDataContainerInterface* popADC() = 0; 
  virtual ExtDataContainerInterface* popSDC() = 0; 
  virtual MsgContainer* popMsgC() = 0; 
  
  virtual bool m_isEmpty() const = 0; 
  virtual MsgContainer *m_getNext() = 0; 
  
  virtual void m_convert2Send() = 0; 
  virtual void m_convert2Rec() = 0; 
  virtual void m_extMakeGCpreps() = 0;
};



class CsSiteInterface{
public:
  CsSiteInterface();
  virtual void    marshalCsSite( DssWriteBuffer* const buf) = 0; 
  virtual void    updateCsSite( DssReadBuffer* const buf) = 0; 
  virtual void    disposeCsSite() = 0; 
  // monitor() is called when the connection can be monitored
  virtual void    monitor() = 0; 
  virtual void    reportRtViolation(int measuredRT, int installedLow, int installedHigh) = 0; 
  virtual VirtualChannelInterface *establishConnection() = 0;
  virtual void closeConnection( VirtualChannelInterface* con) = 0;
};

class ChannelRequest{
};

class VcCbkClassInterface{
public:
  virtual void connectionLost()     = 0;
  virtual bool readDataAvailable()  = 0;
  virtual bool writeDataAvailable() = 0;
};

class AppMslClbkInterface
{
public:
  virtual void m_MessageReceived(MsgContainer* const msgC,DSite* const sender) = 0; 
  virtual void m_stateChange(DSite*, const DSiteState&) = 0; 
  virtual void m_unsentMessages(DSite* s, MsgContainer* msgs) = 0; 
  virtual ExtDataContainerInterface* m_createExtDataContainer(BYTE) = 0; 
};

class ComServiceInterface{
public:
  ComServiceInterface();
  
  // Connections 
  virtual void closeAnonConnection(VirtualChannelInterface* con) = 0;

  // The CsSite Object
  virtual CsSiteInterface* unmarshalCsSite(DSite* Ds, DssReadBuffer* const buf) = 0; 
  virtual CsSiteInterface *connectSelfReps(MsgnLayer* ,DSite*) = 0; 
  virtual ExtDataContainerInterface* m_createExtDataContainer(BYTE) = 0; 
  
  // Explicit site handeling
  virtual void m_MsgReceived(CsSiteInterface*, MsgContainer*) = 0; 
  
  // Mark all DSites used by the CSC. 
  virtual void m_gcSweep() = 0; 
  
  // Channel establishemnt 
  virtual void channelEstablished(ChannelRequest *CR, VirtualChannelInterface *vc) = 0;
  virtual void connectionFailed(ChannelRequest *CR, ConnectionFailReason reason) = 0;
};


class IoFactoryInterface{
public:
  IoFactoryInterface();
  // Connection establishment ComService only
  virtual VirtualChannelInterface* establishTCPchannel(int IP, int port, ChannelRequest *CR) = 0; 
  virtual void terminateTCPchannel(VirtualChannelInterface *vc) = 0; 
  
  // Communicatoin, both comservice and dss
  virtual int  readData(VirtualChannelInterface *, void *buf, const unsigned int& len)  = 0; 
  virtual int  writeData(VirtualChannelInterface *, void *buf, const unsigned int& len) = 0;
  virtual void registerRead(VirtualChannelInterface*, bool on) = 0; 
  virtual void registerWrite(VirtualChannelInterface*, bool on) = 0; 
  virtual bool setCallBackObj(VirtualChannelInterface*, VcCbkClassInterface *) = 0; 
  virtual int  setupTCPconnectPoint(int) = 0;
};




namespace _msl_internal{
  class MsgnLayerEnv;
}



class MsgnLayer{
private: 
  _msl_internal::MsgnLayerEnv *a_mslEnv;
public: //******************* DSites, marshaling and identities
  DSite*         a_myDSite; 
  DSite*         m_getDestDSite();
  DSite*         m_getSourceDSite(); 
  DSite*         m_UnmarshalDSite(DssReadBuffer* buf);

public: // ****************** MsgContainers
  MsgContainer *createAppSendMsgContainer();
  MsgContainer *createCscSendMsgContainer();
  
public: // ******************* Aux
  MsgnLayer(AppMslClbkInterface* , ComServiceInterface*, IoFactoryInterface*, const bool&);  
  virtual ~MsgnLayer();
  void m_gcResources(); 
  
public: //**************** TIMERS *******************************
  DSS_LongTime m_getCurrTime();
  TimerElementInterface* m_setTimer( const unsigned int& time, TimerWakeUpProc t, void* const arg);
  void m_clearTimer(TimerElementInterface* tel);
  
public: //**************** CSC **********************************
  void  m_anonymousChannelEstablished(VirtualChannelInterface* channel);
  void  m_heartBeat(const int&  TimePassedInMs); 

  MACRO_NO_DEFAULT_CONSTRUCTORS(MsgnLayer);
};


#endif

