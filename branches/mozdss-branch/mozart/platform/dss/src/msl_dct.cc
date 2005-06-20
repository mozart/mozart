/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 1998
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
#pragma implementation "msl_dct.hh"
#endif

//#include "DKS_routingTable.hh"
//#include "DKS_dataItem.hh"
#include "msl_serialize.hh"
#include "msl_dct.hh"
#include "msl_buffer.hh"
#include "dss_classes.hh"
namespace _msl_internal{

  DssCompoundTerm::~DssCompoundTerm() {}

  DssCompoundTerm *createReceiveDCT(DCT_Types type, MsgnLayerEnv* e){
    
    switch(type){
    case  DctT_DAC:
      return new DssSimpleDacDct();
      //     case DctT_dksRoutingTable:
      //       return new RoutingTableDct(NULL);
      //     case DctT_dksMessage:
      //       return new  DksMessageDct(e);
    case DctT_intList:
      return new IntListDct(NULL);
      //     case  DctT_dksSiteVec:
      //       return new DksSiteVecDct();
      //     default:
      //       Assert(0); 
      //     }
    case DctT_appLayer:
      return NULL; 
    case DctT_cscLayer: 
      return NULL; 
    }
    return NULL; 
  }
    
  // ****************** Data Buffer DCT ********************

  bool
  DssSimpleDacDct::marshal(DssWriteBuffer *bb, MsgnLayerEnv*){
    if(a_mode == DSDD_READ){// loopback
      //printf("DssSimpleDacDct(%p)::",static_cast<void*>(this)); gf_printBuf("loopback",a_buf,a_size);
      a_pos = a_buf;
    }

    a_mode = DSDD_WRITE;
    if(getPosDiff() == 0){ // haven't started yet
      if(bb->availableSpace() > 4){ //can marshal len + one byte at least
	//printf("DssSimpleDacDct(%p)::",static_cast<void*>(this)); gf_printBuf("marshal",a_buf,a_size);
	BYTE a_sizevec[4];
	gf_integer2char(a_sizevec,a_size);
	bb->writeToBuffer(a_sizevec,4);
      } else
	return false; // not done
    }
    int len = t_min(bb->availableSpace(), static_cast<int>(a_size)); // how much can and want we to marshal
    bb->writeToBuffer(a_pos, len);
    a_pos += len;
    //printf("len:%d size:%d done:%s\n",len,a_size, gf_bool2string((getMarshaled() == a_size)));
    return (getPosDiff() == a_size);    // check if done
  }

  bool
  DssSimpleDacDct::unmarshal(DssReadBuffer *bb,MsgnLayerEnv* env){ 
    Assert(a_mode != DSDD_WRITE);
    a_mode = DSDD_READ;
    // this code assumes that we don't store zero length buffers

    if(a_size == 0){ // haven't started
      if (bb->availableData() >= 4){
	BYTE a_sizevec[4];
	bb->readFromBuffer(a_sizevec, 4);
	bb->commitRead(4);
	a_size = gf_char2integer(a_sizevec);
	a_pos  = a_buf = new BYTE[a_size];
      } else 
	return false;
    }
    int len = t_min(static_cast<u32>(bb->availableData()), (a_size - getPosDiff()));
    bb->readFromBuffer(a_pos, len);
    bb->commitRead(len);
    a_pos += len;
    //printf("DssSimpleDacDct(%p)::",static_cast<void*>(this)); gf_printBuf("unmarshal",a_buf,getPosDiff());
    if(getPosDiff() == a_size){
      a_pos = a_buf; // automatically prepare for unmarshalling
      return true;
    } else
      return false;
  }
  
  void
  DssSimpleDacDct::resetMarshaling(){
    if(a_mode == DSDD_WRITE)
      a_pos = a_buf;
    else {
      Assert(a_mode == DSDD_READ);
      delete [] a_buf;
      a_pos = a_buf = NULL;
      a_size = 0;
    }
  }

  int 
  DssSimpleDacDct::getData(BYTE* pos, const int& max){
    //printf("DssSimpleDacDct::"); gf_printBuf("getData",a_buf,a_size);
    int tlen = t_min(max, static_cast<int>( a_size - getPosDiff()));
    memcpy(pos, a_pos, tlen); a_pos += tlen; Assert(a_pos <= a_buf + a_size);
    return tlen;
  }

  void
  DssSimpleDacDct::putData(BYTE* pos, const int& sz){
    Assert(a_buf == a_pos && a_buf == NULL);
    a_size = sz; a_buf = a_pos = new BYTE[sz];
    memcpy(a_pos, pos, sz);
  }


  // *******************************************************
 
  OneTypeContainer<int> *IntListDct::getItems(){
    OneTypeContainer<int> *tmp = a_list; 
    a_list = NULL; 
    a_curPtr = NULL;
    return tmp; 
  }

  IntListDct::IntListDct(OneTypeContainer<int>* data):a_list(data), a_curPtr(data){}

  
#define MIN_DI_SIZE 8
#define DI_ENTRY 0
#define DI_END 1
#define DI_SUSP 2
  
  bool IntListDct::marshal(DssWriteBuffer *bb, MsgnLayerEnv* env){
    for(;bb->availableSpace() > MIN_DI_SIZE &&  a_curPtr != NULL; a_curPtr = a_curPtr->a_next)
      {
	gf_Marshal8bitInt(bb,DI_ENTRY);
	gf_MarshalNumber(bb,a_curPtr->a_contain1);
      }
    if (a_curPtr == NULL){
      gf_Marshal8bitInt(bb,DI_END);
      return true;
    }
    gf_Marshal8bitInt(bb,DI_SUSP);
    return false;
  }
  
  bool IntListDct::unmarshal(DssReadBuffer *bb,MsgnLayerEnv* env){
    while(true){
      switch(gf_Unmarshal8bitInt(bb)){
      case DI_END:
	{
	  a_list = a_curPtr; 
	  return true; 
	}
      case DI_SUSP:
	return false; 
      case DI_ENTRY:
	a_curPtr = new OneTypeContainer<int>(gf_UnmarshalNumber(bb), a_curPtr); 
      }
    }
  }
  
  void IntListDct::dispose(){
    t_deleteList(a_curPtr);
    delete this; 
  }
  
  void IntListDct::resetMarshaling(){
    a_curPtr = a_list;  
  }

  
}
