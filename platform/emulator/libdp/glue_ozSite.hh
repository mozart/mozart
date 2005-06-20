/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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


#ifndef __GLUE_OZ_SITE_HH
#define __GLUE_OZ_SITE_HH

#include "base.hh"
#include "value.hh"
#include "glue_siteRepresentation.hh"

#define OZ_E_SITE  4211

class Oz_Site: public OZ_Extension
{
private:
  Glue_SiteRep *a_gSite; 
public: // From Oz_Extension 
  virtual int           getIdV(void);

  virtual OZ_Extension* gCollectV(void);
  virtual void          gCollectRecurseV(void);
  virtual OZ_Extension* sCloneV(void);
  virtual void          sCloneRecurseV(void);

  virtual OZ_Term       printV(int = 10) ;
  virtual OZ_Term       printLongV(int depth = 10, int offset = 0);
  virtual OZ_Term       typeV(void);
  virtual OZ_Boolean    isChunkV(void) { return OZ_TRUE; }
  virtual OZ_Term       getFeatureV(OZ_Term){;}
  virtual OZ_Return	getFeatureV(OZ_Term,OZ_Term&) { return OZ_FAILED; }
  virtual OZ_Return	putFeatureV(OZ_Term,OZ_Term ) { return OZ_FAILED; }
  virtual OZ_Return     eqV(OZ_Term);
  //
  virtual OZ_Boolean    toBePickledV() { return (OZ_FALSE); }
  virtual void          pickleV(MarshalerBuffer *mb, GenTraverser *gt) {}
  virtual OZ_Boolean    toBeMarshaledV() { return (OZ_TRUE); }
  virtual void          marshalSuspV(OZ_Term te,
				     ByteBuffer *bs, GenTraverser *gt);
  virtual int           minNeededSpace() { return (0); }
  
  void internalMarshal(DssWriteBuffer *buf);
  
  
  void setGSR(Glue_SiteRep*);
  Glue_SiteRep* getGSR(){ return a_gSite; }
};

void OzSite_init();
#endif
