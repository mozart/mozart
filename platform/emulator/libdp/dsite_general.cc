/*
 *  Authors:
 *    Andreas Sundstrom (andreas@sics.se)
 *    Per Brand (perbrand@sics.se)
 *
 *  Contributors:
 *
 *  Copyright:
 *    1997-1998 Konstantin Popov
 *    1997 Per Brand
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

//---------------------------------------------------------------------
// General unmarshaling procedures included in dsite.cc
//---------------------------------------------------------------------
#ifdef ROBUST_UNMARSHALER
#else
#endif

static
#ifdef ROBUST_UNMARSHALER
DSite* unmarshalDSiteInternalRobust(MsgBuffer *buf, DSite *tryS, MarshalTag mt,
                                    int *error)
#else
DSite* unmarshalDSiteInternal(MsgBuffer *buf, DSite *tryS, MarshalTag mt)
#endif
{
  DSite *s;
  int hvalue = tryS->hashPrimary();
#ifdef ROBUST_UNMARSHALER
  int e1=NO, e2=NO;
#endif

  FindType rc = primarySiteTable->findPrimary(tryS,hvalue,s);
  switch(rc){
  case SAME: {
    PD((SITE,"unmarshalsite SAME"));
    if(mt==DIF_SITE_PERM){
      if(s->isPerm()){
        return s;}
      s->discoveryPerm();
      return s;}

    //
    if(mt == DIF_SITE_VI) {
      // Note: that can be GName, in which case we have to fetch
      // virtual info as well;
      if (s != myDSite) {
        Assert(s->remoteComm() || s->virtualComm());
        //
        // 's' could either have or have not virtual info (a master of
        // a group could be already remembered outside as a
        // non-virtual one);
        if (s->hasVirtualInfo()) {
          // should be the same...
          unmarshalUselessVirtualInfo(buf);
        } else {
          // haven't seen it (virtual info) yet;
#ifdef ROBUST_UNMARSHALER
          VirtualInfo *vi = unmarshalVirtualInfoRobust(buf, &e1);
#else
          VirtualInfo *vi = unmarshalVirtualInfo(buf);
#endif

          //
          if (!s->ActiveSite()) {
            if (myDSite->isInMyVSGroup(vi)) {
              s->makeActiveVirtual(vi);
            } else {
              s->makeActiveRemoteVirtual(vi);
            }
          } else {
            s->addVirtualInfoToActive(vi);
          }
        }
      } else {
        // my site is my site... already initialized;
        unmarshalUselessVirtualInfo(buf);
      }

      //
      return (s);
    }

    //
    Assert(mt == DIF_SITE);
    if(s != myDSite && !s->ActiveSite()) {
      s->makeActiveRemote();}
    return s;}

  case NONE:
    PD((SITE,"unmarshalsite NONE"));
    break;

  case I_AM_YOUNGER:{
    PD((SITE,"unmarshalsite I_AM_YOUNGER"));
    if(mt == DIF_SITE_VI) {
      unmarshalUselessVirtualInfo(buf);
    }
    int hvalue=tryS->hashSecondary();
    s=secondarySiteTable->findSecondary(tryS,hvalue);
    if(s){return s;}
    s = new DSite(tryS->getAddress(), tryS->getPort(), tryS->getTimeStamp(),
                  PERM_SITE);
    secondarySiteTable->insertSecondary(s,hvalue);
    return s;}

  case I_AM_OLDER:{
    PD((SITE,"unmarshalsite I_AM_OLDER"));
    primaryToSecondary(s,hvalue);
    break;}

  default: Assert(0);}

  // none

  // type is left blank here:
  s = new DSite(tryS->getAddress(), tryS->getPort(), tryS->getTimeStamp());
  primarySiteTable->insertPrimary(s,hvalue);

  //
  if(mt==DIF_SITE_PERM){
    PD((SITE,"initsite DIF_SITE_PERM"));
    s->initPerm();
    return s;}

  if(mt == DIF_SITE_VI) {
    PD((SITE,"initsite DIF_SITE_VI"));

    //
    // kost@ : fetch virtual info, which (among other things)
    // identifies the 'tryS's group of virtual sites;
#ifdef ROBUST_UNMARSHALER
    VirtualInfo *vi = unmarshalVirtualInfoRobust(buf, &e2);
#else
    VirtualInfo *vi = unmarshalVirtualInfo(buf);
#endif
    if (myDSite->isInMyVSGroup(vi))
      s->initVirtual(vi);
    else
      s->initRemoteVirtual(vi);
    return (s);
  }

  Assert(mt == DIF_SITE);
  PD((SITE,"initsite DIF_SITE"));
  s->initRemote();
#ifdef ROBUST_UNMARSHALER
  *error = e1 || e2;
#endif
  return s;
}
