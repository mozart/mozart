/*
 *  Authors:
 *    Erik Klintskog (erikd@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 1998
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
#include "referenceConsistency.hh"

#ifndef __FRAC_WRC_HH
#define __FRAC_WRC_HH
class EnumDenumPair;

class FracHandler{
public:
  EnumDenumPair *frac;
  EnumDenumPair *findPair(int k);
  Bool insertPair(int e, int k);
  EnumDenumPair *findLargest();
};

class RRinstance_WRC:public RRinstance{
public:
  int enumerator;
  int denominator;

  RRinstance_WRC(RRinstance *n);
  RRinstance_WRC(int e, int d, RRinstance *n);
  void marshal_RR(MarshalerBuffer *buf);
  void unmarshal_RR(MarshalerBuffer *buf);
  virtual ~RRinstance_WRC(){}
};


class WRC: public GCalgorithm, public FracHandler
{
public:
  RRinstance *getBigReference(RRinstance *in);
  RRinstance *getSmallReference(RRinstance *in);
  Bool mergeReference(RRinstance* tmp);
  Bool isGarbage();

  OZ_Term extract_info(OZ_Term in);
  OZ_Term extract_OzId();
  
  WRC(HomeReference *p,GCalgorithm *g);
  WRC(RemoteReference *p, RRinstance *r,GCalgorithm *g);
  void remove();
  void dropReference(DSite* site, int index);
  Bool isRoot();
  virtual ~WRC(){};
};


#endif // __FRAC_WRC_HH


