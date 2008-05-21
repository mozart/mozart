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

  class Site; 
  class DssCompoundTerm; 
  class DssReadByteBuffer; 
  class DssWriteByteBuffer;
  class MsgnLayerEnv; 


  enum MsgFlags {
    MSG_CLEAR,
    MSG_HAS_MARSHALCONT,
    MSG_HAS_UNMARSHALCONT
  };

  // Don't fuck with the order here, the destructors assume that
  // fieldTypes above PSTIN can be skipped through
  enum FieldType {
    FT_ERROR, // DEBUG
    FT_NUMBER,
    FT_SITE,
    FT_DCT, 
    FT_ADC,
    FT_SDC, 
    FT_MSGC
  };

  struct MsgField {
    void *a_arg;    // The value 
    FieldType a_ft; // Type information
#ifdef DEBUG_CHECK
    MsgField() : a_ft(FT_ERROR), a_arg(NULL) {}
#endif
  };


  // ************************* MESSAGE CONTAINER **************************

  class MsgCnt: public MsgContainer{
    
#ifdef DEBUG_CHECK
    static int a_allocated; 
#endif

  private:
    MsgFlags        a_flag:3;      // Windows adapted
    MsgField*       a_fields;      // fields array, resized when needed
    int             a_num;         //queue number?? ZACHARIAS
    bool            a_internalMsg; 
    DSS_LongTime    a_sendTime;

    // Counters
    short int a_max_fields;     // size of the array a_fields
    short int a_nof_fields;     // number of fields in the buffer
    short int a_current;        // current position in the buffer
    
    // Inserting data and deserialization increments a_nof_fields,
    // while reading data and serialization increments a_current.  The
    // counters must satisfy the invariant:
    //
    //         0 <= a_current <= a_nof_fields <= a_max_fields.
    inline bool checkCounters() {
      return (0 <= a_current && a_current <= a_nof_fields &&
	      a_nof_fields <= a_max_fields);
    }

    // resize the array a_fields when all slots are used
    inline void checkSize() {
      if (a_nof_fields == a_max_fields) {
	int old_size = a_max_fields;
	MsgField* old = a_fields;
	Assert(a_max_fields*2 < (1 << 8*sizeof(short int)));
	a_max_fields *= 2;     // double size
	a_fields = new MsgField[a_max_fields];
	for (int i = 0; i < old_size; i++) a_fields[i] = old[i];
	delete [] old;
      }
    }

  public:
    // heaviy used by the prioqueues
    MsgCnt          *a_next;
    
    MsgCnt();
    MsgCnt(int Type, bool internal);
    virtual ~MsgCnt();

    void m_makeGCpreps(); //sending/receiving specific


  public: // Queue manipulating methods
    inline FieldType m_getFT() const { return a_fields[a_current].a_ft; }
    inline void *m_nextVal() const { return a_fields[a_current].a_arg; }
    inline void *m_popVal(){ return a_fields[a_current++].a_arg; }
    inline void *m_popDropVal() {
      void *v = a_fields[a_current].a_arg;
      a_fields[a_current++].a_arg = NULL;
      return v; 
    }
    inline void m_pushVal(void *v, const FieldType& ft){
      checkSize();
      Assert(a_nof_fields < a_max_fields);
      a_fields[a_nof_fields].a_ft    = ft;
      a_fields[a_nof_fields++].a_arg = v;
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
    void m_serialize(DssWriteByteBuffer *bb, Site* destination,
		     MsgnLayerEnv* env);
    void resetMarshaling();
    bool deserialize(DssReadByteBuffer *bb, Site* source, MsgnLayerEnv* env);
    void resetCounter();


  public: // misc methods(debug mainly): 
    virtual char *m_stringrep();


  public: // MSL push and pop methods. 
    inline Site *popSiteVal(){
      Assert(checkCounters() && m_getFT() == FT_SITE);
      return static_cast<Site*>(m_popVal());
    }
    inline DssCompoundTerm *popDctVal(){
      Assert(checkCounters() && m_getFT() == FT_DCT);
      return static_cast<DssCompoundTerm*>(m_popVal());
    }
    inline void pushDctVal(DssCompoundTerm *v) {
      m_pushVal(reinterpret_cast<void*>(v), FT_DCT);
    }
    inline void pushSiteVal(Site *v) {
      m_pushVal(static_cast<void*>(v), FT_SITE);
    }


  public: // External methods used when receiving a MsgC
    virtual DSite* popDSiteVal();
    virtual int popIntVal();
    virtual ExtDataContainerInterface* popADC();
    virtual ExtDataContainerInterface* popSDC();
    virtual PstInContainerInterface* popPstIn();
    virtual MsgContainer* popMsgC(); 


  public: // External methods used when sending a MsgC
    virtual void pushDSiteVal(DSite* s);
    virtual void pushIntVal(int v);
    virtual void pushADC(ExtDataContainerInterface* v);
    virtual void pushSDC(ExtDataContainerInterface* v);
    virtual void pushPstOut(PstOutContainerInterface*);
    virtual void pushMsgC(MsgContainer*); 


  public: // External cntrl methods
    virtual bool m_isEmpty() const ;
    virtual MsgContainer* m_getNext();
    virtual void m_convert2Send(); 
    virtual void m_convert2Rec(); 
    virtual void m_extMakeGCpreps(); 
    virtual int peekMslMessageType();

    // ***************************** 
    MACRO_NO_DEFAULT_CONSTRUCTORS(MsgCnt); 
  };

} // End namespace
#endif
