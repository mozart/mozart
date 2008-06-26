/*
 *  Authors:
 *    Erik Klintskog, 2003 (erik@sics.se)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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

#include "dssBase.hh"
#include "dss_largeMessages.hh"
#include "dss_msgLayerInterface.hh"
#include "msl_serialize.hh"
namespace _dss_internal{

  enum LrgMsgEleType{
    BMET_int,
    BMET_site,
    BMET_dc,
    BMET_ni
  };



  class LrgMsgEle{
  public:
    LrgMsgEleType a_type;
    union {int i; DSite* site; ExtDataContainerInterface* dataC; NetIdentity* ni;} a_val;

    LrgMsgEle(int in):a_type(BMET_int)  {a_val.i = in;}
    LrgMsgEle(DSite *s):a_type(BMET_site) {a_val.site = s;}
    LrgMsgEle(ExtDataContainerInterface* e):a_type(BMET_dc) {a_val.dataC = e;}
    LrgMsgEle(NetIdentity* e):a_type(BMET_ni) {a_val.ni = e;}

  };


  // ******************************* class LrgMsgEleContainer ********************************


  class LrgMsgEleContainer: public ExtDataContainerInterface, public DSS_Environment_Base{
    friend  LargeMessage* gf_popLargeMessage(MsgContainer*);
    friend class LargeMessage;
    enum
      {
        SUSPEND,
        DONE,
        INT,
        SITE,
        DC,
        NI
      };

  private:
    SimpleList<LrgMsgEle*> a_elements;     // for storing fields
    Position<LrgMsgEle*>   a_pos;          // for (un)marshaling

  public:
    LrgMsgEleContainer(DSS_Environment* env) :
      DSS_Environment_Base(env), a_elements(), a_pos(a_elements) {}

    LrgMsgEleContainer(SimpleQueue<LrgMsgEle*> &elements) :
      DSS_Environment_Base(NULL), a_elements(), a_pos(a_elements)
    {
      // Okay, this is not very efficient.  But you should no longer
      // use LargeMessage!
      while (!elements.isEmpty()) a_pos.insert(elements.pop());
      a_pos(a_elements);
    }

    virtual BYTE getType(){
      return ADCT_LMC;
    }

    virtual bool marshal(DssWriteBuffer *bb){
      // Before marshaling a_pos is the first position of a_elements.
      // During marshaling, it gives the position of the element being
      // marshaled.
      while (a_pos()) {
        LrgMsgEle *ele = (*a_pos);
        switch(ele->a_type){
        case BMET_int:
          if(!bb->canWrite(10)){ //MAGIC?
            bb->putByte(SUSPEND);
            return false;
          }
          bb->putByte(INT);
          gf_MarshalNumber(bb, ele->a_val.i);
          break;
        case BMET_site:
          if(!bb->canWrite(300)){ //MAGIC?
            bb->putByte(SUSPEND);
            return false;
          }
          bb->putByte(SITE);
          ele->a_val.site->m_marshalDSite(bb);
          break;

        case BMET_ni:
          if(!bb->canWrite(320)){ //MAGIC?
            bb->putByte(SUSPEND);
            return false;
          }
          bb->putByte(NI);
          gf_marshalNetIdentity(bb, *ele->a_val.ni);
          break;
        case BMET_dc:
          // we don't care if this is a continuation or not.
          // The chance that a suspendion will happend is so small
          // that the extra complexity in rememebering this fact is not
          // worth the hassel.
          bb->putByte(DC);
          bb->putByte(ele->a_val.dataC->getType());
          if(ele->a_val.dataC->marshal(bb))
            break;
          return false;
        }
        a_pos++;
      }
      bb->putByte(DONE);
      return true;
    }

    virtual bool unmarshal(DssReadBuffer *bb){
      // During the unmarshaling process a_pos is positioned on the
      // next element that will be unmarshaled, i.e., either the
      // after-last position, or the last one if unmarshaling was
      // suspended.
      while(true){
        switch(bb->getByte()){
        case DONE:
          return true;
        case SUSPEND:
          return false;
        case INT:
          Assert(a_pos.isEmpty());
          a_pos.insert(new LrgMsgEle(gf_UnmarshalNumber(bb)));
          break;
        case SITE: {
          DSite *sd = m_getEnvironment()->a_msgnLayer->m_UnmarshalDSite(bb);
          Assert(a_pos.isEmpty());
          a_pos.insert(new LrgMsgEle(sd));
          break;
        }
        case NI:{
          NetIdentity *ni = new NetIdentity(gf_unmarshalNetIdentity(bb, m_getEnvironment()));
          Assert(a_pos.isEmpty());
          a_pos.insert(new LrgMsgEle(ni));
          break;
        }
        case DC: {
          BYTE type = bb->getByte();
          if (a_pos.isEmpty()) {
            // we start unmarshaling an element; push an new data
            // container in the queue.
            ExtDataContainerInterface *dac = m_getEnvironment()->a_dssMslClbk->m_createExtDataContainer(type);
            a_pos.push(new LrgMsgEle(dac));
          }
          // fill in the data container at a_pos
          if ((*a_pos)->a_val.dataC->unmarshal(bb)) {
            a_pos++;
            break;
          }
          return false;
        }
        default:
          dssError("Unknown entry type, LargeMessage\n");
        }
      }
    }

    virtual void dispose(){
      // delete all the elements
      a_pos(a_elements);
      while (a_pos()) {
        LrgMsgEle *ele = a_pos.pop();
        if (ele->a_type == BMET_ni) delete ele->a_val.ni;
        delete ele;
      }
    }
    virtual void resetMarshaling(){
      a_pos(a_elements);
    }
  };





  void gf_pushLargeMessage(MsgContainer* msg, LargeMessage* lm){
    LrgMsgEleContainer *cont = new LrgMsgEleContainer(lm->a_elements);
    msg->pushADC(cont);
  }

  LargeMessage* gf_popLargeMessage(MsgContainer* msg){
    LrgMsgEleContainer *cont = static_cast<LrgMsgEleContainer*>(msg->popADC());
    return new LargeMessage(cont->a_elements);
  }

  ExtDataContainerInterface* createLrgMsgContainer(DSS_Environment* env){
    return new LrgMsgEleContainer(env);
  }

  // ****************************** impl class LargeMessage **************************

  void LargeMessage::pushInt(int i){
    a_elements.append(new LrgMsgEle(i));
  }
  void LargeMessage::pushDSiteVal(DSite* s){
    a_elements.append(new LrgMsgEle(s));
  }
  void LargeMessage::pushDC(ExtDataContainerInterface* e){
    a_elements.append(new LrgMsgEle(e));
  }
  void LargeMessage::pushNetId(NetIdentity  ni){
    a_elements.append(new LrgMsgEle(new NetIdentity(ni)));
  }

  void LargeMessage::pushLM(LargeMessage *lm){
    pushDC(new LrgMsgEleContainer(lm->a_elements));
  }

  // raph: I suspect the following methods to leak objects in memory.
  // The LrgMsgEle and LrgMsgEleContainer objects popped from the
  // message are never deallocated.  The fix is: do not use
  // LargeMessage if you can.

  LargeMessage* LargeMessage::popLM(){
    LrgMsgEleContainer *cont = static_cast<LrgMsgEleContainer*>(popDC());
    return new LargeMessage(cont->a_elements);
  }

  int LargeMessage::popInt(){
    Assert(!a_elements.isEmpty());
    LrgMsgEle* bme = a_elements.pop();
    Assert(bme->a_type == BMET_int);
    return bme->a_val.i;
  }
  DSite* LargeMessage::popDSiteVal(){
    Assert(!a_elements.isEmpty());
    LrgMsgEle* bme = a_elements.pop();
    Assert(bme->a_type == BMET_site);
    return bme->a_val.site;
  }
  ExtDataContainerInterface* LargeMessage::popDC(){
    Assert(!a_elements.isEmpty());
    LrgMsgEle* bme = a_elements.pop();
    Assert(bme->a_type == BMET_dc);
    return bme->a_val.dataC;
  }

  NetIdentity LargeMessage::popNetId(){
    Assert(!a_elements.isEmpty());
    LrgMsgEle* bme = a_elements.pop();
    Assert(bme->a_type == BMET_ni);
    NetIdentity ans = *bme->a_val.ni;
    delete bme->a_val.ni;
    return ans;
  }

  bool
  LargeMessage::isEmpty(){
    return a_elements.isEmpty();
  }

  LargeMessage::LargeMessage() : a_elements() {}

  LargeMessage::LargeMessage(SimpleList<LrgMsgEle*> &elements) : a_elements() {
    while (!elements.isEmpty()) a_elements.append(elements.pop());
  }

}
