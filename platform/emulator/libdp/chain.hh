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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
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
  DSite *site;
  ChainElem* next;
  unsigned int flags;
public:
  ChainElem(){Assert(0);}
  
  DSite *getSite(){ return site;}

  void init(DSite *s);

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
  DSite *site;
  short unsigned int watchcond;
  short unsigned int foundcond;
public:
  InformElem(){Assert(0);}

  void init(DSite* s,EntityCond c);

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

  void init(DSite*);

  void setFlagAndCheck(int f){
    Assert(!hasFlag(f));
    setFlag(f);}

  void resetFlagAndCheck(int f){
    Assert(hasFlag(f));
    resetFlag(f);}

  void setFlag(int f){flags |= f;}
  void resetFlag(int f){flags &= ~f;}
  Bool hasFlag(int f){return f & flags;}
    
  DSite* getCurrent(){
    Assert(last != NULL);
    return last->site;}

  DSite* setCurrent(DSite*, Tertiary*);

  Bool hasInform(){
    return inform!=NULL;}

  InformElem *getInform(){return inform;}
  
  ChainElem* getFirst(){return first;}
  ChainElem* getLast(){return last;}

  // failure 
  Bool basicSiteExists(ChainElem*,DSite*); // accessing list of chain elements
  Bool siteExists(DSite* s) {return basicSiteExists(getFirstNonGhost(),s);}
  ChainElem **getFirstNonGhostBase();
  ChainElem *getFirstNonGhost() {return *getFirstNonGhostBase();}
  void makeGhost(ChainElem*);  
  void releaseChainElem(ChainElem*);         
  void removePerm(ChainElem**);                
  void removeBefore(DSite*);
  ChainElem *findAfter(DSite*);
  Bool removeGhost(DSite*);  
  Bool tempConnectionInChain();
  void removeNextChainElem(ChainElem** base);
  void newInform(DSite*,EntityCond); 
  void gcChainSites();                  
  void receiveAnswer(Tertiary*,DSite*,int,DSite*); 
  Bool siteOfInterest(DSite*);
  void managerSeesSitePerm(Tertiary*,DSite* ); // probe
  void managerSeesSiteTemp(Tertiary*,DSite* );
  void managerSeesSiteOK(Tertiary*,DSite* );
  void removeInformOnPerm(DSite*);
  void dealWithTokenLostBySite(OwnerEntry*,int,DSite*);
  void shortcutCrashLock(LockManager*);       // shortcutting chain methods
  void shortcutCrashCell(CellManager*,TaggedRef);
  void handleTokenLost(OwnerEntry*,int);
  void informHandle(OwnerEntry*,int,EntityCond);
  void probeTemp(Tertiary*);
  void deProbeTemp();
  void receiveUnAsk(DSite*,EntityCond);

  void informHandleTempOnAdd(OwnerEntry*,Tertiary*,DSite*s);
  /* PER-HANDLE  
  void releaseInformElem(InformElem*);  // manipulating one inform element
  */ 
};

Chain* newChain();

void freeInformElem(InformElem*);

int printChain(Chain*);

/* __CHAINHH */
#endif 




