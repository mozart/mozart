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
    FifoQueue< OneContainer<LrgMsgEle> > a_elements; 
    OneContainer<LrgMsgEle> *a_ptr; 
    
  public:
    LrgMsgEleContainer(DSS_Environment* env):DSS_Environment_Base(env), a_elements(), a_ptr(NULL){;}
    LrgMsgEleContainer(FifoQueue< OneContainer<LrgMsgEle> > elements):
      DSS_Environment_Base(NULL),a_elements(elements), a_ptr(NULL){
      ;
    }
      
    virtual BYTE getType(){
      return ADCT_LMC; 
    }
    virtual bool marshal(DssWriteBuffer *bb){
      if (a_ptr == NULL){
	a_ptr = a_elements.peek(); 
      }
      
      while(a_ptr != NULL) {
	LrgMsgEle *ele = a_ptr->a_contain1;  
	switch(ele->a_type){
	case BMET_int:
	  if(bb->availableSpace() < 10){
	    bb->putByte(SUSPEND);
	    return false; 
	  }
	  bb->putByte(INT); 
	  gf_MarshalNumber(bb, ele->a_val.i);
	  break; 
	case BMET_site:
	  if(bb->availableSpace() < 300){
	    bb->putByte(SUSPEND);
	    return false; 
	  }
	  bb->putByte(SITE); 
	  ele->a_val.site->m_marshalDSite(bb);
	  break; 
	
	case BMET_ni:
	  if(bb->availableSpace() < 320){
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
	a_ptr = a_ptr->a_next;
      }
      bb->putByte(DONE);
      return true; 
    }
    
    virtual bool unmarshal(DssReadBuffer *bb){
      while(true){
	switch(bb->getByte()){
	case DONE: 
	  return true;
	case SUSPEND:
	  return false; 
	case INT:
	  a_elements.append(new OneContainer<LrgMsgEle>(new LrgMsgEle(gf_UnmarshalNumber(bb)), NULL));
	  break; 
	case SITE:
	  {
	    DSite *sd = m_getEnvironment()->a_msgnLayer->m_UnmarshalDSite(bb);
	    a_elements.append(new OneContainer<LrgMsgEle>(new LrgMsgEle(sd), NULL));
	    break; 
	  }
	case NI:{
	  NetIdentity *ni = new NetIdentity(gf_unmarshalNetIdentity(bb, m_getEnvironment())); 
	  a_elements.append(new OneContainer<LrgMsgEle>(new LrgMsgEle(ni), NULL));
	  break; 
	}
	case DC:
	  {
	    BYTE type = bb->getByte();
	    if (a_ptr == NULL){
	      ExtDataContainerInterface *dac = m_getEnvironment()->a_dssMslClbk->m_createExtDataContainer(type);
	      a_ptr = new OneContainer<LrgMsgEle>(new LrgMsgEle(dac), NULL);
	      a_elements.append(a_ptr); 
	     }

	    if(a_ptr->a_contain1->a_val.dataC->unmarshal(bb)) {
	      a_ptr = NULL; 
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
      printf("At this point we must delete all the elements\n"); 
    }
    virtual void resetMarshaling(){
      a_ptr = NULL; 
    }
  };
  
  
  
  
   
  void gf_pushLargeMessage(MsgContainer* msg, LargeMessage* lm){
    LrgMsgEleContainer *cont = new LrgMsgEleContainer(lm->a_elements); 
    msg->pushADC(cont); 
    
  }
  
  LargeMessage* gf_popLargeMessage(MsgContainer* msg){
    LrgMsgEleContainer *cont = static_cast<LrgMsgEleContainer*>(msg->popADC()); 
    return new LargeMessage(&cont->a_elements); 
  }

  ExtDataContainerInterface* createLrgMsgContainer(DSS_Environment* env){
    return new LrgMsgEleContainer(env); 
  }
  // ****************************** impl class LargeMessage **************************

  void LargeMessage::pushInt(int i){
    a_elements.append(new OneContainer<LrgMsgEle>(new LrgMsgEle(i), NULL));
  }
  void LargeMessage::pushDSiteVal(DSite* s){
    a_elements.append(new OneContainer<LrgMsgEle>(new LrgMsgEle(s), NULL));
  }
  void LargeMessage::pushDC(ExtDataContainerInterface* e){
    a_elements.append(new OneContainer<LrgMsgEle>(new LrgMsgEle(e), NULL));
  }
  void LargeMessage::pushNetId(NetIdentity  ni){
    a_elements.append(new OneContainer<LrgMsgEle>(new LrgMsgEle(new NetIdentity(ni)), NULL));
  }
  
  void LargeMessage::pushLM(LargeMessage *lm){
    pushDC(new LrgMsgEleContainer(lm->a_elements)); 
  }
  
  
  LargeMessage* LargeMessage::popLM(){
    LrgMsgEleContainer *cont = static_cast<LrgMsgEleContainer*>(popDC()); 
    return new LargeMessage(&cont->a_elements); 
  }
  
  int LargeMessage::popInt(){
    Assert(a_elements.peek()); 
    LrgMsgEle* bme = a_elements.drop()->a_contain1;
    Assert(bme->a_type == BMET_int); 
    return bme->a_val.i; 
  }
  DSite* LargeMessage::popDSiteVal(){
    Assert(a_elements.peek()); 
    LrgMsgEle* bme = a_elements.drop()->a_contain1;
    Assert(bme->a_type == BMET_site); 
    return bme->a_val.site; 
  }
  ExtDataContainerInterface* LargeMessage::popDC(){
    Assert(a_elements.peek()); 
    LrgMsgEle* bme = a_elements.drop()->a_contain1;
    Assert(bme->a_type == BMET_dc); 
    return bme->a_val.dataC; 
  }

  NetIdentity LargeMessage::popNetId(){
    
    Assert(a_elements.peek()); 
    LrgMsgEle* bme = a_elements.drop()->a_contain1;
    Assert(bme->a_type == BMET_ni); 
    NetIdentity ans = *bme->a_val.ni; 
    delete bme->a_val.ni; 
    return ans;
  }
  
  bool
  LargeMessage::isEmpty(){
    return a_elements.isEmpty();
  }
  
  LargeMessage::LargeMessage(){;}
  LargeMessage::LargeMessage(FifoQueue< OneContainer<LrgMsgEle> > *elements):a_elements(*elements){;}
  


}

