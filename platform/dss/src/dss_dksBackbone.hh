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

#ifndef __DSS_DKS_BACKBONE_HH
#define __DSS_DKS_BACKBONE_HH


#include "dss_largeMessages.hh"
#include "dss_dksInstance.hh"
#include "dssBase.hh"

namespace _dss_internal{


  class DksInstance;
  class NetIdHT;
  class DSS_Environment;

  enum BackboneServiceTypes{
    BST_MOBILE_COORDINATOR
  };



  class BackboneService{
  public:
    virtual void m_messageReceived(LargeMessage*, DSS_Environment* env) = 0;
    virtual LargeMessage* m_transferService() = 0;
    virtual int m_getType() = 0;
  };

  class BackboneServiceNode: public NetIdNode,
                             public BucketHashNode<BackboneServiceNode> {
  public:
    BackboneService* a_srv;
    BackboneServiceNode(NetIdentity ni,BackboneService* srv ):
      NetIdNode(ni), BucketHashNode<BackboneServiceNode>(), a_srv(srv){;}
  };

  class BackboneServiceTable : public NetIdHT, public BucketHashTable<BackboneServiceNode> {
  public:
    BackboneServiceTable(const int &sz, DSS_Environment* env) :
      NetIdHT(env), BucketHashTable<BackboneServiceNode>(sz) {}
  };


  class DksBackbone: public DKS_userClass, public DSS_Environment_Base{
  private:
    BackboneServiceTable a_serviceHT;
  public:
    DksInstance *a_instance;


    void m_sendToService(NetIdentity, LargeMessage*);
    void m_insertService(NetIdentity, BackboneService*);

    DksBackbone(DksInstance *instance,DSS_Environment* env);
    DksBackbone(DSS_Environment* env);

    void  m_installBackboneService(LargeMessage* lm);
  public: // DKS_userClass virtuals
    virtual void m_receivedRoute(int Key, DksMessage*);
    virtual void m_receivedRouteNext(int Key, DksMessage*);
    virtual DksMessage* m_divideResp(int start, int stop, int n);
    virtual void m_newResponsability(int begin, int end, int n, DksMessage*);
    virtual void dks_functional();
    virtual void pushDksMessage(MsgContainer*, DksMessage*);
    virtual DksMessage *popDksMessage(MsgContainer*);

    // These methods will not be properly implemented since
    // the backbone(currently) shoudl not be used for broadcasts.
    virtual void m_receivedBroadcast(DksBcMessage*);
    virtual void pushDksBcMessage(MsgContainer*, DksBcMessage*);
    virtual DksBcMessage *popDksBcMessage(MsgContainer*);
    MACRO_NO_DEFAULT_EQUALITY(DksBackbone);

  private:
    DksBackbone(const DksBackbone&);
};




}
#endif
