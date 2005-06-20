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

#include "glue_ozSite.hh"
#include "pickle.hh"
#include "base.hh"
#include "glue_base.hh"
#include "glue_buffer.hh"
#include "glue_siteRepresentation.hh"
#include "glue_interface.hh"

// OZ_extension methods --------------------------------------------

int Oz_Site::getIdV(void) {
  return OZ_E_SITE;
}

OZ_Term Oz_Site::typeV(void) {
  return OZ_atom("site");
}

OZ_Return Oz_Site::eqV(OZ_Term t) {
  if (OZ_isExtension(t)) {
    OZ_Extension *e = OZ_getExtension(t);
    if (e->getIdV() == OZ_E_SITE) {
      Oz_Site *w = static_cast<Oz_Site *>(e);
      if (w->a_gSite == a_gSite)
	return PROCEED;
    }
  }
  return FAILED;
}

OZ_Term Oz_Site::printV(int depth) {
  return OZ_atom("<site>");
}

OZ_Term Oz_Site::printLongV(int depth, int offset) {
  return printV(depth); 
}

OZ_Extension *Oz_Site::gCollectV(void) {
  return new Oz_Site(*this);
}

OZ_Extension *Oz_Site::sCloneV(void) {
   printf("Should not clone address sites DSite!!\n"); 
   Assert(0); 
   return new Oz_Site(*this);
}

void Oz_Site::gCollectRecurseV(void) 
{
}

void Oz_Site::sCloneRecurseV(void) {}

void  Oz_Site::setGSR(Glue_SiteRep* gsa){
  a_gSite = gsa;
}

OZ_Boolean toBePickledV() { return (OZ_TRUE); }
void pickleV(MarshalerBuffer *bs, GenTraverser *gt) {
  // This should only be called when we explicit put a site reference into 
  // a pickle save. 
  
  // call the DSite and ask for serialization... 
  
}


// The idea goes as follows. The Oz-object calls the DSS object that will 
// create a serialized representation. The oz-object does thus not serialize
// a thing. It is completly done by the DSS. 
void Oz_Site::marshalSuspV(OZ_Term te, ByteBuffer *bs, GenTraverser *gt) {
  GlueWriteBuffer *gwb = static_cast<GlueWriteBuffer *>(bs);
  (a_gSite->m_getDssSite())->m_marshalDSite(gwb);
}


static
OZ_Term unmarshalOzSite(MarshalerBuffer *bs, Builder*)
{
  Assert(0); 
  return (UnmarshalEXT_Error);
}

static
OZ_Term suspUnmarshalOzSite(ByteBuffer *mb, Builder*,
			    GTAbstractEntity* &bae)
{
  GlueReadBuffer *grb = static_cast<GlueReadBuffer *>(mb); 
  DSite *ds = glue_com_connection->a_msgnLayer->m_UnmarshalDSite(grb); 
  Glue_SiteRep *gsa = static_cast< Glue_SiteRep *>(ds->m_getCsSiteRep());
  return gsa->m_getOzSite();
}

static
OZ_Term unmarshalOzSiteCont(ByteBuffer *mb, Builder*,
			    GTAbstractEntity* bae)
{
  Assert(0);
  return (UnmarshalEXT_Error);
}

void OzSite_init()
{
  static Bool done = NO;
  if (done == NO) {
    done = OK;
    oz_registerExtension(OZ_E_SITE, unmarshalOzSite,
			 suspUnmarshalOzSite, unmarshalOzSiteCont);
  }
}

//

