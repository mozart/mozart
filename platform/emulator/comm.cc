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


#if defined(INTERFACE)
#pragma implementation "perdio.hh"
#endif

#if defined(INTERFACE)
#pragma implementation "msgbuffer.hh"
#endif

#if defined(INTERFACE)
#pragma implementation "comm.hh"
#endif

#if defined(INTERFACE)
#pragma implementation "chain.hh"
#endif

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>

#include "comm.hh"
#include "msgbuffer.hh"
#include "vs_comm.hh"
#include "chain.hh"

#define SITE_CUTOFF           100

/**********************************************************************/
/*   SECTION :: class SiteManager                                     */
/**********************************************************************/

class SiteManager: public FreeListManager{

  Site* newSite(){
    Site* s;
    PD((SITE,"New Site allocated"));
    FreeListEntry *f=getOne();
    if(f==NULL) {s=new Site();}
    else GenCast(f,FreeListEntry*,s,Site*);
    return s;}

  void deleteSite(Site *s){
    PD((SITE,"Site deleted %x %s",s,s->stringrep()));
    FreeListEntry *f;
    GenCast(s,Site*,f,FreeListEntry*);
    if(putOne(f)) {return;}
    delete s;
    return;}

public:
  SiteManager():FreeListManager(SITE_CUTOFF){}

  void freeSite(Site *s){
    Assert(!(s->getType() & MY_SITE));
    deleteSite(s);}

  Site* allocSite(Site* s){
    Site *newS=newSite();
    newS->init(s->address,s->port,&s->timestamp);
    PD((SITE,"allocated site:%s ",newS->stringrep()));
    return newS;}

  Site* allocSite(Site* s,unsigned int type){
    Site *newS=newSite();
    newS->init(s->address,s->port,&s->timestamp,type);
    PD((SITE,"allocated site:%s ",newS->stringrep()));
    return newS;}
}siteManager;

/**********************************************************************/
/*   SECTION ::  Site Hash Table                                     */
/**********************************************************************/

enum FindType{
  SAME,
  NONE,
  I_AM_YOUNGER,
  I_AM_OLDER};

#define PRIMARY_SITE_TABLE_SIZE    10
#define SECONDARY_SITE_TABLE_SIZE  50

class SiteHashTable: public GenHashTable{

public:
  SiteHashTable(int size):GenHashTable(size){}

  FindType findPrimary(Site *s,int hvalue,Site* &found){
    GenHashNode *ghn=htFindFirst(hvalue);
    while(ghn!=NULL){
      GenCast(ghn->getBaseKey(),GenHashBaseKey*,found,Site*);
      if(s->compareSitesNoTimestamp(found)==0) {
        int ft=s->compareSites(found);
        if(ft==0) {return SAME;}
        if(ft<0) {return I_AM_YOUNGER;}
        return I_AM_OLDER;}
      ghn=htFindNext(ghn,hvalue);}
    found=NULL; // optimize
    return NONE;}

  Site* findSecondary(Site *s,int hvalue){
    GenHashNode *ghn=htFindFirst(hvalue);
    Site* found;
    while(ghn!=NULL){
      GenCast(ghn->getBaseKey(),GenHashBaseKey*,found,Site*);
      if(s->compareSites(found)==0) return found;
      ghn=htFindNext(ghn,hvalue);}
    return NULL;}

  void insertPrimary(Site *s,int hvalue){
    GenHashBaseKey *ghn_bk;
    GenHashEntry *ghn_e=NULL;
    GenCast(s,Site*,ghn_bk,GenHashBaseKey*);
    PD((SITE,"add hvalue:%d site:%s",hvalue,s->stringrep()));
    htAdd(hvalue,ghn_bk,ghn_e);}

  void insertSecondary(Site *s,int hvalue){
    insertPrimary((Site*)s,hvalue);}

  void removePrimary(Site *s,int hvalue){
    GenHashNode *ghn=htFindFirst(hvalue);
    Site* found;
    while(ghn!=NULL){
      GenCast(ghn->getBaseKey(),GenHashBaseKey*,found,Site*);
      if(s->compareSites(found)==0){
        htSub(hvalue,ghn);
        return;}
      ghn=htFindNext(ghn,hvalue);}
    Assert(0);}

  void removeSecondary(Site *s,int hvalue){
    removePrimary(s,hvalue);}

  void cleanup();
};

SiteHashTable* primarySiteTable=new SiteHashTable(PRIMARY_SITE_TABLE_SIZE);
SiteHashTable* secondarySiteTable=new SiteHashTable(SECONDARY_SITE_TABLE_SIZE);

void SiteHashTable::cleanup(){
  GenHashNode *ghn,*ghn1;
  Site* s;
  int i=0;
  ghn=getFirst(i);
  while(ghn!=NULL){
    GenCast(ghn->getBaseKey(),GenHashBaseKey*,s,Site*);

    if(!(s->isGCMarkedSite())){
      PD((SITE,"Head: Not Marked Site %x %s",s, s->stringrep()));
      if(s->canBeFreed()){
        siteManager.freeSite(s);
        deleteFirst(ghn);
        ghn=getByIndex(i);
        continue;}}
    else{
      PD((SITE,"Head: Marked Site %x %s",s, s->stringrep()));
      s->removeGCMarkSite();}
    ghn1=ghn->getNext();
    while(ghn1!=NULL){
      GenCast(ghn1->getBaseKey(),GenHashBaseKey*,s,Site*);
      if(s->isGCMarkedSite()){
        PD((SITE,"      : Marked Site %x %s",s, s->stringrep()));
        s->removeGCMarkSite();}
      else{
        if(s->canBeFreed()){
          PD((SITE,"      : Not Marked Site %x %s",s, s->stringrep()));
          siteManager.freeSite(s);
          deleteNonFirst(ghn,ghn1);
          ghn1=ghn->getNext();
          continue;}}
      ghn=ghn1;
      ghn1=ghn1->getNext();}
    i++;
    ghn=getByIndex(i);}
  return;
}

void gcSiteTable(){
  primarySiteTable->cleanup();
  secondarySiteTable->cleanup();}

/**********************************************************************/
/*   SECTION ::  NetworkStatistics                                   */
/**********************************************************************/

GenHashNode *getPrimaryNode(GenHashNode* node, int &indx){
  if(node == NULL)
    return primarySiteTable->getFirst(indx);
  return primarySiteTable->getNext(node,indx);}

GenHashNode *getSecondaryNode(GenHashNode* node, int &indx){
  if(node == NULL)
    return secondarySiteTable->getFirst(indx);
  return secondarySiteTable->getNext(node,indx);}


/**********************************************************************/
/*   SECTION ::  General unmarshaling routines                        */
/**********************************************************************/

inline void primaryToSecondary(Site *s,int hvalue){
  primarySiteTable->removePrimary(s,hvalue);
  int hvalue2=s->hashSecondary();
  s->discoveryPerm();
  s->putInSecondary();
  secondarySiteTable->insertSecondary(s,hvalue2);}

Site *unmarshalPSite(MsgBuffer *buf){
  PD((UNMARSHAL,"Psite"));
  MarshalTag mt= (MarshalTag) buf->get();
  Site tryS;
  Site *s;
  tryS.unmarshalBaseSite(buf);
  PD((UNMARSHAL,"Psite base site fin %s",tryS.stringrep()));
  PD((SITE,"Psite found %s",tryS.stringrep()));
  int hvalue=tryS.hashPrimary();
  FindType rc=primarySiteTable->findPrimary(&tryS,hvalue,s);
  switch(rc) {
  case SAME:
    PD((SITE,"unmarshalPsite SAME"));
    if((mt==DIF_SITE_PERM) && (!s->isPerm())) {
      s->discoveryPerm();}
    return s;
  case NONE:
    PD((SITE,"unmarshalPsite NONE"));
    break;
  case I_AM_YOUNGER:{
    int hvalue=tryS.hashSecondary();
    s=secondarySiteTable->findSecondary(&tryS,hvalue);
    if(s){ PD((SITE,"unmarshalPsite I_AM_YOUNGER site found"));return s;}
    PD((SITE,"unmarshalPsite I_AM_YOUNGER site inserted"));
    s=siteManager.allocSite(&tryS,PERM_SITE);
    secondarySiteTable->insertSecondary(s,hvalue);
    return s;}
  case I_AM_OLDER:{
    PD((SITE,"unmarshalPsite I_AM_OLDER"));
    primaryToSecondary(s,hvalue);
    break;}}

  s=siteManager.allocSite(&tryS);
  primarySiteTable->insertPrimary(s,hvalue);
  if(mt==DIF_SITE_PERM){
    PD((SITE,"initPsite DIF_SITE_PERM"));
    s->initPerm();
    return s;}
  Assert(mt==DIF_PASSIVE);
  PD((SITE,"initPsite DIF_PASSIVE"));
  s->initPassive();
  return s;}


static
Site* unmarshalSiteInternal(MsgBuffer *buf, Site *tryS, MarshalTag mt)
{
  Site *s;
  int hvalue = tryS->hashPrimary();

  FindType rc = primarySiteTable->findPrimary(tryS,hvalue,s);
  switch(rc){
  case SAME: {
    PD((SITE,"unmarshalsite SAME"));
    if(mt==DIF_SITE_PERM){
      if(s->isPerm()){return s;}
      s->discoveryPerm();
      return s;}

#ifdef VIRTUALSITES
    //
    if(mt == DIF_SITE_VI) {
      // Noote: that can be GName, in which case we have to fetch
      // virtual info as well;
      if (s != mySite) {
        //

        // "remote"/"virtual" can be either active or down,
        // in both cases there is no need for virtual info:
        if (s->remoteComm() || s->virtualComm()) {
          unmarshalUselessVirtualInfo(buf);
        } else {
          // passive (GName);
          VirtualInfo *vi = unmarshalVirtualInfo(buf);

          //
          if (mySite->isInMyVSGroup(vi))
            s->makeActiveVirtual(vi);
          else
            s->makeActiveRemoteVirtual(vi);
        }
      } else {
        // my site is my site... already initialized;
        unmarshalUselessVirtualInfo(buf);
      }

      //
      return (s);
    }
#endif // VIRTUALSITES

    //
    Assert(mt == DIF_SITE);
    if(s != mySite && !s->ActiveSite()) {
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
    s=siteManager.allocSite(tryS,PERM_SITE);
    secondarySiteTable->insertSecondary(s,hvalue);
    return s;}

  case I_AM_OLDER:{
    PD((SITE,"unmarshalsite I_AM_OLDER"));
    primaryToSecondary(s,hvalue);
    break;}

  default: Assert(0);}

  // none

  s=siteManager.allocSite(tryS);
  primarySiteTable->insertPrimary(s,hvalue);

  //
  if(mt==DIF_SITE_PERM){
    PD((SITE,"initsite DIF_SITE_PERM"));
    s->initPerm();
    return s;}

#ifdef VIRTUALSITES
  if(mt == DIF_SITE_VI) {
    PD((SITE,"initsite DIF_SITE_VI"));

    //
    // kost@ : fetch virtual info, which (among other things)
    // identifies the 'tryS's group of virtual sites;
    VirtualInfo *vi = unmarshalVirtualInfo(buf);
    if (mySite->isInMyVSGroup(vi))
      s->initVirtual(vi);
    else
      s->initRemoteVirtual(vi);
    return (s);
  }
#endif // VIRTUALSITES

  Assert(mt == DIF_SITE);
  PD((SITE,"initsite DIF_SITE"));
  s->initRemote();
  return s;
}

Site *findSite(ip_address a,int port,TimeStamp &stamp)
{
  Site tryS(a,port,stamp);
  return unmarshalSiteInternal(NULL, &tryS, DIF_SITE);
}

Site* unmarshalSite(MsgBuffer *buf)
{
  PD((UNMARSHAL,"site"));
  MarshalTag mt = (MarshalTag) buf->get();
  Assert(mt == DIF_SITE || mt == DIF_SITE_VI);
  Site tryS;

  tryS.unmarshalBaseSite(buf);
  return unmarshalSiteInternal(buf, &tryS, mt);
}

/**********************************************************************/
/*   SECTION :: BaseSite object methods                               */
/**********************************************************************/

char *BaseSite::stringrep()
{
  static char buf[100];
  ip_address a=getAddress();
  sprintf(buf,"type:%d %ld.%ld.%ld.%ld:%d:%ld/%d",
          getType(),
          (a/(256*256*256))%256,
          (a/(256*256))%256,
          (a/256)%256,
          a%256,
          getPort(), getTimeStamp()->start,getTimeStamp()->pid);
  return buf;
}

int BaseSite::hashSecondary(){
  BYTE *p=(BYTE*)&address;
  unsigned h=0,g;
  int i;
  int limit=sizeof(address);
  for(i=0;i<limit;i++,p++){
    h = (h << 4) + (*p);
    if ((g = h & 0xf0000000)) {
      h = h ^ (g >> 24);
      h = h ^ g;}}
  p= (BYTE*) &port;
  limit=sizeof(port);
  for(i=0;i<limit;i++,p++){
    h = (h << 4) + (*p);
    if ((g = h & 0xf0000000)) {
      h = h ^ (g >> 24);
      h = h ^ g;}}
  p= (BYTE*) &timestamp;
  limit=sizeof(port);
  for(i=0;i<limit;i++,p++){
    h = (h << 4) + (*p);
    if ((g = h & 0xf0000000)) {
      h = h ^ (g >> 24);
      h = h ^ g;}}
  return (int) h;}

int BaseSite::hashPrimary(){
  BYTE *p=(BYTE*)&address;
  unsigned h=0,g;
  int i;
  int limit=sizeof(address);
  for(i=0;i<limit;i++,p++){
    h = (h << 4) + (*p);
    if ((g = h & 0xf0000000)) {
      h = h ^ (g >> 24);
      h = h ^ g;}}
  p= (BYTE*) &port;
  limit=sizeof(port);
  for(i=0;i<limit;i++,p++){
    h = (h << 4) + (*p);
    if ((g = h & 0xf0000000)) {
      h = h ^ (g >> 24);
      h = h ^ g;}}
  return (int) h;}

/**********************************************************************/
/*   SECTION :: Site object methods                                   */
/**********************************************************************/

void Site :: marshalPSite(MsgBuffer *buf){
  PD((MARSHAL,"Psite"));
  buf->put((flags & PERM_SITE)? DIF_SITE_PERM: DIF_PASSIVE);
  marshalBaseSite(buf);}

#ifdef VIRTUALSITES
//
// Modifies the 'VirtualInfo'!
void Site::initVirtualInfoArg(VirtualInfo *vi)
{
  vi->setAddress(getAddress());
  vi->setTimeStamp(*getTimeStamp());
  vi->setPort(getPort());
}

//
Bool Site::isInMyVSGroup(VirtualInfo *vi)
{
  if (getType() & VIRTUAL_INFO) {
    VirtualInfo *myVI = getVirtualInfo();

    //
    return (myVI->cmpVirtualInfos(vi));
  } else {
    return (FALSE);
  }
}
#endif

void Site::marshalSite(MsgBuffer *buf){
  PD((MARSHAL,"Site"));
  unsigned int type=getType();
  if(type & PERM_SITE){
    buf->put(DIF_SITE_PERM);
    marshalBaseSite(buf);
    return;}
  if (type & VIRTUAL_INFO) {
    // A site with a virtual info can be also remote (in the case of a
    // site in a remote virtual site group);
    VirtualInfo *vi = getVirtualInfo();
    buf->put(DIF_SITE_VI);
    marshalBaseSite(buf);
    marshalVirtualInfo(vi,buf);
    return;}
  Assert((type & REMOTE_SITE) || (this==mySite) );
  buf->put(DIF_SITE);
  marshalBaseSite(buf);
  return;}

/**********************************************************************/
/*   SECTION :: memory management  methods                            */
/**********************************************************************/

//
// kost@ : that's a part of the boot-up procedure ('perdioInit()');
Site* makeMySite(ip_address a, port_t p, TimeStamp &t) {
  Site *s = new Site(a,p,t);
  int hvalue = s->hashPrimary();
  primarySiteTable->insertPrimary(s,hvalue);
  s->initMySite();
  return s;
}
