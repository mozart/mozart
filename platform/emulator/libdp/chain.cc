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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE)
#pragma implementation "chain.hh"
#endif

#include "dpBase.hh"
#include "base.hh"
#include "dsite.hh"
#include "chain.hh"

/**********************************************************************/
/*   Memory management                                               */
/**********************************************************************/

ChainElem *newChainElem(){
  return (ChainElem*) genFreeListManager->getOne_3();}

void freeChainElem(ChainElem* e){
  genFreeListManager->putOne_3((FreeListEntry*) e);}

Chain *newChain(){
  return (Chain*) genFreeListManager->getOne_4();}

void freeChain(Chain* e){
  genFreeListManager->putOne_4((FreeListEntry*) e);}

InformElem* newInformElem(){
  return (InformElem*) genFreeListManager->getOne_3();}

void freeInformElem(InformElem* e){
  genFreeListManager->putOne_3((FreeListEntry*) e);}

/**********************************************************************/
/*   Basic                                              */
/**********************************************************************/

void ChainElem::init(DSite *s){
  next=NULL;
  flags=0;
  site=s;}

Bool Chain::basicSiteExists(ChainElem *ce,DSite* s){
  while(ce!=NULL){
    if(ce->site==s) {return OK;}
    ce=ce->next;}
  return NO;}

ChainElem** Chain::getFirstNonGhostBase(){
  if(first==last) {return &first;}
  ChainElem **ce=&first;
  while((*ce)->next->flagIsSet(CHAIN_GHOST)){
    ce= &((*ce)->next);}
  return ce;}

void Chain::makeGhost(ChainElem* ce){
  ce->setFlagAndCheck(CHAIN_GHOST);
  ce->resetFlagAndCheck(CHAIN_QUESTION_ASKED);
  Assert(0);
  /* PER-HANDLE
  if(hasFlag(INTERESTED_IN_TEMP)){
    deinstallProbe(ce->site,PROBE_TYPE_ALL);}
  else{
  deinstallProbe(ce->site,PROBE_TYPE_PERM);} */
}

void Chain::removeBefore(DSite* s){
  ChainElem **base,*ce;
  base=getFirstNonGhostBase();
  Assert(siteExists(s));
  while(((*base)->site!=s) || (*base)->flagIsSet(CHAIN_DUPLICATE)){
    if((*base)->flagIsSet(CHAIN_QUESTION_ASKED)){
      makeGhost(*base);
      base=&((*base)->next);}
    else{
      removeNextChainElem(base);}}}

ChainElem *Chain::findAfter(DSite *s){
  Assert(siteExists(s));
  if(first->next==NULL){
    return NULL;}
  ChainElem *ce=getFirstNonGhost();
  while(ce->site!=s){
    ce=ce->next;}
  return ce->next;}

Bool Chain::removeGhost(DSite* s){
  ChainElem **ce=&first;
  while(TRUE){
    if((*ce)==NULL) return NO;
    if(!(*ce)->flagIsSet(CHAIN_GHOST)) return NO;
    if((*ce)->site==s) {
      removeNextChainElem(ce);
      return OK;}
    ce = &((*ce)->next);}}

Bool Chain::tempConnectionInChain(){
  ChainElem *ce=first;
  while(ce!=NULL){
    if(ce->site->siteStatus()==SITE_TEMP) return OK;
    ce=ce->next;}
  return NO;}

void Chain::probeTemp(Tertiary* t){
  PD((CHAIN,"Probing Temp"));
  ChainElem *ce=getFirstNonGhost();
  while(ce!=NULL){
    tertiaryInstallProbe(ce->site,PROBE_TYPE_ALL,t);
    deinstallProbe(ce->site,PROBE_TYPE_PERM);
    ce=ce->next;}}

void Chain::deProbeTemp(){
  PD((CHAIN,"DeProbing Temp"));
  ChainElem *ce=getFirstNonGhost();
  while(ce!=NULL){
    installProbe(ce->site,PROBE_TYPE_PERM);
    deinstallProbe(ce->site,PROBE_TYPE_ALL);
    ce=ce->next;}}

//
void Chain::removeInformOnPerm(DSite *s)
{ Assert(0); }
void Chain::informHandle(OwnerEntry* oe,int OTI,EntityCond ec)
{ Assert(0); }
void Chain::informHandleTempOnAdd(OwnerEntry* oe,Tertiary *t,DSite *s)
{ Assert(0); }

/*
// PER-LOOK
void Chain::removeInformOnPerm(DSite *s){
  InformElem **ce= &inform;
  InformElem *tmp;
  while(*ce!=NULL){
    if((*ce)->site==s){
      tmp=*ce;
      releaseInformElem(tmp);
      *ce=tmp->next;
      return;}
    ce= &((*ce)->next);}}

void Chain::informHandle(OwnerEntry* oe,int OTI,EntityCond ec){
  Assert(somePermCondition(ec));
  InformElem **base=&inform;
  InformElem *cur=*base;
  while(cur!=NULL){
    if(cur->watchcond & ec){
      sendTellError(oe,cur->site,OTI,cur->watchcond & ec,TRUE);
      *base=cur->next;
      freeInformElem(cur);
      cur=*base;
      continue;}
    base=&(cur->next);
    cur=*base;}}

void Chain::informHandleTempOnAdd(OwnerEntry* oe,Tertiary *t,Site *s){
Assert(ch->tempConnectionInChain());
  InformElem *ie=getInform();
  while(ie!=NULL){
    if(ie->site==s){
      EntityCond ec=ie->wouldTrigger(TEMP_BLOCKED|TEMP_SOME|TEMP_ME);
      if(ec!=ENTITY_NORMAL){
        sendTellError(oe,s,t->getIndex(),ec,TRUE);}}
    ie=ie->next;}}

*/

void Chain::init(DSite *s){
  ChainElem *e=newChainElem();
  e->init(s);
  inform = NULL;
  first=last=e;}

DSite* Chain::setCurrent(DSite* s, Tertiary* t){
  ChainElem *e=newChainElem();
  e->init(s);
  DSite *toS=last->site;
  last->next=e;
  last= e;
  if(s==myDSite){
    return toS;}
  ChainElem *de = getFirstNonGhost();
  if(de->site==s){
    de->setFlagAndCheck(CHAIN_DUPLICATE);}
  /* PER-HANDLE
  if(hasFlag(INTERESTED_IN_TEMP)){
    tertiaryInstallProbe(s,PROBE_TYPE_ALL,t);}
  else{
    tertiaryInstallProbe(s,PROBE_TYPE_PERM,t);}
  */
  return toS;}

void Chain::newInform(DSite* toS,EntityCond ec){
  InformElem *ie=newInformElem();
  ie->init(toS,ec);
  ie->next=inform;
  inform=ie;}

void Chain::releaseChainElem(ChainElem *ce){
  PD((CHAIN,"Releaseing Element"));
  if((!ce->flagIsSet(CHAIN_GHOST))){
    /* PER-HANDLE
    if(hasFlag(INTERESTED_IN_TEMP)){
      deinstallProbe(ce->getSite(),PROBE_TYPE_ALL);}
    else{
      deinstallProbe(ce->getSite(),PROBE_TYPE_PERM);}
    */
  }
  freeChainElem(ce);}

void Chain::removePerm(ChainElem** base){
  ChainElem *ce=*base;
  *base=ce->next;
  freeChainElem(ce);}

void Chain::removeNextChainElem(ChainElem** base){
  ChainElem *ce=*base;
  *base=ce->next;
  releaseChainElem(ce);}

/* PER-HANDLE
void Chain::releaseInformElem(InformElem *ie){
  EntityCond ec=ie->watchcond;
  if(someTempCondition(ie->watchcond)){
    InformElem *tmp=inform;
    int interestedInTemp=NO;
    while(tmp!=NULL){
      if(someTempCondition(tmp->watchcond)) {
        interestedInTemp=OK;
        break;}
      tmp=tmp->next;}
    if(!interestedInTemp){
      deProbeTemp();}}
  freeInformElem(ie);}
*/

void InformElem::init(DSite*s,EntityCond c){
  site=s;
  next=NULL;
  watchcond=c;
  foundcond=0;}


/**********************************************************************/
/*   gc                                                */
/**********************************************************************/

void Chain::gcChainSites(){
  ChainElem *ce=first;
  while(ce!=NULL){
    ce->site->makeGCMarkSite();
    ce=ce->next;}
  InformElem *ie=inform;
  while(ie!=NULL){
    ie->site->makeGCMarkSite();
    ie=ie->next;}}

/**********************************************************************/
/*   failure                                                */
/**********************************************************************/

void Chain::receiveUnAsk(DSite* s,EntityCond ec)
{ Assert(0); }
void Chain::receiveAnswer(Tertiary* t,DSite* site,int ans,DSite* deadS)
{ Assert(0); }

/* PER-HANDLE

void Chain::receiveUnAsk(DSite* s,EntityCond ec){
  InformElem **ie=&inform;
  InformElem *tmp;
  while(*ie!=NULL){
    if(((*ie)->site==s) && ((*ie)->watchcond==ec)){
      tmp=*ie;
      *ie=tmp->next;
      releaseInformElem(tmp);
      break;}
    ie=&((*ie)->next);}
  PD((WEIRD,"unaskerror with no error"));
  return;}

void Chain::receiveAnswer(Tertiary* t,DSite* site,int ans,DSite* deadS){
  PD((ERROR_DET,"chain receive answer %d",ans));
  if(hasFlag(TOKEN_LOST)) return;
  if(removeGhost(site)) return;
  Assert(siteExists(site));
  ChainElem **base=getFirstNonGhostBase();
  ChainElem *dead,*answer;

  while(((*base)->site!=deadS) && ((*base)->site!=site)){
    base= &((*base)->next);}
  if((*base)->site==site){ //      order Answer-Dead
    PD((ERROR_DET,"chain receive answer - order answer-dead"));
    answer=*base;
    answer->resetFlagAndCheck(CHAIN_QUESTION_ASKED);
    dead=answer->next;
    if(dead->site!=deadS) {
      PD((ERROR_DET,"dead->site!=deadS"));
      Assert(answer==getFirstNonGhost());
      return;}
    if(answer->flagIsSet(CHAIN_DUPLICATE)){
      PD((ERROR_DET,"answer->flagIsSet(CHAIN_DUPLICATE)"));
      dead->setFlagAndCheck(CHAIN_PAST);
      managerSeesSitePerm(t,deadS);
      return;}
    if(ans==PAST_ME){
      PD((ERROR_DET,"ans==PAST_ME"));
      dead->setFlagAndCheck(CHAIN_PAST);
      managerSeesSitePerm(t,deadS);
      return;}
    PD((ERROR_DET,"Manager will receive CANT_PUT from %s",site->stringrep()));
    dead->setFlagAndCheck(CHAIN_CANT_PUT);
    dead->resetFlag(CHAIN_BEFORE);
    dead->resetFlag(CHAIN_PAST);
    PD((CHAIN,"%d",printChain(this)));
    return;}
  PD((ERROR_DET,"chain receive answer - order dead-answer"));
  dead= *base;                      // order Dead-Answer
  answer=dead->next;
  Assert(answer->site=site);
  Assert(ans==BEFORE_ME);
  answer->resetFlagAndCheck(CHAIN_QUESTION_ASKED);
  dead->setFlagAndCheck(CHAIN_BEFORE);
  managerSeesSitePerm(t,deadS);
  return;}

*/

/**********************************************************************/
/*   Debug                                                */
/**********************************************************************/

int printChain(Chain* chain){
  printf("Chain ### Flags: [");
  if(chain->hasFlag(INTERESTED_IN_OK))
    printf(" INTERESTED_IN_OK");
  if(chain->hasFlag(INTERESTED_IN_TEMP ))
    printf(" INTERESTED_IN_TEMP");
  if(chain->hasFlag( TOKEN_PERM_SOME))
    printf(" TOKEN_PERM_SOME");
  if(chain->hasFlag(TOKEN_LOST))
    printf(" TOKEN_LOST");
  printf("]\n");
  ChainElem* cp = chain->getFirst();
  while(cp!= chain->getLast()){
    Assert(cp!=NULL);
    printf("Elem  Flags: [ ");
    if(cp->flagIsSet(CHAIN_GHOST ))
       printf("CHAIN_GHOST ");
    if(cp->flagIsSet( CHAIN_QUESTION_ASKED))
       printf("CHAIN_QUESTION_ASKED ");
    if(cp->flagIsSet( CHAIN_BEFORE))
       printf("CHAIN_BEFORE ");
    if(cp->flagIsSet( CHAIN_PAST))
       printf("CHAIN_PAST ");
    if(cp->flagIsSet( CHAIN_CANT_PUT))
       printf("CHAIN_CANT_PUT ");
    if(cp->flagIsSet( CHAIN_DUPLICATE))
       printf("CHAIN_DUPLICATE ");
    printf("] %s\n",cp->getSite()->stringrep());
    cp = cp->getNext();}
  Assert(cp!=NULL);
  printf("Elem  Flags: [ ");
  if(cp->flagIsSet(CHAIN_GHOST ))
    printf(" CHAIN_GHOST");
  if(cp->flagIsSet( CHAIN_QUESTION_ASKED))
    printf(" CHAIN_QUESTION_ASKED");
  if(cp->flagIsSet( CHAIN_BEFORE))
    printf(" CHAIN_BEFORE");
  if(cp->flagIsSet( CHAIN_PAST))
    printf(" CHAIN_PAST");
  if(cp->flagIsSet( CHAIN_CANT_PUT))
    printf(" CHAIN_CANT_PUT");
  if(cp->flagIsSet( CHAIN_DUPLICATE))
    printf(" CHAIN_DUPLICATE");
  printf("] %s\n",cp->getSite()->stringrep());
  return 8;
}
