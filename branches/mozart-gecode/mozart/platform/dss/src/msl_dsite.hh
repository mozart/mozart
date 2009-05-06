/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Copyright:
 *    Per Brand, 1998
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

#ifndef __DSITE_HH
#define __DSITE_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "bucketHashTable.hh"
#include "dss_comService.hh" 
#include "msl_crypto.hh"
#include "msl_dct.hh"

namespace _msl_internal{ //Start namespace

  // must be declared (because msl_comObj.hh includes this file)
  class ComObj; 
  
  /*
   * SEC-TODO: DSites should store their marshaled representation of
   * their id + address TOGETHER with a version number. Addresses is
   * stored in the CS but when updated the CS_site should call some
   * 'updateMarshalRepresentation' to notify the DSite of
   * changes. This schema simplifies marshalling and updates AS WELL
   * AS securing the address part of a site from an adversary!
   *
   */

  class Site : public DSite, public BucketHashNode<Site> {
    friend class SiteHT;
  public:
#ifdef DEBUG_CHECK
    static int a_allocated;
#endif

  private:
    u32              a_shortId;     // used for hash
    RSA_public*      a_key;
    MsgnLayerEnv*    a_msgnLayerEnv; 
    ComObj*          a_comObj;
    CsSiteInterface* a_csSite;
    FaultState       a_state;
    u32              a_version;
    BYTE*            a_MarshaledRepresentation;
    int              a_MRlength;
    bool             a_secChannel;
    bool             a_isRemote;
    bool             a_isGcMarked;

    // not used
    Site(const Site&);
    Site& operator=(const Site&) { return *this; }

  public:
    Site(const u32&, RSA_private* const key, MsgnLayerEnv* const env, bool sec);
    Site(const u32&, RSA_public*  const key, MsgnLayerEnv* const env, bool sec,
	 const u32& ver, BYTE* const MRstr, const int& MRlen);
    virtual ~Site();

    // hash table stuff
    unsigned int hashCode() const { return a_shortId; }
    bool hashMatch(const RSA_public &k) const { return *a_key == k; }
    bool hashMatch(BYTE* const &) const;

    // ********* SIGNING / VERIFY - ENCRYPT /DECRYPT ***********
    //
    // Use the keys of this site to sign/verify messages.
    //
    // These methods require at least 16 bytes available space in
    // inBuf that will be used for hash. AND REMEMBER TO DELETE THE
    // inBuf/retBuf AFTERWARDS
    void m_encrypt(int& retlen, BYTE*& retBuf,const int& inlen, BYTE* const inBuf);
    bool m_decrypt(int& retlen, BYTE*& retBuf,const int& inlen, BYTE* const inBuf);

    // For these functions just delete the buffer controller since the
    // buffer is unhooked and vice versa
    DssSimpleDacDct*   m_encrypt(DssSimpleWriteBuffer* const);
    DssSimpleReadBuffer* m_decrypt(DssSimpleDacDct* const);

    // ******************* Sets n' gets ************************

    bool m_useSecureChannel() const { return a_secChannel; }
    void m_useSecureChannel(const bool& u){ a_secChannel = u; }
   
    void        m_setComObj(ComObj *comObj) {a_comObj = comObj;}    
    ComObj*     m_getComObj() const { return a_comObj;}

    void        m_setCsSite(CsSiteInterface *s){  a_csSite = s; };
    CsSiteInterface*     m_getCsSite() const { return a_csSite; }

    // ********************************************************


    bool        m_canBeFreed();
    // ERIK, used only once, when a proxy installs a fault state...
    // What should we do? 
    bool        m_connect();
  
    bool        m_sendMsg(MsgCnt *msgC);
    
    //**********************************************
    // Virtuals from the DSite interface
    virtual void  m_marshalDSite(DssWriteBuffer* buf);
    virtual int m_getMarshaledSize() const;

    virtual void  m_makeGCpreps() { a_isGcMarked = true; }
    virtual FaultState  m_getFaultState() const {return a_state;}
    virtual char*       m_stringrep();
    virtual bool m_sendMsg(::MsgContainer *);
    virtual bool operator<(const ::DSite& s); 
    virtual unsigned int  m_getShortId();
    
    //*******************************************************
    inline bool operator<(const Site& s){return (*a_key < *(s.a_key));}
    inline bool operator>(const Site& s){return (*a_key > *(s.a_key));}

    virtual ConnectivityStatus m_getChannelStatus();
    virtual void m_connectionEstablished(DssChannel*);  
    virtual void m_stateChange(FaultState newState);
    virtual void m_virtualCircuitEstablished(int len , DSite *route[]);
    virtual void m_monitorRTT(int maxrtt);
    // returns stringrep of id + length of string
    virtual BYTE* m_getId(int &len);
    virtual void m_invalidateMarshaledRepresentation();
    virtual CsSiteInterface* m_getCsSiteRep();
  };



  class SiteHT : public BucketHashTable<Site> {
  private:
#ifdef DEBUG_CHECK
    bool has_mySite;
#endif
    MsgnLayerEnv* a_msgnLayerEnv;

  public: 
    SiteHT(const int& size, MsgnLayerEnv* const env);
    ~SiteHT(){}

    Site* m_unmarshalSite(DssReadBuffer *buf);
    Site* m_findDigest(const u32&, BYTE* const buf);
    Site* m_findSiteKey(const u32&, const RSA_public&);
    void m_insert(Site *site) { insert(site); }
    
    // Called after gc, when all used Sites have been marked. 
    void gcSiteTable();

#ifdef DSS_LOG
    void log_print_content();
#endif
  
  private: // just for the darn compiler
    SiteHT(const SiteHT&);
    SiteHT& operator=(const SiteHT&){ return *this;}
  };
  
} //end namespace
#endif // __DSITE_HH

