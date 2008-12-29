/*
 *  Authors:
 *   Zacharias El Banna
 *   Erik Klintskog 
 *
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
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

#ifdef DSS_INTERFACE  
#pragma interface
#endif

#include "dss_enums.hh"
#include <stddef.h> // size_t

// Forwarders

class MsgContainer; 
class MsgnLayer; 
class CsSiteInterface;
class DssChannelCallback;
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
protected:
  virtual ~DssReadBuffer() {}

public:
  virtual int availableData() const = 0; 
  virtual bool canRead(size_t wanted) const = 0;

  // Read out data which should be deserialized. Assumes available
  // data has been checked before
  virtual void readFromBuffer(BYTE* ptr, size_t wanted) = 0;
  virtual void commitRead(size_t read) = 0; 

  virtual const BYTE   getByte() = 0; 
};

class  DssWriteBuffer{
protected:
  virtual ~DssWriteBuffer() {}

public:
  virtual int availableSpace() const = 0;
  virtual bool canWrite(size_t wanted) const = 0;

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
const int MSG_PRIO_HIGH   = 3; //Used only for routing messages (indirect connections)
const int MSG_PRIO_MEDIUM = 2;
const int MSG_PRIO_LOW    = 1; //Not used for now/anymore
const int MSG_PRIO_LAZY   = 0;



// interface of DSS communication channels; their implementation is
// provided by the user
class DssChannel {
public:
  virtual ~DssChannel() {}

  // set callback object (when data available)
  virtual bool setCallback(DssChannelCallback*) = 0;

  // register for reading/writing (unregister if argument is false)
  virtual void registerRead(bool) = 0;
  virtual void registerWrite(bool) = 0;

  // read/write when channel ready, returns how many bytes read/written
  virtual int read(void* buf, const unsigned int& len) = 0;
  virtual int write(void* buf, const unsigned int& len) = 0;

  // close the channel
  virtual void close() = 0;
};

// interface of channel readers/writers; provided by the DSS
class DssChannelCallback{
protected:
  virtual ~DssChannelCallback() {}
public:
  virtual void connectionLost()     = 0;
  virtual bool readDataAvailable()  = 0;
  virtual bool writeDataAvailable() = 0;
};



// interface of site representation; provided by the DSS
class DSite{
protected:
  virtual ~DSite() {}

public:
  //*************** General Methods *****************************
  virtual void m_marshalDSite(DssWriteBuffer*) = 0; 
  virtual int  m_getMarshaledSize() const = 0;
  virtual void m_makeGCpreps() = 0;
  virtual char*  m_stringrep() = 0; 
  virtual bool m_sendMsg(MsgContainer*) = 0; 
  virtual bool operator<(const DSite&) = 0; 
  virtual unsigned int m_getShortId() = 0; 
  
  //***************  APP methods  *******************************
  virtual FaultState m_getFaultState() const = 0; 
  
  //***************  CSC methods: ******************************'
  virtual void m_connectionEstablished(DssChannel* con) = 0; 
  virtual void m_virtualCircuitEstablished( int len, DSite *route[]) = 0; 

  // m_monitorRTT() asks the site to monitor message round-trip times.
  // Measured rtt are reported, and a timeout event is triggered
  // whenever no message is received within maxrtt.  Upon timeout, the
  // rtt monitoring is stopped.
  virtual void m_monitorRTT(int maxrtt) = 0;

  virtual void m_stateChange(FaultState newState) = 0; 
  virtual ConnectivityStatus  m_getChannelStatus() = 0;
  // instruct DSite that CsSite has changed M-R
  virtual void m_invalidateMarshaledRepresentation() = 0; 
  // return a newly allocated string of size 'len' containing the
  // identity of the site
  virtual unsigned char* m_getId(int &len) = 0;
  virtual CsSiteInterface* m_getCsSiteRep() = 0; 
};

// interface of sites provided by the user; it implements the specific
// way to marshal sites, how to provide communication channels, and
// how to diagnose failures.
class CsSiteInterface {
public:
  CsSiteInterface();
  virtual ~CsSiteInterface() {}
  virtual int  getCsSiteSize() = 0;     // size of marshaled CsSite
  virtual void marshalCsSite( DssWriteBuffer* const buf) = 0; 
  virtual void updateCsSite( DssReadBuffer* const buf) = 0; 
  virtual void disposeCsSite() = 0; 

  // working() is called when the connection is ready to work
  virtual void working() = 0; 
  virtual void reportRTT(int rtt) = 0;
  virtual void reportTimeout(int maxrtt) = 0;

  virtual void reportFaultState(FaultState) = 0;

  virtual DssChannel *establishConnection() = 0;
  virtual void receivedMsg(MsgContainer*) = 0;
};




class ExtDataContainerInterface{
protected:
  virtual ~ExtDataContainerInterface() {}
public:
  virtual BYTE getType() = 0; 
  virtual bool marshal(DssWriteBuffer *bb)=0;
  virtual bool unmarshal(DssReadBuffer *bb)=0;
  virtual void dispose()=0;
  virtual void resetMarshaling() = 0; 
};

class PstInContainerInterface;
class PstOutContainerInterface;

class TimerElementInterface{
};

typedef unsigned int (*TimerWakeUpProc)(void *);


class MsgContainer{
public:
  virtual ~MsgContainer() {}

  virtual void pushDSiteVal(DSite*) = 0;
  virtual void pushIntVal(int) = 0; 
  virtual void pushADC(ExtDataContainerInterface*) = 0; 
  virtual void pushPstOut(PstOutContainerInterface*) = 0;
  virtual void pushMsgC(MsgContainer*) = 0; 
  
  virtual DSite* popDSiteVal() = 0; 
  virtual int popIntVal() = 0; 
  virtual ExtDataContainerInterface* popADC() = 0; 
  virtual PstInContainerInterface* popPstIn() = 0;
  virtual MsgContainer* popMsgC() = 0; 
  
  virtual bool m_isEmpty() const = 0; 
  virtual MsgContainer *m_getNext() = 0; 
  
  virtual void m_convert2Send() = 0; 
  virtual void m_convert2Rec() = 0; 
  virtual void m_extMakeGCpreps() = 0;

  // create a copy of this message, with all fields moved to the copy
  virtual MsgContainer* reincarnate() = 0;
};



class AppMslClbkInterface
{
public:
  virtual ~AppMslClbkInterface() {}
  virtual void m_MessageReceived(MsgContainer* const msgC,DSite* const sender) = 0; 
  virtual void m_stateChange(DSite*, const FaultState&) = 0; 
  virtual void m_unsentMessages(DSite* s, MsgContainer* msgs) = 0; 
  virtual ExtDataContainerInterface* m_createExtDataContainer(BYTE) = 0; 
};

class ComServiceInterface{
public:
  ComServiceInterface();
  virtual ~ComServiceInterface() {}

  // The CsSite Object
  virtual CsSiteInterface* unmarshalCsSite(DSite* Ds, DssReadBuffer* const buf) = 0; 
  virtual CsSiteInterface *connectSelfReps(MsgnLayer* ,DSite*) = 0; 
  
  // Mark all DSites used by the CSC. 
  virtual void m_gcSweep() = 0; 
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
  MsgnLayer(AppMslClbkInterface* , ComServiceInterface*, const bool&);  
  virtual ~MsgnLayer();
  void m_gcResources(); 
  
public: //**************** TIMERS *******************************
  DSS_LongTime m_getCurrTime();
  TimerElementInterface* m_setTimer( const unsigned int& time, TimerWakeUpProc t, void* const arg);
  void m_clearTimer(TimerElementInterface* tel);
  
public: //**************** CSC **********************************
  void  m_anonymousChannelEstablished(DssChannel* channel);
  void  m_heartBeat(const int&  TimePassedInMs); 

  MACRO_NO_DEFAULT_CONSTRUCTORS(MsgnLayer);
};


#endif

