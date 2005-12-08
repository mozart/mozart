/*
 *  Authors:
 *    Zacharias El Banna (zeb@sics.se)
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
#if defined(INTERFACE)
#pragma implementation "msl_buffer.hh"
#endif

#include <string.h>
#include "msl_buffer.hh"
#include "dss_enums.hh"
#include "dss_templates.hh"

namespace _msl_internal{ //Start namespace
  // Utilities 

  namespace {
    const int C_HEADER = 12; // or 20
    const int TCP_SIZE_POS   = 5;
    const size_t SIZE_INT    = 4;
  }

#ifdef DEBUG_CHECK
  int DssSimpleReadBuffer::a_allocated = 0;
  int DssSimpleWriteBuffer::a_allocated = 0;
#endif

  
  void DssAdvancedBufferController::m_reinitBuffer() {
    a_pos  = a_putptr = a_getptr = const_cast<BYTE *>(a_bufMB);
    a_used = 0;
#ifdef DEBUG_CHECK
    for (int i=0; i<a_size; ++i) a_pos[i] = 0xde; 
    a_pos = a_putptr;
#endif
  } 




  void DssAdvancedBufferController::m_putInt(int i){
#ifndef MSB
    if((a_pos + SIZE_INT) < a_lastMB){
      // Will not circle this int, write it straight through (faster)
      a_pos += SIZE_INT;
      // cast to an int pointer and then de-ref it
      (*reinterpret_cast<int *>(a_pos - SIZE_INT)) = i;
      return;
    }
#endif
    for (unsigned int k=0; k<SIZE_INT; ++k) { 
      m_putByte((i & 0xFF)); 
      i = i>>8;
    }
  }


  // Speed up the getInt by assuming the marshaller knows what it's
  // doing and hence we might fast-call this

  // The buffer is LSB in the case of integers, this since we are mostly running
  // on intel machines
  int DssAdvancedBufferController::m_getInt(){
#ifndef MSB
    if((a_pos + SIZE_INT) < a_lastMB){ 
      // Will not circle this int, read it straight through (faster)
      a_pos += SIZE_INT;
      // cast to an int pointer and then de-ref it
      return (*reinterpret_cast<int*>(a_pos - SIZE_INT));
    }
#endif
    
    int i = 0;
    for (unsigned int k=0; k < SIZE_INT; ++k) {
      i = i + ((m_getByte())<<(k*8));
    }
    return i;
  }

 

  
  void DssCryptoController::m_reinitCrypto() {
    c_putptr = c_getptr = const_cast<BYTE *>(c_bufMB);
    c_used = 0;
#ifdef DEBUG_CHECK
    for (int i=0; i< c_size; ++i)
      c_putptr[i] = 0xDE; 
    c_putptr = c_getptr;
#endif
  }


  // *************************** DssReadByteBuffer ******************************


  // We rely on the DSS_LOG to print the contenct
  const char*
  DssReadByteBuffer::m_stringrep()const{
    static char buf[140];
    sprintf(buf, "size:%d used:%d putptr:%d getptr:%d pos:%d framesize:%d",
	    a_size,
	    a_used, 
	    (a_putptr-a_bufMB), 
	    (a_getptr-a_bufMB),
	    (a_pos-a_bufMB),
	    a_framesize);
    return buf;
  }

  // *********************** PST functions ***********************
  //
  // Check how much is here (in our current frame)
  int DssReadByteBuffer::availableData() const {
    return m_availableData();
  }



  void DssReadByteBuffer::readFromBuffer(BYTE* ptr, size_t wanted){
    // Must protect ourselves from a_pos == a_endMB (otherwise we
    // could move this to the else clause)
    Assert(a_pos <= a_endMB);
    if (m_posOutOfBounds())
      m_circlePos();

    Assert(wanted <= static_cast<unsigned int>(m_availableData()));
    if (a_pos < a_putptr){ // just go!
      Assert(wanted <= static_cast<unsigned int>(a_putptr-a_pos)); // Else we will copy unread data
      memcpy(ptr, a_pos, wanted);
    } else {
      int chunk = t_min(static_cast<int>(wanted),(a_endMB - a_pos)); // sequential chunk
      Assert(chunk >= 0);
      int rest  = wanted - chunk;  // next chunk
      memcpy(ptr, a_pos, chunk);
      if(rest){ // if something left copy from beginning
	Assert(a_pos >= a_putptr && a_putptr <= a_getptr && rest <= (a_putptr-a_bufMB));
	memcpy((ptr + chunk),const_cast<BYTE*>(a_bufMB),rest);
      }
    }
  }

  // Moves the posMB according to how many BYTEs have been successfully commited from the PST
  void DssReadByteBuffer::commitRead(size_t len){
    a_pos += len;
    if(m_posOutOfBounds())
      a_pos -= a_size;
  }

  const BYTE DssReadByteBuffer::getByte(){
    Assert(a_pos <= a_endMB);
    if (m_posOutOfBounds())
      m_circlePos(); // if we're about to circle

    Assert(!m_noRead()); // Someone has to put something in front of us
    Exception((m_noRead()), EXCEPTION_NO_DATA);
    return (*a_pos++);
  }
  
 
  // ****************************** UnMarshal interface ****************************

  // getCommit may never be called when no data has been read, that
  // case looks equivalent to the case of all data being read.
  void DssReadByteBuffer::m_commitReadOfData()
  {
#ifdef DEBUG_CHECK
    BYTE *p;
    if (a_pos > a_getptr)
      for (p=a_getptr;p<a_pos;++p)    
	*p=0xde;
    else { // Fills all of the buffer if a_getptr==a_pos
      for (p=a_getptr;p < a_endMB;++p)    
	*p=0xde;
      for (p = const_cast<BYTE *>(a_bufMB);p < a_pos;++p)    
	*p=0xde;
    }
#endif

    a_used = (m_allRead()) ? 0 : (a_used - m_readBytes());
 
    Assert(a_used >= 0);

    if (a_used == 0) {
      m_reinitBuffer();
    } else {
      if(m_posOutOfBounds())
	m_circlePos();
      a_getptr = a_pos;
      Assert(a_getptr < a_endMB);
    }
  }


  bool DssReadByteBuffer:: m_frameGrab(BYTE& mrk, BYTE*& frm_end){
    int len = m_getInt();
    mrk     = len & 0xFF;
    len >>= 8;
    // Determine if we have recieved length and then place frm_end there
    frm_end = a_pos + len;
    if (a_pos >= a_putptr){
      if(frm_end < a_endMB)
	return true;
      else
	frm_end -= a_size;
    }
    return(frm_end < a_putptr);
  }


  // ****************************** TCP interface ****************************

  //
  // For the read buffer the space from getptr to putptr is used
  // by the marshaler and the PST while the area from putptr to
  // getptr is used by the TransportLayer
  //
  //
  int DssReadByteBuffer::m_getReadParameters(BYTE *&buf) const {
    buf = a_putptr;
    if (a_getptr < a_putptr || (a_putptr == a_getptr && a_used == 0)){
      // If normal frame (no circle) or first time used
      Assert(a_used + (a_endMB - a_putptr) <= a_size);
      return (a_endMB - a_putptr); // possible to write until end
    } else if (a_putptr < a_getptr){
      // if has circled the put but not get
      Assert(a_used + (a_getptr - a_putptr) <= a_size);
      return (a_getptr - a_putptr);
    } else {
      Assert(a_used <= a_size);
      return 0; // We're full
    }
  }


  void DssReadByteBuffer::m_hasRead(const int& sizeRead){
    Assert(a_putptr+sizeRead <= a_getptr ||
	   a_getptr < a_putptr || (a_getptr == a_putptr && a_used == 0));
    // Better test needed to check for overflow AN

    Assert(sizeRead <= a_size);

    a_used   += sizeRead;  Assert(a_used <= a_size);
    a_putptr += sizeRead;  Assert(a_putptr <= a_endMB);
    if (m_putOutOfBounds()) // If wrote full frame circle it
      m_circlePut();
    Assert(a_putptr < a_endMB);
  }

 
  // *************************** CRYPTO ******************************

  
  void DssCryptoReadByteBuffer::m_hasRead(const int& sizeRead){
    c_used   += sizeRead;
    c_putptr += sizeRead;
    if (c_putptr > c_lastMB) // If wrote full frame circle it
      c_putptr = const_cast<BYTE*>(c_bufMB);
    //printf("Has read %d bytes from the socket, now avail:%d\n",sizeRead,c_used);
  }


  //
  int DssCryptoReadByteBuffer::m_getReadParameters(BYTE *&buf) const{
    buf = c_putptr;
    if (c_getptr < c_putptr || (c_putptr == c_getptr && c_used == 0)){
      Assert(c_used + (c_endMB - c_putptr) <= c_size);
      return (c_endMB - c_putptr); // possible to write until end
    } else if (c_putptr < c_getptr){
      // if has circled the put but not get
      Assert(c_used + (c_getptr - c_putptr) <= c_size);
      return (c_getptr - c_putptr);
    } else {
      Assert(c_used <= c_size);
      return 0; // We're full
    }
  }
  

  // return total length, no matter wrapping

  inline int DssCryptoReadByteBuffer::m_getPlainLength() const{
    if(a_putptr == a_getptr)
      return (a_used == 0)? a_size : 0;
    else
      return (a_getptr - a_putptr + ((a_getptr > a_putptr) ? 0 : a_size));
  }


  inline int DssCryptoReadByteBuffer::m_getCryptoLength() const{
    if(c_putptr != c_getptr)
      return (c_putptr - c_getptr + ((c_putptr > c_getptr) ? 0 : c_size));
    else
      return c_used;
  }
  
  

  //
  // For the read buffer area from put -> get is used by the Crypto
  // buffer

  // For crypto buffer the area from c_put to c_get is used by the
  // Transportlayer, i.e. get -> put is used by transform
  //

  // Now this part of the code has to mind the "frames" i.e. len +
  // data (of len size) the idea is to not try to decode data that is
  // a part of a not fully sent frame... Thus we have the length
  // stored in the beginning of our buffer.
  //
  
  // transform puts length in first 4 bytes and checksum in next 4 ->
  // 8 / 20 bytes (C_HEADER)

  bool DssCryptoReadByteBuffer::m_transform(){
    int copy, tmp;
    BYTE *from, *to, c_digest[MD5_SIZE];
    static BYTE from_b[512], to_b[516];
  READ_TRANSFORM_START:
    //printf("READ_TRANSFORM_START\n");
    
    if((copy = t_min((m_getPlainLength()), (m_getCryptoLength() - C_HEADER))) <= 0)
      return true;

    from = c_getptr;

    // ************* LENGTH *************
    for(unsigned int i = 0; i < SIZE_INT; ++i){
      if(from == c_endMB) 
	from = const_cast<BYTE*>(c_bufMB);
      reinterpret_cast<BYTE*>(&tmp)[i] = *from++;
    }
    // *********************************
    //printf("Found len to be:%d, copy:%d\n",tmp,copy);

    if(tmp > copy) return (tmp <= 512); // return if buffers are incorrect, (terminate session)
    
    copy = tmp; c_getptr = from;
    
    if(c_getptr <= c_putptr || copy <= (c_endMB - c_getptr)){
      // from is already set above
      c_getptr += (copy + C_HEADER - 4);
    } else {
      from = &from_b[0];
      int j = c_endMB - c_getptr;
      memcpy(&from_b[0]  , c_getptr, j);
      memcpy(&from_b[tmp], const_cast<BYTE*>(c_bufMB) , copy + C_HEADER - 4 - j);
      c_getptr = const_cast<BYTE*>(c_bufMB) + copy + C_HEADER - 4 - j;
    }
    to  = (a_putptr <= a_getptr || copy <= (a_endMB - a_putptr)) ? a_putptr : &to_b[0];
    
    // 'from' points to 'copy' length of sequential data and to points
    // to available space for direct copying

    // ************ DO TRANSFER ***********
    a_crypto.decrypt(to, from, copy);
    a_crypto.decrypt(c_digest, from + copy, C_HEADER - 4);

    // ************* CHECKSUM *************

    u32 digest = adler32(0, to , copy);
    if (digest != gf_char2integer(c_digest))
      return false;

    //if(!md5.compare(c_digest, to, copy)) return false;
    //

    // *********** CHECKSUM DONE **********
 
    if(to != &to_b[0]){
      a_putptr += copy;
      if(a_putptr > a_lastMB) 
	a_putptr = const_cast<BYTE*>(a_bufMB);
    } else {
      int k = a_endMB - a_putptr;
      memcpy(a_putptr, &to_b[0]  , k);
      memcpy(const_cast<BYTE*>(a_bufMB) , &to_b[tmp], copy - k);
      a_putptr = const_cast<BYTE*>(a_bufMB) + copy - k;
    }
    
    //printf("Transfered %d bytes from crypt -> plain\n",copy);
    
    c_used   -= (copy + C_HEADER); // plus the header (inc checksum)
    a_used   += copy;
    
    if(c_used == 0)
      m_reinitCrypto();
    else if(c_getptr > c_lastMB){
      Assert(c_getptr == c_endMB);
      c_getptr = const_cast<BYTE*>(c_bufMB);
    }
    goto READ_TRANSFORM_START;
  }





  //
  //
  //
  //
  //
  // ********************************* DssWriteByteBuffer ********************************
  //
  //
  //
  //
  //


  // We rely on the DSS_LOG to print the content
  const char*
  DssWriteByteBuffer::m_stringrep()const{
    static char buf[140];
    sprintf(buf, "size:%d used:%d putptr:%d getptr:%d pos:%d",
	    a_size,
	    a_used, 
	    a_putptr-a_bufMB, 
	    a_getptr-a_bufMB,
	    a_pos-a_bufMB);
    return buf;
  }


  // *********************** PST functions ***********************


  int DssWriteByteBuffer::availableSpace() const{
    return m_availableSpace();
  }


  // REMEMBER THAT THIS METHOD IS NOT 'SAFE', always check available
  // space before copying (one should do that anyway, thus it makes no
  // sense to do it safe twice)
  //
  void DssWriteByteBuffer::writeToBuffer(const BYTE* ptr, size_t write){
    int rest = write;
    if(m_posOutOfBounds())
      m_circlePos();

    // == to get_ptr means == to putptr => empty (or full without trailer)
    if(a_pos > a_getptr){
      Assert(static_cast<int>(a_endMB - a_pos) >= 0);
      unsigned int chunk = t_min(write,static_cast<unsigned int>(a_endMB - a_pos));
      memcpy(a_pos,ptr,chunk);
      if (chunk == write){
	a_pos +=chunk;
	return;
      }
      rest -= chunk;
      m_circlePos();
    }
    //int tmp  = rest;
    //rest = t_min(rest ,static_cast<int>(a_getptr - a_pos));
    Assert(rest >= 0);
    memcpy(a_pos, ptr, rest);
    a_pos += rest;
    Assert(a_pos <= a_getptr);
  }

  void DssWriteByteBuffer::putByte(const BYTE& b){
    if (m_posOutOfBounds()){
      Assert(a_pos == a_endMB);
      Assert(a_pos >= a_putptr);
      Assert(a_used+a_pos-a_putptr <= a_size);
      m_circlePos();
    }
    (*a_pos++) = b;
  }

  // *********************** Marshal functions ***********************
  //

  void DssWriteByteBuffer::m_marshalEnd(){
    // Fix pointers (so that availSpace returns correct info or in case of a_pos == a_endMB)
    Assert(a_pos <= a_endMB);
    if (m_posOutOfBounds())
      m_circlePos();

    // ************************ PUTSIZE ***********************
    int size = m_lenEstimate();
    
    // Find position for size X bytes from the beginning of this frame
    // (very tcpTrans specific) and write down the size integer
    BYTE* offset = (a_putptr + TCP_SIZE_POS);
    BYTE *tmp = a_pos; // save posMB

    a_pos = (offset <= a_lastMB) ? offset : (offset - a_size); // Wrappa if offset is out of range
    m_putInt(size);
    a_pos   = tmp;     // restore posMB

    // *******************************************************

    a_used += size;
    Assert(a_used <= a_size);

    // Correct pointers so that we start from the beginning (next time).
    a_putptr = a_pos;
  }


  BYTE*
  DssWriteByteBuffer::m_frameMark(){
    BYTE*tmp = a_pos;
    m_putInt(0xAABBCCDD); // pre-mark
    return tmp;
  }

  void
  DssWriteByteBuffer::m_frameMarkFinal(BYTE* frm, const BYTE& mrk){
    BYTE* tmp = a_pos;
    a_pos = frm;
#ifdef DEBUG_CHECK
    u32 chk = m_getInt();
    Assert(chk == 0xAABBCCDD);
    a_pos = frm;
#endif
    // calculate length
    Assert(0); // verify below
    int len = tmp - frm + (tmp > frm)? 0 : a_size; // account wrapping in length
    len = len << 8; // assume <= 24 bit buffer and..
    len |= mrk; // .. place mark in least significant bits
    m_putInt(len);
    a_pos = tmp;
  }

  // *********************** TRANSPORT **********************


  int DssWriteByteBuffer::m_getWriteParameters(BYTE *&buf) const {
    buf = a_getptr;
    if (a_getptr < a_putptr)
      return (a_putptr-a_getptr);
    else if (a_getptr > a_putptr || (a_getptr == a_putptr && a_used == a_size)){
      Assert(a_endMB - a_getptr != 0);
      return (a_endMB - a_getptr);
    }    else{
      Assert(0);
      return 0;
    }
  }

  void DssWriteByteBuffer::m_hasWritten(const int& sizeWritten){
    Assert(a_getptr+sizeWritten <= a_endMB); // subsumes (a_used < a_size);
    Assert(a_getptr+sizeWritten <= a_putptr ||
	   a_putptr < a_getptr ||
	   // Not allowed to circle on write.
	   (a_putptr == a_getptr && a_used == a_size));
#if defined(DEBUG_CHECK)
    for (BYTE *p=a_getptr;p<a_getptr+sizeWritten;++p)    
      *p=0xde;
#endif
    //
    Assert(sizeWritten <= a_size);
    a_used -= sizeWritten;
    Assert(a_used >= 0);

    if (a_used == 0) {
      m_reinitBuffer();
    } else {
      a_getptr += sizeWritten;
      if (m_getOutOfBounds())
	m_circleGet();
    }
  }



  //
  // ************************ CRYPTO **************************
  //

  int DssCryptoWriteByteBuffer::m_getWriteParameters(BYTE *&buf) const{
    buf = c_getptr;
    if (c_getptr < c_putptr)
      return (c_putptr-c_getptr);
    else if (c_getptr > c_putptr || (c_getptr == c_putptr && c_used == c_size))
      return (c_endMB - c_getptr);
    else
      return 0;
  }



  void DssCryptoWriteByteBuffer::m_hasWritten(const int& sizeWritten){
    c_used -= sizeWritten;
    Assert(c_used >= 0);

    // check crypt buffer if we've cleared it
    if (c_used == 0) {
      m_reinitCrypto();
    } else {
      c_getptr += sizeWritten;
      if (c_getptr > c_lastMB)
	c_getptr = const_cast<BYTE *>(c_bufMB);
    }
    //printf("Has written %d bytes to the socket, still left:%d\n",sizeWritten,c_used);
  }


  inline int DssCryptoWriteByteBuffer::m_getPlainLength() const {
    if(a_getptr != a_putptr)
      return (a_putptr - a_getptr + ((a_putptr > a_getptr) ? 0 : a_size));
    else
      return a_used;
  }


  inline int DssCryptoWriteByteBuffer::m_getCryptoLength() const {
    if(c_getptr == c_putptr)
      return (c_used == 0) ? c_size : 0;
    else 
      return (c_getptr-c_putptr + ((c_getptr > c_putptr)? 0: c_size));
  }


  //
  // For the write buffer the space from putptr to getptr is used
  // by the PST and the marshaler while the area from getptr to
  // putptr is used by the Crypto

  // For crypto buffer the area from cgetptr to cputptr is used by the
  // Transportlayer, i.e transform uses put -> get
  //

  // transform puts length in first 4 bytes and checksum in next 4 ->
  // 8 / 20 bytes

  void DssCryptoWriteByteBuffer::m_transform(){
    int copy;
    BYTE *from, *to;
    BYTE digest[MD5_SIZE];  // MD5 | CRC => 8 -> 20
    static BYTE from_b[512], to_b[516];

  WRITE_TRANSFORM_START:  
    //printf("WRITE_TRANSFORM_START\n");
    // Find out how much data there is and take 512 byte packets

    if((copy = t_min((m_getPlainLength()), (m_getCryptoLength() - C_HEADER))) <= 0){
      //printf("transform done, not enough data for transferring\n");
      return;
    }
    
    if(copy > 512) copy = (copy > 640) ? 512: 384;
    
    //printf("copying packet of %d bytes\n",copy);
    
    // set correct buffer and fix the get pointer at the same time
    if(a_getptr <= a_putptr || copy <= (a_endMB - a_getptr)){
      from   =  a_getptr;
      a_getptr += copy;
    } else {
      from = &from_b[0];
      int tmp = a_endMB - a_getptr;
      memcpy(&from_b[0]  , a_getptr, tmp);
      memcpy(&from_b[tmp], a_bufMB , copy - tmp);
      a_getptr = const_cast<BYTE*>(a_bufMB) + copy - tmp;
    }
    to  = (c_putptr <= c_getptr || (copy + C_HEADER) <= (c_endMB - c_putptr)) ? c_putptr : &to_b[0];
    // 'from' points to 'copy' length of sequential data to points to
    // available space for direct copying

    // ************** LENGTH **************

    gf_integer2char(to,copy);    

    // ************* CHECKSUM *************

    // md5.digest(from,copy);
    // md5.final(digest);
    gf_integer2char(digest,adler32(0,from,copy));
    gf_integer2char(&digest[4],random_u32()); // just to fill 64 bit
    
    // ************************************

    a_crypto.encrypt(to + 4, from, copy);
    a_crypto.encrypt(to + 4 + copy, digest, C_HEADER - 4);
    copy += C_HEADER;

    // *********** TRANSFER DONE **********

    // set correct buffer and update putptr
    if(to != (&to_b[0])){
      c_putptr += copy;
      if(c_putptr > c_lastMB) 
	c_putptr = const_cast<BYTE*>(c_bufMB);
    } else {
      int tmp = c_endMB - c_putptr;
      memcpy(c_putptr, &to_b[0]  , tmp);
      memcpy(const_cast<BYTE*>(c_bufMB) , &to_b[tmp], copy - tmp);
      c_putptr = const_cast<BYTE*>(c_bufMB) + copy - tmp;
    }
    
    //printf("Transfered %d bytes from plain -> crypt\n",copy);
    
    a_used -= (copy - C_HEADER);
    c_used += copy;
    
    if (a_used == 0){
      m_reinitBuffer();}
    else if (m_getOutOfBounds())
      m_circleGet();
    
    goto WRITE_TRANSFORM_START;
  }
} //End namespace
