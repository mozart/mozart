/* 
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 * 
 *  Contributors:
 *    Konstantin Popov <kost@sics.se>
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

#if defined(INTERFACE)
#pragma implementation "msl_dsite.hh"
#endif

#include <stdio.h>
#include "msl_msgContainer.hh"
#include "msl_dsite.hh"
#include "msl_comObj.hh"
#include "msl_transObj.hh"
#include "msl_buffer.hh"
#include "msl_dct.hh"
#include "msl_serialize.hh"
#include "mslBase.hh"
#include "msl_crypto.hh"
#include "dss_enums.hh"
#include "dss_comService.hh"

namespace _msl_internal{ //Start namespace

  enum SiteMarshalTag{
    DMT_SITE_PERM = 0x01,
    DMT_SITE_OK   = 0x02,
    DMT_DEST_SITE = 0x04,
    DMT_SRC_SITE  = 0x08
  };
  
  

  // **********************************************************************
  // *   SECTION :: BaseSite object methods                               *
  // **********************************************************************

  const int BUILD_SIZE = 200;
  
  int Site::sm_getMRsize(){
    return 200;
  }
  
#ifdef DEBUG_CHECK
  int Site::a_allocated=0;
#endif

  Site::Site(const u32& id, RSA_private* const key,
	     MsgnLayerEnv* const env, bool sec):
    a_shortId(id),
    a_key(key),
    a_msgnLayerEnv(env),
    a_comObj(NULL),
    a_csSite(NULL),
    a_state(DSite_OK),
    a_version(1),
    a_MarshaledRepresentation(NULL),
    a_MRlength(-1),
    a_secChannel(sec),
    a_isRemote(false),
    a_isGcMarked(false)
  {
    DebugCode(a_allocated++);
  }

  Site::Site(const u32& id, RSA_public* const key, MsgnLayerEnv* const env, 
	     bool sec, const u32& ver, BYTE* const MRstr, const int& MRlen):
    a_shortId(id),
    a_key(key),
    a_msgnLayerEnv(env),
    a_comObj(NULL),
    a_csSite(NULL),
    a_state(DSite_OK),
    a_version(ver),
    a_MarshaledRepresentation(MRstr),
    a_MRlength(MRlen),
    a_secChannel(sec),
    a_isRemote(true),
    a_isGcMarked(false)
  {
    dssLog(DLL_ALL,"REMOTE SITE: created %p",this);
    DebugCode(a_allocated++);
  }

  Site::~Site(){
    delete a_key;
    delete a_comObj;
    delete [] a_MarshaledRepresentation;
    DebugCode(a_allocated--);
  }

  /************************* encryption stuff *************************/

  void
  Site::m_encrypt(int& retLen, BYTE*& retBuf, const int& inLen, BYTE* const inBuf){
    md5.compute_digest(inBuf + inLen, inBuf, inLen);
    retBuf = new BYTE[RSA_public::encrypt_space_needed(inLen + MD5_SIZE)];
    retLen = a_key->encrypt_text(retBuf,inBuf,inLen + MD5_SIZE);
  }
  
  DssSimpleDacDct*
  Site::m_encrypt(DssSimpleWriteBuffer* const d){
    // should get size too...
    Assert(d->availableSpace() >= static_cast<int>(MD5_SIZE));
    int   inlen = d->getUsed();
    BYTE* plain = d->unhook();
    //gf_printBuf("Site:encrypt",plain,inlen);
    int retlen; BYTE* cipher;
    m_encrypt(retlen,cipher,inlen,plain);
    //gf_printBuf("Site:encrypted",cipher,retlen);
    delete [] plain;
    return new DssSimpleDacDct(retlen,cipher); // write
  }
  
  bool
  Site::m_decrypt(int& retlen, BYTE*& retBuf,const int& inlen, BYTE* const inBuf){
    retBuf = new BYTE[RSA_public::decrypt_space_needed(inlen)];
    retlen = a_key->decrypt_text(retBuf, inBuf, inlen) - MD5_SIZE;
    Assert(retlen <= static_cast<int>(RSA_public::decrypt_space_needed(inlen)));
    delete [] inBuf;
    if(retlen > 0){ // -1 or at least MD5_SIZE bytes of hash
      return(md5.compare(retBuf + retlen, retBuf, retlen));
    }
    dssLog(DLL_BEHAVIOR,"Public-key encrypted data is not correct");
    return false;
  }

  DssSimpleReadBuffer*
  Site::m_decrypt(DssSimpleDacDct* const dsrd){
    int retlen;
    int size = dsrd->getSize();
    BYTE *retbuf, *inp = dsrd->unhook();
    bool ok = m_decrypt(retlen, retbuf, size, inp);
    return (ok) ? new DssSimpleReadBuffer(retbuf, retlen) : NULL;
  }

  /************************* specific stuff *************************/

  char *Site::m_stringrep() {
    static char buf[140];
    sprintf(buf,"name (%p): ",static_cast<void*>(this));
    char* begin = &buf[17];
    BYTE* p = a_key->getStringRep();
    for(int i = 0; i < RSA_MARSHALED_REPRESENTATION; ++i)
      sprintf((begin + i),"%02x",p[i]);
    return buf;
  }

  bool Site::m_canBeFreed(){
    if(a_isGcMarked) {
      a_isGcMarked = false;
      return false; 
    }
    if(a_isRemote == false) return false; 
    if(a_comObj == NULL || a_state == DSite_GLOBAL_PRM) return true; 
    if(a_comObj->canBeFreed()) {
      delete a_comObj;
      a_comObj = NULL; 
      return true;
    }
    return false;
  }

  bool
  Site::m_connect(){
    if (a_isRemote == false || a_comObj != NULL) return true;
    if (a_state == DSite_GLOBAL_PRM) return false; 
    a_comObj=new ComObj(this, a_msgnLayerEnv);
    printf("Monitor!\n"); 
    return true;
  }

  bool Site::m_sendMsg(MsgCnt* msgC){
    if (a_isRemote == false){ //condition changed	
      a_msgnLayerEnv->m_loopBack(msgC);
      return true; 
    }
    if (a_state == DSite_GLOBAL_PRM)
      {
	delete msgC;
	return false; 
      }
    if(a_comObj == NULL)
      a_comObj=new ComObj(this, a_msgnLayerEnv);
    a_comObj->m_send(msgC, MSG_PRIO_MEDIUM);
    a_comObj->m_setLocalRef();
    return true; 
  }

  /************************* DSite interface *************************/

  bool Site::m_sendMsg(::MsgContainer* msg){
    MsgCnt *msgC = static_cast<MsgCnt*>(msg);
    return m_sendMsg(msgC);  
  }

  unsigned int Site::m_getShortId(){
    return a_shortId;
  }

  void Site::m_connectionEstablished(VirtualChannelInterface* channel){
    a_comObj->handover(channel);
  }

  void Site::m_stateChange(DSiteState stat){
    switch(stat){
    case DSite_GLOBAL_PRM:
      {
	if (a_state == DSite_GLOBAL_PRM) return;
	a_state = DSite_GLOBAL_PRM;
	MsgCnt *msgs = NULL; 
	if (a_comObj) {
	  msgs = a_comObj->m_clearQueues();
	  delete a_comObj;
	  a_comObj = NULL;
	}
	a_msgnLayerEnv->m_stateChange(this, a_state);
	a_msgnLayerEnv->m_unsentMessages(this, msgs); 
	break; 
      }
    case DSite_TMP:
      {
	if (a_state == DSite_TMP) return;	
	a_state = DSite_TMP;
	a_msgnLayerEnv->m_stateChange(this, a_state);
	break;
      }
    case DSite_OK:
      {
	if (a_state == DSite_OK) return;	
	a_state = DSite_OK; 
	a_msgnLayerEnv->m_stateChange(this, a_state);
	break;
      }
    default: 
      dssError("Not handled fault state\n"); 
    }
  }

  // Faults
  int Site::m_installRTmonitor(int lowLimit,int highLimit)
  {
    if(a_comObj == NULL)
      {
	a_comObj=new ComObj(this, a_msgnLayerEnv);
      }
    a_comObj->installProbe(lowLimit,highLimit);
    return 0; 
  }

  void Site::m_marshalDSite(DssWriteBuffer* buf){
    // one byte
    Site *dest = a_msgnLayerEnv->a_destSite;
    dssLog(DLL_DEBUG,"SITE          (%p): Marshal! Dest set to:(%p)",this,a_msgnLayerEnv->a_destSite);
    if(dest == this) {// Receiving site
	    gf_Marshal8bitInt(buf,DMT_DEST_SITE);
    }
    else if(a_msgnLayerEnv->a_mySite == this &&
	    dest != NULL &&  
	    dest->a_comObj->getState() == WORKING){
      gf_Marshal8bitInt(buf,DMT_SRC_SITE);
    } else {
      gf_Marshal8bitInt(buf,((a_state != DSite_GLOBAL_PRM) ?  DMT_SITE_OK : DMT_SITE_PERM));
      Assert(a_MarshaledRepresentation != NULL);
      Assert(buf->availableSpace() > a_MRlength);
      gf_MarshalNumber(buf, a_MRlength);
      buf->writeToBuffer(a_MarshaledRepresentation, a_MRlength);
    }
  }

  /************************* site table lookup *************************/

  Site* SiteHT::m_findSiteKey(const u32& id, const RSA_public& key){
    return lookup(id, key);
  }

  bool Site::hashMatch(BYTE* const &buf) const {
    return memcmp(a_MarshaledRepresentation + 4, buf, CIPHER_BLOCK_BYTES) == 0;
  }

  Site* SiteHT::m_findDigest(const u32& id, BYTE* const buf){
    return lookup(id, buf);
  }


  // ********************** Marshaled representation layout ***********************
  //
  //
  // start      00 : PRIM_KEY(4)         - used for hashing 
  //            04 : SIGNED_DIGEST(32)   - signature of the "rest of the buffer"
  // buf_start  36 : LENGTH_OF_BUFFER(4) - length of the "rest of..."
  //               : LENGTH_OF_RSA(1)
  //               : USE_SECURED_CHANNELS(1)
  //               : VERSION_OF_CSC_DATA(4)
  //               : RSA_REP(X == LENGTH_OF_RSA)
  //               : CSC_REP(Y)

  void 
  Site::m_invalidateMarshaledRepresentation(){
    Assert(a_MarshaledRepresentation == NULL);
    delete [] a_MarshaledRepresentation; // should be the same size but one never knows...
    BYTE* start = new BYTE[BUILD_SIZE];
    BYTE* buf_start = start + 4 + CIPHER_BLOCK_BYTES;
    BYTE* str_rep = a_key->getStringRep();
    //printf("I_KEY:");gf_printBuf(a_key->getStringRep(),RSA_MARSHALED_REPRESENTATION);
    int len = RSA_MARSHALED_REPRESENTATION; // for compiler to know key_len
    BYTE  digest[PLAIN_BLOCK_BYTES - 4];
    DssSimpleWriteBuffer dswb(buf_start, BUILD_SIZE - (4 + CIPHER_BLOCK_BYTES));

    // **** start marshaling of to-be-signed area ****
    dswb.m_putInt(0xFFFFFFFF);
    gf_Marshal8bitInt(&dswb, len);
    gf_Marshal8bitInt(&dswb, a_secChannel);
    dswb.m_putInt(a_version);  // printf("version:%x\n",a_version);
    dswb.writeToBuffer(str_rep, len);
    a_csSite->marshalCsSite( &dswb);

    len = dswb.getUsed(); // reuse len as buf_len
    gf_integer2char(buf_start, len);  //printf("buf_len:%d\n",len);
    // ********** calculate digest and sign **********
    md5.digest(buf_start,len);
    md5.final(digest);
    gf_integer2char(&digest[MD5_SIZE],    random_u32());

    
    DebugCode(int rLen =) a_key->encrypt_text(start + 4, digest, PLAIN_BLOCK_BYTES - 4);
    Assert(rLen == CIPHER_BLOCK_BYTES);
    gf_integer2char(start, a_shortId);
    //printf("pk:%x\n",getPrimKey());

    // ********* DONE, save in MarshaledRepr *********

    a_MRlength = len + (4 + CIPHER_BLOCK_BYTES);
    //printf("MR_length:%d\n",a_MRlength);
    a_MarshaledRepresentation = new BYTE[a_MRlength];
    memcpy(a_MarshaledRepresentation, start, a_MRlength);
    // printf("SIGN (IMR)\n");  gf_printBuf(a_MarshaledRepresentation, a_MRlength);
    dswb.hook(start, BUILD_SIZE);
  }

  void Site::m_virtualCircuitEstablished(int len, DSite *dstSite[]){
    a_comObj->handoverRoute(dstSite, len);
  }

  void Site::m_takeDownConnection(){
    if (a_comObj)
      a_comObj->m_closeDownConnection();
  }

  unsigned char* Site::m_getId(int &len){
    BYTE* p = a_key->getStringRep();
    BYTE* ret = new BYTE[RSA_MARSHALED_REPRESENTATION];
    if (ret) memcpy(ret,p,RSA_MARSHALED_REPRESENTATION);
    return ret;
  }
  CsSiteInterface* Site::m_getCsSiteRep(){
    return a_csSite; 
  }
  
  bool Site::operator<(const DSite& ds){
    const Site* s = static_cast<const Site*>(&ds);
    return (*a_key < *((*s).a_key));
  }
    
  // ************************* Site lookup table ************************
  
  SiteHT::SiteHT(const int& size,   MsgnLayerEnv* const env ):
    BucketHashTable<Site>(size),
#ifdef DEBUG_CHECK
    has_mySite(false), 
#endif
    a_msgnLayerEnv(env) {}

  //! this record must still be completed with the other flags 
  ConnectivityStatus Site::m_getChannelStatus(){
    ConnectivityStatus conStatus = CS_NONE;
    if (a_comObj != NULL) { 
      if (a_comObj->isConnected())
	conStatus |= CS_COMMUNICATING;
      if (a_comObj->getTransportMedium() == TM_TCP)
	conStatus |= CS_CHANNEL;
      else if (a_comObj->getTransportMedium() == TM_ROUTE) 
	conStatus |= CS_CIRCUIT;
    }
    return conStatus;
  }

  Site* SiteHT::m_unmarshalSite(DssReadBuffer *buf){
    SiteMarshalTag type = static_cast<SiteMarshalTag>(gf_Unmarshal8bitInt(buf));
    //printf("Unmarshal Site\n");
    if(type == DMT_DEST_SITE)
      return a_msgnLayerEnv->a_mySite;
    if (type == DMT_SRC_SITE) {
      Assert(a_msgnLayerEnv->a_srcSite);
      return a_msgnLayerEnv->a_srcSite;
    }
    Assert(type == DMT_SITE_OK || type == DMT_SITE_PERM);
    // Zacharias, remove perm since it is an open question how to decide it
    
    int len = gf_UnmarshalNumber(buf); // check size of package...
    if(buf->availableData() < len || len < (4 + CIPHER_BLOCK_BYTES + 10))
      return NULL;
    
    BYTE* marshaled_representation = new BYTE[len];
    buf->readFromBuffer(marshaled_representation,len);
    buf->commitRead(len);
    //printf("SIGN (U)\n");  gf_printBuf(marshaled_representation,len);
    //printf("SIGN (U)\n");  gf_printBuf(marshaled_representation+4,CIPHER_BLOCK_BYTES);
    u32 pk  = gf_char2integer(marshaled_representation); //printf("pk:%x\n",pk);
    Site *found = m_findDigest(pk, marshaled_representation+4);
 
    // verify found against malicious addresses
    if (found == NULL){
      // these were accounted for in the prev len check, now recheck buffer
      DssSimpleReadBuffer
	dsrb(marshaled_representation + 4 + CIPHER_BLOCK_BYTES,
	     len - (4 + CIPHER_BLOCK_BYTES));

      int  buf_len = dsrb.m_getInt();             //printf("buf_len:%d\n",buf_len);
      int  key_len = gf_Unmarshal8bitInt(&dsrb);  //printf("key_len:%d\n",key_len); 
      bool sec     = gf_Unmarshal8bitInt(&dsrb);  //printf("use sec:%s\n",gf_bool2string(sec));
      u32 version  = dsrb.m_getInt();             //printf("version:%x\n",version);

      Assert(key_len == RSA_MARSHALED_REPRESENTATION);
      if(dsrb.availableData() == (buf_len - 10) &&
	 buf_len >  RSA_MARSHALED_REPRESENTATION + 4 &&
	 key_len == RSA_MARSHALED_REPRESENTATION){
	
	RSA_public* key = new RSA_public(dsrb.m_getReadPos(),key_len);
	dsrb.commitRead(key_len);
	//printf("U_KEY:");gf_printBuf(key->getStringRep(),RSA_MARSHALED_REPRESENTATION);
	
	// ok now we have a key, verify the buffer,
	BYTE digest[PLAIN_BLOCK_BYTES];
	int decrypt_sign = key->decrypt_text(digest, marshaled_representation + 4, CIPHER_BLOCK_BYTES);
	//printf("DIGEST:");gf_printBuf(key->getStringRep(),RSA_MARSHALED_REPRESENTATION);
	if(decrypt_sign == (PLAIN_BLOCK_BYTES - 4) &&
	   md5.compare(digest, marshaled_representation + 4 + CIPHER_BLOCK_BYTES, buf_len)){
	  // now we only have to guard from a malicious site address (i.e. the original site is evil)
	  // check if the site is here but has changed address
	  found = m_findSiteKey(pk,*key);
	  if(found != NULL){
	    Assert(found->a_version != version);
	    // 3,Check that signature is not the same with different
	    // sign (o-wise we're in deep shit and ahve to check our
	    // prime generator as well as randomizer)
	    DebugCode(if(found->a_version != version) dssError("Site identity problem - key's are equal"));
	    if(found->a_version < version){ // skip all other
	      //update here (should check if ok)
	      (found->a_csSite)->updateCsSite( &dsrb);
	      found->a_MarshaledRepresentation = marshaled_representation;
	      found->a_MRlength = len;
	      found->a_version  = version;
	    } else 
	      delete [] marshaled_representation;
	    delete key; // we had it already....
	  } else {
	    found = new Site(pk, key, a_msgnLayerEnv, sec, version, marshaled_representation, len);
	    //printf("found new site:%p\n",static_cast<void*>(found));
	    insert(found);
	    // should check here too
	    CsSiteInterface *cs = a_msgnLayerEnv->a_comService->unmarshalCsSite(found, &dsrb);
	    if(cs)
	      found->m_setCsSite(cs);
	    else
	      found->m_stateChange(DSite_GLOBAL_PRM); // KILL IT SINCE WE HAVE NO CS INFO
	  }
	  if (type == DMT_SITE_PERM && found->m_getFaultState() != DSite_GLOBAL_PRM)
	    found->m_stateChange(DSite_GLOBAL_PRM);

	  // OK, leave this place
	  dsrb.drop();
	  return found; 
	}
	delete key;
      }
      dsrb.drop();
    }
    delete [] marshaled_representation;
    return found;
  }
  

  void SiteHT::gcSiteTable() {
    // we have first to gc all msgs stored in the comobjects
    for (Site* s = getFirst(); s; s = getNext(s)) {
      if (s->m_getComObj()) s->m_getComObj()->m_makeGCpreps();
    }

    // now we check and delete unmarked sites
    for (Site* s = getFirst(); s;) {
      Site* cur = s;
      s = getNext(s);
      if (cur->m_canBeFreed()) {
	remove(cur); delete cur;
      }
    }
  }


  // We rely on the DSS_LOG to print the contenct
#ifdef DSS_LOG
  void SiteHT::log_print_content(){
    for (Site* s = getFirst(); s; s = getNext(s)) {
      dssLog(DLL_PRINT,"\t%d:%p=>%s", s->a_shortId, s->a_key, s->m_stringrep());
    }
  }
#endif

} //End namespace
