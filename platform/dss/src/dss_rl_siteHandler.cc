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

  SiteHandler::SiteHandler() : a_siteList() {}

  bool SiteHandler::isEmpty() const {
    return a_siteList.isEmpty();
  }


  void SiteHandler::modifyDSite(DSite* site, int no){
    Position<Pair<DSite*, int> > pos(a_siteList);
    if (pos.find(site)) {
      // found site
      (*pos).second += no;     // decs < 0 => return of Site
      if ((*pos).second == 0) pos.remove();
    } else {
      // No site found, insert this one
      a_siteList.push(makePair(site, no));
    }
  }


  void SiteHandler::gcPreps() {
    //Markup sites and remove perms
    Position<Pair<DSite*, int> > pos(a_siteList);
    while (pos()) {
      if ((*pos).first->m_getFaultState() & FS_PERM == 0) {
        (*pos).first->m_makeGCpreps();
        pos++;
      } else {
        dssLog(DLL_BEHAVIOR,"RL: Removing failed site: %s",
               (*pos).first->m_stringrep());
        pos.remove();
      }
    }
  }

}
