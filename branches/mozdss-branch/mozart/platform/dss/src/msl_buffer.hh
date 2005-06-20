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

#ifndef __MSL_BUFFER_HH
#define __MSL_BUFFER_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "msl_crypto.hh"
#include "dss_classes.hh"

namespace _msl_internal{ //Start namespace

  // limited buffers can be used for easy serializing to data areas

  class DssSimpleReadBuffer: public DssReadBuffer{
  private:
    BYTE* a_buf;
    BYTE* a_pos;
    u32   a_size;
#ifdef DEBUG_CHECK
    static int a_allocated;
#endif

    DssSimpleReadBuffer(const DssSimpleReadBuffer&):a_buf(NULL),a_pos(NULL),a_size(0){}
    DssSimpleReadBuffer& operator=(const  DssSimpleReadBuffer&){ return *this; }

  public:
    
    DssSimpleReadBuffer():a_buf(NULL),a_pos(NULL),a_size(0){
#ifdef DEBUG_CHECK
      ++a_allocated;
#endif
}
    
    DssSimpleReadBuffer(const u32& sz, BYTE* const bf):a_buf(bf),a_pos(bf),a_size(sz){
#ifdef DEBUG_CHECK
      ++a_allocated;
#endif
    }
    
    virtual ~DssSimpleReadBuffer(){
#ifdef DEBUG_CHECK
      --a_allocated;
#endif
      delete [] a_buf; 
    }

    inline u32 getUsed() const { return (a_pos - a_buf); }
    void hook(const u32& sz, BYTE* const bf){ a_size = sz; a_buf = a_pos = bf; }
    void drop(){ a_buf = a_pos = NULL; a_size = 0; }
    // ********* Read **********

    virtual int  availableData() const{ Assert(getUsed() <= a_size); return (a_size - getUsed()); };
    virtual void readFromBuffer(BYTE* ptr, size_t wanted){  memcpy(ptr,a_pos,wanted);   }
    virtual void commitRead(size_t read){ a_pos+=read; }; 
    
    virtual const BYTE getByte(){ return *(a_pos)++; }; 

    int   m_getInt(){ int i = gf_char2integer(a_pos); a_pos+=4; return i; }
    void  m_readOutBuffer(BYTE* ptr, size_t wanted){  memcpy(ptr,a_pos,wanted); a_pos += wanted;  }
    BYTE* m_getReadPos() const { return a_pos; };
  };


  class DssSimpleWriteBuffer: public DssWriteBuffer{
  private:
    friend class DssSimpleDacDct;
    BYTE* a_buf;
    BYTE* a_pos;
    u32   a_size;
#ifdef DEBUG_CHECK
    static int a_allocated;
#endif

    DssSimpleWriteBuffer(const DssSimpleWriteBuffer&):a_buf(NULL),a_pos(NULL),a_size(0){}
    DssSimpleWriteBuffer& operator=(const  DssSimpleWriteBuffer&){ return *this; }

  public:
    
    DssSimpleWriteBuffer(const u32& sz, BYTE* const bf): a_buf(bf), a_pos(bf), a_size(sz){
#ifdef DEBUG_CHECK
      ++a_allocated;
#endif
    }
    virtual ~DssSimpleWriteBuffer(){
#ifdef DEBUG_CHECK
      --a_allocated;
#endif
      delete [] a_buf; 
    }
    
    u32 getSize() const { return a_size; }
    u32 getUsed() const { return a_pos - a_buf; }
    
    // *********** Buffer manipulation *********
    inline BYTE* unhook(){ BYTE* tmp = a_buf; a_buf = NULL; a_pos = NULL; return tmp; }
    inline void  hook(const u32& sz, BYTE* const bf){ a_buf = a_pos = bf; a_size = sz;  }
    
    inline void  resetWriting(){ if (a_buf == NULL){ a_buf = new BYTE[a_size]; } a_pos = a_buf; }
    inline void  setWritten(const u32& len){ Assert(len <= a_size); a_pos+= len; }
    
    // ****************** Write ****************
    virtual int  availableSpace() const { return a_size - getUsed(); };
    virtual void writeToBuffer(const BYTE* ptr, size_t write){
      memcpy(a_pos,ptr,write);
      setWritten(write);
    };
    
    virtual void putByte(const BYTE& b){ *(a_pos)++ = b; }
    void m_putInt(const int& i){ gf_integer2char(a_pos,i); a_pos+=4; }
    BYTE* m_getWritePos() const { return a_pos; };
    
  };


  //    A guide to the pointers of this circular buffer(s):
  //
  //    !! The Read- and Write-ByteBuffers are organized the same
  //    !! They only expose a different set of operations
  //
  //     Static            Dynamic
  //
  //            ----------
  //        buf |        |
  //            |////////| getptr
  //            |////////|
  //            |////////| posMB
  //            |////////|
  //            |        | putptr
  //            |        |
  //     lastMB |        |
  //      endMB ----------
  //
  //      /// = filled area
  //      view each line as a byte and the pointers as pointing to the 
  //      byte of their line

  //      buf:      the beginning of the buffer
  //      lastMB:   the end (last byte) of the buffer
  //      endMB:    next after last.
  //      getptr:   the beginning of the filled area, 
  //                except in the case of an empty buffer: putptr==getptr
  //      posMB:    the next byte to be read (unmarshaling) or written (marshaling)
  //      putptr:   the beginning of the empty area,
  //                except in the case of a full buffer:   putptr==getptr

  //      getptr and putptr are static while marshaling or unmarshaling
  //      and will be adjusted together with the used field when
  //      getCommit or marshalEnd are called.
  
  //      posMB is moved by the get and put methods implemented in
  //      mbuffer.hh. For efficiency reasons it is not possible to
  //      adjust anything else in those methods. If posMB reaches the
  //      end of the buffer the corresponding putNext or getNext method
  //      will be called, allowing this buffer to be circular.

  //      Note:
  //      . getPtr,putPtr always point to a byte WITHIN buffer;
  //      . posMB can point (temporarily) to the byte immediately after
  //        the buffer;


  static const int TRAILER=1; // CF_CONT | CF_FINAL

  static const int MUSTREAD = 9;

  const u32 sz_framemark = 4; // How much a frame mark takes

  class DssAdvancedBufferController{
  protected:
    const BYTE *a_bufMB;  //  start of buffer
    const BYTE *a_lastMB; //  last byte in buffer i.e a_endMB - 1 
    const BYTE *a_endMB;  //  end of buffer
    const int   a_size;   //  the total size ( = endMB - bufMB)
    int         a_used;   // total used buffer space - current writing
			  // (i.e. adjusted after commit)    
    // Dynamic
    BYTE *a_pos;
    BYTE *a_getptr;      //  start of frame
    BYTE *a_putptr;      //  end of frame

    inline bool  m_posOutOfBounds() const { return (a_pos  > a_lastMB); }
    inline void  m_circlePos() { a_pos  = const_cast<BYTE *>(a_bufMB); }
    inline bool  m_putOutOfBounds() const { return (a_putptr > a_lastMB); }
    inline void  m_circlePut() { a_putptr = const_cast<BYTE *>(a_bufMB); }
    inline bool  m_getOutOfBounds() const { return (a_getptr > a_lastMB); }
    inline void  m_circleGet() { a_getptr = const_cast<BYTE *>(a_bufMB); }
 
    void m_reinitBuffer();

    DssAdvancedBufferController(BYTE* const buf, const int& sz):
      a_bufMB(buf),
      a_lastMB(buf + sz - 1),
      a_endMB(buf + sz),
      a_size(sz),
      a_used(0),
      a_pos(NULL),
      a_getptr(NULL),
      a_putptr(NULL){
      m_reinitBuffer();
    }

    virtual ~DssAdvancedBufferController(){}; // someone else is responsible for de-allocating

  private: // not to be used
    DssAdvancedBufferController(const DssAdvancedBufferController&):
      a_bufMB(NULL), a_lastMB(NULL), a_endMB(NULL),
      a_size(0), a_used(0), a_pos(NULL), a_getptr(NULL),
      a_putptr(NULL){
    }

    DssAdvancedBufferController& operator=(const DssAdvancedBufferController&){ return *this; }

  public:

    // ******************** SERIALIZE ********************
    //
    // These serialization functions are somewhat general... You can
    // use read or write functions on the buffer as long as the
    // context permits it

    const BYTE m_getByte(){ if (m_posOutOfBounds()) m_circlePos(); return (*a_pos++); }
    void       m_putByte(const BYTE& b){ if (m_posOutOfBounds()) m_circlePos(); (*a_pos++) = b; }

    int        m_getInt();
    void       m_putInt(int i);

    void dispose_buf(){ delete [] a_bufMB; a_bufMB = NULL;  };
  };


  class DssCryptoController{
  protected:
    const BYTE *c_bufMB;  //  start of buffer
    const BYTE *c_lastMB; //  last byte in buffer i.e c_endMB - 1 
    const BYTE *c_endMB;  //  end of buffer

    // framing for read and writes
    BYTE *c_getptr; 
    BYTE *c_putptr;
    
    int c_size; //size of this buffer
    int c_used; // used bytes, how much can we read out/ write in

    // encryption technique
    BlowFish    a_crypto;

    void m_reinitCrypto();

    DssCryptoController(BYTE* const key, const u32& keylen,
			const u32& iv1,  const u32& iv2,
			BYTE* const buffer, const int& sz):
      c_bufMB(buffer),
      c_lastMB(buffer + sz - 1),
      c_endMB(buffer + sz),
      c_getptr(NULL),
      c_putptr(NULL),
      c_size(sz),
      c_used(0),
      a_crypto(key,keylen,iv1,iv2){
      m_reinitCrypto();
      
    }

    virtual ~DssCryptoController(){ delete [] c_bufMB; }

  private: // not to be used
    DssCryptoController(const DssCryptoController&):
      c_bufMB(NULL), c_lastMB(NULL), c_endMB(NULL),
      c_getptr(NULL),c_putptr(NULL), c_size(0),
      c_used(0), a_crypto(NULL,0,0,0){
    };
 
    DssCryptoController& operator=(const DssCryptoController&){ return *this; };
  };
  // ********************************** READING ***************************************


  //
  // For the read buffer the space from getptr to putptr is used
  // by the marshaler and the PST while the area from putptr to
  // getptr is used by the TransportLayer
  //

  class DssReadByteBuffer:public ::DssReadBuffer, public DssAdvancedBufferController {
    friend class DssCryptoReadByteBuffer;

  private:

    int a_framesize;      // For unmarshaling (fake frames)
    
    // Check how long we have moved the position (i.e how many bytes we have read)
    // i.e 
    inline int  m_readBytes() const{ return (a_pos - a_getptr + ((a_pos >= a_getptr) ? 0 :a_size)); }
    inline bool m_allRead()   const{ return (a_pos == a_getptr); }
    inline bool m_noRead()    const{ return (a_pos == a_putptr) && (a_used < a_size); }

  public:
    const  char* m_stringrep() const;

    DssReadByteBuffer(BYTE* const buf, const int& sz): DssAdvancedBufferController(buf, sz), a_framesize(0){ }
    
    virtual ~DssReadByteBuffer(){ };


   inline int m_availableData() const { Assert(a_framesize - m_readBytes() >=0); return (a_framesize - m_readBytes()); };


    // ******************** SERIALIZE ********************

    virtual int availableData() const;         // Read framesize number of bytes from th buffer, starting at posMB
    virtual void readFromBuffer(BYTE* ptr, size_t read); // .. and commit when done
    virtual void commitRead(size_t len);

    bool  m_frameGrab(BYTE& mrk, BYTE*& frm_end); // returns true if frame can be okey
    void  m_frameFinalize(BYTE* frm_end){ a_pos = frm_end; }


    // Reads a byte out of the byte stream, throws EXCEPTION_NO_DATA
    // if no byte is available
    virtual const BYTE getByte(); 

    // ************** Interface to the Transport channel *************
    //
    // locate a char buffer for reading, returning the amount of free space
    virtual int  m_getReadParameters(BYTE *&buf) const;
    // ... and make the byteBuffer aware that a read operation has been done
    virtual void m_hasRead(const int& sizeRead);
    virtual bool m_transform(){ /* no-op, buffer is valid */ return true; };

    inline void m_unmarshalBegin()       { a_pos = a_getptr; }; // reposition
    inline void m_unmarshalEnd()         { a_framesize = 0; }

    // *************** Interface to the Marshaler ******************    

    // Checks whether cgSize bytes are read into the buffer. 
    // Used by the transportlayer to find if a complete 
    // frame header has been written to the buffer.   
    inline bool m_canGet(const int& cgSize) const { return (a_used - m_readBytes()) >= cgSize; };

    inline void m_setFrameSize(const int& size) { Assert(size >= 0); a_framesize = size; }  // set working space size (i.e. don't assume put - get)

    // Called when all data has been deserialized
    void  m_commitReadOfData();

  public:
    virtual void m_reinit(){ m_reinitBuffer(); }
  };


  class DssCryptoReadByteBuffer: public DssReadByteBuffer, private DssCryptoController{
  private:
    inline int  m_getPlainLength() const;
    inline int  m_getCryptoLength() const;
  public:
    DssCryptoReadByteBuffer(BYTE* const key, const u32& keylen,
			    const u32& iv1,  const u32& iv2,
			    DssReadByteBuffer* const old):
      DssReadByteBuffer(const_cast<BYTE*>(old->a_bufMB),old->a_size),
      DssCryptoController(key,keylen,iv1,iv2, new BYTE[old->a_size], old->a_size){
    }

    //virtual ~DssCryptoReadByteBuffer(){}

    virtual int  m_getReadParameters(BYTE *&buf) const;
    virtual void m_hasRead(const int& sizeRead);
    virtual bool m_transform();

    virtual void m_reinit(){ m_reinitBuffer(); m_reinitCrypto(); }
  };


  // ********************************** WRITING ***************************************


  // For the write buffer the space from putptr to getptr is used
  // by the PST and the marshaler while the area from getptr to
  // putptr is used by the TransportLayer
  //

  class DssWriteByteBuffer: public ::DssWriteBuffer, public DssAdvancedBufferController{
    friend class DssCryptoWriteByteBuffer;
  protected:

    // Differ from read estimate in that we want to see "all free" if
    // pos == put
    //
    inline int m_lenEstimate() const{ return (a_pos - a_putptr + ((a_pos >= a_putptr) ? 0 : a_size)); }

  public:

    const  char* m_stringrep() const;

    DssWriteByteBuffer(BYTE* const buf, const int& sz): DssAdvancedBufferController(buf,sz){  };
    
    virtual ~DssWriteByteBuffer(){ };

    // ****************** SERIALIZE ********************
    //
    inline int m_availableSpace() const{ Assert((a_size-a_used - TRAILER) - m_lenEstimate() >= 0); return ((a_size-a_used - TRAILER) - m_lenEstimate());  }

    // Secure framing. Use when testing marshaling to check
    // correctness of marshaled data lengths

    BYTE* m_frameMark();
    void  m_frameMarkFinal(BYTE* frm, const BYTE& mrk);
 
    // ******************** Interface to the PST *************************
    //
    // Returns the available amount of free bytes from 
    // a bytebuffer. The value is true for a buffer that 
    // is currently written to.

    virtual int  availableSpace() const;
    virtual void writeToBuffer(const BYTE* ptr, size_t write);
    virtual void putByte(const BYTE&);


    // ************** Interface to the Transport channel *************
    virtual int m_getUsed()   const {  Assert(a_used >=0); return (a_used);  }

    // prepare for writing out a portion of the buffer.
    // A size/location of a sequential chunk of bytes is returned:
    virtual int  m_getWriteParameters(BYTE *&buf) const;
    // ... and report writing out that portion:
    virtual void m_hasWritten(const int& sizeWritten);

    virtual void m_transform(){ /* no-op */ };


    // ******************* Interface to the Marshaler ********************
    inline int  m_getUnused() const { Assert((a_size - a_used) >= 0); return (a_size - a_used);}
    inline void m_marshalBegin() { a_pos = a_putptr; };
    void        m_marshalEnd();

  public:
    virtual void m_reinit(){ m_reinitBuffer(); }
  };



  class DssCryptoWriteByteBuffer: public DssWriteByteBuffer, private DssCryptoController{
  private:
    inline int  m_getPlainLength() const;
    inline int  m_getCryptoLength() const;
  public:

    DssCryptoWriteByteBuffer(BYTE* const key, const u32& keylen,
			     const u32& iv1,  const u32& iv2,
			     DssWriteByteBuffer* const old):
      DssWriteByteBuffer(const_cast<BYTE*>(old->a_bufMB),old->a_size),
      DssCryptoController(key,keylen,iv1,iv2, new BYTE[old->a_size], old->a_size){
    }


    //virtual ~DssCryptoWriteByteBuffer(){ }

    virtual int  m_getUsed()   const { Assert(c_used >=0); return (c_used); }
    virtual int  m_getWriteParameters(BYTE *&buf) const;
    virtual void m_hasWritten(const int& sizeWritten);
    virtual void m_transform();

    virtual void m_reinit(){ m_reinitBuffer(); m_reinitCrypto(); }
  };


} //End namespace
#endif
