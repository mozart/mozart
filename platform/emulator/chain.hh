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
  Site *site;
public:
  ChainElem(){}

  void init(Site *s){
    next = NULL;
    site=s;}

  void setNext(ChainElem* n){
    next = n;}

  ChainElem *getNext(){
    return next;}

  void deinstallProbe(ProbeType pt){
    site->deinstallProbe(pt);}

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
protected:
  ChainElem* first;  
  ChainElem* last;
  InformElem* inform;
public:
  Chain(){first = NULL;last=NULL;inform=NULL;}

  void init(Site*);

  void informHandle(Tertiary*,EntityCond);

  ChainElem *getFirstChainElem(){return first;}

  Site* getCurrent(){
    Assert(last != NULL);
    return last->site;}

  Site* getFirstSite(){
    Assert(first!=NULL);
    return first->site;}

  void hasDumped(){
    first=NULL;
    last=NULL;}

  Bool hasInform(){
    return inform!=NULL;}
  
  void setCurrent(Site* s);

  int siteRemove(Site*);
  Bool siteExists(Site*);

  void installProbeAfterAck(){
    if(first->site!=mySite){
      first->site->installProbe(PROBE_TYPE_PERM,0);}}

  void installTwoProbesAfterAck(){
    installProbeAfterAck();
    Site *s=first->next->site;
    if(s!=mySite){s->installProbe(PROBE_TYPE_PERM,0);}}

  Bool afterOneAnother(Site *s1,Site* s2);

  Site* removeSecondElem();
  void removeFirstElem();
  void gcChainSites();
  void receiveAck(Site*,int);

  void managerSeesSiteCrash(Tertiary*,Site* );
  Site* proxySeesSiteCrash(Tertiary *,Site*);
};

/* __CHAINHH */
#endif 
