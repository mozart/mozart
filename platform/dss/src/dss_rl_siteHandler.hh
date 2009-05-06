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
#ifndef __RL_SITE_HANDLER_HH
#define __RL_SITE_HANDLER_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "dss_templates.hh"
#include "dssBase.hh"

namespace _dss_internal{ // Start namespace

  //
  // The siteHandler takes care of dsites for the two RL versions
  //

  class SiteHandler{
  private:
    SimpleList<Pair<DSite*, int> > a_siteList;

    SiteHandler(const SiteHandler&):a_siteList(){}
    SiteHandler& operator=(const SiteHandler&){ return *this; }

  public:
    SiteHandler();
    bool isEmpty() const;

    void modifyDSite(DSite* site, int no);
    
    void insertDSite(DSite* site)          { modifyDSite(site,1); }
    void removeDSite(DSite* site, int dec) { modifyDSite(site,0-dec); }
    
    void gcPreps(); // Must be run before gc-ing of sites

    virtual ~SiteHandler() {}
  };
  
}

#endif
