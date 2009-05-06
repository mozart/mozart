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
    a_state(FS_OK),
    a_version(0),
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
    a_state(FS_OK),
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
    if (a_csSite) a_csSite->disposeCsSite();     // dispose CsSite!
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
    Assert(d->canWrite(static_cast<int>(MD5_SIZE)));
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
    if(a_comObj == NULL || a_state & FS_PERM) return true; 
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
    if (a_state & FS_PERM) return false; 
    a_comObj=new ComObj(this, a_msgnLayerEnv);
    printf("Monitor!\n"); 
    return true;
  }

  bool Site::m_sendMsg(MsgCnt* msgC){
    if (a_isRemote == false){ //condition changed	
      a_msgnLayerEnv->m_loopBack(msgC);
      return true; 
    }
    if (a_state & FS_PERM) {
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

  void Site::m_connectionEstablished(DssChannel* channel){
    a_comObj->handover(channel);
  }

  void Site::m_stateChange(FaultState s){
    switch (s) {
    case FS_LOCAL_PERM:
      if (a_state == FS_LOCAL_PERM) return;
      // fall through
    case FS_GLOBAL_PERM: {
      if (a_state == FS_GLOBAL_PERM) return;
      a_state = s;
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
    case FS_TEMP: {
      if (a_state != FS_OK) return;
      a_state = FS_TEMP;
      a_msgnLayerEnv->m_stateChange(this, a_state);
      break;
    }
    case FS_OK: {
      if (a_state != FS_TEMP) return;
      a_state = FS_OK;
      a_msgnLayerEnv->m_stateChange(this, a_state);
      break;
    }
    default: 
      dssError("Not handled fault state\n"); 
      return;
    }
    // notify CsSite
    if (a_csSite) a_csSite->reportFaultState(s);
  }

  // Faults
  void Site::m_monitorRTT(int maxrtt) {
    if (!a_comObj) a_comObj = new ComObj(this, a_msgnLayerEnv);
    a_comObj->installProbe(maxrtt);
  }

  void Site::m_marshalDSite(DssWriteBuffer* buf){
    Site *dest = a_msgnLayerEnv->a_destSite;
    dssLog(DLL_DEBUG,"SITE (%p): Marshal! Dest set to:(%p)", this, dest);
    if (dest == this) {
      // Receiving site: one byte
      gf_Marshal8bitInt(buf, DMT_DEST_SITE);
    } else if (a_msgnLayerEnv->a_mySite == this &&
	       dest != NULL &&  
	       dest->a_comObj->getState() == WORKING) {
      // source site: one byte
      gf_Marshal8bitInt(buf,DMT_SRC_SITE);
    } else {
      // other site
      Assert(a_MarshaledRepresentation != NULL);
      Assert(buf->canWrite(a_MRlength+1));
      gf_Marshal8bitInt(buf, (a_state == FS_GLOBAL_PERM ?
			      DMT_SITE_PERM : DMT_SITE_OK));
      gf_MarshalNumber(buf, a_MRlength);
      buf->writeToBuffer(a_MarshaledRepresentation, a_MRlength);
    }
  }

  int Site::m_getMarshaledSize() const {
    // size depends on destination (see m_marshalDSite())
    Site* dest = a_msgnLayerEnv->a_destSite;
    if (dest == this) return 1;
    if (a_msgnLayerEnv->a_mySite == this &&
	dest &&
	dest->a_comObj->getState() == WORKING) return 1;
    return 1 + a_MRlength;
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


  // *************** Marshaled representation layout ***************
  //
  // head: - PRIM_KEY (4)             - used for hashing 
  //       - SIGNED_DIGEST (32)       - signature of the body
  // body: - LENGTH_OF_BUFFER (4)     - length of the body
  //       - LENGTH_OF_RSA (1)        - see RSA_REP below
  //       - USE_SECURED_CHANNELS (1) - boolean flag
  //       - VERSION_OF_CSC_DATA (4)  - version number
  //       - RSA_REP (LENGTH_OF_RSA)  - RSA key
  //       - CSC_REP (CSC_SIZE)       - CsSite representation

  void 
  Site::m_invalidateMarshaledRepresentation(){
    // cleanup, and increment a_version
    delete [] a_MarshaledRepresentation;
    a_version++;

    // allocate a new a_MarshaledRepresentation
    int head_len = SIZE_INT + CIPHER_BLOCK_BYTES;
    int body_len = (2*SIZE_INT + 2 + RSA_MARSHALED_REPRESENTATION +
		    a_csSite->getCsSiteSize());
    a_MRlength = head_len + body_len;
    a_MarshaledRepresentation = new BYTE[a_MRlength];

    BYTE* head = a_MarshaledRepresentation;
    BYTE* body = head + head_len;

    // fill in body with a DssWriteBuffer
    DssSimpleWriteBuffer buf(body, body_len);
    buf.m_putInt(body_len);
    buf.m_putByte(RSA_MARSHALED_REPRESENTATION);
    buf.m_putByte(a_secChannel);
    buf.m_putInt(a_version);
    buf.writeToBuffer(a_key->getStringRep(), RSA_MARSHALED_REPRESENTATION);
    a_csSite->marshalCsSite(&buf);
    Assert(buf.getUsed() == body_len);
    buf.drop();     // detach buffer from body (to avoid deallocation)

    // compute body signature (and pad with random data)
    static BYTE digest[PLAIN_BLOCK_BYTES - SIZE_INT];
    md5.compute_digest(digest, body, body_len);
    for (int i = MD5_SIZE; i < PLAIN_BLOCK_BYTES - SIZE_INT; i += SIZE_INT)
      gf_integer2char(digest + i, random_u32());

    // write header
    gf_integer2char(head, a_shortId);
    DebugCode(int rlen = )
      a_key->encrypt_text(head + SIZE_INT, digest, PLAIN_BLOCK_BYTES-SIZE_INT);
    Assert(rlen == CIPHER_BLOCK_BYTES);
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
    if( (!buf->canRead(len)) || len < (4 + CIPHER_BLOCK_BYTES + 10))
      return NULL;
    
    BYTE* marshaled_representation = new BYTE[len];
    buf->readFromBuffer(marshaled_representation,len);
    buf->commitRead(len);
    u32 pk  = gf_char2integer(marshaled_representation);
    Site *found = m_findDigest(pk, marshaled_representation+4);
 
    // verify found against malicious addresses
    if (found == NULL){
      // these were accounted for in the prev len check, now recheck buffer
      int head_len = SIZE_INT + CIPHER_BLOCK_BYTES;
      DssSimpleReadBuffer body(marshaled_representation+head_len, len-head_len);

      int body_len = body.m_getInt();
      int key_len  = body.m_getByte();
      bool sec     = body.m_getByte();
      u32 version  = body.m_getInt();
      Assert(key_len == RSA_MARSHALED_REPRESENTATION);

      if (body.availableData() + 2 + 2*SIZE_INT == body_len &&
	  body_len > RSA_MARSHALED_REPRESENTATION + SIZE_INT &&
	  key_len == RSA_MARSHALED_REPRESENTATION) {
	
	RSA_public* key = new RSA_public(body.m_getReadPos(),key_len);
	body.commitRead(key_len);
	
	// ok now we have a key, verify the buffer,
	static BYTE digest[PLAIN_BLOCK_BYTES];
	int decrypt_sign =
	  key->decrypt_text(digest, marshaled_representation + SIZE_INT,
			    CIPHER_BLOCK_BYTES);
	if (decrypt_sign == (PLAIN_BLOCK_BYTES - SIZE_INT) &&
	    md5.compare(digest, marshaled_representation + head_len, body_len)){
	  // now we only have to guard from a malicious site address
	  // (i.e. the original site is evil).  Check if the site is
	  // here but has changed address
	  found = m_findSiteKey(pk,*key);
	  if(found != NULL){
	    Assert(found->a_version != version);
	    // 3,Check that signature is not the same with different
	    // sign (o-wise we're in deep shit and ahve to check our
	    // prime generator as well as randomizer)
	    if(found->a_version < version){ // skip all other
	      //update here (should check if ok)
	      (found->a_csSite)->updateCsSite( &body);
	      found->a_MarshaledRepresentation = marshaled_representation;
	      found->a_MRlength = len;
	      found->a_version  = version;
	    } else 
	      delete [] marshaled_representation;
	    delete key; // we had it already....
	  } else {
	    found = new Site(pk, key, a_msgnLayerEnv, sec, version,
			     marshaled_representation, len);
	    insert(found);
	    // should check here too
	    CsSiteInterface *cs =
	      a_msgnLayerEnv->a_comService->unmarshalCsSite(found, &body);
	    if (cs) {
	      found->m_setCsSite(cs);
	    } else {   // no CsSiteInterface means no communication...
	      found->m_stateChange(FS_LOCAL_PERM);
	    }
	  }
	  if (type == DMT_SITE_PERM) {
	    found->m_stateChange(FS_GLOBAL_PERM);
	  }
	  // OK, leave this place
	  body.drop();
	  return found; 
	}
	delete key;
      }
      body.drop();
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

    checkSize();
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
