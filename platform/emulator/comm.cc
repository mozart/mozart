/*
  Perdio Project, DFKI & SICS,
  Universit"at des Saarlandes
  Postfach 15 11 59, D-66041 Saarbruecken, Phone (+49) 681 302-5312
  SICS
  Box 1263, S-16428 Sweden, Phone (+46) 8 7521500
  Author: brand
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
  ------------------------------------------------------------------------
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

#define SITE_CUTOFF           100
#define SITEEXTENSION_CUTOFF   50

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
/*   SECTION :: class SiteExtensionManager                            */
/**********************************************************************/

class SiteExtensionManager: public FreeListManager{

  SiteExtension* newSiteExtension(){
    SiteExtension* s;
    FreeListEntry *f=getOne();
    if(f==NULL) {s=new SiteExtension();}
    else{GenCast(f,FreeListEntry*,s,SiteExtension*);}
    return s;}

  void deleteSiteExtension(SiteExtension *s){
    FreeListEntry *f;
    GenCast(s,SiteExtension*,f,FreeListEntry*);
    if(putOne(f)) {return;}
    delete s;
    return;}

public:
  SiteExtensionManager():FreeListManager(SITEEXTENSION_CUTOFF){}

  void freeSiteExtension(SiteExtension *s){ 
    deleteSiteExtension(s);}

  SiteExtension* allocSiteExtension(){
    SiteExtension *newS = newSiteExtension();
    return newS;}

}siteExtensionManager;

  
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
};

SiteHashTable* primarySiteTable=new SiteHashTable(PRIMARY_SITE_TABLE_SIZE);
SiteHashTable* secondarySiteTable=new SiteHashTable(SECONDARY_SITE_TABLE_SIZE);

/**********************************************************************/
/*   SECTION ::  General unmarshaling routines                        */
/**********************************************************************/

Site *unmarshalPSite(MsgBuffer *buf){
  PD((UNMARSHAL,"Psite"));
  MarshalTag mt= (MarshalTag) buf->get();
  Site tryS;
  tryS.unmarshalBaseSite(buf);
  int hvalue=tryS.hashSecondary();
  Site *s=secondarySiteTable->findSecondary(&tryS,hvalue);    
  if(s) {
    if(mt==DIF_PERM){
      s->passiveToPerm();}
    return s;}
  if(mt==DIF_PERM){
    return siteManager.allocSite(&tryS,PERM_SITE);}
  return siteManager.allocSite(&tryS);}

void primaryToSecondary(Site *s,int hvalue){
  primarySiteTable->removePrimary(s,hvalue);
  int hvalue2=s->hashSecondary();
  s->discoveryPerm();
  s->putInSecondary();
  secondarySiteTable->insertSecondary(s,hvalue2);}

Site* unmarshalSite(MsgBuffer *buf){
  PD((UNMARSHAL,"site"));
  MarshalTag mt= (MarshalTag) buf->get();
  FindType rc;
  int hvalue;
  Site *s;
  Site tryS;

  tryS.unmarshalBaseSite(buf);
  hvalue=tryS.hashPrimary();
  rc=primarySiteTable->findPrimary(&tryS,hvalue,s);    

  switch(rc){
  case SAME: {
    if(mt==DIF_PERM){
      if(s->isPerm()){return s;}
      s->discoveryPerm();
      return s;}
    if(mt==DIF_VIRTUAL){
      unmarshalUselessVirtualInfo(buf);}
    Assert(mt==DIF_REMOTE);
    return s;}

  case NONE: break;
    
  case I_AM_OLDER:{
    if(mt==DIF_VIRTUAL){unmarshalUselessVirtualInfo(buf);}
    int hvalue=tryS.hashSecondary();
    s=secondarySiteTable->findSecondary(&tryS,hvalue);
    if(s){return s;}
    s=siteManager.allocSite(&tryS,PERM_SITE);
    secondarySiteTable->insertSecondary(s,hvalue);
    return s;}

  case I_AM_YOUNGER:{
    primaryToSecondary(s,hvalue);
    break;}

  default: Assert(0);}

  // none

  s=siteManager.allocSite(&tryS);    
  primarySiteTable->insertPrimary(s,hvalue);
  if(mt==DIF_PERM){
    s->initPerm();
    return s;}
  SiteExtension *se=siteExtensionManager.allocSiteExtension();
  if(mt==DIF_VIRTUAL){
    VirtualInfo * vi=unmarshalVirtualInfo(buf);
    if(inMyGroup(&tryS,vi)){
      s->initVirtual(se,vi);
      return s;}
    s->initVirtualRemote(se,vi);      
    return s;}
  Assert(mt==DIF_REMOTE);
  s->initRemote(se);
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
  if(flags & PERM_SITE){
    buf->put(DIF_PERM);
    return;}
  buf->put(DIF_PASSIVE);
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

void Site::disconnectInPerm(){
  setType(getType() & PERM_SITE);
  Assert((getType() & CONNECTED)==0); 
  if(getType() & VIRTUAL_SITE){
    dumpVirtualInfo(getVirtualInfo());}
  setType(getType() & (~(VIRTUAL_SITE|REMOTE_SITE)));
  int i=getRefCtrS();
  siteExtensionManager.freeSiteExtension(getSiteExtension());
  extension = (unsigned int) i;}

/**********************************************************************/
/*   SECTION :: memory management  methods                            */
/**********************************************************************/

void siteZeroPassiveRef(Site *s){
  if(s->isInSecondary()){
    secondarySiteTable->removeSecondary(s,s->hashSecondary());}
  else{
    primarySiteTable->removePrimary(s,s->hashPrimary());}
  siteManager.freeSite(s);
  return;}

void siteZeroActiveRef(Site *s){
  siteExtensionManager.freeSiteExtension(s->getSiteExtension());
  primarySiteTable->removePrimary(s,s->hashPrimary());
  siteManager.freeSite(s);
  return;}

Site* initMySite(ip_address a,port_t p,time_t t){
  return new Site(a,p,t);}


