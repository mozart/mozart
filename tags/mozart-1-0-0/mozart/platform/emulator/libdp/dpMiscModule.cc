/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Erik Klintskog (erik@sics.se)
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

#include "base.hh"
#include "dpBase.hh"

#include "perdio.hh"
#include "table.hh"
#include "dpMarshaler.hh"

#include "builtins.hh"
#include "os.hh"
#include "space.hh"

#ifdef NEWMARSHALER
#include "newmarshaler.hh"
#endif

#ifdef VIRTUALSITES
#define USE_VS_MSGBUFFERS
#endif

// 
#ifdef USE_VS_MSGBUFFERS
#include "virtual.hh"
#endif

OZ_BI_define(BIcrash,0,0)   /* only for debugging */
{
  initDP();

  exit(1);  

  return PROCEED;
} OZ_BI_end


#ifdef DEBUG_PERDIO

OZ_BI_define(BIdvset,2,0)
{
  initDP();

  OZ_declareInt(0,what);
  OZ_declareInt(1,val);

  if (val) {
    DV->set(what);
  } else {
    DV->unset(what);
  }
  return PROCEED;
} OZ_BI_end

#else

OZ_BI_define(BIdvset,2,0)
{
  initDP();

  OZ_declareInt(0,what);
  OZ_declareInt(1,val);
  OZ_warning("has no effect - you must compile with DEBUG_PERDIO");
  return PROCEED;
} OZ_BI_end

#endif


/**********************************************************************/
/*   Misc Builtins                                            */
/**********************************************************************/

OZ_BI_define(BIslowNet,2,0)
{
  initDP();

  oz_declareIntIN(0,arg0);
  oz_declareIntIN(1,arg1);
#ifdef SLOWNET
  TSC_LATENCY = arg0;
  TSC_TOTAL_A = arg1;
  printf("New slownetvals ms:%d buff:%d \n", TSC_LATENCY, TSC_TOTAL_A);
#else
  printf("Slownet not installed\n");
#endif
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIclose,1,0)   
{
  oz_declareIntIN(0,time);
  dpExitWithTimer((unsigned int) time);
  osExit(0);
  return PROCEED;
} OZ_BI_end

//
// kost@: temporary (?) builtin for comparing performances of
// marshalling/unmarshalling stuff. It takes a data structure to be
// tested and a number of times to repeat. Before loop it does it once
// and checks correctness of the operation;
// 
// Here is essentially a copy of builtin's 'oz_eqeq()':
inline 
Bool oz_eqeq(TaggedRef Ain,TaggedRef Bin)
{
  // simulate a shallow guard
  am.trail.pushMark();
  am.setShallowHeapTop(heapTop);
  OZ_Return ret = oz_unify(Ain,Bin,(ByteCode*)1);
  am.setShallowHeapTop(NULL);

  if (ret == PROCEED) {
    if (am.trail.isEmptyChunk()) {
      am.trail.popMark();
      return (OK);
    }

    oz_reduceTrailOnEqEq();
    return (NO);
  }

  oz_reduceTrailOnFail();
  return (NO);
}

#ifdef NEWMARSHALER

//
OZ_BI_define(BImarshalerPerf,2,0)
{
  initDP();

  oz_declareIN(0, value);
  oz_declareIntIN(1, count);
  OZ_Term result;
  MsgBuffer *buf;
  unsigned int timeNow;
  int heapNow;

  // check marshaler (old one);
#ifdef USE_VS_MSGBUFFERS
  buf = getCoreVirtualMsgBuffer((DSite *) 0);
#else
  buf = getRemoteMsgBuffer((DSite *) 0);
#endif
  buf->marshalBegin();
  marshalTermRT(value, buf);
  buf->marshalEnd();
  buf->unmarshalBegin();
  result = unmarshalTermRT(buf);
  buf->unmarshalEnd();
#ifdef USE_VS_MSGBUFFERS
  dumpVirtualMsgBufferImpl(buf);
#else
  dumpRemoteMsgBuffer(buf);
#endif
  if (!oz_eqeq(value, result))
    return (oz_raise(E_ERROR, E_SYSTEM, "odd old marshaler", 0));
  // ... new one:
#ifdef USE_VS_MSGBUFFERS
  buf = getCoreVirtualMsgBuffer((DSite *) 0);
#else
  buf = getRemoteMsgBuffer((DSite *) 0);
#endif
  buf->marshalBegin();
  newMarshalTerm(value, buf);
  buf->marshalEnd();
  buf->unmarshalBegin();
  result = newUnmarshalTerm(buf);
  buf->unmarshalEnd();
#ifdef USE_VS_MSGBUFFERS
  dumpVirtualMsgBufferImpl(buf);
#else
  dumpRemoteMsgBuffer(buf);
#endif
  if (!oz_eqeq(value, result))
    return (oz_raise(E_ERROR, E_SYSTEM, "odd new marshaler", 0));

  // Now let's spin for a while. First, do marshaling speed:
  timeNow = osUserTime();
  heapNow = getUsedMemory();
  int i;
  for (i = 0; i < count; i++) {
#ifdef USE_VS_MSGBUFFERS
    buf = getCoreVirtualMsgBuffer((DSite *) 0);
#else
    buf = getRemoteMsgBuffer((DSite *) 0);
#endif
    buf->marshalBegin();
    marshalTermRT(value, buf);
    buf->marshalEnd();
#ifdef USE_VS_MSGBUFFERS
    dumpVirtualMsgBufferImpl(buf);
#else
    dumpRemoteMsgBuffer(buf);
#endif
  }
  fprintf(stdout, "old marshaler/marshaling:");
  printTime(stdout, "r: ", (osUserTime() - timeNow));
  printMem(stdout, ", h: ", (getUsedMemory() - heapNow)*KB);
  fprintf(stdout, "\n"); fflush(stdout);

  // ... new marshaler:
  timeNow = osUserTime();
  heapNow = getUsedMemory();
  for (i = 0; i < count; i++) {
#ifdef USE_VS_MSGBUFFERS
    buf = getCoreVirtualMsgBuffer((DSite *) 0);
#else
    buf = getRemoteMsgBuffer((DSite *) 0);
#endif
    buf->marshalBegin();
    newMarshalTerm(value, buf);
    buf->marshalEnd();
#ifdef USE_VS_MSGBUFFERS
    dumpVirtualMsgBufferImpl(buf);
#else
    dumpRemoteMsgBuffer(buf);
#endif
  }
  fprintf(stdout, "new marshaler/marshaling:");
  printTime(stdout, "r: ", (osUserTime() - timeNow));
  printMem(stdout, ", h: ", (getUsedMemory() - heapNow)*KB);
  fprintf(stdout, "\n"); fflush(stdout);

  // unmarshalling:
#ifdef USE_VS_MSGBUFFERS
  buf = getCoreVirtualMsgBuffer((DSite *) 0);
#else
  buf = getRemoteMsgBuffer((DSite *) 0);
#endif
  buf->marshalBegin();
  marshalTermRT(value, buf);
  buf->marshalEnd();
  //
  timeNow = osUserTime();
  heapNow = getUsedMemory();
  for (i = 0; i < count; i++) {
    buf->unmarshalBegin();
    result = unmarshalTermRT(buf);
    buf->unmarshalEnd();
  }
  fprintf(stdout, "old marshaler/unmarshaling:");
  printTime(stdout, "r: ", (osUserTime() - timeNow));
  printMem(stdout,", h: ", (getUsedMemory() - heapNow)*KB);
  fprintf(stdout, "\n"); fflush(stdout);
#ifdef USE_VS_MSGBUFFERS
  dumpVirtualMsgBufferImpl(buf);
#else
  dumpRemoteMsgBuffer(buf);
#endif

#ifdef USE_VS_MSGBUFFERS
  buf = getCoreVirtualMsgBuffer((DSite *) 0);
#else
  buf = getRemoteMsgBuffer((DSite *) 0);
#endif
  buf->marshalBegin();
  newMarshalTerm(value, buf);
  buf->marshalEnd();
  //
  timeNow = osUserTime();
  heapNow = getUsedMemory();
  for (i = 0; i < count; i++) {
    buf->unmarshalBegin();
    result = newUnmarshalTerm(buf);
    buf->unmarshalEnd();
  }
  fprintf(stdout, "new marshaler/unmarshaling:");
  printTime(stdout, "r: ", (osUserTime() - timeNow));
  printMem(stdout, ", h: ", (getUsedMemory() - heapNow)*KB);
  fprintf(stdout, "\n"); fflush(stdout);
  //
#ifdef USE_VS_MSGBUFFERS
  dumpVirtualMsgBufferImpl(buf);
#else
  dumpRemoteMsgBuffer(buf);
#endif

  //
  return (PROCEED);
} OZ_BI_end

#endif

/*
 * The builtin table
 */

#ifndef MODULES_LINK_STATIC

#include "modDPMisc-if.cc"

#endif
