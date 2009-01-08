/*
 *  Authors:
 *    Erik Klintskog(erik@sics.se)
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

#ifndef __MSGNLAYERINTERFACE_HH
#define __MSGNLAYERINTERFACE_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dss_comService.hh"
#include "dssBase.hh"
namespace _dss_internal{ //Start namespace


  enum MessageType {
    M_NONE = 0,
    M_PROXY_PROXY_PROTOCOL,
    M_COORD_PROXY_PROTOCOL,
    M_PROXY_COORD_PROTOCOL,

    M_PROXY_PROXY_REF,
    M_COORD_PROXY_REF,
    M_PROXY_COORD_REF,

    M_PROXY_PROXY_CNET,
    M_COORD_PROXY_CNET,
    M_PROXY_COORD_CNET,
    M_COORD_COORD_CNET,

    M_PROXY_CNET,
    M_COORD_CNET,


    M_PROXY_PROXY_NODEST,
    M_COORD_PROXY_NODEST,
    M_PROXY_COORD_NODEST,
    M_COORD_COORD_NODEST,

    M_LAST
  };


  class DssMslClbk: public AppMslClbkInterface, public DSS_Environment_Base{
  public:
    // ************* Functions provided as callbacks to the MsgnLayer
    virtual void m_MessageReceived(MsgContainer* const msgC,DSite* const sender);
    virtual void m_stateChange(DSite*, const FaultState&);
    virtual void m_unsentMessages(DSite* s, MsgContainer* msgs);
    virtual ExtDataContainerInterface* m_createExtDataContainer(BYTE);
    DssMslClbk(DSS_Environment *env);
    virtual ~DssMslClbk();

  private:
    void m_noDestProxy2Coord(MsgContainer *msgC, DSite *sender);
    void m_noDestCoord2Proxy(MsgContainer *msgC, DSite *sender);
    void m_noDestProxy2Proxy(MsgContainer *msgC, DSite *sender);
    void m_noDestCoord2Coord(MsgContainer *msgC, DSite *sender);

  };


  class PstContainer:public ExtDataContainerInterface, public DSS_Environment_Base{
  private:
    PstOutContainerInterface* a_pstOut;
    PstInContainerInterface*  a_pstIn;
  public:
    virtual BYTE  getType();
    virtual bool marshal(DssWriteBuffer *bb);
    virtual bool unmarshal(DssReadBuffer *bb);
    virtual void dispose();
    virtual void resetMarshaling();


    PstInContainerInterface* m_getPstIn();
    PstOutContainerInterface* m_getPstOut();
    // For some use cases the actuall pst container is not
    // known when the message is created, e.g. mutable abstract
    // operations. This is solved by passing a ptr to the
    // memory location of the pstoutcontainer. If the
    // calling routine have to pass a pst, in the case of
    // a remote operation, this is solved by assiginig the
    // memory field with a pst ptr.
    PstOutContainerInterface**  getPstOutContainerHandle();

    PstContainer(DSS_Environment *, PstOutContainerInterface*);
    PstContainer(DSS_Environment *);

  private:
    PstContainer(const PstContainer&):DSS_Environment_Base(NULL), a_pstOut(NULL), a_pstIn(NULL){}
    PstContainer& operator=(const PstContainer&){ return *this; }
  };


  class RefCntdBuffer;

  class SimpleBlockBuffer{
    public:
    BYTE *a_vec;
    BYTE *a_end;
    SimpleBlockBuffer(int sz):
      a_vec(new BYTE[sz]), a_end(0)
    {
      a_end = a_vec + sz;
    }
    ~SimpleBlockBuffer(){
      delete [] a_vec;
    }
    MACRO_NO_DEFAULT_CONSTRUCTORS(SimpleBlockBuffer);
  };

  // A container class that takes a pstout as argument and isntantly masrhals it into
  // a serialzied representation. Replicas of the container can be created, that all
  // uses the same serilized representation. Thus the cost of marshaling is keept to O(1)
  // even if multiple copies of the pst is to be sent to remote nodes(i.e. broadcast).
  // However, the current version marshals in "file" mode, i.e. all entities are made
  // persistent.
  //
  // Unmarshaling is only done when the m_getPstIn method is called. It is thus
  // safe to receive a container withot building a structure on the PS-heap.

  class PstDataContainer:public ExtDataContainerInterface, public DSS_Environment_Base{
  public:
    PstDataContainer(DSS_Environment*, PstOutContainerInterface**&);
    PstDataContainer(DSS_Environment*, RefCntdBuffer*);
    PstDataContainer(DSS_Environment*);
    virtual ~PstDataContainer();
    PstInContainerInterface* m_getPstIn();
    PstDataContainer*      m_createReplica();
  public:
    virtual BYTE  getType();
    virtual bool marshal(DssWriteBuffer *bb);
    virtual bool unmarshal(DssReadBuffer *bb);
    virtual void dispose();
    virtual void resetMarshaling();
  private:
    RefCntdBuffer *a_cntdBuf;
    BYTE*          a_cur;
    MACRO_NO_DEFAULT_CONSTRUCTORS(PstDataContainer);
  };


  class EdcByteArea:public ExtDataContainerInterface{
  public:
    EdcByteArea(SimpleBlockBuffer*);
    DssReadBuffer *m_getReadBufInterface();
  public:
    virtual BYTE  getType();
    virtual bool marshal(DssWriteBuffer *bb);
    virtual bool unmarshal(DssReadBuffer *bb);
    virtual void dispose();
    virtual void resetMarshaling();
  private:
    SimpleBlockBuffer *a_buffer;
    BYTE *a_cur;
    MACRO_NO_DEFAULT_CONSTRUCTORS(EdcByteArea);
  };



  class InfiniteWriteBuffer:public DssWriteBuffer {
    SimpleBlockBuffer *a_curBuf;
    BYTE *a_cur;
  public:
    virtual size_t  availableSpace() const;
    virtual bool canWrite(size_t len) const {return this->availableSpace()>=len;}
    virtual void writeToBuffer(const BYTE* ptr, size_t len);
    virtual void putByte(const BYTE& b);
    InfiniteWriteBuffer();
    virtual ~InfiniteWriteBuffer();
    SimpleBlockBuffer* m_getBuffer();
    MACRO_NO_DEFAULT_CONSTRUCTORS(InfiniteWriteBuffer);
  };






  enum AppDataContainerType{
    ADCT_PST,
    ADCT_PDC,
    ADCT_EBA
  };



  void gf_pushEBA(MsgContainer*, EdcByteArea* );
  EdcByteArea* gf_popEBA(MsgContainer*);

  // Global helpers
  void gf_pushPstOut(MsgContainer*, PstOutContainerInterface*);
  PstInContainerInterface* gf_popPstIn(MsgContainer*);
  PstOutContainerInterface** gf_pushUnboundPstOut(MsgContainer*);

  // ************************ PURE INTS *******************************

  void gf_createSndMsg(::MsgContainer *msgC, int i1);
  void gf_createSndMsg(::MsgContainer *msgC, int i1, int i2);
  void gf_createSndMsg(::MsgContainer *msgC, int i1, int i2, int i3);


  // *********************** INTS & SITES ******************************
  void gf_createSndMsg(::MsgContainer *msgC, ::DSite *s);
  void gf_createSndMsg(::MsgContainer *msgC, int i1, ::DSite *s);
  void gf_createSndMsg(::MsgContainer *msgC, int i1, int i2, ::DSite *s);
  void gf_createSndMsg(::MsgContainer *msgC, int i1, int i2, int i3, ::DSite *s);

  // *********************** INTS & PSTS  ******************************
  void gf_createSndMsg(::MsgContainer *msgC, ::PstOutContainerInterface *s);
  void gf_createSndMsg(::MsgContainer *msgC, int i1, ::PstOutContainerInterface *s);
  void gf_createSndMsg(::MsgContainer *msgC, int i1, int i2, ::PstOutContainerInterface *s);
  void gf_createSndMsg(::MsgContainer *msgC, int i1, int i2, int i3, ::PstOutContainerInterface *s);

  // *********************** THR  & INT*********************************
  void gf_createSndMsg(::MsgContainer *msgC, GlobalThread* th);
  void gf_createSndMsg(::MsgContainer *msgC, int i1, GlobalThread* th);
  void gf_createSndMsg(::MsgContainer *msgC, int i1, int i2, GlobalThread* th);

  // *********************** THR & PSTS & INT *********************************

  void gf_createSndMsg(::MsgContainer *msgC, GlobalThread* th, ::PstOutContainerInterface *s);
  void gf_createSndMsg(::MsgContainer *msgC, int i1, GlobalThread* th, ::PstOutContainerInterface *s);
  void gf_createSndMsg(::MsgContainer *msgC, int i1, int i2, GlobalThread* th, ::PstOutContainerInterface**& s);


}
#endif
