/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 * 
 *  Contributors:
 *    Konstantin Popov <kost@sics.se>
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

#if defined(INTERFACE)
#pragma implementation "dsite.hh"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>

#include "os.hh"
#include "dsite.hh"
#include "comm.hh"
#include "mbuffer.hh"
#include "genhashtbl.hh"

#define SITE_CUTOFF           100

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

class DSiteHashTable : public GenHashTable {
    
public:
  DSiteHashTable(int size):GenHashTable(size){}

  FindType findPrimary(DSite *s,int hvalue,DSite* &found){
    GenHashNode *ghn=htFindFirst(hvalue);
    while(ghn!=NULL){
      found = (DSite*) (ghn->getBaseKey());
      if(s->compareSitesNoTimestamp(found)==0) {
	int ft=s->compareSites(found);
	if(ft==0) {return SAME;}
	if(ft<0) {return I_AM_YOUNGER;}
	return I_AM_OLDER;}
      ghn=htFindNext(ghn,hvalue);}
    found=NULL; // optimize
    return NONE;}

  DSite* findSecondary(DSite *s, int hvalue){
    GenHashNode *ghn=htFindFirst(hvalue);
    DSite* found;
    while(ghn!=NULL){
      found = (DSite*) (ghn->getBaseKey());
      if(s->compareSites(found)==0) return found;
      ghn=htFindNext(ghn,hvalue);}
    return NULL;}

  void insertPrimary(DSite *s,int hvalue){
    GenHashBaseKey *ghn_bk;
    GenHashEntry *ghn_e=NULL;  
    ghn_bk = (GenHashBaseKey*)(void*) s;
    PD((SITE,"add hvalue:%d site:%s",hvalue,s->stringrep()));
    htAdd(hvalue,ghn_bk,ghn_e);}

  void insertSecondary(DSite *s,int hvalue){
    insertPrimary(s,hvalue);}

  void removePrimary(DSite *s,int hvalue){
    GenHashNode *ghn=htFindFirst(hvalue);
    DSite* found;
    while(ghn!=NULL){
      found = (DSite*) (ghn->getBaseKey());
      if(s->compareSites(found)==0){
	htSub(hvalue,ghn);
	return;}
      ghn=htFindNext(ghn,hvalue);}
    Assert(0);}

  void removeSecondary(DSite *s,int hvalue){
    removePrimary(s,hvalue);}

  void cleanup();
};

DSiteHashTable* primarySiteTable=new DSiteHashTable(PRIMARY_SITE_TABLE_SIZE);
DSiteHashTable* secondarySiteTable=new DSiteHashTable(SECONDARY_SITE_TABLE_SIZE);

void DSiteHashTable::cleanup(){
  GenHashNode *ghn,*ghn1;
  DSite* s;
  int i=0;
  ghn=getFirst(i);
  while(ghn!=NULL){
    s = (DSite*) (ghn->getBaseKey());

    if(!(s->isGCMarkedSite())){
      PD((SITE,"Head: Not Marked Site %x %s",s, s->stringrep()));
      if(s->canBeFreed()){
	s->freeSite();
	deleteFirst(ghn);
	ghn=getByIndex(i);
	continue;}}
    else{
      PD((SITE,"Head: Marked Site %x %s",s, s->stringrep()));
      s->removeGCMarkSite();}
    ghn1=ghn->getNext();
    while(ghn1!=NULL){
      s = (DSite*) (ghn1->getBaseKey());
      if(s->isGCMarkedSite()){
	PD((SITE,"      : Marked Site %x %s",s, s->stringrep()));
	s->removeGCMarkSite();}
      else{
	if(s->canBeFreed()){
	  PD((SITE,"      : Not Marked Site %x %s",s, s->stringrep()));
	  s->freeSite();
	  deleteNonFirst(ghn,ghn1);
	  ghn1=ghn->getNext();
	  continue;}}
      ghn=ghn1;
      ghn1=ghn1->getNext();}
    i++;
    ghn=getByIndex(i);}
  return;
}

void gcDSiteTable(){
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

inline void primaryToSecondary(DSite *s, int hvalue) {
  primarySiteTable->removePrimary(s,hvalue);
  int hvalue2=s->hashSecondary();
//    s->discoveryPerm(); // AN! 
  s->putInSecondary();
  secondarySiteTable->insertSecondary(s,hvalue2);}

static
DSite* unmarshalDSiteInternal(MarshalerBuffer *buf, DSite *tryS, MarshalTag mt)
{
  DSite *s;
  int hvalue = tryS->hashPrimary();

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
    Assert(mt == DIF_SITE);
    if(s != myDSite && !s->ActiveSite()) {
      s->makeActiveRemote();}
    return s;}

  case NONE:
    PD((SITE,"unmarshalsite NONE"));
    break;
    
  case I_AM_YOUNGER:{
    PD((SITE,"unmarshalsite I_AM_YOUNGER"));
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

  Assert(mt == DIF_SITE);
  PD((SITE,"initsite DIF_SITE"));
  s->initRemote();
  return s;
}

DSite *findDSite(ip_address a,int port,TimeStamp &stamp)
{
  DSite tryS(a, port, stamp);
  return (unmarshalDSiteInternal(NULL, &tryS, DIF_SITE));
}

#ifdef USE_FAST_UNMARSHALER
DSite* unmarshalDSite(MarshalerBuffer *buf)
{
  PD((UNMARSHAL,"site"));
  MarshalTag mt = (MarshalTag) buf->get();
  Assert(mt == DIF_SITE || mt == DIF_SITE_PERM);
  DSite tryS;

  tryS.unmarshalBaseSite(buf);
  return unmarshalDSiteInternal(buf, &tryS, mt);
}
#else
DSite* unmarshalDSiteRobust(MarshalerBuffer *buf, int *error)
{
  PD((UNMARSHAL,"site"));
  MarshalTag mt = (MarshalTag) buf->get();
  Assert(mt == DIF_SITE || mt == DIF_SITE_PERM);
  DSite tryS;

  tryS.unmarshalBaseSiteRobust(buf, error);
  if (*error) return ((DSite *) 0);
  return unmarshalDSiteInternal(buf, &tryS, mt);
}
#endif

/**********************************************************************/
/*   SECTION :: BaseSite object methods                               */
/**********************************************************************/

OZ_Term DSite::getStateStatistics() {
  if(isConnected()) {
    // We have a comObj, the question is, is that physically connected,
    // i.e. does it have a transObj. Responses may be connected or passive
    return comObj->getStateStatistics();
  }
  else if(isPerm()) {
    return oz_atom("perm");
  }
  else if(this==myDSite) {
    return oz_atom("mine");
  }
  else 
    return oz_atom("passive");
}

char *DSite::stringrep()
{
  static char buf[100];
  ip_address a=getAddress();
  sprintf(buf,"type:%d %d.%d.%d.%d:%d:%ld/%d",
	  getType(),
	  (a/(256*256*256))%256,
	  (a/(256*256))%256,
	  (a/256)%256,
	  a%256,
	  getPort(), getTimeStamp()->start,getTimeStamp()->pid);
  return buf;
}

char *DSite::stringrep_notype()
{
  static char buf[100];
  ip_address a=getAddress();
  sprintf(buf,"%d.%d.%d.%d:%d:%ld/%d",
	  (a/(256*256*256))%256,
	  (a/(256*256))%256,
	  (a/256)%256,
	  a%256,
	  getPort(), getTimeStamp()->start,getTimeStamp()->pid);
  return buf;
}

char *oz_site2String(DSite *s) { return s->stringrep(); }

//
int DSite::hashWOTimestamp(){
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


void DSite::marshalDSite(MarshalerBuffer *buf){
  PD((MARSHAL,"Site"));
  unsigned int type=getType();
  if(type & PERM_SITE){
    marshalDIF(buf,DIF_SITE_PERM);
    marshalBaseSite(buf);
    return;}
  Assert((type & REMOTE_SITE) || (this==myDSite) );
  marshalDIF(buf,DIF_SITE);
  marshalBaseSite(buf);
  return;}


/**********************************************************************/
/*   SECTION :: memory management  methods                            */
/**********************************************************************/

//
DSite* myDSite =  NULL;

//
// kost@ : that's a part of the boot-up procedure ('perdioInit()');
// Actually, it is used by 'initNetwork()' because ip, port, timestamp
// are not known prior its initialization;
DSite* makeMyDSite(ip_address a, oz_port_t p, TimeStamp &t) {
  DSite *s = new DSite(a,p,t);
  s->setMyDSite();
  int hvalue = s->hashPrimary();
  primarySiteTable->insertPrimary(s,hvalue);
  s->initMyDSite();
  return s;
}


