/*
 *  Authors:
 *    Zacharias El Banna (zeb@sics.se)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Zacharias El Banna, 2002
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
#pragma implementation "referenceConsistency.hh"
#endif

#include "referenceConsistency.hh"
#include "coordinator.hh"
#include "dss_templates.hh"

// Include algs
#include "dgc_fwrc.hh"
#include "dgc_rl1.hh"
#include "dgc_rl2.hh"
#include "dgc_tl.hh"

namespace _dss_internal{ //Start namespace



  RCalg
  Reference::m_getAlgorithms(){
    if (a_algs == NULL) return RC_ALG_PERSIST;

    unsigned int gc_annot = 0;
    for (GCalgorithm *ptr = a_algs; ptr!=NULL; ptr = ptr->a_next)
      gc_annot += ptr->a_type;
    return static_cast<RCalg>(gc_annot);
  }

  void
  Reference::m_removeAlgs(){
    t_deleteList(a_algs);
    m_computeReferenceSize();
  }


  void
  Reference::m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest){
    Assert(dest != NULL || m_getNoOfAlgs() == 0); //Since we make it persistent if we want to marshal without site
    gf_Marshal8bitInt(bs, m_getNoOfAlgs()); //      1: save length,  always less than 255
    for(GCalgorithm *tmp = a_algs; tmp != NULL ; tmp = tmp->a_next){
      gf_Marshal8bitInt(bs, tmp->a_type); //        2: save type
      tmp->m_getReferenceInfo(bs, dest); //   3: save data
    }
  }

  void
  Reference::m_computeReferenceSize() {
    a_maxsize = 1;     // length
    for (GCalgorithm* alg = a_algs; alg; alg = alg->a_next) {
      a_maxsize += 1 + alg->m_getReferenceSize();     // type + data
    }
  }



   void Reference::sf_cleanType(const RCalg& type, DssReadBuffer* bs){
     printf("sf_cleanType, check this one\n");
     switch(type){
     case RC_ALG_WRC:  gf_UnmarshalNumber(bs); gf_UnmarshalNumber(bs); break;
     case RC_ALG_TL:   gf_UnmarshalNumber(bs); break;
     case RC_ALG_RLV1: break;
     case RC_ALG_RLV2: break;
     default:          Assert(0);
       dssError("cleanType got illegal type (%d)\n",type);
       break;
     }
   }

  RCalg
  Reference::m_msgToGcAlg(MsgContainer *msgC, DSite* fromsite) {
    RCalg type       = static_cast<RCalg>(msgC->popIntVal()); // need type for pushing alg_removed
    Assert((type == RC_ALG_WRC) ||
           (type == RC_ALG_TL)  ||
           (type == RC_ALG_RLV1)||
           (type == RC_ALG_RLV2)
           );
    GCalgorithm *tmp = m_findAlg(type);
    if (tmp){
      tmp->m_getCtlMsg(fromsite, msgC);
      return RC_ALG_PERSIST; }
    else {
      return type;
    }
  }


  void
  Reference::m_makeGCpreps(){
    t_gcList(a_algs);
  }


  bool
  Reference::m_removeAlgorithmType(const RCalg& atype){
    Assert((atype == RC_ALG_WRC) ||
           (atype == RC_ALG_TL)  ||
           (atype == RC_ALG_RLV1)||
           (atype == RC_ALG_RLV2)
           );
    GCalgorithm **tmp = &a_algs;
    while((*tmp)!=NULL) {
      if(atype == (*tmp)->a_type) {
        GCalgorithm *del_tmp = *tmp;
        (*tmp)=(*tmp)->a_next;
        delete del_tmp;
        m_computeReferenceSize();
        return true;
      }
      tmp = &((*tmp)->a_next);
    }
    return false;
  }


  DSS_Environment* HomeReference::m_getEnvironment() const { return a_coordinator->m_getEnvironment(); }
  DSS_Environment* RemoteReference::m_getEnvironment() const { return a_proxy->m_getEnvironment(); }

  // ***********************  Home Reference ***************************

  HomeReference::HomeReference(Coordinator *c, const RCalg& gc_annot):
    Reference(),
    a_coordinator(c){
    if (gc_annot == RC_ALG_WRC) {
      // optimization for the most common case
      a_algs = new WRC_Home(this,NULL,m_getEnvironment()->a_dssconf.gc_wrc_alpha);
    } else {
      Assert(gc_annot);
      a_algs = NULL;
      if(!(gc_annot & RC_ALG_PERSIST)){ // If not persistent, add algs
        if(gc_annot & RC_ALG_WRC)  a_algs = new WRC_Home(this,a_algs,m_getEnvironment()->a_dssconf.gc_wrc_alpha);
        if(gc_annot & RC_ALG_TL)   a_algs = new TL_Home(this,a_algs,m_getEnvironment()->a_dssconf.gc_tl_leaseTime);
        if(gc_annot & RC_ALG_RLV1) a_algs = new RLV1_Home(this,a_algs);
        if(gc_annot & RC_ALG_RLV2) a_algs = new RLV2_Home(this,a_algs);
        Assert(a_algs != NULL); //Else the glue made us persistent "accidentaly"
      }
    }
    m_computeReferenceSize();
  }

  bool
  HomeReference::m_isRoot(){
    dssLog(DLL_DEBUG,"HomeReference::isRoot");
    for(GCalgorithm *tmp = a_algs; tmp != NULL; tmp = tmp->a_next){
      dssLog(DLL_MOST," checking alg %d",tmp->a_type);
      if (!tmp->m_isRoot()) return false;
    }
    dssLog(DLL_DEBUG," ROOT!\n");
    return true;
  }

  void
  HomeReference::m_mergeReferenceInfo(DssReadBuffer*bs){
    // I dont get it... From what I see nothing is done if the
    // epochs are the same.. This might have something to do with the fact
    // that noting is actually sent to the coord site... ok, I think thats
    // the reason.
    //
    // Anyway, I'll skip that in the next version of fwdchain.
    return;



//     DSite* tmp_mgrsite     = m_getEnvironment()->a_msgnLayer->m_UnmarshalDSite(bs);
//     unsigned int tmp_epoch = gf_UnmarshalNumber(bs);

//     if(a_epoch == tmp_epoch) return;
//     // The bytebuffer contains algorithm info.
//     // Either, we dont need the info and has thus to drop all the algs.
//     // Or, we should use the info, in that case we have to move the
//     // HR to the set of dirty HR's and install this RemoteReference.
//     RemoteReference *tmpRR = new RemoteReference(a_asnode, bs, tmp_mgrsite, tmp_epoch);
//     if(a_epoch > tmp_epoch)
//       {
//      // Old version.
//      tmpRR->m_dropReference();
//      delete tmpRR;
//       }
//     else
//       {
//      // New version
//      a_asnode->m_replaceReference(this, tmpRR);
//       }
  }


  char *
  HomeReference::m_stringrep(){
    static char buf[120];
    static int pos;
    pos = sprintf(buf, "HR:  algs:");
    if (a_algs == NULL)
      sprintf((buf+pos)," PERSISTENT!");
    else
      for(GCalgorithm *tmp = a_algs; tmp != NULL; tmp = tmp->a_next)
        pos = pos + sprintf((buf+pos),"%s",tmp->m_stringrep());
    return buf;
  }


  bool
  HomeReference::m_manipulateRC(const RCalg& alg, const RCop& op, opaque& data){
    GCalgorithm *tmp; // Only cost an memory pointer to place it here (if remove op)
    switch(op){
    case RC_OP_REMOVE_ALG:
      if(m_removeAlgorithmType(alg))
        return true;
      break;
    case RC_OP_SET_WRC_ALPHA:
      if (alg == RC_ALG_WRC){
        tmp = m_findAlg(RC_ALG_WRC);
        if (tmp && static_cast<WRC_Home*>(tmp)->setAlpha(reinterpret_cast<int>(data)))
          return true;
      }
      break;
    case RC_OP_GET_WRC_ALPHA:
      if (alg == RC_ALG_WRC){
        tmp = m_findAlg(RC_ALG_WRC);
        if (tmp) {
          data = reinterpret_cast<opaque>(static_cast<WRC_Home*>(tmp)->getAlpha());
          return true;
        }
      }
      break;
    case RC_OP_SET_TL_LEASE_PERIOD:
      if (alg == RC_ALG_TL){
        tmp = m_findAlg(RC_ALG_TL);
        if (tmp && static_cast<TL_Home*>(tmp)->setLeasePeriod(reinterpret_cast<int>(data)))
          return true;
      }
      break;
    case RC_OP_GET_TL_LEASE_PERIOD:
      if (alg == RC_ALG_TL){
        tmp = m_findAlg(RC_ALG_TL);
        if (tmp) {
          data = reinterpret_cast<opaque>(static_cast<TL_Home*>(tmp)->getPeriod());
          return true;
        }
      }
      break;
    default:
      break;
    }
    return false;
  }



  // ***********************  Remote Reference ***************************

  RemoteReference::RemoteReference(Proxy *p):Reference(), a_proxy(p){
    a_algs    = NULL;
  }


  RemoteReference::RemoteReference(Proxy *p, DssReadBuffer *bs):Reference(), a_proxy(p){
    a_algs    = NULL;
    m_buildAlgorithms(bs);
  }

  void RemoteReference::m_buildAlgorithms(DssReadBuffer *bs){
    int len = gf_Unmarshal8bitInt(bs); //  1: load length
    for(int i = 0; i < len; i++){
      RCalg type = static_cast<RCalg>(gf_Unmarshal8bitInt(bs)); // 2: load type
      switch(type){
      case RC_ALG_WRC:  a_algs = new  WRC_Remote(this,bs,a_algs, m_getEnvironment()->a_dssconf.gc_wrc_alpha); break;
      case RC_ALG_TL:   a_algs = new   TL_Remote(this,bs,a_algs, m_getEnvironment()->a_dssconf.gc_tl_updateTime); break;
      case RC_ALG_RLV1: a_algs = new RLV1_Remote(this,bs,a_algs); break;
      case RC_ALG_RLV2: a_algs = new RLV2_Remote(this,bs,a_algs); break;
      default:
        dssError("Remote Reference found illegal type (%d), check buffer space\n",type);
                                Assert(0);
        break;
      }
    }
    m_computeReferenceSize();
  }


  bool RemoteReference::m_isRoot(){
    for(GCalgorithm *tmp = a_algs; tmp != NULL; tmp = tmp->a_next)
      if(tmp->m_isRoot()) return true;
    return false;
  }

  void RemoteReference::m_mergeReferenceInfo(DssReadBuffer*bs)
  {
    m_mergeAlgorithms(bs);
  }



  // Uses the notion of two lists of algorithms, if an algorithm is
  // needed it is moved to the "use" list
  // when all are merged the algorithms in "old" are removed
  void RemoteReference::m_mergeAlgorithms(DssReadBuffer*bs){
    int len = gf_Unmarshal8bitInt(bs); //  1: load length
    GCalgorithm *new_algs = NULL;

    for (int i = 0; i < len; i++) {   // For all marshaled algorithms
      RCalg type = static_cast<RCalg>(gf_Unmarshal8bitInt(bs)); // 2: load type
      GCalgorithm* tmp = m_takeAlg(type);     // take alg out of list
      if (tmp) {
        // insert in new list, and 3: get info
        tmp->a_next = new_algs;
        new_algs = tmp;
        static_cast<RemoteGCalgorithm*>(tmp)->m_mergeReferenceInfo(bs);
      } else {
        // No alg, clean out info.  This is acheived by looking at
        // what the algs would have done and then...
        sf_cleanType(type, bs);
      }
    }

    // remove remaining old algs
    t_deleteList(a_algs);
    a_algs = new_algs;
  }


  void Reference::m_dropReference(){
    while(a_algs!=NULL)
      {
        RemoteGCalgorithm *tmp = static_cast<RemoteGCalgorithm*>(a_algs);
        tmp->m_dropReference();
        a_algs = tmp->a_next;
        delete tmp;
      }
    m_computeReferenceSize();
  }


  bool
  RemoteReference::m_manipulateRC(const RCalg& alg, const RCop& op, opaque& data){
    GCalgorithm *tmp; // Only cost an memory pointer to place it here (if remove op)
    switch(op){
    case RC_OP_SET_WRC_ALPHA:
      if (alg == RC_ALG_WRC){
        tmp = m_findAlg(RC_ALG_WRC);
        if (tmp && static_cast<WRC_Remote*>(tmp)->setAlpha(reinterpret_cast<int>(data)))
          return true;
      }
      break;
    case RC_OP_GET_WRC_ALPHA:
      if (alg == RC_ALG_WRC){
        tmp = m_findAlg(RC_ALG_WRC);
        if (tmp) {
          data = reinterpret_cast<opaque>(static_cast<WRC_Remote*>(tmp)->getAlpha());
          return true;
        }
      }
      break;
    case RC_OP_SET_TL_UPDATE_PERIOD:
      if (alg == RC_ALG_TL){
        tmp = m_findAlg(RC_ALG_TL);
        if (tmp && static_cast<TL_Remote*>(tmp)->setUpdatePeriod(reinterpret_cast<int>(data)))
          return true;
      }
      break;
    case RC_OP_GET_TL_UPDATE_PERIOD:
      if (alg == RC_ALG_TL){
        tmp = m_findAlg(RC_ALG_TL);
        if (tmp) {
          data = reinterpret_cast<opaque>(static_cast<TL_Remote*>(tmp)->getPeriod());
          return true;
        }
      }
      break;
    default:
      break;
    }
    return false;
  }

  char *
  RemoteReference::m_stringrep(){
    static char buf[120];
    static int pos;
    pos = sprintf(buf, "RR:  MGR: algs:");
    if (a_algs == NULL)
      sprintf((buf+pos)," REMOVABLE!");
    else
      for(GCalgorithm *tmp = a_algs; tmp != NULL; tmp = tmp->a_next)
        pos = pos + sprintf((buf+pos),"%s",tmp->m_stringrep());
    return buf;
  }

} //End namespace
