/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __CHAINHH
#define __CHAINHH

#ifdef INTERFACE  
#pragma interface
#endif


enum ChainElemCode{
    CHAIN_GHOST=1,
    CHAIN_QUESTION_ASKED=2,
    CHAIN_BEFORE=4,
    CHAIN_PAST=8,
    CHAIN_CANT_PUT=16,
    CHAIN_DUPLICATE=32};

class ChainElem{
friend class Chain;
friend class InformElem;
protected:
  Site *site;
  ChainElem* next;
  unsigned int flags;
public:
  ChainElem(){Assert(0);}
  
  Site *getSite(){ return site;}

  void init(Site *s);

  Bool flagIsSet(ChainElemCode cec){return cec & flags;}

  void setFlag(ChainElemCode cec){flags |= cec;}
  void resetFlag(ChainElemCode cec){flags &= ~cec;}
  ChainElem *getNext(){return next;}
  void removeNext(Bool,Bool);

  void setFlagAndCheck(ChainElemCode cec){
    Assert(!flagIsSet(cec));
    setFlag(cec);}

  void resetFlagAndCheck(ChainElemCode cec){
    Assert(flagIsSet(cec));
    resetFlag(cec);}
};

class InformElem{
friend class Chain;
protected:
  InformElem *next;
  Site *site;
  short unsigned int watchcond;
  short unsigned int foundcond;
public:
  InformElem(){Assert(0);}

  void init(Site* s,EntityCond c);

  EntityCond wouldTrigger(EntityCond c){
    EntityCond ec= ((watchcond & c) & (~(foundcond)));
    foundcond |= ec;
    return ec;}

  EntityCond wouldTriggerOK(EntityCond c){
    EntityCond ec= (foundcond & c);
    foundcond &= ~ec;
    return ec;}
};

enum ChainFlags{
  INTERESTED_IN_OK=1,
  INTERESTED_IN_TEMP=2,
  TOKEN_PERM_SOME=8,
  TOKEN_LOST=4};

enum ChainAnswer{
  BEFORE_ME=1,
  AT_ME=2,
  PAST_ME=4};

class Chain{
protected:
  ChainElem* first;  
  ChainElem* last;
  InformElem* inform;
  unsigned int flags;
  
public:
  Chain(){Assert(0);}

  void init(Site*);

  void setFlagAndCheck(int f){
    Assert(!hasFlag(f));
    setFlag(f);}

  void resetFlagAndCheck(int f){
    Assert(hasFlag(f));
    resetFlag(f);}

  void setFlag(int f){
    flags |= f;}

  void resetFlag(int f){
    flags &= ~f;}

  Bool hasFlag(int f){
    return f & flags;}

  Site* getCurrent(){
    Assert(last != NULL);
    return last->site;}

  Site* setCurrent(Site*, Tertiary*);

  Bool hasInform(){
    return inform!=NULL;}

  InformElem *getInform(){return inform;}
  
  /* Just for debugging, should be ifdeffat  */
  ChainElem* getFirst();
  ChainElem* getLast();
  

  void receiveAnswer(Tertiary*,Site*,int,Site*); // messages 
  void receiveUnAsk(Site*,EntityCond);
  void managerSeesSitePerm(Tertiary*,Site* ); // probe
  void managerSeesSiteTemp(Tertiary*,Site* );
  void managerSeesSiteOK(Tertiary*,Site* );
  Bool removeGhost(Site*);                    // manipulating list of chain elements
  void removeBefore(Site*);
  void removePerm(ChainElem**);                
  void removeNextChainElem(ChainElem** base);
  void probeTemp(Tertiary*);                          // for all manipulation of list of chain elements
  void deProbeTemp();
  Bool basicSiteExists(ChainElem*,Site*);    // accessing list of chain elements
  Bool siteExists(Site*);
  Bool siteOrGhostExists(Site*);
  ChainElem **getFirstNonGhostBase();
  ChainElem *findChainElemFrom(ChainElem*,Site*);
  ChainElem *getFirstNonGhost();
  ChainElem *findAfter(Site*);
  Bool tempConnectionInChain();
  void makeGhost(ChainElem*);                // manipulating one chain element
  void releaseChainElem(ChainElem*);         
  void newInform(Site*,EntityCond);          // manipulating list of inform elements
  void removeInformOnPerm(Site*);
  void informHandle(OwnerEntry*,int,EntityCond);
  void informHandleTempOnAdd(OwnerEntry*,Tertiary*,Site*s);
  void dealWithTokenLostBySite(OwnerEntry*,int,Site*);
  void releaseInformElem(InformElem*);       // manipulating one inform element
  void gcChainSites();                       // gc
  void shortcutCrashLock(LockManager*);       // shortcutting chain methods
  void shortcutCrashCell(CellManager*,TaggedRef);

  Bool siteOfInterest(Site* s){
    if(inform!=NULL) return OK;
    ChainElem *tmp=first;
    while(tmp!=NULL){
      if(tmp->site==s) return OK;
      tmp=tmp->next;}
    return NO;}
  void handleTokenLost(OwnerEntry*,int);


};

/* __CHAINHH */
#endif 




