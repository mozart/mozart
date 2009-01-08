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
#if defined(INTERFACE)
#pragma implementation "msl_buffer.hh"
#endif

#include <string.h>
#include "msl_buffer.hh"
#include "dss_enums.hh"
#include "dss_templates.hh"

namespace _msl_internal{ //Start namespace

  // utils
  inline int min(const int &a, const int &b) { return (a <= b ? a : b); }



  /************************* CircularBuffer *************************/

  void
  CircularBuffer::m_read(BYTE* const &ptr, size_t const &len) const {
    Assert(len <= used);
    if (getpos + len <= end) {
      memcpy(ptr, getpos, len);
    } else {
      int chunk = end - getpos;     // 0 < chunk < len
      memcpy(ptr, getpos, chunk);
      memcpy(ptr + chunk, buf, len - chunk);
    }
  }

  void
  CircularBuffer::m_write(const BYTE* const &ptr, size_t const &len) {
    Assert(len <= size - used);
    if (putpos + len <= end) {
      memcpy(putpos, ptr, len);
    } else {
      int chunk = end - putpos;     // 0 < chunk < len
      memcpy(putpos, ptr, chunk);
      memcpy(buf, ptr + chunk, len - chunk);
    }
  }

  void
  CircularBuffer::m_commitRead(const int &len) {
    used -= len;
    getpos += (len >= 0 ? len : len + size); checkPos(getpos);
    if (used == 0) reinit();      // optimize representation
  }

  void
  CircularBuffer::m_commitWrite(const int &len) {
    used += len;
    putpos += (len >= 0 ? len : len + size); checkPos(putpos);
  }

  size_t
  CircularBuffer::getReadBlock(BYTE* &pos) const {
    pos = getpos;
    return min(getUsed(), end - getpos);
  }

  size_t
  CircularBuffer::getWriteBlock(BYTE* &pos) const {
    pos = putpos;
    return min(getFree(), end - putpos);
  }



  /************************* DssReadByteBuffer *************************/

  size_t DssReadByteBuffer::availableData() const {
    return getUsed() - outerframe;
  }

  void DssReadByteBuffer::readFromBuffer(BYTE* ptr, size_t len) {
    m_read(ptr, len);
  }

  void DssReadByteBuffer::commitRead(size_t len) {
    m_commitRead(len);
  }

  const BYTE DssReadByteBuffer::getByte() {
    return m_getByte();
  }



  /************************* DssWriteByteBuffer *************************/

  size_t DssWriteByteBuffer::availableSpace() const {
    return getFree() - reserved;
  }

  void DssWriteByteBuffer::writeToBuffer(const BYTE* ptr, size_t len) {
    m_write(ptr, len); m_commitWrite(len);
  }

  void DssWriteByteBuffer::putByte(const BYTE& b) {
    m_putByte(b);
  }



  /***** DssCryptoReadByteBuffer / DssCryptoWriteByteBuffer *****/

  // Encoding/decoding data
  //
  // This part of the code has to use frames.  Because encryption is
  // block-oriented, we should never try to decode a partial block...
  // For security reasons, we cannot leave a field giving the block
  // length unencrypted.  Therefore we encrypt fixed size frames of
  // 512 bytes.  Random padding is added if there is not enough data.
  //
  // The decrypted frame has the following format.  The first field
  // gives the actual data length in the second field.  The CRC is
  // computed on the first 508 bytes of the frame.
  //
  //  +-------------+---------------------+-------------+
  //  | data length | data (with padding) |     CRC     |
  //  +-------------+---------------------+-------------+
  //      4 bytes          504 bytes          4 bytes
  //
  // The only drawback of this method is that plain data must be
  // copied between the buffer and the decrypted frame.

  const size_t FRAME_SIZE = 512;
  const int DATA_SIZE  = 512 - 2 * SIZE_INT;

  static BYTE plain[FRAME_SIZE], cipher[FRAME_SIZE];
  static BYTE* plainData = plain + SIZE_INT;
  static BYTE* plainCrc  = plainData + DATA_SIZE;

  // encode data from databuffer
  void DssCryptoWriteByteBuffer::encode() {
    int len;                   // actual data length inside frame
    BYTE *pos, *dest;          // pointers in buffers

    while (getFree() >= FRAME_SIZE) {
      len = min(databuffer->getUsed(), DATA_SIZE);
      if (len <= 0) return;

      // write data length
      gf_integer2char(plain, len);

      // write data
      databuffer->m_read(plainData, len);
      databuffer->m_commitRead(len);

      // add random padding if necessary
      for (pos = plainData + len; pos < plainCrc; pos += SIZE_INT)
        gf_integer2char(pos, random_u32());

      // write CRC
      gf_integer2char(plainCrc, adler32(0, plain, SIZE_INT + DATA_SIZE));

      // encrypt frame (directly in buffer if possible)
      if (getWriteBlock(dest) < FRAME_SIZE) dest = cipher;
      crypto.encrypt(dest, plain, FRAME_SIZE);

      // finalize writing
      if (dest == cipher) m_write(cipher, FRAME_SIZE);
      m_commitWrite(FRAME_SIZE);
    }
  }

  // decode data to databuffer
  bool DssCryptoReadByteBuffer::decode() {
    int len;                   // actual data length inside frame
    BYTE *source;              // pointer in buffer

    while (getUsed() >= FRAME_SIZE) {
      // decrypt frame directly from buffer if possible
      if (getReadBlock(source) < FRAME_SIZE) {
        source = cipher; m_read(source, 512);
      }
      crypto.decrypt(plain, source, FRAME_SIZE);

      // check CRC
      if (adler32(0, plain, SIZE_INT + DATA_SIZE) != gf_char2integer(plainCrc))
        return false;

      // read data length, and check data buffer space
      len = gf_char2integer(plain);
      Assert(len >= 0);
      if (databuffer->getFree() < static_cast<size_t>(len)) return true;

      // finalize reading, and write data
      m_commitRead(FRAME_SIZE);
      databuffer->m_write(plainData, len);
      databuffer->m_commitWrite(len);
    }

    return true;     // everything went fine
  }

} //End namespace
