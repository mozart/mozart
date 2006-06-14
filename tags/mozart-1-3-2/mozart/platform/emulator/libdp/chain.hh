/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Per Brand, 1998
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

  void* operator new(size_t size) {
    Assert(size<=sizeof(Construct_3));
    return (ChainElem *)genFreeListManager->getOne_3();}

  void free(){
    genFreeListManager->putOne_3((FreeListEntry*) this);}    

  ChainElem(DSite* s){site=s;flags=0;next=NULL;}

  void reinit(DSite* s){site=s;flags=0;}
  
  DSite *getSite(){ return site;}

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
public:
  InformElem *next;
  DSite *site;
  short unsigned int watchcond;
  short unsigned int foundcond;
public:

  void* operator new(size_t size) {
    Assert(size<=sizeof(Construct_3));
    return (InformElem *)genFreeListManager->getOne_3();}

  InformElem(DSite *s,EntityCond ec){
    site=s;
    watchcond=ec;
    foundcond=0;}

  void free(){
    genFreeListManager->putOne_3((FreeListEntry*) this);}

  Bool maybeTrigger(OwnerEntry*, EntityCond);
  void maybeTriggerOK(OwnerEntry*, EntityCond);
};

enum ChainFlags{
  INTERESTED_IN_OK=1,
  TOKEN_TEMP_SOME =2,
  TOKEN_PERM_SOME=4,
  TOKEN_LOST=8};

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
  void* operator new(size_t size) {
    Assert(size<=sizeof(Construct_4));
    return (Chain *)genFreeListManager->getOne_4();}    

  void free(){
    // Must destruct all its sub element. 
    while(first!=NULL){
      ChainElem *tmp = first->next;
      first->free();
      first = tmp;
    }
    
    while(inform!=NULL){
      InformElem *tmp = inform->next;
      inform->free();
      inform = tmp;
    }
    
    genFreeListManager->putOne_4((FreeListEntry*) this);}    
  
  Chain(DSite* s){
    ChainElem *e=new ChainElem(s);
    inform=NULL;
    flags=0;
    first=last=e;}

  void setFlag(int f){flags |= f;}
  void resetFlag(int f){flags &= ~f;}
  Bool hasFlag(int f){return f & flags;}
    
  void setFlagAndCheck(int f){
    Assert(!hasFlag(f));
    setFlag(f);}

  void resetFlagAndCheck(int f){
    Assert(hasFlag(f));
    resetFlag(f);}

  DSite* getCurrent(){
    Assert(last != NULL);
    return last->site;}

  DSite* setCurrent(DSite*, Tertiary*);

  Bool hasInform(){
    return inform!=NULL;}

  InformElem *getInform(){return inform;}
  InformElem **getInformBase(){return &inform;}
  
  ChainElem* getFirst(){return first;}
  ChainElem* getLast(){return last;}

  void removeBefore(DSite*);
  void gcChainSites();                  

  // failure
  // ghosts are chain elements where error recovery mechanism has
  //    asked them but they have not answered but would have been 
  //    removed by normal chain operation

  ChainElem **getFirstNonGhostBase();
  ChainElem *getFirstNonGhost() {return *getFirstNonGhostBase();}
  void makeGhost(ChainElem*);  
  Bool removeGhost(DSite*);  // returns TRUE if ghost removed, FALSE if no
                             // corresponding ghost exists

  Bool siteExists(DSite* s); // return TRUE if non-ghost elem with site s exists

  ChainElem *findAfter(DSite*);
  void releaseChainElem(ChainElem*);         

  void newInform(DSite*,EntityCond); 

  Bool tempConnectionInChain();
  void removeNextChainElem(ChainElem** base);

  void receiveAskError(OwnerEntry*,DSite*,EntityCond);
  void receiveAnswer(Tertiary*,DSite*,int,DSite*); 
  void managerSeesSitePerm(Tertiary*,DSite* ); // probe
  void managerSeesSiteTemp(Tertiary*,DSite* );
  void managerSeesSiteOK(Tertiary*,DSite* );
  void shortcutCrashLock(LockManager*);       // shortcutting chain methods
  void shortcutCrashCell(CellManager*,TaggedRef);
  void handleTokenLost(Tertiary*,OwnerEntry*,Ext_OB_TIndex);
  void receiveUnAsk(DSite*,EntityCond);
  void establish_PERM_SOME(Tertiary*);
  void establish_TOKEN_LOST(Tertiary*);
};

#ifdef DEBUG_PERDIO
int printChain(Chain*);
#endif

/* __CHAINHH */
#endif 




