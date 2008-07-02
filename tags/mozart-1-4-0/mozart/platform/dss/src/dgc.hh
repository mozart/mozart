/*
 *  Authors:
 *    Erik Klintskog (erikd@sics.se)
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
#ifndef __DGC_HH
#define __DGC_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "dssBase.hh"

namespace _dss_internal{ // Start namespace
  
  class HomeReference; 
  class RemoteReference; 

  
  class GCalgorithm{
  public: 
#ifdef DEBUG_CHECK
    static int a_allocated;
#endif
    GCalgorithm*     a_next; 
    RCalg            a_type:RC_ALG_NBITS;
    
    GCalgorithm( GCalgorithm* const g, const RCalg& tp ):
      a_next(g),a_type(tp){ DebugCode(a_allocated++);};
    
    virtual ~GCalgorithm(){ DebugCode(a_allocated--);};
    
    // ******************* General Interfaces(for both home and remote **********'''
    
    virtual char *m_stringrep();

    // marshal ref, and maximum space needed, in bytes
    virtual void m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest)=0;
    virtual int  m_getReferenceSize() const = 0;

    virtual void m_getCtlMsg(DSite* msite, MsgContainer* msg)=0;
    virtual bool m_isRoot()=0;
    virtual void m_makeGCpreps(){};
    
    
    MACRO_NO_DEFAULT_CONSTRUCTORS(GCalgorithm);
  };

  class HomeGCalgorithm: public GCalgorithm{
  private:
    HomeReference* a_homeRef; 
  public:

    HomeGCalgorithm(HomeReference* const r, GCalgorithm* const g, const RCalg& tp ):
       GCalgorithm(g, tp), a_homeRef(r){;};
    DSS_Environment* m_getEnvironment() const ;
    
    MsgContainer* m_createRemoteMsg();
    bool m_sendToRemote(DSite*, MsgContainer*); 
    MACRO_NO_DEFAULT_CONSTRUCTORS(HomeGCalgorithm);
}; 

  class RemoteGCalgorithm: public GCalgorithm{
  private:
    RemoteReference* a_remoteRef; 
  public: 
    // ******************* Constructors ******************************
    RemoteGCalgorithm(RemoteReference* const r, GCalgorithm* const g, const RCalg& tp ):
      GCalgorithm(g, tp), a_remoteRef(r){;};
    
    
    // ******************** Provided  Functionality *****************
    DSS_Environment* m_getEnvironment() const ;
    bool m_isHomeSite(DSite* ); 
    MsgContainer* m_createHomeMsg();
    MsgContainer* m_createRemoteMsg();
    void m_sendToHome(MsgContainer*); 
    void m_sendToRemote(DSite*, MsgContainer*); 
    
    // ******************* REQUIRED INTERFACE ***********************
    virtual void m_mergeReferenceInfo(DssReadBuffer *) = 0; 
    virtual void m_dropReference() = 0; 
    
    MACRO_NO_DEFAULT_CONSTRUCTORS(RemoteGCalgorithm);     
  }; 
  
}

#endif
