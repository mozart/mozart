/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
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

#ifndef __DSS_DCT_HH
#define __DSS_DCT_HH

#ifdef INTERFACE
#pragma interface
#endif

//#include "dssBase.hh"
#include "msl_buffer.hh"
#include "dss_templates.hh"
#include "dss_comService.hh"
namespace _msl_internal{

  enum DCT_Types{
    DctT_DAC
  };

  // ERIK, VALENTIN, ZACHARIAS: To ease development, please comment
  // what return values stand for, typically we use enumerations to
  // limit typed return values for data type tags.

  class DssCompoundTerm{
  public:
    // returns whether or not the DCT was entirely marshaled, i.e. true means done
    virtual bool marshal(DssWriteBuffer *bb,MsgnLayerEnv*) = 0;
    virtual bool unmarshal(DssReadBuffer *bb,MsgnLayerEnv* env) = 0;
    virtual ~DssCompoundTerm();
    virtual void dispose() = 0;
    virtual DCT_Types getType() const = 0;
    virtual void resetMarshaling() = 0;
  };

  // Used by the msgContainer when receiving a DCT
  DssCompoundTerm *createReceiveDCT(DCT_Types, MsgnLayerEnv*);



  // ****************************** BUFFER TRANSPORTING*********************************
  //
  // Transports data areas, perfectly suitable to match with the
  // simple buffers
  class DssSimpleDacDct: public DssCompoundTerm{
  private:
    enum DSDD_Mode{
      DSDD_UNDEF,
      DSDD_READ,
      DSDD_WRITE
    };

    BYTE*     a_buf;
    BYTE*     a_pos;
    u32       a_size;
    DSDD_Mode a_mode;

    inline u32 getPosDiff() const { return a_pos - a_buf; }

    DssSimpleDacDct(const DssSimpleDacDct&):
      a_buf(NULL),a_pos(NULL),a_size(0),a_mode(DSDD_UNDEF){}
    DssSimpleDacDct& operator=(const DssSimpleDacDct&){ return *this; }

  public:

    DssSimpleDacDct():a_buf(NULL),a_pos(NULL),a_size(0),a_mode(DSDD_UNDEF){
      //printf("DssSimpleDacDct(%p)::create empty\n",static_cast<void*>(this));
    }

    DssSimpleDacDct(const u32& sz, BYTE* const bf):a_buf(bf),a_pos(bf),a_size(sz),a_mode(DSDD_UNDEF){
      //printf("DssSimpleDacDct(%p)::create %d\n",static_cast<void*>(this),sz); gf_printBuf("buffer", a_buf,a_size);
    }

    virtual ~DssSimpleDacDct(){ delete [] a_buf; }


    u32 getSize() const { return a_size; }

    int  getData(BYTE*, const int&);
    void putData(BYTE*, const int&);

    BYTE* unhook(){ BYTE* tmp = a_buf; a_buf = a_pos = NULL; return tmp; }

    // *********** DctT **********

    virtual bool marshal(DssWriteBuffer *bb, MsgnLayerEnv*);
    virtual bool unmarshal(DssReadBuffer *bb,MsgnLayerEnv* env);

    virtual void dispose(){ delete this; };
    virtual DCT_Types getType() const { return DctT_DAC; };
    virtual void resetMarshaling();
  };
}
#endif
