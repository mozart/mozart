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
#include "runtime.hh"
#include "codearea.hh"
#include "indexing.hh"


#include "perdio.hh"
#include "perdio_debug.hh"
#include "genvar.hh"
#include "perdiovar.hh"
#include "gc.hh"
#include "dictionary.hh"
#include "urlc.hh"
#include "marshaler.hh"
#include "comm.hh"
#include "msgbuffer.hh"
#include "chain.hh"

#define SITE_CUTOFF           100

/**********************************************************************/
/*   SECTION :: class SiteManager                                     */
/**********************************************************************/

class SiteManager: public FreeListManager{

  Site* newSite(){
    Site* s;
    FreeListEntry *f=getOne();
    if(f==NULL) {s=new Site();}
    else{GenCast(f,FreeListEntry*,s,Site*);}
    return s;}

  void deleteSite(Site *s){
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
    newS->init(s->address,s->port,s->timestamp);
    PD((SITE,"allocated site:%s ",newS->stringrep()));
    return newS;}

  Site* allocSite(Site* s,unsigned int type){
    Site *newS=newSite();
    newS->init(s->address,s->port,s->timestamp,type);
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
      if(s->canBeFreed()){
        siteManager.freeSite(s);
        deleteFirst(ghn);
        ghn=getByIndex(i);
        continue;}}
    else{
      s->removeGCMarkSite();}
    ghn1=ghn->getNext();
    while(ghn1!=NULL){
      GenCast(ghn1->getBaseKey(),GenHashBaseKey*,s,Site*);
      if(s->isGCMarkedSite()){
        s->removeGCMarkSite();}
      else{
        if(s->canBeFreed()){
          siteManager.freeSite(s);
          deleteNonFirst(ghn,ghn1);}}
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
  int hvalue=tryS.hashPrimary();
  FindType rc=primarySiteTable->findPrimary(&tryS,hvalue,s);
  switch(rc) {
  case SAME:
    PD((SITE,"unmarshalPsite SAME"));
    if((mt==DIF_PERM) && (!s->isPerm())) {
      s->discoveryPerm();}
    return s;
  case NONE:
    PD((SITE,"unmarshalPsite NONE"));
    break;
  case I_AM_YOUNGER:{
    PD((SITE,"unmarshalPsite I_AM_YOUNGER"));
    int hvalue=tryS.hashSecondary();
    s=secondarySiteTable->findSecondary(&tryS,hvalue);
    if(s){return s;}
    s=siteManager.allocSite(&tryS,PERM_SITE);
    secondarySiteTable->insertSecondary(s,hvalue);
    return s;}
  case I_AM_OLDER:{
    PD((SITE,"unmarshalPsite I_AM_OLDER"));
    primaryToSecondary(s,hvalue);
    break;}}

  s=siteManager.allocSite(&tryS);
  primarySiteTable->insertPrimary(s,hvalue);
  if(mt==DIF_PERM){
    PD((SITE,"initPsite DIF_PERM"));
    s->initPerm();
    return s;}
  Assert(mt==DIF_PASSIVE);
  PD((SITE,"initPsite DIF_PASSIVE"));
  s->initPassive();
  return s;}


Site *findSite(ip_address a,int port,time_t stamp)
{
  Site tryS(a,port,stamp);
  int hvalue=tryS.hashPrimary();
  Site *s;
  FindType rc=primarySiteTable->findPrimary(&tryS,hvalue,s);

  switch(rc) {
  case SAME:
    if(!s->ActiveSite()) {
      s->makeActiveRemote();
    }
    return s;
  case NONE:
    s=siteManager.allocSite(&tryS);
    primarySiteTable->insertPrimary(s,hvalue);
    s->initRemote();
    return s;
  case I_AM_YOUNGER:
    {
      int hvalue=tryS.hashSecondary();
      s = secondarySiteTable->findSecondary(&tryS,hvalue);
      if(s) return s;
      s = siteManager.allocSite(&tryS,PERM_SITE);
      secondarySiteTable->insertSecondary(s,hvalue);
      return s;
    }
  case I_AM_OLDER:
    primaryToSecondary(s,hvalue);
    return s;
  default:
    error("impossible");
    return 0;
  }
}

Site* unmarshalSite(MsgBuffer *buf){
  PD((UNMARSHAL,"site"));
  MarshalTag mt= (MarshalTag) buf->get();
  Assert((mt==DIF_REMOTE) || (mt==DIF_VIRTUAL) || (mt==DIF_PERM));
  FindType rc;
  int hvalue;
  Site *s;
  Site tryS;

  tryS.unmarshalBaseSite(buf);
  hvalue=tryS.hashPrimary();
  rc=primarySiteTable->findPrimary(&tryS,hvalue,s);

  switch(rc){
  case SAME: {
    PD((SITE,"unmarshalsite SAME"));
    if(mt==DIF_PERM){
      if(s->isPerm()){return s;}
      s->discoveryPerm();
      return s;}
    if(mt==DIF_VIRTUAL){
      unmarshalUselessVirtualInfo(buf);
      if(!s->ActiveSite()) {
        s->makeActiveVirtual();}
      return s;}
    Assert(mt==DIF_REMOTE);
    if(!s->ActiveSite()) {
      s->makeActiveRemote();}
    return s;}

  case NONE: {
    PD((SITE,"unmarshalsite NONE"));break;}

  case I_AM_YOUNGER:{
    PD((SITE,"unmarshalsite I_AM_YOUNGER"));
    if(mt==DIF_VIRTUAL){unmarshalUselessVirtualInfo(buf);}
    int hvalue=tryS.hashSecondary();
    s=secondarySiteTable->findSecondary(&tryS,hvalue);
    if(s){return s;}
    s=siteManager.allocSite(&tryS,PERM_SITE);
    secondarySiteTable->insertSecondary(s,hvalue);
    return s;}

  case I_AM_OLDER:{
    PD((SITE,"unmarshalsite I_AM_OLDER"));
    primaryToSecondary(s,hvalue);
    break;}

  default: Assert(0);}

  // none

  s=siteManager.allocSite(&tryS);
  primarySiteTable->insertPrimary(s,hvalue);
  if(mt==DIF_PERM){
    PD((SITE,"initsite DIF_PERM"));
    s->initPerm();
    return s;}
  if(mt==DIF_VIRTUAL){
    PD((SITE,"initsite DIF_VIRTUAL"));
    VirtualInfo * vi=unmarshalVirtualInfo(buf);
    if(inMyGroup(&tryS,vi)){
      s->initVirtual(vi);
      return s;}
    s->initVirtualRemote(vi);
    return s;}
  Assert(mt==DIF_REMOTE);
  PD((SITE,"initsite DIF_REMOTE"));
  s->initRemote();
  return s;
}

/**********************************************************************/
/*   SECTION :: BaseSite object methods                               */
/**********************************************************************/

char *BaseSite::stringrep()
{
  static char buf[100];
  ip_address a=getAddress();
  sprintf(buf,"type:%d %ld.%ld.%ld.%ld:%d:%ld",
          getType(),
          (a/(256*256*256))%256,
          (a/(256*256))%256,
          (a/256)%256,
          a%256,
          getPort(), getTimeStamp());
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
  buf->put((flags & PERM_SITE)? DIF_PERM: DIF_PASSIVE);
  marshalBaseSite(buf);}

void Site::marshalSite(MsgBuffer *buf){
  PD((MARSHAL,"Site"));
  unsigned int type=getType();
  if(type & PERM_SITE){
    buf->put(DIF_PERM);
    marshalBaseSite(buf);
    return;}
  if(type & VIRTUAL_SITE){
    VirtualInfo *vi=getVirtualInfo();
    buf->put(DIF_VIRTUAL);
    marshalBaseSite(buf);
    marshalVirtualInfo(vi,buf);
    return;}
  Assert((type & REMOTE_SITE) || (this==mySite) );
  buf->put(DIF_REMOTE);
  marshalBaseSite(buf);
  return;}

/**********************************************************************/
/*   SECTION :: memory management  methods                            */
/**********************************************************************/

Site* initMySite(ip_address a,port_t p,time_t t){
  Site *s =new Site(a,p,t);
  int hvalue = s->hashPrimary();
  primarySiteTable->insertPrimary(s,hvalue);
  s->initMySiteR();
  return s;}

Site* initMySiteVirtual(ip_address a,port_t p,time_t t,VirtualInfo *vi){
  Site *s =new Site(a,p,t);
  int hvalue = s->hashPrimary();
  primarySiteTable->insertPrimary(s,hvalue);
  s->initMySiteV(vi);
  return s;}
