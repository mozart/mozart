#ifndef __CHAINHH
#define __CHAINHH

#ifdef INTERFACE
#pragma interface
#endif

#define CHAIN_PROBE 1

enum TokenState{
TOKEN_MAYBE_LOST,
TOKEN_UNTOUCHED,
TOKEN_NOT_LOST
};

class ChainElem{
friend class Chain;
protected:
  ChainElem* next;
  TaggedPtr tagged;
  int numId;
public:
  ChainElem(){}

  void init(Site *s, int nr){
    next = NULL;
    tagged.init();
    tagged.setPtr(s);
    numId = nr;}

  Bool isProbing(){
    return (tagged.getType() & CHAIN_PROBE);}

  void probeStarted(){
    Assert(!isProbing());
    Site *s = getSite();
    if(s != mySite)
      s->installProbe(PROBE_TYPE_ALL,0);
    tagged.setType(CHAIN_PROBE);}

  void probeStop(){
    Assert(isProbing());
    tagged.setType(0);
    getSite()->deinstallProbe(PROBE_TYPE_ALL);}

  Site* getSite(){
    return (Site *) tagged.getPtr();}

  void setNext(ChainElem* n){
    next = n;}

  ChainElem *getNext(){
    return next;}

  int getNum(){
    return numId;}
};

class InformElem{
friend class Chain;
protected:
  InformElem *next;
  Site *site;
  EntityCond watchcond;
public:
  InformElem(){}
  void init(Site* s,EntityCond c){site=s;next=NULL;watchcond=c;}
};

class Chain{
private:
  ChainElem* first;
  ChainElem* last;
  InformElem* inform;
public:

  Chain(){first = NULL;last=NULL;inform=NULL;}

  void init(Site*,int);

  void informHandle(Tertiary*,EntityCond);

  Site* getCurrent(){
    Assert(last != NULL);
    return last->getSite();}

  Site* getFirst(int &a){
    if(first==NULL)
      return NULL;
    a = first->getNum();
    return first->getSite();}

  Bool siteIsFirst(Site* s,int nr){
    if(first==NULL) return NO;
    if(first->getSite()!=s) return NO;
    if(first->numId!=nr) return NO;
    return OK;}

  Site* getFirstSite(){
    Assert(first!=NULL);
    return first->getSite();}

  void removeFirstSite(){
    Assert(first!=NULL);
    first=first->getNext();}

  void informAll(Tertiary*,Site* s);
  void maybeInform(EntityCond c,Site *s,int);

  void setCurrent(Site* s, int);

  void siteNrRemove(Site*,int);
  Bool siteNrExists(Site*,int);
  Site* tokenSent(Site*,int,int&);
  void installProbes();
  Site* removeSiteNext(Site*,Site*);
  TokenState removeLostSite(Site*);
  Bool siteListCheck();
  void gcChainSites();
  void receiveAck(Site*,int);

  void managerSeesSiteCrash(Tertiary*,Site* );
  Site* proxySeesSiteCrash(Tertiary *,Site*,Site*);
};

/* __CHAINHH */
#endif
