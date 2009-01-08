/*
 *  Authors:
 *    Zacharias El Banna (zeb@sics.se)
 *    Raphael Collet (raph@info.ucl.ac.be)
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
  const int SIZE_INT = 4;

  /************************* SimpleBuffer *************************/

  // SimpleBuffer: an implementation of a non-circular buffer.  It
  // defines a position, which can be used both for reading and
  // writing.  The buffer area is automatically deallocated.

  class SimpleBuffer {
  private:
    BYTE*  buf;      // start of the buffer
    BYTE*  pos;      // position inside the buffer
    size_t size;     // size (in bytes) of the buffer

    // those are not allowed
    SimpleBuffer(const SimpleBuffer&):buf(NULL),pos(NULL),size(0){ Assert(0); }
    SimpleBuffer& operator= (const SimpleBuffer&) { return *this; }

  public:
    // constructors and destructor
    SimpleBuffer() : buf(NULL), pos(NULL), size(0) {}
    SimpleBuffer(BYTE* const &b, size_t const &sz)
      : buf(b), pos(b), size(sz) {}
    virtual ~SimpleBuffer() { delete [] buf; }

    // set/get buffer
    void hook(BYTE* const &b, size_t const &sz) { buf = pos = b; size = sz; }
    void drop() { buf = pos = NULL; size = 0; }
    BYTE* unhook() { BYTE* b = buf; drop(); return b; }

    // get total size/space before pos/space after pos
    size_t getSize() const { return size; }
    size_t getUsed() const { return (pos - buf); }
    size_t getFree() const { return size - (pos - buf); }

    // get/reset the position
    BYTE* getPos() const { return pos; }
    void rewindPos() { pos = buf; }

    // simple read/write
    BYTE m_getByte() { return *(pos++); }
    void m_putByte(const BYTE &b) { *(pos++) = b; }
    int m_getInt() {
      int i = gf_char2integer(pos); pos += SIZE_INT; return i; }
    void m_putInt(const int &i) {
      gf_integer2char(pos, i); pos += SIZE_INT; }

    // read/write blocks (require explicit commit())
    void m_read(BYTE* const &ptr, size_t const &len) {
      memcpy(ptr, pos, len); }
    void m_write(const BYTE* const &ptr, size_t const &len) {
      memcpy(pos, ptr, len); }
    void m_commit(size_t const &len) { pos += len; }
  };



  // a SimpleBuffer dressed as a DssReadBuffer
  class DssSimpleReadBuffer : public DssReadBuffer, public SimpleBuffer {
  public:
    DssSimpleReadBuffer() : SimpleBuffer() {}
    DssSimpleReadBuffer(BYTE* const &buffer, size_t const &len)
      : SimpleBuffer(buffer, len) {}

    // implementation of DssReadBuffer
    virtual size_t availableData() const { return getFree(); }
    virtual bool canRead(size_t len) const {return getFree()>=len;}
    virtual void readFromBuffer(BYTE* ptr, size_t len) { m_read(ptr, len); }
    virtual void commitRead(size_t len) { m_commit(len); }
    virtual const BYTE getByte() { return m_getByte(); }

    // extra methods
    void m_readOutBuffer(BYTE* const &ptr, size_t const &len) {
      m_read(ptr, len); m_commit(len);
    }
    BYTE* m_getReadPos() const { return getPos(); }
  };



  // a SimpleBuffer dressed as a DssWriteBuffer
  class DssSimpleWriteBuffer : public DssWriteBuffer, public SimpleBuffer {
  public:
    DssSimpleWriteBuffer(BYTE* const &buffer, size_t const &len)
      : SimpleBuffer(buffer, len) {}

    // implementation of DssWriteBuffer
    virtual size_t availableSpace() const { return getFree(); }
    virtual bool canWrite(size_t len) const {return getFree()>=len;}
    virtual void writeToBuffer(const BYTE* ptr, size_t len) {
      m_write(ptr, len); m_commit(len);
    };
    virtual void putByte(const BYTE& b) { m_putByte(b); }
  };



  /************************* CircularBuffer *************************/

  // CircularBuffer: an implementation of a circular buffer, whose
  // interface is the one of a FIFO queue.  Sheme:
  //
  //        +--------+                +--------+
  //    buf |        |            buf |////////|
  //        |////////| getpos         |        | putpos
  //        |////////|                |        |
  //        |////////|                |        |
  //        |        | putpos         |////////| getpos
  //        |        |                |////////|
  //    end +--------+            end +--------+
  //
  // The following invariant is maintained by the implementation:
  // end = buf + size, buf <= getpos < end, buf <= putpos < end,
  // putpos = getpos + used (modulo size).
  //
  // Note. The buffer area is automatically deallocated.

  class CircularBuffer {
  private:
    BYTE*  const buf;      // first position
    BYTE*  const end;      // = buf + size
    size_t const size;     // buffer size (in bytes)

    BYTE*  getpos;         // position of first byte to be read
    BYTE*  putpos;         // position of first byte to be written
    size_t used;           // amount of bytes between getpos and putpos

    // not allowed
    CircularBuffer(const CircularBuffer&)
      : buf(NULL), end(NULL), size(0), getpos(NULL), putpos(NULL), used(0) {}
    CircularBuffer& operator= (const CircularBuffer&) { return *this; }

  protected:
    void reinit() { getpos = putpos = buf; used = 0; }

    // correct a given position (provided buf <= pos < buf + 2*size)
    void checkPos(BYTE* &pos) { if (pos >= end) pos -= size; }

  public:
    CircularBuffer(BYTE* const &b, const size_t& sz)
      : buf(b), end(b + sz), size(sz), getpos(b), putpos(b), used(0) {}
    virtual ~CircularBuffer() { delete [] buf; }

    // space total/used/available
    size_t getSize() const { return size; }
    size_t getUsed() const { return used; }
    size_t getFree() const { return size - used; }

    // read/write a byte
    BYTE m_getByte() {
      BYTE b = *(getpos++); checkPos(getpos); used--; return b; }
    void m_putByte(const BYTE& b) {
      *(putpos++) = b; checkPos(putpos); used++; }

    // read/write integers (in little endian encoding format)
    int m_getInt() {
      int i = 0;
      for (int k=0; k < SIZE_INT; k++) i = i | (m_getByte() << (k*8));
      return i;
    }
    void m_putInt(int i) {
      for (int k=0; k < SIZE_INT; k++) { m_putByte((i & 0xFF)); i >>= 8; }
    }

    // modifies an int at a given position (without updating used)
    void m_putInt(BYTE* pos, int i) {
      for (int k=0; k < SIZE_INT; k++) {
        *(pos++) = i & 0xFF; checkPos(pos); i >>= 8;
      }
    }

    // read/write a block of data, without moving the position
    void m_read(BYTE* const &ptr, size_t const &len) const;
    void m_write(const BYTE* const &ptr, size_t const &len);

    // moves the read/write position forwards (or backwards if len<0)
    void m_commitRead(const int &len);
    void m_commitWrite(const int &len);

    // return read/write pointer
    BYTE* getReadPos() const { return getpos; }
    BYTE* getWritePos() const { return putpos; }

    // set 'pos' to read/write position, and return the maximum
    // available block size from that position in the buffer
    size_t getReadBlock(BYTE* &pos) const;
    size_t getWriteBlock(BYTE* &pos) const;
  };



  // a CircularBuffer dressed as a DssReadBuffer
  class DssReadByteBuffer : public DssReadBuffer, public CircularBuffer {
  private:
    size_t outerframe;     // available data outside frame

  public:
    DssReadByteBuffer(size_t const &sz)
      : CircularBuffer(new BYTE[sz], sz), outerframe(0) {}

    // set/reset frame size for user.  BEWARE!  The frame size must be
    // reset every time you write in the buffer!
    void setFrameSize(size_t const &sz) { outerframe = getUsed() - sz; }
    void resetFrame() { outerframe = 0; }

    // implementation of DssReadBuffer
    virtual size_t availableData() const;
    virtual bool canRead(size_t len) const {return this->availableData()>=len;}
    virtual void readFromBuffer(BYTE* ptr, size_t len);
    virtual void commitRead(size_t len);
    virtual const BYTE getByte();

    // decoding (see DssCryptoReadByteBuffer)
    virtual bool decode() { return true; }
  };



  // the crypto part of a DssReadByteBuffer
  class DssCryptoReadByteBuffer : public DssReadByteBuffer {
  private:
    DssReadByteBuffer* const databuffer;     // destination of decoding
    BlowFish                 crypto;         // encryption technique

  public:
    DssCryptoReadByteBuffer(BYTE* const key, const u32& keylen,
                            const u32& iv1,  const u32& iv2,
                            DssReadByteBuffer* const &buffer)
      : DssReadByteBuffer(buffer->getSize()), databuffer(buffer),
        crypto(key, keylen, iv1, iv2)
    {}

    // decode data to databuffer; returns true if no error occurred
    virtual bool decode();
  };



  // a CircularBuffer dressed as a DssWriteBuffer
  class DssWriteByteBuffer : public DssWriteBuffer, public CircularBuffer {
  private:
    size_t reserved;     // amount of reserved space (in bytes)

  public:
    DssWriteByteBuffer(size_t const &sz)
      : CircularBuffer(new BYTE[sz], sz), reserved(0) {}

    // manages reserved space.  Don't forget to reset it after usage!
    void reserveSpace(int const &len) { reserved += len; }
    void resetReserve() { reserved = 0; }

    // implementation of DssWriteBuffer
    virtual size_t availableSpace() const;
    virtual bool canWrite(size_t len) const {return this->availableSpace()>=len;}
    virtual void writeToBuffer(const BYTE* ptr, size_t len);
    virtual void putByte(const BYTE& b);

    // coding (see DssCryptoWriteByteBuffer)
    virtual void encode() {}
  };



  // the crypto part of a DssWriteBuffer
  class DssCryptoWriteByteBuffer : public DssWriteByteBuffer {
  private:
    DssWriteByteBuffer* const databuffer;     // source of encoding
    BlowFish                  crypto;         // encryption technique

  public:
    DssCryptoWriteByteBuffer(BYTE* const key, const u32& keylen,
                             const u32& iv1,  const u32& iv2,
                             DssWriteByteBuffer* const &buffer)
      : DssWriteByteBuffer(buffer->getSize()), databuffer(buffer),
        crypto(key, keylen, iv1, iv2)
    {}

    // encode data from data buffer
    virtual void encode();
  };

} //End namespace
#endif
