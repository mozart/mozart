/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Copyright:
 *    Erik Klintskog, 2002
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

#ifndef __ENGINE_INTERFACE_HH
#define __ENGINE_INTERFACE_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "base.hh"
#include "value.hh"
#include "dss_enums.hh"



/* Annotations */

struct Annotation {
  ProtocolName       pn : PN_NBITS;
  AccessArchitecture aa : AA_NBITS;
  RCalg              rc : RC_ALG_NBITS;
};

const Annotation emptyAnnotation = { PN_NO_PROTOCOL,
				     AA_NO_ARCHITECTURE,
				     RC_ALG_NONE };

inline
Annotation makeAnnotation(ProtocolName pn, AccessArchitecture aa, RCalg rc) {
  Annotation annot = { pn, aa, rc };
  return annot;
}

void setAnnotation(TaggedRef, Annotation);
Annotation getAnnotation(TaggedRef);



/* DP initialization */

void initDP(int port, int ip, int id);

#endif



