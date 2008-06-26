/*
 *  Authors:
 *    Zacharias El Banna (zeb@sics.se)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Organization or Person (Year(s))
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
#ifndef __CRYPTO_HH
#define __CRYPTO_HH

#ifdef INTERFACE
#pragma interface
#endif

// comment this line to disable actual encryption
//#define CRYPTO_ENABLED


#include <gmp.h>
#include <string.h>
#include <stdio.h>
#include "mslBase.hh"

namespace _msl_internal{ //Start namespace

  // ******************************* MISC ******************************

  typedef u32 capability;

  void randomize_crypto(); // init this 'lib'

  u32  random_u32();

  void generate_garbage(BYTE* dest, size_t len);

  void free_crypto_mem(); // END

  // ******************************* RSA *******************************
  //
  // for now we just scramble data but later on we might need 'sign',
  // 'verify', etc. then the below structure is insufficient
  //
  // RSA in this implementation assumes 256 bits n and 128 bits p and
  // q. If changing these numbers or doing something that affect them,
  // the code has to be checked and verified again.

  const int LIMB_BYTES         = 4; // a gmp _mp_struct limb is X bytes
  const int LIMB_BITS          = 8 * LIMB_BYTES;

  const int PRIME_BITS         = 128; //  change this one

  const int CIPHER_BLOCK_BITS  = 2*PRIME_BITS;
  const int CIPHER_BLOCK_BYTES = CIPHER_BLOCK_BITS / 8;

  const int PLAIN_BLOCK_BITS   = CIPHER_BLOCK_BITS - LIMB_BITS; // each plain block is the size of a cipher block minus one limb
  const int PLAIN_BLOCK_BYTES  = PLAIN_BLOCK_BITS / 8;

  const int RSA_MARSHALED_REPRESENTATION = CIPHER_BLOCK_BYTES + 4;

  class RSA_public{
  protected:
    u32 e;
#ifdef CRYPTO_ENABLED
    mpz_t n;
#else
    BYTE *n;
#endif

    // encrypt or verify
    virtual void    scramble(BYTE* const output, BYTE* const input);
    virtual void de_scramble(BYTE* const output, BYTE* const input);

    inline RSA_public();

  private:
    RSA_public(const RSA_public&):e(0){}
    RSA_public& operator=(const RSA_public&){ return *this; }

  public:
    RSA_public(BYTE* const str, size_t len);
    virtual ~RSA_public();

    int encrypt_text(BYTE* const outbuf, BYTE* inbuf, size_t inlen);
    int decrypt_text(BYTE* outbuf,       BYTE* inbuf, size_t inlen);

    // calculate needed space according to number of frames produced
    // in 'encrypt_text' alt. 'decrypt_text'
    static u32 encrypt_space_needed(size_t plainlen);
    static u32 decrypt_space_needed(size_t cryptlen);

    // ***************** KEY COMPARISON ******************
    // we assume that e is almost always the same....

#ifdef CRYPTO_ENABLED
    bool operator==(const RSA_public& rsa){return (mpz_cmp(n,rsa.n) == 0); }
    bool operator> (const RSA_public& rsa){return (mpz_cmp(n,rsa.n) >  0); }
    bool operator< (const RSA_public& rsa){return (mpz_cmp(n,rsa.n) <  0); }
#else
    bool operator==(const RSA_public& rsa){return (memcmp(n,rsa.n,CIPHER_BLOCK_BYTES) == 0); }
    bool operator> (const RSA_public& rsa){return (memcmp(n,rsa.n,CIPHER_BLOCK_BYTES) >  0); }
    bool operator< (const RSA_public& rsa){return (memcmp(n,rsa.n,CIPHER_BLOCK_BYTES) <  0); }
#endif

    // pointer to static data, do not delete, in the future we might
    // want different sizes ont the key but for now we leave that to a
    // predefined size (i.e. then we would return the length of buf too)
    BYTE* getStringRep() const;
  };


  class RSA_private: public RSA_public{
  protected:
#ifdef CRYPTO_ENABLED
    mpz_t p,q,d;
    mpz_t p_1,q_1,dp,dq,u;  // helpers for CRT
#endif

    // decrypt or sign
    virtual void    scramble(BYTE* const output, BYTE* const input);
    virtual void de_scramble(BYTE* const output, BYTE* const input);
  public:

    RSA_private();
    virtual ~RSA_private();

  };


  // *********************** Blowfish **************************

  // This implementation requires the buffer to be transmitted in 64b =
  // 8byte blocks, thus the transmitter must pad... the last data


  // Fix so that we can take other than full 64 bit frames...
  class BlowFish{
  private:
    u32 s[4][256]; // S-boxes
    u32 p[18]; // = 16rounds + 2, P-boxes

    // CBC data
    u32 e_prev1;
    u32 e_prev2;
    u32 d_prev1;
    u32 d_prev2;

    inline void readblock(u32& d1, u32& d2, BYTE* const inbuf);
    inline void writeblock(const u32& d1, const u32& d2, BYTE* const outbuf);

    inline void do_encrypt(u32& xl, u32& xr);
    inline void do_decrypt(u32& xl, u32& xr);
  public:
    BlowFish(BYTE* const key, const u32& keylen,
             const u32& d1, const u32& d2);
    ~BlowFish(){};

    void encrypt(BYTE* const cipher, BYTE* const plain, size_t plainlen);
    void decrypt(BYTE* const plain, BYTE* const cipher, size_t cryptlen);

    bool  check_weak_key();
  };


  // **************************** HASH *****************************

  //****************** ADLER32 *****************
  //
  // 32 bit CRC
  //

  u32 adler32(const u32& adler, BYTE* buf, u32 len);

  // ******************* MD5 *******************
  //
  // 128 bit message digest
  //

  const u32 MD5_SIZE = 16;

  class MD5{
  private:
    u32  state[4];
    u32  count[2];
    BYTE buffer[64];

    inline void reset();
    void transform(BYTE* const  block);
  public:
    MD5();
    ~MD5(){};

    // 'digest' inputs until done when 'final' is invoked with the
    // destination for the digest (space for 16 bytes)

    void digest(BYTE* const input, size_t inputLen);
    void final(BYTE* const output);

    // smart functions - compare tests a buffer against a hash value
    // and returns equality
    bool compare(BYTE hash1[16], BYTE* const buf, size_t bufLen){
      BYTE hash2[16];
      digest(buf,bufLen);
      final(hash2);
      return (memcmp(hash1,hash2,16) == 0);
    }

    // compound of digest and final
    inline void compute_digest(BYTE* const output, BYTE* const input, size_t inputLen){
      digest(input, inputLen);
      final(output);
    }

  };


    extern MD5 md5;


}

#endif
