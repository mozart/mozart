/*
 *  Authors:
 *    Zacharias El Banna (zeb@sics.se)
 *    Erik Klintskog (erik@sics.se)
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

#if defined(INTERFACE)
#pragma implementation "msl_msgContainer.hh"
#endif


#include "msl_msgContainer.hh"
#include "msl_transObj.hh"
#include "msl_dsite.hh"
#include "msl_buffer.hh"
#include "msl_dct.hh"
#include "msl_serialize.hh"

// borrowed from dss_msgLayerInterface.hh
namespace _dss_internal {
  void gf_pushPstOut(MsgContainer*, PstOutContainerInterface*); 
  PstInContainerInterface* gf_popPstIn(MsgContainer*);
}

namespace _msl_internal{ //Start namespace

  const int INITIAL_SIZE = 8;     // initial size of a_fields

  namespace{
    const int sz_MAX_DP_STRING = 4; // This is based on what??
  }

  enum DataTag{
    TYPE_INT,
    TYPE_SITE,
    TYPE_END,
    TYPE_DCT,
    TYPE_ADC,
    TYPE_CDC,
    TYPE_MSG
  };

#ifdef DEBUG_CHECK
  int MsgCnt::a_allocated=0;
#endif


  // Constructors/Destructor

  MsgCnt::MsgCnt() :
    a_flag(MSG_CLEAR), a_num(-1), a_internalMsg(false), a_sendTime(),
    a_max_fields(INITIAL_SIZE), a_nof_fields(0), a_current(0), a_next(NULL)
  {
    a_fields = new MsgField[a_max_fields];
    DebugCode(a_allocated++);
  }
  
  MsgCnt::MsgCnt(int Type, bool internal) :
    a_flag(MSG_CLEAR), a_num(-1), a_internalMsg(internal), a_sendTime(),
    a_max_fields(INITIAL_SIZE), a_nof_fields(0), a_current(0), a_next(NULL)
  {
    a_fields = new MsgField[a_max_fields];
    DebugCode(a_allocated++);
    pushIntVal(Type); 
  }
  
  MsgCnt::~MsgCnt() {
    DebugCode(a_allocated--);
    for (int i = 0; i < a_nof_fields; i++) { // while a valid FT
      if(a_fields[i].a_ft == FT_DCT && a_fields[i].a_arg != NULL)
	static_cast<DssCompoundTerm*>(a_fields[i].a_arg)->dispose(); 
      if(a_fields[i].a_ft >= FT_ADC && a_fields[i].a_arg != NULL)
	static_cast<ExtDataContainerInterface*>(a_fields[i].a_arg)->dispose(); 
    }
    delete [] a_fields;
  }


  // ******************************** ALL ************************************

  char *
  MsgCnt::m_stringrep() {
    static char buf[140];
    static int  pos;
    pos = sprintf(buf, "MSGCONTAINER: nof:%d cur:%d DATA:",
		  a_nof_fields, a_current);
    for (int i = 0; i < a_nof_fields; i++) {
      pos += sprintf((buf+pos),"%d|%x ", a_fields[i].a_ft,
		     reinterpret_cast<int>(a_fields[i].a_arg));
    }
    return buf;
  }

  void
  MsgCnt::m_makeGCpreps() {
    for (int i = 0; i < a_nof_fields; i++) { //For all unmarshaled stuff...
      if (a_fields[i].a_ft == FT_SITE) { // The only resource in need of gc 
	static_cast<Site*>(a_fields[i].a_arg)->m_makeGCpreps();
      }
    }
  }



  // ******************** RECEIVE MESSAGE CONTAINERS ***********************
  
  // Returns true if unmarshal is ok, false if something is wrong
  // (like too many fields, ...)
  bool
  MsgCnt::deserialize(DssReadByteBuffer *bb, Site* source, MsgnLayerEnv* env) {
    // a_nof_fields counter keeps track of how much is unmarshaled.
    // It is initialized at creation, and incremented for each item
    // totally (!) unmarshaled.
    
    while (true) {
      if (bb->availableData() == 0) {
	setFlag(MSG_HAS_UNMARSHALCONT);     // suspended outside a field
	return true;
      }
      
      BYTE msg_type = bb->m_getByte();
      switch (msg_type) {
      case TYPE_INT: {
	int i = bb->m_getInt();
	m_pushVal(reinterpret_cast<void *>(i), FT_NUMBER);
	dssLog(DLL_DEBUG, "MSGCONTAINER  (%p): deserilize INT %d", this, i);
	continue;
      }
      case TYPE_SITE: {
	Site *s = env->a_siteHT->m_unmarshalSite(bb);
	pushSiteVal(s);
	dssLog(DLL_DEBUG, "MSGCONTAINER  (%p): deserilize SITE %s", this,
	       s->m_stringrep());
	continue;
      }
	//
	// The EVM does not have to send instantiated PSTS.  Instead
	// it can choose to not pass information with an abstract
	// operation, or a result from an abstract operation.
      case TYPE_DCT: {
	dssLog(DLL_DEBUG,"MSGCONTAINER  (%p): deserilizing DATA AREA",this);
	DssCompoundTerm *dac;

	checkSize();
	if (checkFlag(MSG_HAS_UNMARSHALCONT) && a_suspf) {
	  Assert(a_fields[a_nof_fields].a_ft == FT_DCT);
	  // continue with current one
	  dac = static_cast<DssCompoundTerm*>(a_fields[a_nof_fields].a_arg);
	  // The marshaler always sets the DCT type. NOT USED
	  bb->m_getByte();
	} else {
	  DCT_Types dctType = static_cast<DCT_Types>(bb->m_getByte());
	  dac = createReceiveDCT(dctType, env);
	  a_fields[a_nof_fields].a_arg = dac;
	  a_fields[a_nof_fields].a_ft  = FT_DCT;
	}
	Assert(dac);
	env->a_srcSite = source; 
	bool m_res = dac->unmarshal(bb, env);
	env->a_srcSite = NULL; 
	if (m_res) {
	  a_nof_fields++; // commit
	  continue;
	} else {
	  setFlag(MSG_HAS_UNMARSHALCONT, true);   // suspended inside a field
	  return true; 
	}
      }
      case TYPE_MSG: {
	checkSize();
	if (!a_suspf) {
	  a_fields[a_nof_fields].a_arg = new MsgCnt(); 
	  a_fields[a_nof_fields].a_ft  = FT_MSGC;  
	}
	MsgCnt *msg = static_cast<MsgCnt*>(a_fields[a_nof_fields].a_arg);
	Assert(a_fields[a_nof_fields].a_ft == FT_MSGC && msg);
	
	if (msg->deserialize(bb, source, env) && msg->checkFlag(MSG_CLEAR)) {
	  Assert(!msg->m_isEmpty());
	  // raph: I don't understand the purpose of the following statement
	  msg->popIntVal(); // removing the stop marker
	  a_nof_fields++; // commit
	  continue; 
	} else {
	  setFlag(MSG_HAS_UNMARSHALCONT, true);   // suspended inside a field
	  return true; 
	}
      }
      case TYPE_CDC:
      case TYPE_ADC: {
	dssLog(DLL_DEBUG,"MSGCONTAINER  (%p): deserilizing DATA AREA",this);
	ExtDataContainerInterface *dac;
	
	checkSize();
	if (checkFlag(MSG_HAS_UNMARSHALCONT) && a_suspf){
	  Assert(a_fields[a_nof_fields].a_ft == FT_ADC ||
		 a_fields[a_nof_fields].a_ft == FT_SDC);
	  // continue with current one
	  dac = static_cast<ExtDataContainerInterface*>(a_fields[a_nof_fields].a_arg);
	  // The marshaler always sets the DCT type. NOT USED
	  bb->m_getByte(); 
	} else {
	  BYTE type = bb->m_getByte();
	  if (msg_type == TYPE_ADC) {
	    dac = env->a_clbck->m_createExtDataContainer(type);
	    a_fields[a_nof_fields].a_ft = FT_ADC; 
	  } else {
	    dac = env->a_comService->m_createExtDataContainer(type);
	    a_fields[a_nof_fields].a_ft = FT_SDC;
	  }
	  a_fields[a_nof_fields].a_arg = dac;
	}
	Assert(dac);
	env->a_srcSite = source; 
	bool m_res = dac->unmarshal(bb);
	env->a_srcSite = NULL; 
	if (m_res) {
	  a_nof_fields++; // commit
	  continue;
	} else {
	  setFlag(MSG_HAS_UNMARSHALCONT, true);   // suspended inside a field
	  return true; 
	}
      }
      case TYPE_END:
	dssLog(DLL_DEBUG,"MSGCONTAINER  (%p): deserilizing done",this);
	setFlag(MSG_CLEAR);
	return true;
	
      default:
	dssError("unexpected tag in MsgCnt::unmarshal()");
      }
      // We bail out of the case statement iff we have a suspension,
      // otherwise do 'continue' directly:
      Assert(0);
    }
    // all elements are finished;
    // We should have returned out of the while loop.
    Assert(0);
    return false;
  }



  // ******************** SEND MESSAGE CONTAINERS ***********************
  
  void
  MsgCnt::m_serialize(DssWriteByteBuffer *bb, Site* destination,
		      MsgnLayerEnv* env) {
    // a_current keeps track of how much is marshaled.  It is
    // initialized at creation and incremented for each item totally
    // marshaled.  When a_current is equal to a_nof_fields, TYPE_END
    // is marshaled, and the loop will never be able to start again.
    
    while (a_current < a_nof_fields) {
      switch (a_fields[a_current].a_ft) {
      case FT_NUMBER:
	if (bb->canWrite(1+sz_MNumberMax)) {
	  int i = reinterpret_cast<int>(m_popVal());
	  dssLog(DLL_DEBUG,"MSGCONTAINER  (%p): serilize INT %d", this, i);
	  bb->m_putByte(TYPE_INT);
	  bb->m_putInt(i);
	  continue; // next field;
	} else goto HAS_CONTINUE;
	
      case FT_SITE: {
	Site* s = static_cast<Site*>(m_nextVal());
	if (bb->canWrite(1+s->m_getMarshaledSize())) {
	  (void) m_popVal();
	  dssLog(DLL_DEBUG,"MSGCONTAINER  (%p): serilize SITE %s", this,
		 s->m_stringrep());
	  bb->m_putByte(TYPE_SITE);
	  s->m_marshalDSite(bb);
	  continue; // next field;
	} else goto HAS_CONTINUE;
      }
	
      case FT_DCT:
	if (bb->canWrite(1+20)) { 
	  dssLog(DLL_DEBUG,"MSGCONTAINER  (%p): serilize DATA_AREA", this);
	  DssCompoundTerm *dac =
	    static_cast<DssCompoundTerm*>(a_fields[a_current].a_arg);
	  bb->m_putByte(TYPE_DCT);
	  bb->m_putByte(dac->getType()); // we always put the type
	  env->a_destSite = destination;
	  bool m_res = dac->marshal(bb, env); 
	  env->a_destSite = NULL;
	  if (m_res) { a_current++; continue; } // next field;
	}
	goto HAS_CONTINUE;
	
      case FT_ADC:
	if (bb->canWrite(1+20)) { 
	  dssLog(DLL_DEBUG,"MSGCONTAINER  (%p): serilize DATA_AREA", this);
	  ExtDataContainerInterface *dac =
	    static_cast<ExtDataContainerInterface*>(a_fields[a_current].a_arg);
	  bb->m_putByte(TYPE_ADC);
	  bb->m_putByte(dac->getType()); // we always put the type
	  env->a_destSite = destination;
	  bool m_res = dac->marshal(bb); 
	  env->a_destSite = NULL;
	  if (m_res) { a_current++; continue; } // next field;
	}
	goto HAS_CONTINUE;
	
      case FT_MSGC:
	if (bb->canWrite(1+100)){
	  MsgCnt *msg = static_cast<MsgCnt*>(a_fields[a_current].a_arg);
	  bb->m_putByte(TYPE_MSG); 
	  msg->m_serialize(bb, destination, env); 
	  if (!msg->checkFlag(MSG_HAS_MARSHALCONT)) {
	    a_current++; continue; // next field;
	  }
	}
	goto HAS_CONTINUE;
	
      default:
	dssError("Not supported ft %d", a_fields[a_current].a_ft); 
      }
      // We bail out of the case statement if we have a suspension,
      // otherwise do 'continue' directly:
      Assert(0);
    }

    Assert(a_current == a_nof_fields); // Must be last
    // reached last, send stop marker
    if (bb->canWrite(1)) {
      bb->m_putByte(TYPE_END);
      setFlag(MSG_CLEAR);
      return;
    }

  HAS_CONTINUE:
    // set continue flag and exit
    setFlag(MSG_HAS_MARSHALCONT);
  }

  void
  MsgCnt::resetMarshaling() {
    resetCounter();
    
    // If the message has a continuation, scan all the fields and use
    // the reset pst interface.
    if (checkFlag(MSG_HAS_MARSHALCONT)) {
      setFlag(MSG_CLEAR);
      for (int i = 0; i < a_nof_fields; i++) {
	if (a_fields[i].a_ft == FT_DCT)
	  static_cast<DssCompoundTerm*>(a_fields[i].a_arg)->resetMarshaling();
	if (a_fields[i].a_ft >= FT_ADC)
	  static_cast<ExtDataContainerInterface*>(a_fields[i].a_arg)->resetMarshaling();
      }
    }
  }

  void
  MsgCnt::resetCounter() {
    Assert(checkCounters());
    a_current = 0;
  }



  //******************** "Pop" methods

  int 
  MsgCnt::popIntVal(){
    Assert(checkCounters() && m_getFT() == FT_NUMBER);
    return reinterpret_cast<int>(m_popVal());
  }

  ExtDataContainerInterface* 
  MsgCnt::popADC(){
    Assert(checkCounters() && m_getFT() == FT_ADC);
    return static_cast<ExtDataContainerInterface*>(m_popVal());
  }

  ExtDataContainerInterface* 
  MsgCnt::popSDC(){
    Assert(checkCounters() && m_getFT() == FT_SDC);
    return static_cast<ExtDataContainerInterface*>(m_popVal());
  }

  PstInContainerInterface*
  MsgCnt::popPstIn() {
    return _dss_internal::gf_popPstIn(this);
  }

  MsgContainer* 
  MsgCnt::popMsgC(){
    Assert(checkCounters() && m_getFT() == FT_MSGC);
    return static_cast<MsgCnt*>(static_cast<MsgContainer*>(m_popVal()));
  }

  DSite* 
  MsgCnt::popDSiteVal() {
    Site* ans = popSiteVal();
    return dynamic_cast<DSite*>(ans); 
  }

  
  
  // ************************ "Push"  methods
  
  void MsgCnt::pushIntVal(int v) {
    m_pushVal(reinterpret_cast<void*>(v), FT_NUMBER);
  }

  void MsgCnt::pushADC(ExtDataContainerInterface* v) {
    m_pushVal(static_cast<void*>(v), FT_ADC);
  }

  void MsgCnt::pushSDC(ExtDataContainerInterface* v) {
    m_pushVal(static_cast<void*>(v), FT_SDC);
  }

  void MsgCnt::pushPstOut(PstOutContainerInterface* v) {
    _dss_internal::gf_pushPstOut(this, v);
  }

  void MsgCnt::pushMsgC(MsgContainer* v) {
    MsgCnt* msg = static_cast<MsgCnt*>(v);
    msg->resetCounter();
    m_pushVal(static_cast<void*>(msg), FT_MSGC);
  }

  void MsgCnt:: pushDSiteVal(DSite* s) {
    pushSiteVal(static_cast<_msl_internal::Site*>(s));
  }



  // ************************ external misc

  bool MsgCnt::m_isEmpty() const { 
    return (a_current == a_nof_fields); 
  }

  MsgContainer* MsgCnt::m_getNext() {
    return static_cast<MsgContainer*>(a_next); 
  }

  void MsgCnt::m_convert2Send() {
    resetCounter();
  }

  void MsgCnt::m_convert2Rec() {
    resetCounter();
  }

  void MsgCnt::m_extMakeGCpreps() {
    Assert(0); 
  }

  int MsgCnt::peekMslMessageType() {
    Assert(a_fields[0].a_ft == FT_NUMBER);
    return reinterpret_cast<int>(a_fields[0].a_arg);
  }

  //******************* DUPLICATED ******************************'
} //End namespace
