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

namespace _msl_internal{ //Start namespace

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



  MsgCnt::MsgCnt():a_rw_counter(0), 
		   a_um_counter(0), 
		   a_flag(MSG_CLEAR),  
		   a_num(-1),
		   a_internalMsg(false),
		   a_sendTime(),
		   a_next(NULL)
  {
    DebugCode(a_allocated++);
  }
  
  MsgCnt::MsgCnt(int Type, bool internal):a_rw_counter(0), 
					  a_um_counter(0), 
					  a_flag(MSG_CLEAR),  
					  a_num(-1),
					  a_internalMsg(internal),
					  a_sendTime(),
					  a_next(NULL)
  {
    DebugCode(a_allocated++);
    pushIntVal(Type); 
  }

  // ******************************** ALL ************************************

  char *
  MsgCnt::m_stringrep(){
    static char buf[140];
    static int  pos;
    pos = sprintf(buf,"MSGCONTAINER: um:%d rw:%d DATA:",a_um_counter,a_rw_counter);
    int i = 0;
    while(i < t_max(a_um_counter,a_rw_counter)){
      pos = pos + sprintf((buf+pos),"%d|%x ",a_fields[i].a_ft,reinterpret_cast<int>(a_fields[i].a_arg));
      i++;
    }
    return buf;
  }

  void
  MsgCnt::m_makeGCpreps() {
    for(int i=0; i<t_max(a_um_counter,a_rw_counter); i++){ //For all unmarshaled stuff...
      if (a_fields[i].a_ft == FT_SITE){ // The only resource in need of gc 
	static_cast<Site*>(a_fields[i].a_arg)->m_makeGCpreps();
      }
    }
  }



  // ************************* RECEIVE MESSAGE CONTAINERS ***********************

  
  // Returns true if unmarshal is ok, false if something is wrong (like to many fields, ....)
  bool
  MsgCnt::deserialize(DssReadByteBuffer *bb, Site* source, MsgnLayerEnv* env){
    // u(n)/m-(arshal) counter keeps track of how much is unmarshaled
    // it is initialized at creation and increamented for each "object"
    // totally (!) unmarshaled.
    

    
    for( ; a_um_counter < MAX_NOF_FIELDS; a_um_counter++){
      if(bb->m_availableData()==0){
	setFlag(MSG_HAS_UNMARSHALCONT);
	return true;
      }
      BYTE msg_type = bb->m_getByte();
      switch(msg_type){
      case TYPE_INT:
	a_fields[a_um_counter].a_arg = reinterpret_cast<void *>(bb->m_getInt());
	a_fields[a_um_counter].a_ft = FT_NUMBER;
	// Assert(reinterpret_cast<int>(a_fields[a_um_counter].a_arg) != 0x10000); // GET THE FAULT
	dssLog(DLL_DEBUG,"MSGCONTAINER  (%p): deserilize INT %d",this,reinterpret_cast<int>(a_fields[a_um_counter].a_arg));
	continue;
      case TYPE_SITE:
	a_fields[a_um_counter].a_arg = reinterpret_cast<void *>(env->a_siteHT->m_unmarshalSite(bb));
	a_fields[a_um_counter].a_ft = FT_SITE;
	dssLog(DLL_DEBUG,"MSGCONTAINER  (%p): deserilize SITE %s",this,static_cast<Site*>(a_fields[a_um_counter].a_arg)->m_stringrep());
	continue;
	//
	// The EVM does not have to send instantiated 
	// PSTS. Instead it can choose to not pass information
	// with an abstract operation, or a result from an abstract 
	// operation.
      case TYPE_DCT: 
	{
	  dssLog(DLL_DEBUG,"MSGCONTAINER  (%p): deserilizing DATA AREA",this);
	  DssCompoundTerm *dac;
	  
	  if (checkFlag(MSG_HAS_UNMARSHALCONT)){
	    dac =  reinterpret_cast<DssCompoundTerm*>(a_fields[a_um_counter].a_arg);
	    // The marshaler allways sets the DCT type. NOT USED
	    bb->m_getByte();
	   }
	   else {
	     DCT_Types dctType = static_cast<DCT_Types>(bb->m_getByte());
	     dac = createReceiveDCT(dctType, env); 
	     a_fields[a_um_counter].a_arg = dac;
	     a_fields[a_um_counter].a_ft  = FT_DCT; 
	   }
	   env->a_srcSite = source; 
	   bool m_res = dac->unmarshal(bb,env);
	   env->a_srcSite = NULL; 
	   if (m_res){
	     setFlag(MSG_CLEAR);
	     continue;
	   } else {
	     setFlag(MSG_HAS_UNMARSHALCONT);
	     return true; 
	   }
	 }
      case TYPE_MSG:
	{

	  if( checkFlag(MSG_CLEAR)){
	    a_fields[a_um_counter].a_arg = new MsgCnt(); 
	    a_fields[a_um_counter].a_ft  = FT_MSGC;  
	  }
	  MsgCnt *msg = static_cast<MsgCnt*>(a_fields[a_um_counter].a_arg); 
	  
	  if (msg->deserialize(bb, source, env)) {
	    setFlag(MSG_CLEAR);
	    msg->popIntVal(); // removing the msgtype...
	    continue; 
	  }
	  else {
	    setFlag(MSG_HAS_UNMARSHALCONT);
	    return true; 
	  }
	}
      case TYPE_CDC:
      case TYPE_ADC: 
	 {
	   dssLog(DLL_DEBUG,"MSGCONTAINER  (%p): deserilizing DATA AREA",this);
	   ExtDataContainerInterface *dac;

	   if (checkFlag(MSG_HAS_UNMARSHALCONT)){
	     dac =  reinterpret_cast<ExtDataContainerInterface*>(a_fields[a_um_counter].a_arg);
	     // The marshaler allways sets the DCT type. NOT USED
	     bb->m_getByte(); 
	   }

	   else {
	     BYTE type = bb->m_getByte();
	     if (msg_type == TYPE_ADC) 
	       {
		 dac = env->a_clbck->m_createExtDataContainer(type);
		 a_fields[a_um_counter].a_ft  = FT_ADC; 
	       }
	     else
	       {
		 dac = env->a_comService->m_createExtDataContainer(type);
		 a_fields[a_um_counter].a_ft  = FT_SDC;
	       }
	     a_fields[a_um_counter].a_arg = dac;
			 
	   }
	   env->a_srcSite = source; 
	   bool m_res = dac->unmarshal(bb);
	   env->a_srcSite = NULL; 
	   if (m_res){
	     setFlag(MSG_CLEAR);
	     continue;
	   } else {
	     setFlag(MSG_HAS_UNMARSHALCONT);
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




   // ************************* SEND MESSAGE CONTAINERS ***********************
  
  void
   MsgCnt::m_serialize(DssWriteByteBuffer *bb, Site* destination, MsgnLayerEnv* env){
     // u(n)/m-(arshal) counter keeps track of how much is marshaled
     // it is initialized at creation and increamented for each "object"
     // totally marshaled.
     // When a_um_counter is equal to a_rw_counter the TYPE_END is marshaled
     // and the loop will never be able to start again

     // Start
    
     for(; a_um_counter < a_rw_counter; a_um_counter++) {
       int space = bb->m_availableSpace();
       switch(a_fields[a_um_counter].a_ft) {
       case FT_NUMBER:
	 if (space > sz_MNumberMax) {
	   dssLog(DLL_DEBUG,"MSGCONTAINER  (%p): serilize INT %d",this,reinterpret_cast<int>(a_fields[a_um_counter].a_arg));
	   bb->m_putByte(TYPE_INT);
	   bb->m_putInt(reinterpret_cast<int>(a_fields[a_um_counter].a_arg));
	   continue;		// next field;
	 } else goto HAS_CONTINUE;
       case FT_SITE:
	 if (space > Site::sm_getMRsize()) {
	   dssLog(DLL_DEBUG,"MSGCONTAINER  (%p): serilize SITE %s",this,static_cast<Site*>(a_fields[a_um_counter].a_arg)->m_stringrep());
	   bb->m_putByte(TYPE_SITE);
	   static_cast<Site*>(a_fields[a_um_counter].a_arg)->m_marshalDSite(bb);
	   continue;
	 } else goto HAS_CONTINUE;
       case FT_DCT:
	 if (space > 20) { 
	   dssLog(DLL_DEBUG,"MSGCONTAINER  (%p): serilize DATA_AREA",this);
	   bb->m_putByte(TYPE_DCT);
	   DssCompoundTerm *dac  = static_cast<DssCompoundTerm*>( a_fields[a_um_counter].a_arg);
	   bb->m_putByte(dac->getType()); // we allways put the type
	   env->a_destSite  = destination;
	   bool m_res = dac->marshal(bb, env); 
	   env->a_destSite  = NULL;
	   if (m_res) continue;
	 }
	 goto HAS_CONTINUE;
       case FT_ADC:
	 if (space > 20) { 
	   dssLog(DLL_DEBUG,"MSGCONTAINER  (%p): serilize DATA_AREA",this);
	   bb->m_putByte(TYPE_ADC);
	   ExtDataContainerInterface *dac  = static_cast<ExtDataContainerInterface*>( a_fields[a_um_counter].a_arg);
	   bb->m_putByte(dac->getType()); // we allways put the type
	   env->a_destSite  = destination;
	   bool m_res = dac->marshal(bb); 
	   env->a_destSite  = NULL;
	   if (m_res) continue;
	 }
	 goto HAS_CONTINUE;
       
       case FT_MSGC:
	 if (space > 100){
	   bb->m_putByte(TYPE_MSG); 
	   MsgCnt *msg = static_cast<MsgCnt*>(a_fields[a_um_counter].a_arg);
	   msg->m_serialize(bb, destination, env); 
	   if(!msg->checkFlag(MSG_HAS_MARSHALCONT)){

	     continue; 
	   }
	 }
	 goto HAS_CONTINUE;
       default:
	 dssError("Not supported ft %d",a_fields[a_um_counter].a_ft); 
       }
       // We bail out of the case statement if we have a suspension,
       // otherwise do 'continue' directly:
       Assert(0);
     }

     Assert(a_um_counter == a_rw_counter); // Must be last
     //reached last, send stop marker
     if(bb->availableSpace() > 0){
       bb->putByte(TYPE_END);
       setFlag(MSG_CLEAR);
       return;
     }
     HAS_CONTINUE:
     // set continue flag and exit
     setFlag(MSG_HAS_MARSHALCONT);
   }

   void
   MsgCnt::resetMarshaling() {
     a_um_counter=0;

     // If the message has a continuation, 
     // scan all the fields and use the reset 
     // pst interface. 

     if(checkFlag(MSG_HAS_MARSHALCONT)) {
       setFlag(MSG_CLEAR);
       for(int i=0; i<MAX_NOF_FIELDS; i++)
	 {
	   if (a_fields[i].a_ft == FT_DCT)
	     static_cast<DssCompoundTerm*>(a_fields[i].a_arg)->resetMarshaling();
	   if (a_fields[i].a_ft >= FT_ADC)
	     static_cast<ExtDataContainerInterface*>(a_fields[i].a_arg)->resetMarshaling();
	 }
     }
   }

   void
   MsgCnt::m_loopBack(){
     a_um_counter = a_rw_counter; 
     a_rw_counter = 0;
     dssLog(DLL_DEBUG,"MSGCONTAINER  (%p): - LOOPBACK ACTIVATED -",this);
   }
  



  //********************'' Pop methods

  int 
  MsgCnt::popIntVal(){
    Assert(a_rw_counter < a_um_counter);
    Assert(a_fields[a_rw_counter].a_ft == FT_NUMBER);
    return reinterpret_cast<int>(m_popVal());
  }
    
  ExtDataContainerInterface* 
  MsgCnt::popADC(){
    Assert(a_rw_counter < a_um_counter);
    Assert(a_fields[a_rw_counter].a_ft == FT_ADC);
    return static_cast<ExtDataContainerInterface*>(m_popVal());
  }
  

  ExtDataContainerInterface* 
  MsgCnt::popSDC(){
    Assert(a_rw_counter < a_um_counter);
    Assert(a_fields[a_rw_counter].a_ft == FT_SDC);
    return static_cast<ExtDataContainerInterface*>(m_popVal());
  }
  
  MsgContainer* 
  MsgCnt::popMsgC(){
    Assert(a_rw_counter < a_um_counter);
    Assert(a_fields[a_rw_counter].a_ft == FT_MSGC);
    return static_cast<MsgCnt*>(static_cast<MsgContainer*>(m_popVal()));
  }

  DSite* 
  MsgCnt::popDSiteVal() {
    Site* ans = popSiteVal();
    return dynamic_cast<DSite*>(ans); 
  }

  
  
  // ************************ Push  methods
  
  void  MsgCnt::pushIntVal(int v){
    m_pushVal(reinterpret_cast<void*>(v), FT_NUMBER); }
  
  void  MsgCnt::pushADC(ExtDataContainerInterface* v){
    m_pushVal(reinterpret_cast<void*>(v), FT_ADC); }
  
  void  MsgCnt::pushSDC(ExtDataContainerInterface* v){
    m_pushVal(reinterpret_cast<void*>(v), FT_SDC); }
  
  void MsgCnt::pushMsgC(MsgContainer* v){
    MsgCnt* msg = static_cast<MsgCnt*>(v); 
    msg->a_rw_counter = msg->a_um_counter;
    msg->a_um_counter = 0; 
    m_pushVal(reinterpret_cast<void*>(msg), FT_MSGC); }
  
  
  void MsgCnt:: pushDSiteVal(DSite* s){
    pushSiteVal(static_cast<_msl_internal::Site*>(s));
  }
  
  
  // ********************************** external misc

  bool MsgCnt::m_isEmpty() const { 
     return (a_um_counter == a_rw_counter); 
   }
  MsgContainer* MsgCnt::m_getNext(){
    return static_cast<MsgContainer*>(a_next); 
  }
  
  void MsgCnt::m_convert2Send(){
    if( a_rw_counter < a_um_counter) {
      a_rw_counter = a_um_counter; 
    }
    a_um_counter = 0; 
  }
  
    
  void MsgCnt::m_convert2Rec(){
    printf("not impl yet - comvert2Rec\n"); 
    Assert(0);
  }
  void MsgCnt::m_extMakeGCpreps(){
    Assert(0); 
  }
	
	int MsgCnt::peekMslMessageType() {
		Assert(a_fields[0].a_ft == FT_NUMBER);
		return reinterpret_cast<int>(a_fields[0].a_arg);
	}
  
  //******************* DUPLICATED ******************************'
  
  MsgCnt::~MsgCnt(){
    DebugCode(a_allocated--);
    for(int i = 0; i < t_max(a_um_counter, a_rw_counter); i++){// while a valid FT
      if(a_fields[i].a_ft == FT_DCT && a_fields[i].a_arg != NULL)
	reinterpret_cast<DssCompoundTerm*>(a_fields[i].a_arg)->dispose(); 
      if(a_fields[i].a_ft >= FT_ADC && a_fields[i].a_arg != NULL)
	reinterpret_cast<ExtDataContainerInterface*>(a_fields[i].a_arg)->dispose(); 
    }
  }
} //End namespace
