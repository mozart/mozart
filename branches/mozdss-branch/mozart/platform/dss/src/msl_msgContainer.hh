/*
 *  Authors:
 *    Erik Klintskog(erik@sics.se)
 *    Zacharias ElBanna (zeb@sics.se)
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

#ifndef __MSGCONTAINER_HH
#define __MSGCONTAINER_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "msl_timers.hh"
#include "dss_classes.hh"

/*
 * SEC-TODO: To secure the MsgContainer several changes has to be completed
 *
 * 1) We have to be able to verify som level of integrity. To avoid
 * changing the entire structure of the protocols the msgRecieve
 * function must be able to tell whether a message type is complete or
 * not (eg >= fields have been recieved and the types are correct).
 *
 * 2) For those messages that actually are dynamic checks for the
 * field type should be done during message handling (eg. check valFT
 * == expected).
 *
 * 3) During deserialization the unmarshaler should return whether a
 * correct item was recieved, typically for a Site, PST , ref and (?)
 *
 * 4) 
 *
 */



namespace _msl_internal{ //Start namespace

  const int MAX_NOF_FIELDS=15;

  class ByteBuffer;
  class DssCompoundTerm; 
  class Site; 
  class DssReadByteBuffer; 
  class MsgnLayerEnv; 
  class DssWriteByteBuffer;


  enum MsgFlags {
    MSG_CLEAR,
    MSG_HAS_MARSHALCONT,
    MSG_HAS_UNMARSHALCONT
  };

  // Don't fuck with the order here, the destructors assume that
  // fieldTypes above PSTIN can be skipped through
  typedef enum {
    FT_ERROR, // DEBUG
    FT_NUMBER,
    FT_SITE,
    FT_DCT, 
    FT_ADC,
    FT_SDC, 
    FT_MSGC
  } fieldType;

  struct msgField {
    void *a_arg;    // The value 
    fieldType a_ft; // Type information
  };


  // ****************************** MESSAGE CONTAINER *******************************

  //
  //  TODO: In order to extend the message container with arbitrarily many fields we
  //  have to define a new structure which can resize itself depending on need.
  //
  //  As of now there can only be MAX_NOF_FIELDS arguments
  //

  //
  // Furthermore one should evalute whether we do want autodeletion of
  // elements when destroying a Recieve container
  
  
  class MsgCnt: public MsgContainer{
    
    
#ifdef DEBUG_CHECK
    static int a_allocated; 
#endif
  private:
    
    // Counters for current position
    // 
    // When using the MsgCnt as a send container, i.e. inserting data 
    // structures in the buffer, the a_rw_counter is a indesx to the a_fields
    // vector. The indesx points at the next position to insert a data item 
    // into. Thus, when serializeing the vector, all entries up to the 
    // a_rw_counter should be serialized. The a_um_counter is used as a
    // current pointer, starting at zero and pointing at the current item 
    // to serialize. A send MsgCnt should have the a_rw_counter indexing the 
    // first empty item in the vector and the a_um_counter indexing the first
    // position in the vector, i.e. zero.
    //
    // When using the MsgCnt as a receive container, the a_um_counter is used
    // as an index to the current empty position in the vector, where to 
    // insert unmarshaled items. The a_rw_conter initially points at the first 
    // position in the vector. 
    
    short int       a_rw_counter;
    short int       a_um_counter;
    


    MsgFlags        a_flag:3; // Windows adapted
    struct msgField a_fields[MAX_NOF_FIELDS];
    int             a_num; //queue number?? ZACHARIAS
    bool            a_internalMsg; 
    DSS_LongTime    a_sendTime;
  public:
    // heaviy used by the prioqueues
    MsgCnt          *a_next;

    
    
    MsgCnt();
    virtual ~MsgCnt();
    
    MsgCnt(int Type, bool internal);

    void m_makeGCpreps(); //sending/receiving specific
    
    
    
  public: // Queue manipulating methods
    inline fieldType m_getFT() const { return a_fields[a_rw_counter].a_ft; }
    inline void *m_popVal(){ return a_fields[a_rw_counter++].a_arg; }
    inline void *m_popDropVal() {
      void *v = a_fields[a_rw_counter].a_arg;
      a_fields[a_rw_counter++].a_arg = NULL;
      return v; 
    }
    
    inline void m_pushVal(void *v, const fieldType& ft){
      Assert(a_rw_counter + 1 < MAX_NOF_FIELDS);
      a_fields[a_rw_counter].a_ft    = ft;
      a_fields[a_rw_counter++].a_arg = v;
    }
    

  public: // control methods(flags and msgnums)
    inline void setMsgNum(const int& num) { a_num = num; }
    inline int  getMsgNum() { return a_num; }
    inline void setFlag(  const MsgFlags& flag) { a_flag = flag; }
    inline bool checkFlag(const MsgFlags& flag) { return (a_flag == flag); }
    inline bool m_isInternalMsg(){ return a_internalMsg; }
    
    void setSendTime(DSS_LongTime sTime) { a_sendTime = sTime; }
    DSS_LongTime getSendTime() { return a_sendTime; }
    
  public: // methods for transfering(receiving and sending) the container
    void m_serialize(DssWriteByteBuffer *bb, Site* destination,MsgnLayerEnv* env);
    void resetMarshaling();
    bool deserialize(DssReadByteBuffer *bb, Site* source, MsgnLayerEnv* env);
  
  public: // misc methods(debug mainly): 
    virtual char *m_stringrep();
    
  public: // MSL push and pop methods. 
    
    inline Site *popSiteVal(){
      Assert(a_rw_counter < a_um_counter && a_fields[a_rw_counter].a_ft == FT_SITE);
      return static_cast<Site*>(m_popVal());
    }

    inline DssCompoundTerm *popDctVal(){
      Assert(a_rw_counter < a_um_counter && a_fields[a_rw_counter].a_ft == FT_DCT);
      return static_cast<DssCompoundTerm*>(m_popVal());
    }

    inline void pushDctVal(DssCompoundTerm *v){
      m_pushVal(reinterpret_cast<void*>(v), FT_DCT); }
    
    inline void pushSiteVal(Site *v){
      m_pushVal(static_cast<void*>(v), FT_SITE);   }
    
    
  public: // External methods used when receiving a MsgC
    
    virtual DSite* popDSiteVal();
    virtual int popIntVal();
    virtual ExtDataContainerInterface* popADC();
    virtual ExtDataContainerInterface* popSDC();
    virtual MsgContainer* popMsgC(); 
  public: // External methods used when sending a MsgC
    virtual void pushDSiteVal(DSite* s);

    virtual void pushIntVal(int v);
    virtual void pushADC(ExtDataContainerInterface* v);
    virtual void pushSDC(ExtDataContainerInterface* v);
    virtual void pushMsgC(MsgContainer*); 

  public: // External cntrl methods
    virtual bool m_isEmpty() const ;
    virtual MsgContainer* m_getNext();
    virtual void m_convert2Send(); 
    virtual void m_convert2Rec(); 
    virtual void m_extMakeGCpreps(); 
    virtual int peekMslMessageType();

    void m_loopBack();
    // ***************************** 
    MACRO_NO_DEFAULT_CONSTRUCTORS(MsgCnt); 
  };

} // End namespace
#endif
