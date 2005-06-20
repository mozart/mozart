/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 2004
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
#pragma implementation "dss_rl_siteHandler.hh"
#endif

#include "dss_rl_siteHandler.hh"
#include "dss_comService.hh"
namespace _dss_internal{

  
  // ******************************** SiteHandler2 ********************************

  SiteHandler::SiteHandler():a_siteList(NULL){};

  bool SiteHandler::isEmpty() const { 
    return (a_siteList == NULL);
  };


  void SiteHandler::modifyDSite(DSite* site, int no){
    TwoContainer<DSite,int>** tmpSite = &a_siteList;
    while((*tmpSite) != NULL){
      if ((*tmpSite)->a_contain1 != site){
	tmpSite = &((*tmpSite)->a_next);
      } else { // Found site
	(*tmpSite)->a_contain2 = (*tmpSite)->a_contain2 + no; // decs < 0 => return of Site
	if ((*tmpSite)->a_contain2 == 0){ // Delete it
	  TwoContainer<DSite,int>* tmpSiteDel = (*tmpSite);
	  (*tmpSite) = (*tmpSite)->a_next;
	  delete tmpSiteDel;
	}
	return;
      }
    }
    // No site found, insert this one
    a_siteList = new TwoContainer<DSite,int>(site, no, a_siteList);
  }


  void SiteHandler::gcPreps(){ //Markup sites and remove perms
    TwoContainer<DSite,int>** tmpSite = &a_siteList;
    while ((*tmpSite) != NULL){
      if((*tmpSite)->a_contain1->m_getFaultState() != DSite_GLOBAL_PRM){
	(*tmpSite)->m_makeGCpreps();
	tmpSite = &((*tmpSite)->a_next);
      } else {
	dssLog(DLL_BEHAVIOR,"RL: Removeing failed site: %s",(*tmpSite)->a_contain1->m_stringrep());
	TwoContainer<DSite,int>* tmpSiteDel = (*tmpSite);
	(*tmpSite) = (*tmpSite)->a_next;
	delete tmpSiteDel;
      }
    }
  }

  SiteHandler::~SiteHandler(){
    t_deleteList(a_siteList);
  }

  

}
