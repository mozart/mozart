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
#ifdef INTERFACE
#pragma implementation "msl_crypto.hh"
#endif

#include "msl_crypto.hh"
#include "mslBase.hh"
#include <string.h>
#include "dss_templates.hh"

namespace _msl_internal{ //Start namespace

#ifdef STAND_ALONE_TEST
  // ******************************** MISC **************************************

  template<class T>
  void t_swap(T& t1, T& t2){
    T tmp = t1; t1 = t2; t2 = tmp;
  }
  
  inline void gf_printBuf(BYTE* const buf, size_t len){
    for(unsigned int i = 0; i< len; ++i){
      printf("%02x",static_cast<int>(buf[i]));
    }
    printf("\n");
  }

  // Create an integer2char, taking int and BYTE*
  inline void gf_integer2char(BYTE* const buf, const u32& e){
#ifdef BIG_ENDIAN_HOST
    buf[0] = e, buf[1] = (e >>  8), buf[2] = (e >> 16), buf[3] = (e >> 24);
#else
    *reinterpret_cast<u32*>(buf) = e;
#endif
  }
  
  inline void gf_char2integer(u32& e, BYTE* const buf){
#ifdef BIG_ENDIAN_HOST
    e = buf[3] << 24 | buf[2] << 16 | buf[1] << 8 | buf[0];
#else
    e = *reinterpret_cast<u32*>(buf);
#endif
  }
  inline int gf_char2integer(BYTE* const buf){
#ifdef BIG_ENDIAN_HOST
    return buf[3] << 24 | buf[2] << 16 | buf[1] << 8 | buf[0];
#else
    return *reinterpret_cast<u32*>(buf);
#endif
  }
  
#endif //STAND_ALONE_TEST


  inline void gf_printMPZ(mpz_t p){
    printf("S:%d A:%d: ",p->_mp_size, p->_mp_alloc);
    for (int i = 0; i < p->_mp_size; ++i)
      printf("%08x",static_cast<int>(p->_mp_d[i]));    
    printf(" :\n");
  }


  // ******************************** MISC **************************************
  namespace{
 
    
    const int NO_OF_PRIMES=109;
    // r * k = x * 10,  (-1 since we don't have 2)
    const u32 primes[NO_OF_PRIMES] = {
               3,     5,     7,    11,    13,    17,    19,    23,    29,
       31,    37,    41,    43,    47,    53,    59,    61,    67,    71,
       73,    79,    83,    89,    97,   101,   103,   107,   109,   113,
      127,   131,   137,   139,   149,   151,   157,   163,   167,   173,
      179,   181,   191,   193,   197,   199,   211,   223,   227,   229,
      233,   239,   241,   251,   257,   263,   269,   271,   277,   281,
      283,   293,   307,   311,   313,   317,   331,   337,   347,   349,
      353,   359,   367,   373,   379,   383,   389,   397,   401,   409,
      419,   421,   431,   433,   439,   443,   449,   457,   461,   463,
      467,   479,   487,   491,   499,   503,   509,   521,   523,   541,
      547,   557,   563,   569,   571,   577,   587,   593,   599,   601,
    };

  // MEMORY: Don't forget to clear random state when exiting
    const  int N_LIMBS = 32;
    static mpz_t s_rand_limbs;
    static gmp_randstate_t s_state;
    static mpz_t s_cipher;
    static mpz_t s_plain;
    static mpz_t s_a;
    static mpz_t s_b;
    static mpz_t s_v;
    
  }

  MD5 md5;


  void
  randomize_crypto(){
    //get unsigned seed from random device
    FILE* fp;
    u32 seed(0);
    BYTE bytes[4];
    fp = fopen("/dev/random","r");
    for (unsigned int i=0; i < 4; ++i){ bytes[i] = fgetc(fp); }
    seed = gf_char2integer(bytes);
    fclose(fp);
    gmp_randinit_default(s_state);
    gmp_randseed_ui(s_state,seed);

    mpz_init2(s_rand_limbs, N_LIMBS*32);
    mpz_urandomb(s_rand_limbs, s_state, N_LIMBS*32);

    mpz_init(s_a);
    mpz_init(s_b);
    mpz_init(s_v);
    mpz_init2(s_cipher,CIPHER_BLOCK_BITS); mpz_urandomb(s_cipher, s_state, CIPHER_BLOCK_BITS);
    mpz_init2(s_plain ,CIPHER_BLOCK_BITS); mpz_urandomb(s_plain , s_state, CIPHER_BLOCK_BITS);
    s_plain->_mp_size  = PLAIN_BLOCK_BYTES  / LIMB_BYTES;
    s_cipher->_mp_size = CIPHER_BLOCK_BYTES / LIMB_BYTES;
  }

  void
  free_crypto_mem(){
    mpz_clear(s_rand_limbs);
    gmp_randclear(s_state);
    mpz_clear(s_cipher);
    mpz_clear(s_plain);
    mpz_clear(s_v);
    mpz_clear(s_b);
    mpz_clear(s_a);

  }



  u32
  random_u32(){
    static int pos;
    pos = (++pos) % N_LIMBS;
    if(pos != 0)
      return static_cast<u32>(s_rand_limbs->_mp_d[pos]);
    // else get new random vector
    mpz_urandomb(s_rand_limbs, s_state, N_LIMBS*32);
    return static_cast<u32>(s_rand_limbs->_mp_d[0]);
  }


  // SEC-TODO: FIX THIS WHEN GOING REAL!!!

  void 
  generate_garbage(BYTE* dest, size_t len){
    if(len > 0){
      BYTE* end = (dest + len);
      while(dest + 4 <= end){
	*reinterpret_cast<int*>(dest) = random_u32();
	dest += 4;
      }
      while(dest < end){ // rest just fill with "S-BOX" values
	*dest = static_cast<char>(0xFF);
	++dest;
      }
    }
  }


  void
  generate_prime(mpz_t rop, const u32& bits){
    mpz_t modulo;
    mpz_init2(modulo,32);
  random:
    // generate odd random number
    mpz_urandomb(rop,s_state,bits);
    mpz_setbit(rop,0);
    mpz_setbit(rop,(bits-1)); //ensure length
    for(int i=0; i < NO_OF_PRIMES; ++i){
      if(mpz_mod_ui(modulo,rop,primes[i]) == 0) goto random;// divisible -> retry
    }
    // ok now Rabin-Miller (use built in of gmp)
    if(mpz_probab_prime_p(rop,10) == 0) goto random; //retry
    mpz_clear(modulo);
  }


  inline void mp_move(mpz_t rop, BYTE* inp, int n_bytes){
    memcpy(reinterpret_cast<BYTE*>(&(rop->_mp_d[0])),inp, n_bytes);
    
    //int n_limbs = n_bytes / LIMB_BYTES;
    //Assert(rop->_mp_size == n_limbs);
    //for (int i = 0; i < n_limbs; ++i)
    //  static_cast<int>(rop->_mp_d[i]) = gf_char2integer(inp + i*LIMB_BYTES);
  }

  inline void mp_move(BYTE* out, mpz_t rop, int n_bytes){
    memcpy(out, reinterpret_cast<BYTE*>(&(rop->_mp_d[0])), n_bytes);

    //int n_limbs = n_bytes / LIMB_BYTES;
    //Assert(rop->_mp_size == n_limbs);
    //for (int i = 0; i < n_limbs; ++i)
    //  gf_integer2char((out + i*LIMB_BYTES), static_cast<int>(rop->_mp_d[i]));
  }


  // ************************************* RSA ************************************
  //

  inline u32 frame_est(size_t len){
    return ((len + 4)/ PLAIN_BLOCK_BYTES) + ((((len + 4)% PLAIN_BLOCK_BYTES) > 0) ? 1 : 0);
  }

  u32
  RSA_public::encrypt_space_needed(size_t input){
    return CIPHER_BLOCK_BYTES * frame_est(input);
  }


  u32
  RSA_public::decrypt_space_needed(size_t input){
    Assert(input % CIPHER_BLOCK_BYTES == 0);
    return (input / CIPHER_BLOCK_BYTES * PLAIN_BLOCK_BYTES);
  }


  // ******************************* PUBLIC ******************************

  RSA_public::RSA_public():e(65537){
    //n = new BYTE[CIPHER_BLOCK_BYTES];
    mpz_init2(n, CIPHER_BLOCK_BITS);
  }


  RSA_public::RSA_public(BYTE* const str, size_t len): e(0){
    e = gf_char2integer(str);
    //n = new BYTE[CIPHER_BLOCK_BYTES];
    //memcpy(n,str,len - 4);

    int limbs = len/4 - 1;
    mpz_init2(n,limbs*LIMB_BITS);
    n->_mp_size = limbs;   // set number used and..
    for (int i = 0; i < limbs; ++i){
      (n->_mp_d[i] = gf_char2integer(&str[4*(i+1)])); // ...fill them
    }
  }

  RSA_public::~RSA_public(){
    //delete [] n;
    mpz_clear(n); 
  }

  BYTE*
  RSA_public::getStringRep() const{
    static BYTE buf[RSA_MARSHALED_REPRESENTATION];

    //    e +    limbs
    Assert(4 + n->_mp_size * 4 == RSA_MARSHALED_REPRESENTATION);
    gf_integer2char(buf,e);
    for (int i = 0; i < n->_mp_size; ++i){
      gf_integer2char(&buf[4*(i+1)],n->_mp_d[i]);
    }

    //gf_integer2char(buf,e);
    //memcpy(buf+4,n,CIPHER_BLOCK_BYTES);

    return buf;
  }


  RSA_private::RSA_private():
    RSA_public(){

    mpz_init2(p,  PRIME_BITS);   mpz_init2(q,  PRIME_BITS);
    mpz_init2(dp, PRIME_BITS);   mpz_init2(dq, PRIME_BITS);
    mpz_init2(p_1,PRIME_BITS + LIMB_BITS);   mpz_init2(q_1,PRIME_BITS + LIMB_BITS);
    mpz_init2(u,  PRIME_BITS);   mpz_init2(d,  CIPHER_BLOCK_BITS);
    
    generate_prime(p, PRIME_BITS); // p
    generate_prime(q, PRIME_BITS); // q
    mpz_sub_ui(p_1, p, 1);  mpz_sub_ui(q_1, q, 1);
    
    mpz_mul(n,p,q); // n
    
    // public key
    while ( mpz_gcd_ui(NULL,p_1,e) != 1 || mpz_gcd_ui(NULL,q_1,e) != 1 ) e += 2;
    
    mpz_mul(u, p_1, q_1);
    mpz_set_ui(dp,e);
    mpz_invert(d, dp, u); // d, e, ((p-1)*(q-1)) , here we could test that mpz_inverts doesn't return 0
    // .. d is now private key
    
    
    //cached values: CRT
    mpz_invert(u,q,p);
    mpz_mod(dp,d,p_1);
    mpz_mod(dq,d,q_1);

    //generate_garbage(n,CIPHER_BLOCK_BYTES);
  }

  RSA_private::~RSA_private(){
    mpz_clear(p);
    mpz_clear(q);
    mpz_clear(d);
    mpz_clear(p_1);
    mpz_clear(q_1);
    mpz_clear(dp);
    mpz_clear(dq);
    mpz_clear(u);
  }


  void
  RSA_public::scramble(BYTE* const output, BYTE* const input){
    mp_move(s_plain,input, PLAIN_BLOCK_BYTES);
    mpz_powm_ui(s_cipher,s_plain,e,n);
    mp_move(output, s_cipher, CIPHER_BLOCK_BYTES);
    
    //memcpy(output,input,PLAIN_BLOCK_BYTES);
    //gf_integer2char((output+PLAIN_BLOCK_BYTES), random_u32());
  }
  void
  RSA_public::de_scramble(BYTE* const output, BYTE* const input){
    mp_move(s_cipher,input, CIPHER_BLOCK_BYTES);
    mpz_powm_ui(s_plain,s_cipher,e,n);
    mp_move(output, s_plain, PLAIN_BLOCK_BYTES);

    //memcpy(output,input,PLAIN_BLOCK_BYTES);
  }


  void
  RSA_private::scramble(BYTE* const output, BYTE* const input){
    mp_move(s_plain, input, PLAIN_BLOCK_BYTES);
    // Apply CRT
    mpz_mod( s_v, s_plain, p);
    mpz_powm(s_a, s_v, dp, p);
    mpz_mod( s_v, s_plain, q);
    mpz_powm(s_b, s_v, dq, q);
  
    if(mpz_cmp(s_a,s_b) < 0) // a < b
      mpz_add(s_a,s_a,p);
  
    //  x = b + q * ( ((a-b)*u) % p );
    mpz_sub(s_v,s_a,s_b); //   (a-b)
    mpz_mul(s_v,s_v,  u); //   (a-b) * u
    mpz_mod(s_v,s_v,  p); //  ((a-b) * u) % p
    mpz_mul(s_v,s_v,  q); // (((a-b) * u) % p ) * q
    
    mpz_add(s_cipher ,s_v, s_b); // (((a-b) * u) % p ) * p + b
    mp_move(output, s_cipher, CIPHER_BLOCK_BYTES);
 
    //memcpy(output,input,PLAIN_BLOCK_BYTES);
    //gf_integer2char((output + PLAIN_BLOCK_BYTES), random_u32());
  }
  void
  RSA_private::de_scramble(BYTE* const output, BYTE* const input){
    mp_move(s_cipher,input, CIPHER_BLOCK_BYTES);
    // Apply CRT
    mpz_mod( s_v, s_cipher, p);
    mpz_powm(s_a, s_v, dp, p);
    mpz_mod( s_v, s_cipher, q);
    mpz_powm(s_b, s_v, dq, q);
  
    if(mpz_cmp(s_a,s_b) < 0) // a < b
      mpz_add(s_a,s_a,p);
  
    //  x = b + q * ( ((a-b)*u) % p );
    mpz_sub(s_v,s_a,s_b); //   (a-b)
    mpz_mul(s_v,s_v,  u); //   (a-b) * u
    mpz_mod(s_v,s_v,  p); //  ((a-b) * u) % p
    mpz_mul(s_v,s_v,  q); // (((a-b) * u) % p ) * q
    
    mpz_add(s_plain ,s_v, s_b); // (((a-b) * u) % p ) * p + b
    mp_move(output, s_plain, PLAIN_BLOCK_BYTES);

    // memcpy(output,input,PLAIN_BLOCK_BYTES);
  }

  
  
  // ****************** ENCRYPT TEXT *******************
  
  // requires an output buffer that is (input_size * 8 / 7 + 4)
  // and rounded upwards to nearest divisible with 32
  
  // algorithm: Insert the size into the first 4 BYTEs (plain),
  // break the text into 224 bit frames and encrypt to 256 bit
  // frames. Pad the last frame with garbage to complete the
  // frames. Run the last block structure through 'encrypt'
  //
  // - possible TODO: use md5 and add signature in the front/end.
  
  // another idea is to store garbage in the first 4 BYTEs of every
  // frame.
  
  
  int
  RSA_public::encrypt_text(BYTE* const outbuf, BYTE* inbuf, size_t len){
    // requires a buffer that is 'frames' * 256 / 224....

    // break the text into 224 bit frames, insert the size into the
    // first 4 BYTEs. Pad the last frame with garbage to complete the
    // frames. Run the entire block structure through 'encrypt'

    // 28 * 8 = 224 => 28 BYTEs per frame, add 4 since we store the
    // length too
    
    // another idea is to store garbage in the first 4 BYTEs of every frame

    //static int ctr;  ctr++;
    int frames  = frame_est(len);
    int padding = frames * PLAIN_BLOCK_BYTES - (len + 4);

    BYTE  plain[PLAIN_BLOCK_BYTES];
    BYTE* endptr = outbuf;
    BYTE* ptr    = plain;
    
    gf_integer2char(ptr, len); ptr += 4; // store length

    //printf("ENCRYPTION(%02d): %d data into %d frames and %d padding\n",ctr,len,frames,padding);
    //gf_printBuf("inbuf",inbuf,len);

    if (frames > 1){
      --frames;
      memcpy(ptr,inbuf, PLAIN_BLOCK_BYTES - 4); inbuf += (PLAIN_BLOCK_BYTES - 4);
      scramble(endptr,plain); endptr += CIPHER_BLOCK_BYTES;
      
      while(frames > 1){
	--frames;
	scramble(endptr,inbuf); inbuf += PLAIN_BLOCK_BYTES; endptr += CIPHER_BLOCK_BYTES;
      }
      memcpy(plain, inbuf, (PLAIN_BLOCK_BYTES - padding));
      ptr = plain + (PLAIN_BLOCK_BYTES - padding);
    } else {
      memcpy(ptr,inbuf,len); // just store data
      ptr +=len;
    }
    // last frame or single frame, if padding == 0 then this is a complete frame
    generate_garbage(ptr, padding);
    scramble(endptr,plain); endptr += CIPHER_BLOCK_BYTES;

    //gf_printBuf("outbuf",outbuf,(endptr - outbuf));
    return (endptr - outbuf);
  }


  int
  RSA_public::decrypt_text(BYTE* outbuf, BYTE* inbuf, size_t inlen){
    static int ctr;
    ctr++;
    int frames  = inlen / CIPHER_BLOCK_BYTES;
    int padding = 0;
    BYTE  plain[PLAIN_BLOCK_BYTES];
    BYTE* ptr = plain;

    de_scramble(plain,inbuf); inbuf += CIPHER_BLOCK_BYTES;

    u32 len = gf_char2integer(ptr); ptr +=4;

    padding = frames * PLAIN_BLOCK_BYTES - (len + 4);
    
    //printf("DECRYPTION(%02d) %d data, from %d buffer, into %d frames and %d padding\n",ctr, len,inlen,frames,padding);  gf_printBuf("inbuf",inbuf - CIPHER_BLOCK_BYTES,inlen);

    //test that it seems to contain correct length
    if (len > inlen || static_cast<u32>(frames) != frame_est(len)) return -1;

    if (frames > 1){
      --frames;
      memcpy(outbuf,ptr,PLAIN_BLOCK_BYTES-4); outbuf += (PLAIN_BLOCK_BYTES - 4);
      while(frames > 1){
	--frames;
	de_scramble(outbuf,inbuf); inbuf += CIPHER_BLOCK_BYTES; outbuf += PLAIN_BLOCK_BYTES;
      }
      de_scramble(plain,inbuf);
      memcpy(outbuf, plain, PLAIN_BLOCK_BYTES - padding);// outbuf += (PLAIN_BLOCK_BYTES - padding);
    } else {
      memcpy(outbuf, ptr, len);// outbuf += len;
    }
 
    return len;
  }



  // ********************************* BLOWFISH ******************************

  // This blowfish implementation favors intel processors, thus all BYTE
  // blocks (of 4) are little endian instead of big endian. i.e if
  // checking valid outputs against a table: just switch order of BYTEs
  // in an integer.

  // precomputed S boxes
  namespace{
    static const u32 ks[4][256] = {
      {
	0xD1310BA6,0x98DFB5AC,0x2FFD72DB,0xD01ADFB7,0xB8E1AFED,0x6A267E96,
	0xBA7C9045,0xF12C7F99,0x24A19947,0xB3916CF7,0x0801F2E2,0x858EFC16,
	0x636920D8,0x71574E69,0xA458FEA3,0xF4933D7E,0x0D95748F,0x728EB658,
	0x718BCD58,0x82154AEE,0x7B54A41D,0xC25A59B5,0x9C30D539,0x2AF26013,
	0xC5D1B023,0x286085F0,0xCA417918,0xB8DB38EF,0x8E79DCB0,0x603A180E,
	0x6C9E0E8B,0xB01E8A3E,0xD71577C1,0xBD314B27,0x78AF2FDA,0x55605C60,
	0xE65525F3,0xAA55AB94,0x57489862,0x63E81440,0x55CA396A,0x2AAB10B6,
	0xB4CC5C34,0x1141E8CE,0xA15486AF,0x7C72E993,0xB3EE1411,0x636FBC2A,
	0x2BA9C55D,0x741831F6,0xCE5C3E16,0x9B87931E,0xAFD6BA33,0x6C24CF5C,
	0x7A325381,0x28958677,0x3B8F4898,0x6B4BB9AF,0xC4BFE81B,0x66282193,
	0x61D809CC,0xFB21A991,0x487CAC60,0x5DEC8032,0xEF845D5D,0xE98575B1,
	0xDC262302,0xEB651B88,0x23893E81,0xD396ACC5,0x0F6D6FF3,0x83F44239,
	0x2E0B4482,0xA4842004,0x69C8F04A,0x9E1F9B5E,0x21C66842,0xF6E96C9A,
	0x670C9C61,0xABD388F0,0x6A51A0D2,0xD8542F68,0x960FA728,0xAB5133A3,
	0x6EEF0B6C,0x137A3BE4,0xBA3BF050,0x7EFB2A98,0xA1F1651D,0x39AF0176,
	0x66CA593E,0x82430E88,0x8CEE8619,0x456F9FB4,0x7D84A5C3,0x3B8B5EBE,
	0xE06F75D8,0x85C12073,0x401A449F,0x56C16AA6,0x4ED3AA62,0x363F7706,
	0x1BFEDF72,0x429B023D,0x37D0D724,0xD00A1248,0xDB0FEAD3,0x49F1C09B,
	0x075372C9,0x80991B7B,0x25D479D8,0xF6E8DEF7,0xE3FE501A,0xB6794C3B,
	0x976CE0BD,0x04C006BA,0xC1A94FB6,0x409F60C4,0x5E5C9EC2,0x196A2463,
	0x68FB6FAF,0x3E6C53B5,0x1339B2EB,0x3B52EC6F,0x6DFC511F,0x9B30952C,
	0xCC814544,0xAF5EBD09,0xBEE3D004,0xDE334AFD,0x660F2807,0x192E4BB3,
	0xC0CBA857,0x45C8740F,0xD20B5F39,0xB9D3FBDB,0x5579C0BD,0x1A60320A,
	0xD6A100C6,0x402C7279,0x679F25FE,0xFB1FA3CC,0x8EA5E9F8,0xDB3222F8,
	0x3C7516DF,0xFD616B15,0x2F501EC8,0xAD0552AB,0x323DB5FA,0xFD238760,
	0x53317B48,0x3E00DF82,0x9E5C57BB,0xCA6F8CA0,0x1A87562E,0xDF1769DB,
	0xD542A8F6,0x287EFFC3,0xAC6732C6,0x8C4F5573,0x695B27B0,0xBBCA58C8,
	0xE1FFA35D,0xB8F011A0,0x10FA3D98,0xFD2183B8,0x4AFCB56C,0x2DD1D35B,
	0x9A53E479,0xB6F84565,0xD28E49BC,0x4BFB9790,0xE1DDF2DA,0xA4CB7E33,
	0x62FB1341,0xCEE4C6E8,0xEF20CADA,0x36774C01,0xD07E9EFE,0x2BF11FB4,
	0x95DBDA4D,0xAE909198,0xEAAD8E71,0x6B93D5A0,0xD08ED1D0,0xAFC725E0,
	0x8E3C5B2F,0x8E7594B7,0x8FF6E2FB,0xF2122B64,0x8888B812,0x900DF01C,
	0x4FAD5EA0,0x688FC31C,0xD1CFF191,0xB3A8C1AD,0x2F2F2218,0xBE0E1777,
	0xEA752DFE,0x8B021FA1,0xE5A0CC0F,0xB56F74E8,0x18ACF3D6,0xCE89E299,
	0xB4A84FE0,0xFD13E0B7,0x7CC43B81,0xD2ADA8D9,0x165FA266,0x80957705,
	0x93CC7314,0x211A1477,0xE6AD2065,0x77B5FA86,0xC75442F5,0xFB9D35CF,
	0xEBCDAF0C,0x7B3E89A0,0xD6411BD3,0xAE1E7E49,0x00250E2D,0x2071B35E,
	0x226800BB,0x57B8E0AF,0x2464369B,0xF009B91E,0x5563911D,0x59DFA6AA,
	0x78C14389,0xD95A537F,0x207D5BA2,0x02E5B9C5,0x83260376,0x6295CFA9,
	0x11C81968,0x4E734A41,0xB3472DCA,0x7B14A94A,0x1B510052,0x9A532915,
	0xD60F573F,0xBC9BC6E4,0x2B60A476,0x81E67400,0x08BA6FB5,0x571BE91F,
	0xF296EC6B,0x2A0DD915,0xB6636521,0xE7B9F9B6,0xFF34052E,0xC5855664,
	0x53B02D5D,0xA99F8FA1,0x08BA4799,0x6E85076A 
      },{
	0x4B7A70E9,0xB5B32944,0xDB75092E,0xC4192623,0xAD6EA6B0,0x49A7DF7D,
	0x9CEE60B8,0x8FEDB266,0xECAA8C71,0x699A17FF,0x5664526C,0xC2B19EE1,
	0x193602A5,0x75094C29,0xA0591340,0xE4183A3E,0x3F54989A,0x5B429D65,
	0x6B8FE4D6,0x99F73FD6,0xA1D29C07,0xEFE830F5,0x4D2D38E6,0xF0255DC1,
	0x4CDD2086,0x8470EB26,0x6382E9C6,0x021ECC5E,0x09686B3F,0x3EBAEFC9,
	0x3C971814,0x6B6A70A1,0x687F3584,0x52A0E286,0xB79C5305,0xAA500737,
	0x3E07841C,0x7FDEAE5C,0x8E7D44EC,0x5716F2B8,0xB03ADA37,0xF0500C0D,
	0xF01C1F04,0x0200B3FF,0xAE0CF51A,0x3CB574B2,0x25837A58,0xDC0921BD,
	0xD19113F9,0x7CA92FF6,0x94324773,0x22F54701,0x3AE5E581,0x37C2DADC,
	0xC8B57634,0x9AF3DDA7,0xA9446146,0x0FD0030E,0xECC8C73E,0xA4751E41,
	0xE238CD99,0x3BEA0E2F,0x3280BBA1,0x183EB331,0x4E548B38,0x4F6DB908,
	0x6F420D03,0xF60A04BF,0x2CB81290,0x24977C79,0x5679B072,0xBCAF89AF,
	0xDE9A771F,0xD9930810,0xB38BAE12,0xDCCF3F2E,0x5512721F,0x2E6B7124,
	0x501ADDE6,0x9F84CD87,0x7A584718,0x7408DA17,0xBC9F9ABC,0xE94B7D8C,
	0xEC7AEC3A,0xDB851DFA,0x63094366,0xC464C3D2,0xEF1C1847,0x3215D908,
	0xDD433B37,0x24C2BA16,0x12A14D43,0x2A65C451,0x50940002,0x133AE4DD,
	0x71DFF89E,0x10314E55,0x81AC77D6,0x5F11199B,0x043556F1,0xD7A3C76B,
	0x3C11183B,0x5924A509,0xF28FE6ED,0x97F1FBFA,0x9EBABF2C,0x1E153C6E,
	0x86E34570,0xEAE96FB1,0x860E5E0A,0x5A3E2AB3,0x771FE71C,0x4E3D06FA,
	0x2965DCB9,0x99E71D0F,0x803E89D6,0x5266C825,0x2E4CC978,0x9C10B36A,
	0xC6150EBA,0x94E2EA78,0xA5FC3C53,0x1E0A2DF4,0xF2F74EA7,0x361D2B3D,
	0x1939260F,0x19C27960,0x5223A708,0xF71312B6,0xEBADFE6E,0xEAC31F66,
	0xE3BC4595,0xA67BC883,0xB17F37D1,0x018CFF28,0xC332DDEF,0xBE6C5AA5,
	0x65582185,0x68AB9802,0xEECEA50F,0xDB2F953B,0x2AEF7DAD,0x5B6E2F84,
	0x1521B628,0x29076170,0xECDD4775,0x619F1510,0x13CCA830,0xEB61BD96,
	0x0334FE1E,0xAA0363CF,0xB5735C90,0x4C70A239,0xD59E9E0B,0xCBAADE14,
	0xEECC86BC,0x60622CA7,0x9CAB5CAB,0xB2F3846E,0x648B1EAF,0x19BDF0CA,
	0xA02369B9,0x655ABB50,0x40685A32,0x3C2AB4B3,0x319EE9D5,0xC021B8F7,
	0x9B540B19,0x875FA099,0x95F7997E,0x623D7DA8,0xF837889A,0x97E32D77,
	0x11ED935F,0x16681281,0x0E358829,0xC7E61FD6,0x96DEDFA1,0x7858BA99,
	0x57F584A5,0x1B227263,0x9B83C3FF,0x1AC24696,0xCDB30AEB,0x532E3054,
	0x8FD948E4,0x6DBC3128,0x58EBF2EF,0x34C6FFEA,0xFE28ED61,0xEE7C3C73,
	0x5D4A14D9,0xE864B7E3,0x42105D14,0x203E13E0,0x45EEE2B6,0xA3AAABEA,
	0xDB6C4F15,0xFACB4FD0,0xC742F442,0xEF6ABBB5,0x654F3B1D,0x41CD2105,
	0xD81E799E,0x86854DC7,0xE44B476A,0x3D816250,0xCF62A1F2,0x5B8D2646,
	0xFC8883A0,0xC1C7B6A3,0x7F1524C3,0x69CB7492,0x47848A0B,0x5692B285,
	0x095BBF00,0xAD19489D,0x1462B174,0x23820E00,0x58428D2A,0x0C55F5EA,
	0x1DADF43E,0x233F7061,0x3372F092,0x8D937E41,0xD65FECF1,0x6C223BDB,
	0x7CDE3759,0xCBEE7460,0x4085F2A7,0xCE77326E,0xA6078084,0x19F8509E,
	0xE8EFD855,0x61D99735,0xA969A7AA,0xC50C06C2,0x5A04ABFC,0x800BCADC,
	0x9E447A2E,0xC3453484,0xFDD56705,0x0E1E9EC9,0xDB73DBD3,0x105588CD,
	0x675FDA79,0xE3674340,0xC5C43465,0x713E38D8,0x3D28F89E,0xF16DFF20,
	0x153E21E7,0x8FB03D4A,0xE6E39F2B,0xDB83ADF7
      },{
	0xE93D5A68,0x948140F7,0xF64C261C,0x94692934,0x411520F7,0x7602D4F7,
	0xBCF46B2E,0xD4A20068,0xD4082471,0x3320F46A,0x43B7D4B7,0x500061AF,
	0x1E39F62E,0x97244546,0x14214F74,0xBF8B8840,0x4D95FC1D,0x96B591AF,
	0x70F4DDD3,0x66A02F45,0xBFBC09EC,0x03BD9785,0x7FAC6DD0,0x31CB8504,
	0x96EB27B3,0x55FD3941,0xDA2547E6,0xABCA0A9A,0x28507825,0x530429F4,
	0x0A2C86DA,0xE9B66DFB,0x68DC1462,0xD7486900,0x680EC0A4,0x27A18DEE,
	0x4F3FFEA2,0xE887AD8C,0xB58CE006,0x7AF4D6B6,0xAACE1E7C,0xD3375FEC,
	0xCE78A399,0x406B2A42,0x20FE9E35,0xD9F385B9,0xEE39D7AB,0x3B124E8B,
	0x1DC9FAF7,0x4B6D1856,0x26A36631,0xEAE397B2,0x3A6EFA74,0xDD5B4332,
	0x6841E7F7,0xCA7820FB,0xFB0AF54E,0xD8FEB397,0x454056AC,0xBA489527,
	0x55533A3A,0x20838D87,0xFE6BA9B7,0xD096954B,0x55A867BC,0xA1159A58,
	0xCCA92963,0x99E1DB33,0xA62A4A56,0x3F3125F9,0x5EF47E1C,0x9029317C,
	0xFDF8E802,0x04272F70,0x80BB155C,0x05282CE3,0x95C11548,0xE4C66D22,
	0x48C1133F,0xC70F86DC,0x07F9C9EE,0x41041F0F,0x404779A4,0x5D886E17,
	0x325F51EB,0xD59BC0D1,0xF2BCC18F,0x41113564,0x257B7834,0x602A9C60,
	0xDFF8E8A3,0x1F636C1B,0x0E12B4C2,0x02E1329E,0xAF664FD1,0xCAD18115,
	0x6B2395E0,0x333E92E1,0x3B240B62,0xEEBEB922,0x85B2A20E,0xE6BA0D99,
	0xDE720C8C,0x2DA2F728,0xD0127845,0x95B794FD,0x647D0862,0xE7CCF5F0,
	0x5449A36F,0x877D48FA,0xC39DFD27,0xF33E8D1E,0x0A476341,0x992EFF74,
	0x3A6F6EAB,0xF4F8FD37,0xA812DC60,0xA1EBDDF8,0x991BE14C,0xDB6E6B0D,
	0xC67B5510,0x6D672C37,0x2765D43B,0xDCD0E804,0xF1290DC7,0xCC00FFA3,
	0xB5390F92,0x690FED0B,0x667B9FFB,0xCEDB7D9C,0xA091CF0B,0xD9155EA3,
	0xBB132F88,0x515BAD24,0x7B9479BF,0x763BD6EB,0x37392EB3,0xCC115979,
	0x8026E297,0xF42E312D,0x6842ADA7,0xC66A2B3B,0x12754CCC,0x782EF11C,
	0x6A124237,0xB79251E7,0x06A1BBE6,0x4BFB6350,0x1A6B1018,0x11CAEDFA,
	0x3D25BDD8,0xE2E1C3C9,0x44421659,0x0A121386,0xD90CEC6E,0xD5ABEA2A,
	0x64AF674E,0xDA86A85F,0xBEBFE988,0x64E4C3FE,0x9DBC8057,0xF0F7C086,
	0x60787BF8,0x6003604D,0xD1FD8346,0xF6381FB0,0x7745AE04,0xD736FCCC,
	0x83426B33,0xF01EAB71,0xB0804187,0x3C005E5F,0x77A057BE,0xBDE8AE24,
	0x55464299,0xBF582E61,0x4E58F48F,0xF2DDFDA2,0xF474EF38,0x8789BDC2,
	0x5366F9C3,0xC8B38E74,0xB475F255,0x46FCD9B9,0x7AEB2661,0x8B1DDF84,
	0x846A0E79,0x915F95E2,0x466E598E,0x20B45770,0x8CD55591,0xC902DE4C,
	0xB90BACE1,0xBB8205D0,0x11A86248,0x7574A99E,0xB77F19B6,0xE0A9DC09,
	0x662D09A1,0xC4324633,0xE85A1F02,0x09F0BE8C,0x4A99A025,0x1D6EFE10,
	0x1AB93D1D,0x0BA5A4DF,0xA186F20F,0x2868F169,0xDCB7DA83,0x573906FE,
	0xA1E2CE9B,0x4FCD7F52,0x50115E01,0xA70683FA,0xA002B5C4,0x0DE6D027,
	0x9AF88C27,0x773F8641,0xC3604C06,0x61A806B5,0xF0177A28,0xC0F586E0,
	0x006058AA,0x30DC7D62,0x11E69ED7,0x2338EA63,0x53C2DD94,0xC2C21634,
	0xBBCBEE56,0x90BCB6DE,0xEBFC7DA1,0xCE591D76,0x6F05E409,0x4B7C0188,
	0x39720A3D,0x7C927C24,0x86E3725F,0x724D9DB9,0x1AC15BB4,0xD39EB8FC,
	0xED545578,0x08FCA5B5,0xD83D7CD3,0x4DAD0FC4,0x1E50EF5E,0xB161E6F8,
	0xA28514D9,0x6C51133C,0x6FD5C7E7,0x56E14EC4,0x362ABFCE,0xDDC6C837,
	0xD79A3234,0x92638212,0x670EFA8E,0x406000E0
      },{
	0x3A39CE37,0xD3FAF5CF,0xABC27737,0x5AC52D1B,0x5CB0679E,0x4FA33742,
	0xD3822740,0x99BC9BBE,0xD5118E9D,0xBF0F7315,0xD62D1C7E,0xC700C47B,
	0xB78C1B6B,0x21A19045,0xB26EB1BE,0x6A366EB4,0x5748AB2F,0xBC946E79,
	0xC6A376D2,0x6549C2C8,0x530FF8EE,0x468DDE7D,0xD5730A1D,0x4CD04DC6,
	0x2939BBDB,0xA9BA4650,0xAC9526E8,0xBE5EE304,0xA1FAD5F0,0x6A2D519A,
	0x63EF8CE2,0x9A86EE22,0xC089C2B8,0x43242EF6,0xA51E03AA,0x9CF2D0A4,
	0x83C061BA,0x9BE96A4D,0x8FE51550,0xBA645BD6,0x2826A2F9,0xA73A3AE1,
	0x4BA99586,0xEF5562E9,0xC72FEFD3,0xF752F7DA,0x3F046F69,0x77FA0A59,
	0x80E4A915,0x87B08601,0x9B09E6AD,0x3B3EE593,0xE990FD5A,0x9E34D797,
	0x2CF0B7D9,0x022B8B51,0x96D5AC3A,0x017DA67D,0xD1CF3ED6,0x7C7D2D28,
	0x1F9F25CF,0xADF2B89B,0x5AD6B472,0x5A88F54C,0xE029AC71,0xE019A5E6,
	0x47B0ACFD,0xED93FA9B,0xE8D3C48D,0x283B57CC,0xF8D56629,0x79132E28,
	0x785F0191,0xED756055,0xF7960E44,0xE3D35E8C,0x15056DD4,0x88F46DBA,
	0x03A16125,0x0564F0BD,0xC3EB9E15,0x3C9057A2,0x97271AEC,0xA93A072A,
	0x1B3F6D9B,0x1E6321F5,0xF59C66FB,0x26DCF319,0x7533D928,0xB155FDF5,
	0x03563482,0x8ABA3CBB,0x28517711,0xC20AD9F8,0xABCC5167,0xCCAD925F,
	0x4DE81751,0x3830DC8E,0x379D5862,0x9320F991,0xEA7A90C2,0xFB3E7BCE,
	0x5121CE64,0x774FBE32,0xA8B6E37E,0xC3293D46,0x48DE5369,0x6413E680,
	0xA2AE0810,0xDD6DB224,0x69852DFD,0x09072166,0xB39A460A,0x6445C0DD,
	0x586CDECF,0x1C20C8AE,0x5BBEF7DD,0x1B588D40,0xCCD2017F,0x6BB4E3BB,
	0xDDA26A7E,0x3A59FF45,0x3E350A44,0xBCB4CDD5,0x72EACEA8,0xFA6484BB,
	0x8D6612AE,0xBF3C6F47,0xD29BE463,0x542F5D9E,0xAEC2771B,0xF64E6370,
	0x740E0D8D,0xE75B1357,0xF8721671,0xAF537D5D,0x4040CB08,0x4EB4E2CC,
	0x34D2466A,0x0115AF84,0xE1B00428,0x95983A1D,0x06B89FB4,0xCE6EA048,
	0x6F3F3B82,0x3520AB82,0x011A1D4B,0x277227F8,0x611560B1,0xE7933FDC,
	0xBB3A792B,0x344525BD,0xA08839E1,0x51CE794B,0x2F32C9B7,0xA01FBAC9,
	0xE01CC87E,0xBCC7D1F6,0xCF0111C3,0xA1E8AAC7,0x1A908749,0xD44FBD9A,
	0xD0DADECB,0xD50ADA38,0x0339C32A,0xC6913667,0x8DF9317C,0xE0B12B4F,
	0xF79E59B7,0x43F5BB3A,0xF2D519FF,0x27D9459C,0xBF97222C,0x15E6FC2A,
	0x0F91FC71,0x9B941525,0xFAE59361,0xCEB69CEB,0xC2A86459,0x12BAA8D1,
	0xB6C1075E,0xE3056A0C,0x10D25065,0xCB03A442,0xE0EC6E0E,0x1698DB3B,
	0x4C98A0BE,0x3278E964,0x9F1F9532,0xE0D392DF,0xD3A0342B,0x8971F21E,
	0x1B0A7441,0x4BA3348C,0xC5BE7120,0xC37632D8,0xDF359F8D,0x9B992F2E,
	0xE60B6F47,0x0FE3F11D,0xE54CDA54,0x1EDAD891,0xCE6279CF,0xCD3E7E6F,
	0x1618B166,0xFD2C1D05,0x848FD2C5,0xF6FB2299,0xF523F357,0xA6327623,
	0x93A83531,0x56CCCD02,0xACF08162,0x5A75EBB5,0x6E163697,0x88D273CC,
	0xDE966292,0x81B949D0,0x4C50901B,0x71C65614,0xE6C6C7BD,0x327A140A,
	0x45E1D006,0xC3F27B9A,0xC9AA53FD,0x62A80F00,0xBB25BFE2,0x35BDD2F6,
	0x71126905,0xB2040222,0xB6CBCF7C,0xCD769C2B,0x53113EC0,0x1640E3D3,
	0x38ABBD60,0x2547ADF0,0xBA38209C,0xF746CE76,0x77AFA1C5,0x20756060,
	0x85CBFE4E,0x8AE88DD8,0x7AAAF9B0,0x4CF9AA7E,0x1948C25C,0x02FB8A8C,
	0x01C36AE4,0xD6EBE1F9,0x90D4F869,0xA65CDEA0,0x3F09252D,0xC208E69F,
	0xB74E6132,0xCE77E25B,0x578FDFE3,0x3AC372E6
      }
    };

    static const u32 ps[18] = {
      0x243F6A88,0x85A308D3,0x13198A2E,0x03707344,0xA4093822,0x299F31D0,
      0x082EFA98,0xEC4E6C89,0x452821E6,0x38D01377,0xBE5466CF,0x34E90C6C,
      0xC0AC29B7,0xC97C50DD,0x3F84D5B5,0xB5470917,0x9216D5D9,0x8979FB1B
    };

  }
  // block conversion
  inline void
  BlowFish::readblock(u32& d1, u32& d2, BYTE* const inbuf){
    d1 = gf_char2integer(&inbuf[0]);
    d2 = gf_char2integer(&inbuf[4]);
  }

  inline void
  BlowFish::writeblock(const u32& d1, const u32& d2, BYTE* const outbuf){
    gf_integer2char(&outbuf[0],d1);
    gf_integer2char(&outbuf[4],d2);
  }

  
#ifdef BIG_ENDIAN_HOST
#define F(x) ((( s[0][reinterpret_cast<BYTE*>(&x)[0]] + s[1][reinterpret_cast<BYTE*>(&x)[1]])	 \
		   ^ s[2][reinterpret_cast<BYTE*>(&x)[2]]) + s[3][reinterpret_cast<BYTE*>(&x)[3]] )
#else
#define F(x) ((( s[0][reinterpret_cast<BYTE*>(&x)[3]] + s[1][reinterpret_cast<BYTE*>(&x)[2]])	 \
		   ^ s[2][reinterpret_cast<BYTE*>(&x)[1]]) + s[3][reinterpret_cast<BYTE*>(&x)[0]] )
#endif
  
#define R(l,r,i)  l ^= p[(i)]; r ^= F(l);

  inline void
  BlowFish::do_encrypt(u32 &xl, u32 &xr ){
    for(int i=0; i < 16; i+=2){
      R( xl, xr, (i  ));
      R( xr, xl, (i+1));
    }
    xl ^= p[16];
    xr ^= p[17];
    t_swap(xl,xr);
  }

  inline void
  BlowFish::do_decrypt(u32& xl, u32& xr ){
    for(int i=16; i > 0; i-=2){
      R( xl, xr, (i + 1));
      R( xr, xl, (i));
    }
    xl ^= p[1];
    xr ^= p[0];
    t_swap(xl,xr);
  }

#undef F
#undef R


  // Assumes that outbuf is equally sized to inbuf
  // uses CBC, ie. XOR with previously encrypted block
  //
  // !! remember to pad the data such that it has full 64-bit "frames" !!
  // 

  void
  BlowFish::encrypt(BYTE* const cipher, BYTE* const plain, size_t len){
    u32 d1, d2;
    unsigned int j = len - len % 8;
    for(unsigned int i1 = 0; i1 < j; i1+=8){
      readblock(d1,d2,&(plain[i1]));
      e_prev1 ^= d1; e_prev2 ^= d2;
      do_encrypt(e_prev1, e_prev2);
      writeblock(e_prev1, e_prev2,&(cipher[i1]));
    }
    if(j != len){ // extra bits
      do_encrypt(e_prev1, e_prev2);
      for(int i2=0; j < len && i2 < 4; ++i2, ++j)
	cipher[j] = plain[j] ^ (e_prev1 << (8 * i2));
      for(int i3=0; j < len; ++j, ++i3)
	cipher[j] = plain[j] ^ (e_prev2 << (8 * i3));
    }
  }

  void
  BlowFish::decrypt(BYTE* const plain, BYTE* const cipher, size_t len){
    u32 d1, d2, od1, od2;
    unsigned int j = len - len % 8;
    for(unsigned int i = 0; i < j; i+=8){   
      readblock(od1, od2, &(cipher[i]));
      d1 = od1; d2 = od2;
      do_decrypt(d1, d2);
      writeblock((d1 ^ d_prev1), (d2 ^d_prev2), &(plain[i]));
      d_prev1 = od1; d_prev2 = od2;
    }
    if(j != len){ // extra bits
      do_encrypt(d_prev1, d_prev2);
      for(int i1=0; j < len && i1 < 4; ++j, ++i1)
	plain[j] = cipher[j] ^ (d_prev1 << (8 * i1));
      for(int i2=0; j < len; ++j, ++i2)
	plain[j] = cipher[j] ^ (d_prev2 << (8 * i2));
    }
  }




  // Create blowfish environment
  BlowFish::BlowFish(BYTE* const key, const u32& keylen,
		     const u32& d1, const u32& d2):
    e_prev1(d1),e_prev2(d2),d_prev1(d1),d_prev2(d2){
    int i=0, j=0;
    u32 data=0, datal=0, datar=0;
  
    // init
    for(i=0; i < 18; ++i)
      p[i] = ps[i];
    for(i=0; i < 4; ++i){
      memcpy(reinterpret_cast<BYTE*>(&s[i][0]),
	     reinterpret_cast<BYTE*>(const_cast<u32*>(&ks[i][0])),
	     1024);
    }
  
    // set up P- and S-boxes
    for(i=j=0; i < (18); i++ ) {
      // this one is special since the key might wrap in the middle of an integer
#ifdef BIG_ENDIAN_HOST
      reinterpret_cast<BYTE*>(&data)[0] = key[j];
      reinterpret_cast<BYTE*>(&data)[1] = key[(j+1)%keylen];
      reinterpret_cast<BYTE*>(&data)[2] = key[(j+2)%keylen];
      reinterpret_cast<BYTE*>(&data)[3] = key[(j+3)%keylen];
#else
      reinterpret_cast<BYTE*>(&data)[3] = key[j];
      reinterpret_cast<BYTE*>(&data)[2] = key[(j+1)%keylen];
      reinterpret_cast<BYTE*>(&data)[1] = key[(j+2)%keylen];
      reinterpret_cast<BYTE*>(&data)[0] = key[(j+3)%keylen];
#endif
      p[i] ^= data;
      j = (j+4) % keylen;
    }
  
    for(i=0; i < (18); i += 2 ) {
      do_encrypt(datal, datar );
      p[i]   = datal;
      p[i+1] = datar;
    }
    for(j = 0; j < 4; ++j)
      for(i=0; i < 256; i += 2 ) {
	do_encrypt(datal, datar );
	s[j][i]   = datal;
	s[j][i+1] = datar;
      }
  }


  bool
  BlowFish::check_weak_key(){
    // Check for weak key.  A weak key is a key in which a value in
    // the P-array occurs more than once per table.
    for(int i=0; i < 255; i++ ) {
      for(int j=i+1; j < 256; j++) {
	if( (s[0][i] == s[0][j]) || (s[1][i] == s[1][j]) ||
	    (s[2][i] == s[2][j]) || (s[3][i] == s[3][j]) )
	  return true;
      }
    }
    return false;
  }







  // ************************************* MD5 ***********************************


#define ROL(x, n)(((x) << (n)) | ((x) >> (32-(n))))
#define FF(b, c, d) (d ^ (b & (c ^ d)))
#define FG(b, c, d) FF (d, b, c)
#define FH(b, c, d) (b ^ c ^ d)
#define FI(b, c, d) (c ^ (b | ~d))

#define OP(f, a, b, c, d, k, s, T){ a += f(b, c, d) + x[k] + T; a = b + ROL(a, s); }

  inline void
  MD5::reset(){
    count[0] = count[1] = 0;
    state[0] = 0x67452301;
    state[1] = 0xefcdab89;
    state[2] = 0x98badcfe;
    state[3] = 0x10325476;
  }

  MD5::MD5(){
    reset();
  }


  void
  MD5::transform(BYTE* const block){
    u32 a = state[0], b = state[1], c = state[2], d = state[3], x[MD5_SIZE];
    // Move contents of block to x, putting bytes in little-endian order.
#ifdef BIG_ENDIAN_HOST
    for (int i = 0, int j = 0; i < MD5_SIZE; i++, j+= 4){ x[i] = gf_char2integer(block[j]); }
#else
    memcpy( x, block, 64 );
#endif
  
    // Round 1.
    OP (FF, a, b, c, d,  0,  7, 0xd76aa478);
    OP (FF, d, a, b, c,  1, 12, 0xe8c7b756);
    OP (FF, c, d, a, b,  2, 17, 0x242070db);
    OP (FF, b, c, d, a,  3, 22, 0xc1bdceee);
    OP (FF, a, b, c, d,  4,  7, 0xf57c0faf);
    OP (FF, d, a, b, c,  5, 12, 0x4787c62a);
    OP (FF, c, d, a, b,  6, 17, 0xa8304613);
    OP (FF, b, c, d, a,  7, 22, 0xfd469501);
    OP (FF, a, b, c, d,  8,  7, 0x698098d8);
    OP (FF, d, a, b, c,  9, 12, 0x8b44f7af);
    OP (FF, c, d, a, b, 10, 17, 0xffff5bb1);
    OP (FF, b, c, d, a, 11, 22, 0x895cd7be);
    OP (FF, a, b, c, d, 12,  7, 0x6b901122);
    OP (FF, d, a, b, c, 13, 12, 0xfd987193);
    OP (FF, c, d, a, b, 14, 17, 0xa679438e);
    OP (FF, b, c, d, a, 15, 22, 0x49b40821);
    // Round 2.
    OP (FG, a, b, c, d,  1,  5, 0xf61e2562);
    OP (FG, d, a, b, c,  6,  9, 0xc040b340);
    OP (FG, c, d, a, b, 11, 14, 0x265e5a51);
    OP (FG, b, c, d, a,  0, 20, 0xe9b6c7aa);
    OP (FG, a, b, c, d,  5,  5, 0xd62f105d);
    OP (FG, d, a, b, c, 10,  9, 0x02441453);
    OP (FG, c, d, a, b, 15, 14, 0xd8a1e681);
    OP (FG, b, c, d, a,  4, 20, 0xe7d3fbc8);
    OP (FG, a, b, c, d,  9,  5, 0x21e1cde6);
    OP (FG, d, a, b, c, 14,  9, 0xc33707d6);
    OP (FG, c, d, a, b,  3, 14, 0xf4d50d87);
    OP (FG, b, c, d, a,  8, 20, 0x455a14ed);
    OP (FG, a, b, c, d, 13,  5, 0xa9e3e905);
    OP (FG, d, a, b, c,  2,  9, 0xfcefa3f8);
    OP (FG, c, d, a, b,  7, 14, 0x676f02d9);
    OP (FG, b, c, d, a, 12, 20, 0x8d2a4c8a);

    // Round 3.
    OP (FH, a, b, c, d,  5,  4, 0xfffa3942);
    OP (FH, d, a, b, c,  8, 11, 0x8771f681);
    OP (FH, c, d, a, b, 11, 16, 0x6d9d6122);
    OP (FH, b, c, d, a, 14, 23, 0xfde5380c);
    OP (FH, a, b, c, d,  1,  4, 0xa4beea44);
    OP (FH, d, a, b, c,  4, 11, 0x4bdecfa9);
    OP (FH, c, d, a, b,  7, 16, 0xf6bb4b60);
    OP (FH, b, c, d, a, 10, 23, 0xbebfbc70);
    OP (FH, a, b, c, d, 13,  4, 0x289b7ec6);
    OP (FH, d, a, b, c,  0, 11, 0xeaa127fa);
    OP (FH, c, d, a, b,  3, 16, 0xd4ef3085);
    OP (FH, b, c, d, a,  6, 23, 0x04881d05);
    OP (FH, a, b, c, d,  9,  4, 0xd9d4d039);
    OP (FH, d, a, b, c, 12, 11, 0xe6db99e5);
    OP (FH, c, d, a, b, 15, 16, 0x1fa27cf8);
    OP (FH, b, c, d, a,  2, 23, 0xc4ac5665);

    // Round 4.
    OP (FI, a, b, c, d,  0,  6, 0xf4292244);
    OP (FI, d, a, b, c,  7, 10, 0x432aff97);
    OP (FI, c, d, a, b, 14, 15, 0xab9423a7);
    OP (FI, b, c, d, a,  5, 21, 0xfc93a039);
    OP (FI, a, b, c, d, 12,  6, 0x655b59c3);
    OP (FI, d, a, b, c,  3, 10, 0x8f0ccc92);
    OP (FI, c, d, a, b, 10, 15, 0xffeff47d);
    OP (FI, b, c, d, a,  1, 21, 0x85845dd1);
    OP (FI, a, b, c, d,  8,  6, 0x6fa87e4f);
    OP (FI, d, a, b, c, 15, 10, 0xfe2ce6e0);
    OP (FI, c, d, a, b,  6, 15, 0xa3014314);
    OP (FI, b, c, d, a, 13, 21, 0x4e0811a1);
    OP (FI, a, b, c, d,  4,  6, 0xf7537e82);
    OP (FI, d, a, b, c, 11, 10, 0xbd3af235);
    OP (FI, c, d, a, b,  2, 15, 0x2ad7d2bb);
    OP (FI, b, c, d, a,  9, 21, 0xeb86d391);

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

  }


  void
  MD5::digest(BYTE* const input, size_t inputLen){
    u32 index   = ((count[0] >> 3) & 0x3F); //    Compute number of bytes mod 64
    u32 partLen = 64 - index;
    // Update number of bits
    if ((count[0] += (inputLen << 3)) < (inputLen << 3))  count[1]++;  
    count[1] += (inputLen >> 29);

    int i = 0;
    if (inputLen >= partLen) {
      memcpy(&buffer[index], input, partLen);
      transform(buffer);
      for (i = static_cast<int>(partLen); i + 63 < static_cast<int>(inputLen); i += 64)
	transform(input + i);
      index = 0;
    }
    memcpy(&buffer[index], input + i, inputLen-i);
  }



  void
  MD5::final(BYTE* const output){
    BYTE bits[8];
    u32 index, padLen;
    BYTE PADDING[64] =  {
      0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    // Save number of bits
    gf_integer2char(bits  , count[0]);
    gf_integer2char(bits+4, count[1]);
    // Pad out to 56 mod 64.
    index = static_cast<u32>((count[0] >> 3) & 0x3f);
    padLen = (index < 56) ? (56 - index) : (120 - index);
    digest(PADDING, padLen);
    digest(bits, 8);
    for(int i=0; i < 4; ++i)
      gf_integer2char(output + 4*i , state[i]);
    reset();
  }


  /*
    
  Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
  claim that you wrote the original software. If you use this software
  in a product, an acknowledgment in the product documentation would be
  appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
  misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Jean-loup Gailly        Mark Adler
  jloup@gzip.org          madler@alumni.caltech.edu


  * adler32.c -- compute the Adler-32 checksum of a data stream
  * Copyright (C) 1995-2002 Mark Adler
  * For conditions of distribution and use, see copyright notice in zlib.h 
  *
  */

#define BASE 65521L /* largest prime smaller than 65536 */
#define NMAX 5552
  /* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

  u32 adler32(const u32& adler, BYTE* buf, u32 len){

    u32 s1 = adler & 0xffff;
    u32 s2 = (adler >> 16) & 0xffff;

    if (buf == NULL) return 1L;

    while (len > 0) {
      int k = len < NMAX ? len : NMAX;
      len -= k;
      while(k >= 16) {
	for(int i = 0; i < 16; ++i) { s1 += buf[i];  s2 += s1; }
	buf += 16;
	k -= 16;
      }
      while(k > 0) {
	s1 += *buf;
	s2 += s1;
	++buf;
	--k;
      }

      s1 %= BASE;
      s2 %= BASE;
    }

    return ((s2 << 16) | s1);
  }



}
